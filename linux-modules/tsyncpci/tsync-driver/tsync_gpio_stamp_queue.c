#include "tsync_gpio_stamp_queue.h"

TSYNC_ERROR gpioQueueInit(TPRO_INSTANCE_T *hw, unsigned int iGpio) {
    gpio_timestamp_queue_t *queue; 
    if (iGpio >= TMSTMP_SRC_COUNT) {
        return TSYNC_DRV_INVALID_INDEX;
    }
    queue = &hw->gpioTimestampQueue[iGpio];

    queue->writeIndex = 0;
    queue->readIndex = 0;
    queue->timestampCount = 0;

    return TSYNC_SUCCESS;
} 

TSYNC_ERROR gpioQueueAdd(TPRO_INSTANCE_T *hw, unsigned int iGpio, 
                         gpio_timestamp_t *timestamp) {

    gpio_timestamp_queue_t *queue; 
    if (iGpio >= TMSTMP_SRC_COUNT) {
        return TSYNC_DRV_INVALID_INDEX;
    }
    queue = &hw->gpioTimestampQueue[iGpio];


    if (queue->timestampCount == TSYNC_TIMESTAMP_DATA_NUM) {
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }
    
    memcpy(&queue->gpioTimestampRingBuffer[queue->writeIndex], 
           timestamp, sizeof(gpio_timestamp_t));

    queue->writeIndex += 1;
    queue->writeIndex %= TSYNC_TIMESTAMP_DATA_NUM;
    queue->timestampCount += 1;

    return TSYNC_SUCCESS;
}

TSYNC_ERROR gpioQueueRemove(TPRO_INSTANCE_T *hw, unsigned int iGpio, 
                            gpio_timestamp_t *timestamp) {

    gpio_timestamp_queue_t *queue; 
    if (iGpio >= TMSTMP_SRC_COUNT) {
        return TSYNC_DRV_INVALID_INDEX;
    }
    queue = &hw->gpioTimestampQueue[iGpio];


    if (queue->timestampCount == 0) {
        return TSYNC_DRV_QUEUE_EMPTY;
    }
    
    memcpy(timestamp, 
           &queue->gpioTimestampRingBuffer[queue->readIndex],
           sizeof(gpio_timestamp_t));

    queue->readIndex += 1;
    queue->readIndex %= TSYNC_TIMESTAMP_DATA_NUM;
    queue->timestampCount -= 1;

    return TSYNC_SUCCESS;
}

TSYNC_ERROR gpioQueueCount(TPRO_INSTANCE_T *hw, unsigned int iGpio, 
                           unsigned int *count) {
    TSYNC_ERROR drainResult;

    gpio_timestamp_queue_t *queue; 
    if (iGpio >= TMSTMP_SRC_COUNT) {
        return TSYNC_DRV_INVALID_INDEX;
    }
    queue = &hw->gpioTimestampQueue[iGpio];

    drainResult = gpioQueueDrainBoardFIFO(hw);
    if (drainResult != TSYNC_SUCCESS) {
        return drainResult;
    }

    *count = queue->timestampCount;

    return TSYNC_SUCCESS;
}

TSYNC_ERROR gpioQueueDrainBoardFIFO(TPRO_INSTANCE_T *hw) {

    uint16_t tsStatus;

    gpio_timestamp_t timeStampTemp;

    tsStatus = TSYNC_READ_REG(tsync_timestamp_status_offset);

    while ((tsStatus & (1 << 5)) == 0) {
        /* copy the event from the fifo to a temp buffer */
        timeStampTemp.superSecLow =
            TSYNC_READ_REG(tsync_timestamp_fifo_offset);
        timeStampTemp.superSecMidLow =
            TSYNC_READ_REG(tsync_timestamp_fifo_offset + sizeof(uint16_t));
        timeStampTemp.superSecMidHigh =
            TSYNC_READ_REG(tsync_timestamp_fifo_offset + (2*sizeof(uint16_t)));
        timeStampTemp.superSecHigh =
            TSYNC_READ_REG(tsync_timestamp_fifo_offset + (3*sizeof(uint16_t)));
        timeStampTemp.subSecLow =
            TSYNC_READ_REG(tsync_timestamp_fifo_offset + (4*sizeof(uint16_t)));
        timeStampTemp.subSecHigh =
            TSYNC_READ_REG(tsync_timestamp_fifo_offset + (5*sizeof(uint16_t)));

        /* identify which GPIO it came from */
        if (timeStampTemp.superSecHigh & (1 << 4)) {
            gpioQueueAdd(hw, TMSTMP_SRC_HOST, &timeStampTemp);
            gpioQueueAdd(hw, TMSTMP_SRC_HOST_AND_GPI_0, &timeStampTemp);
        }

        if (timeStampTemp.superSecHigh & (1 << 5)) {
            gpioQueueAdd(hw, TMSTMP_SRC_GPI_0, &timeStampTemp);
            gpioQueueAdd(hw, TMSTMP_SRC_HOST_AND_GPI_0, &timeStampTemp);
        }

        if (timeStampTemp.superSecHigh & (1 << 6)) {
            gpioQueueAdd(hw, TMSTMP_SRC_GPI_1, &timeStampTemp);
        }

        if (timeStampTemp.superSecHigh & (1 << 7)) {
            gpioQueueAdd(hw, TMSTMP_SRC_GPI_2, &timeStampTemp);
        }

        if (timeStampTemp.superSecHigh & (1 << 8)) {
            gpioQueueAdd(hw, TMSTMP_SRC_GPI_3, &timeStampTemp);
        }
        
        tsStatus = TSYNC_READ_REG(tsync_timestamp_status_offset);
    }

    return TSYNC_SUCCESS;
}

