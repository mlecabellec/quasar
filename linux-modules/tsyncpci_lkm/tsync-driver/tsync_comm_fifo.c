#include "tsync_driver_helpers.h"
#include "tsync_comm_fifo.h"

TSYNC_ERROR readWordsFromFIFO(TPRO_INSTANCE_T *hw,
                              void *destination, int nWords,
                              int *timeoutRemaining) {


    int wordsRead = 0;
    uint16_t *dst16 = (uint16_t*) destination;

    int localTimeoutRemaining = *timeoutRemaining;
    const unsigned int microsPerDelay = 50;

    while ((wordsRead < nWords) && (localTimeoutRemaining > 0)) {
        if (TSYNC_READ_REG(tsync_fifo_status_offset) & 0x2) {
            BUSY_WAIT(microsPerDelay);
            localTimeoutRemaining -= microsPerDelay;
        }

        while ((wordsRead < nWords) && ((TSYNC_READ_REG(tsync_fifo_status_offset) & 0x2) == 0)) {
            dst16[wordsRead] = TSYNC_READ_REG(tsync_fifo_receive_offset);
            wordsRead++;
        }
    }

    if (wordsRead < nWords) {
        TPRO_PRINT("Timed out: read %u words, expected %u. " SRC_LOCATION_FORMAT_STRING,
                   wordsRead, nWords, SRC_LOCATION_VARIABLES);
        *timeoutRemaining = localTimeoutRemaining;
        return TSYNC_DRV_FIFO_READ_TIMEOUT;
    }

    /* update the caller's timeoutRemaining to reflect what we
     * consumed here. */
    *timeoutRemaining = localTimeoutRemaining;

    return TSYNC_SUCCESS;
}

int writeWordsToFIFO(TPRO_INSTANCE_T *hw, void *source, int nWords) {

    int wordsWritten = 0;
    uint16_t *src16 = (uint16_t*) source;

    const int microsPerDelay = 50;
    const int maxTimeoutMicros = 2000000;
    int timeoutRemaining = maxTimeoutMicros;

    while ((wordsWritten < nWords) && (timeoutRemaining > 0)) {
        while ((TSYNC_READ_REG(tsync_fifo_status_offset) & 0x40) && (timeoutRemaining > 0)) {
            BUSY_WAIT(microsPerDelay);
            timeoutRemaining -= microsPerDelay;
        }

        while ((wordsWritten < nWords) && ((TSYNC_READ_REG(tsync_fifo_status_offset) & 0x40) == 0)) {
            TSYNC_WRITE_REG(src16[wordsWritten], tsync_fifo_transmit_offset);
            wordsWritten++;
        }
    }

    if (wordsWritten < nWords) {
        TPRO_PRINT("Timed out: wrote %u/%u words, waited %d micros " SRC_LOCATION_FORMAT_STRING,
                   wordsWritten, nWords, maxTimeoutMicros, SRC_LOCATION_VARIABLES);
    }

    return wordsWritten;
}

int writeWordToFIFO(TPRO_INSTANCE_T *hw, uint16_t value) {
    return writeWordsToFIFO(hw, &value, 1);
}

int tsyncFlushFifos(TPRO_INSTANCE_T *hw) {
    TSYNC_WRITE_REG(0x11, tsync_fifo_status_offset);

    return 0;
}

