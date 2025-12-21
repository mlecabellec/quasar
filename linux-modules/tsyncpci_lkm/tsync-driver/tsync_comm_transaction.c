#include "tsync_driver_helpers.h"

#include "tsync_comm_transaction.h"
#include "tsync_comm_validation.h"
#include "tsync_comm_fifo.h"
#include <linux/time.h>

#define LOG_TRANSACTIONS 0
#ifdef LAFAYETTE
#define STATISTICS 1
#else
#define STATISTICS 0
#endif

const int DEFAULT_TSYNC_TIMEOUT_MICROS   = 2000000;

#if STATISTICS
static int BytesCountTotalReceived = 0;
static int BytesCountTotalSent     = 0;
static int TransactionCounter      = 0;
#define MAX_RATE_LIMIT 100000 // bytes per second
#define MIN_RATE_LIMIT 10

static struct timespec StartTime = {0};  // start time stamp for statistics
#endif

#define BYTES_PER_LINE 8

void logTransactionBuffer(TPRO_INSTANCE_T *hw, char *message, void *data, unsigned int length) {
    uint8_t *dataPos = (uint8_t*) data;
    int bytesRemaining = length;
    int iPayloadByte = 0;

    TPRO_PRINT("%s %u bytes\n", message, length);
    TPRO_PRINT("%20s: 0x%02x\n", "CL ID", dataPos[0]);
    TPRO_PRINT("%20s: 0x%02x\n", "CL sequence #", dataPos[1]);
    TPRO_PRINT("%20s: %u (0x%02x%02x)\n", "CL length",
               ((unsigned int)dataPos[2] << 8) + dataPos[3], 
               dataPos[2], dataPos[3]);
    dataPos += 4;
    bytesRemaining -= 4;

    if (bytesRemaining >= 4) {
        TPRO_PRINT("%20s: 0x%02x 0x%02x\n", "cai/iid", dataPos[0], dataPos[1]);
        TPRO_PRINT("%20s: 0x%02x%02x\n", "get/set/error", dataPos[2], dataPos[3]);
        dataPos += 4;
        bytesRemaining -= 4;
    }
        
    if (bytesRemaining >= 4) {
        TPRO_PRINT("%20s: %u (0x%02x%02x%02x%02x) \n", "payload length",
                   ((unsigned int)dataPos[0] << 24) +  
                   ((unsigned int)dataPos[1] << 16) +
                   ((unsigned int)dataPos[2] << 8) +
                   dataPos[3],
                   dataPos[0], dataPos[1], dataPos[2], dataPos[3]);
        dataPos += 4;
        bytesRemaining -= 4;
    }

    while (bytesRemaining > 0) {
        uint8_t temp[BYTES_PER_LINE];

        int bytesThisLine = (BYTES_PER_LINE < bytesRemaining) ? BYTES_PER_LINE : bytesRemaining;
        
        memset(temp, 0, BYTES_PER_LINE);
        memcpy(temp, dataPos, bytesThisLine);
        
        TPRO_PRINT("%20u: %02x %02x %02x %02x  %02x %02x %02x %02x\n",
                   iPayloadByte,
                   temp[0], temp[1], 
                   temp[2], temp[3], 
                   temp[4], temp[5], 
                   temp[6], temp[7]); 

        bytesRemaining -= bytesThisLine;
        dataPos += bytesThisLine;
        iPayloadByte += bytesThisLine;
    }
}


/*******************************************************************************
**
** Function:    tsyncLocalTransaction()
** 
** Purpose: Local transaction handles transactions for input and
** output payloads in driver space.  This simplifies writing
** emulations of old ioctls which now leverage transactions.  Can be
** used for user transactions, provided the copying of the input and
** output to and from user space is handled by the caller.
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncLocalTransaction (TPRO_INSTANCE_T *hw, ioctl_trans *transaction)
{
    const unsigned int maxTransactionBufferWords = MAX_TRANSACTION_BUFFER_WORDS;
    uint16_t *requestBuffer = hw->localRequestBuffer;
    uint16_t *responseBuffer = hw->localResponseBuffer;
    uint8_t *bufferPos8;
    uint16_t *bufferPos16;

    uint16_t checksum;

    uint8_t seqNum = tsync_comm_get_next_seq_num();
    uint8_t foundSequenceNumber;
    uint32_t nWords;
    uint32_t nResultBytes = 0;

    TSYNC_ERROR fifoResult;

    uint8_t cai, iid;

    unsigned int iAttempt     = 0;
    unsigned int maxAttempts  = 5;
    unsigned int done         = 0;
    unsigned int skipResponse = 0;

#if STATISTICS
    unsigned int numSec = 1;
    struct timespec currentTime = {0};
#endif

    const unsigned int COMM_LAYER_OVERHEAD_WORDS = 3;

    tsyncFlushFifos(hw);

    /*************************************************************
     * Build the comm-level message 
     */
    bufferPos8 = (uint8_t*)requestBuffer;
    *bufferPos8++ = 0x01; // HL_CL_APPL
    *bufferPos8++ = seqNum;

    bufferPos16 = (uint16_t*)bufferPos8;
    *bufferPos16++ = BIG_END_16(transaction->inBufferLength);
        
    if (transaction->inBufferLength > 
        (maxTransactionBufferWords * sizeof(uint16_t))) {
        TPRO_PRINT("too much data to read in: %zu vs. %zu " 
                   SRC_LOCATION_FORMAT_STRING, 
                   transaction->inBufferLength,
                   maxTransactionBufferWords * sizeof(uint16_t),
                   SRC_LOCATION_VARIABLES);
        transaction->status = TSYNC_DRV_BUFFER_OVERFLOW;
        return 1;
    }

    memcpy(bufferPos16, transaction->inBuffer, transaction->inBufferLength);

    bufferPos8 = (uint8_t*) bufferPos16;
    cai = bufferPos8[0];
    iid = bufferPos8[1];
    if ((cai == 0x25) && (iid == 0x08)) {
        TPRO_PRINT("Reset transaction detected.  skipResponse = 1.\n");
        skipResponse = 1;
    }
    
    nWords = (transaction->inBufferLength + 4) / sizeof(uint16_t);
    checksum = tsync_comm_calculate_checksum(requestBuffer, 
                                             nWords * sizeof(uint16_t));

#if LOG_TRANSACTIONS
    logTransactionBuffer(hw, "Sending:", requestBuffer, nWords * sizeof(uint16_t));
#endif

    while (!done && iAttempt < maxAttempts) {
        int wordsWritten;
        int timeoutRemaining = DEFAULT_TSYNC_TIMEOUT_MICROS;

        int packetValid = 0;

        if (iAttempt > 0) {
            TPRO_PRINT("Attempt %u/%u for cai 0x%x, iid 0x%x:\n", 
                       iAttempt, maxAttempts, cai, iid);
        }

        /* send in the request */
        wordsWritten = writeWordsToFIFO(hw, requestBuffer, nWords);

#if STATISTICS
        BytesCountTotalSent += nWords * 2;
#endif
        if (wordsWritten != nWords) {
            transaction->status = TSYNC_DRV_CONNECTION_ERR;
            return 1;
        }
        wordsWritten = writeWordToFIFO(hw, BIG_END_16(checksum));
#if STATISTICS
        BytesCountTotalSent += 2;
#endif
        if (wordsWritten != 1) {
            transaction->status = TSYNC_DRV_CONNECTION_ERR;
            return 1;
        }

        if (skipResponse) {
            nResultBytes = 0;
            done = 1;
            break;
        }
        
        /*************************************************************
         * Read through response packets until we find ours.
         */
    
        bufferPos8 = (uint8_t*) responseBuffer;

        foundSequenceNumber = INVALID_SEQUENCE_NUMBER;

        while ((foundSequenceNumber != seqNum) && (timeoutRemaining > 0)) {
            memset(responseBuffer, 0, 4);
            fifoResult = readWordsFromFIFO(hw, responseBuffer, 2,
                                           &timeoutRemaining);
#if STATISTICS
            BytesCountTotalReceived += 4;
#endif
            if (fifoResult != TSYNC_SUCCESS) { 
                TPRO_PRINT("read failure %s " SRC_LOCATION_FORMAT_STRING, 
                           tsync_strerror(fifoResult), SRC_LOCATION_VARIABLES);
                transaction->status = fifoResult;
                break;
            }
            
            foundSequenceNumber = bufferPos8[1];
            if (foundSequenceNumber != seqNum) {
                TPRO_PRINT("Found packet #%x, expected #%x.  Discarding:\n",
                           foundSequenceNumber, seqNum);
            }
            
            nResultBytes = BIG_END_16(responseBuffer[1]);
            nWords = nResultBytes / sizeof(uint16_t);
            if ((nWords + COMM_LAYER_OVERHEAD_WORDS) > maxTransactionBufferWords) {
                TPRO_PRINT("Payload exceeds available space: %u words vs. %u words.",
                           nWords+COMM_LAYER_OVERHEAD_WORDS, maxTransactionBufferWords);
                transaction->status = TSYNC_DRV_BUFFER_OVERFLOW;
                return 1;
            }
            
            fifoResult = readWordsFromFIFO(hw, &responseBuffer[2], nWords + 1,
                                           &timeoutRemaining);
#if STATISTICS
            BytesCountTotalReceived += (nWords + 1) * 2;
#endif
            if (fifoResult != TSYNC_SUCCESS) { 
                TPRO_PRINT("read failed at " SRC_LOCATION_FORMAT_STRING, SRC_LOCATION_VARIABLES);
                transaction->status = fifoResult;
                break;
            }
            
            if (foundSequenceNumber == seqNum) {
                packetValid = tsync_comm_validate_packet(responseBuffer);
            }
            
            if (foundSequenceNumber != seqNum) {
                TPRO_PRINT("done discarding #%x\n", foundSequenceNumber);
            }
        } /* end of waiting for the right response */

        if ((foundSequenceNumber == seqNum) && packetValid) {
            done = 1;
        }

        iAttempt++;
    } /* end of retry loops */

    if (!done) {
        TPRO_PRINT("tsyncLocalTransaction giving up after %u attempts " SRC_LOCATION_FORMAT_STRING,
                   maxAttempts, SRC_LOCATION_VARIABLES);
        transaction->status = TSYNC_DRV_CONNECTION_ERR;
        transaction->actOutBufferLength = 0;
        return 1;
    }
        
#if LOG_TRANSACTIONS
    logTransactionBuffer(hw, "Received:", responseBuffer, (nWords + COMM_LAYER_OVERHEAD_WORDS) * sizeof(uint16_t));
#endif

    /*************************************************************
     * Copy response back to user
     */

    if (nResultBytes > transaction->maxOutBufferLength) {
        TPRO_PRINT("output overflow: %u vs. %u " SRC_LOCATION_FORMAT_STRING,
                   nResultBytes, transaction->maxOutBufferLength,
                   SRC_LOCATION_VARIABLES);
        transaction->status = TSYNC_DRV_BUFFER_OVERFLOW;
        transaction->actOutBufferLength = 0;
        return 1;
    }

    memcpy(transaction->outBuffer, &responseBuffer[2], nResultBytes);
    transaction->actOutBufferLength = nResultBytes;
    transaction->status = TSYNC_SUCCESS;

#if STATISTICS
    // get current time
    TransactionCounter++;
    currentTime = current_kernel_time();

    // if StartTime == {0} then fill it in, we are just starting
    if ((StartTime.tv_nsec == 0) && (StartTime.tv_sec == 0))
    {
        memcpy(&StartTime, &currentTime, sizeof(StartTime));
    }
    else
    {
        //printk("Sent(bytes): %d     Received (bytes): %d\n", BytesCountTotalSent, BytesCountTotalReceived);

        //check if 1 or more seconds have passed
        numSec = currentTime.tv_sec - StartTime.tv_sec;
        if (((numSec == 1) && (StartTime.tv_nsec < currentTime.tv_nsec)) || 
            (numSec > 1))
        {
            //printk("SentRate (bytes/sec): %d  ReceivedRate (bytes/sec): %d  Calls: %d\n", BytesCountTotalSent, BytesCountTotalReceived, TransactionCounter);

            // set StartTime to currentTime
            memcpy(&StartTime, &currentTime, sizeof(StartTime));

            if ((BytesCountTotalSent + BytesCountTotalReceived)/numSec > MAX_RATE_LIMIT)
            {
                printk("tsyncpci <WARNING>: Total usage of %d Bytes/sec is greater than %d Bytes/sec\n", BytesCountTotalSent + BytesCountTotalReceived, MAX_RATE_LIMIT);
            }

            if ((BytesCountTotalSent + BytesCountTotalReceived) < MIN_RATE_LIMIT)
            {
                printk("tsyncpci <WARNING>: Total usage of %d Bytes/sec is less than %d Bytes/sec\n", BytesCountTotalSent + BytesCountTotalReceived, MIN_RATE_LIMIT);
            }

            // reset the Counters and Averages
            BytesCountTotalReceived = 0;
            BytesCountTotalSent     = 0;
            TransactionCounter      = 0;
        }
    }
#endif

    return (0);
} /* End - tsyncLocalTransaction() */

