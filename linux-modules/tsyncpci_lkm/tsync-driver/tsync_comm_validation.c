
#include "tsync_comm_fifo.h"
#include "tsync_comm_validation.h"

uint16_t tsync_comm_calculate_checksum(void *buffer, size_t bufferLength) {
    uint64_t byteSum = 0;
    size_t iByte = 0;
    uint8_t *bufferAsChar = (uint8_t *) buffer;

    for (iByte = 0; iByte < bufferLength; iByte++) {
        byteSum += bufferAsChar[iByte];
    }

    return (uint16_t) (byteSum & 0xffff);
}

unsigned int tsync_comm_validate_packet(void *packet) {
    uint16_t payloadLength;
    uint32_t preSumPacketLength;

    uint16_t *packetShorts = (uint16_t*) packet;
    uint16_t calculatedChecksum, foundChecksum;
    uint16_t *foundChecksumPtr;
    
    payloadLength = BIG_END_16(packetShorts[1]);
    preSumPacketLength = payloadLength + 4;
    
    calculatedChecksum =
        tsync_comm_calculate_checksum(packet, preSumPacketLength);

    foundChecksumPtr = &packetShorts[preSumPacketLength / sizeof(uint16_t)];
    foundChecksum = *foundChecksumPtr;
    
    return (foundChecksum == BIG_END_16(calculatedChecksum));
}

uint8_t tsync_comm_get_next_seq_num() {
    static uint8_t nextSeqNum = 0;

    nextSeqNum++;
    if (nextSeqNum > 0xf0) {
        nextSeqNum = 1;
    }
    return nextSeqNum;
}

TSYNC_ERROR tsync_check_board_connection_health(TPRO_INSTANCE_T *hw) {

#define maxTransactionBufferWords 32
    uint16_t buffer[maxTransactionBufferWords];
    uint8_t *bufferPos8;
    uint16_t *bufferPos16;

    uint16_t checksum;

    const uint8_t expectedPacketHeader = 0x82;
    uint8_t seqNum = tsync_comm_get_next_seq_num();
    size_t nWords, nBytes;

    int timeoutRemaining = 2000000; // microseconds

    TSYNC_ERROR fifoResult;

    tsyncFlushFifos(hw);

    bufferPos8 = (uint8_t*)buffer;
    *bufferPos8++ = 0x02; // HL_CL_HLTH
    *bufferPos8++ = seqNum;
    
    bufferPos16 = (uint16_t*)bufferPos8;
    *bufferPos16++ = BIG_END_16(0);

    nBytes = (uintptr_t)bufferPos16 - (uintptr_t)buffer;
    nWords = nBytes / sizeof(uint16_t);
    checksum = tsync_comm_calculate_checksum(buffer, nBytes);

    writeWordsToFIFO(hw, buffer, nWords);
    writeWordToFIFO(hw, BIG_END_16(checksum));
    
    /*************************************************************
     * Wipe out the beginning of the buffer so we can confirm that
     * something came back. Then wait for response and accept.
     */

    memset(buffer, 0, 4);
    
    fifoResult = readWordsFromFIFO(hw, buffer, 3, &timeoutRemaining);
    if (fifoResult != TSYNC_SUCCESS) { 
        TPRO_PRINT("read failed at " SRC_LOCATION_FORMAT_STRING, 
                   SRC_LOCATION_VARIABLES);
        return TSYNC_DRV_FIFO_READ_TIMEOUT; 
    }

    bufferPos8 = (uint8_t*) buffer;
    if ((bufferPos8[0] != expectedPacketHeader) || (bufferPos8[1] != seqNum)) {
        TPRO_PRINT("Unexpected response from board: %02x %02x (expected %02x %02x) " 
                   SRC_LOCATION_FORMAT_STRING, 
                   bufferPos8[0], bufferPos8[1], 
		   expectedPacketHeader, seqNum, SRC_LOCATION_VARIABLES);
        return TSYNC_DRV_CONNECTION_ERR;
    }

    if (! tsync_comm_validate_packet(buffer) ) {
        TPRO_PRINT("Checksum failed for returned transaction data.\n");
        return TSYNC_DRV_CONNECTION_ERR;
    }


    return TSYNC_SUCCESS;
#undef maxTransactionBufferWords
} /* end of tsync_check_board_connection_health */

