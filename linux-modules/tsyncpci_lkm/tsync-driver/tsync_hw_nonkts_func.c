#include "tsync_hw_nonkts_func.h"
#include "ddtsync.h"

#ifdef LAFAYETTE
#include "tsync_hw_nonkts_lafayette.h"
#endif

TSYNC_ERROR hw_nonkts_get_trans(uint16_t iid,
                                TPRO_INSTANCE_T *hw,
                                uint8_t *inPayload,
                                uint32_t inLength,
                                uint8_t *outPayload,
                                uint32_t maxOutLength,
                                uint32_t *actualOutLength) {

    TSYNC_ERROR result = TSYNC_DRV_INVALID_INDEX;

    switch(iid) {

#ifdef LAFAYETTE
        case HW_NONKTS_LCD:
            result = get_hw_lcd_byte(hw,
                                     inPayload,
                                     inLength,
                                     outPayload,
                                     maxOutLength,
                                     actualOutLength);
            break;

        case HW_NONKTS_KEYPAD:
            result =
                get_hw_keypad(hw,
                              outPayload,
                              maxOutLength,
                              actualOutLength);
            break;

        case HW_NONKTS_POWER:
            result =
                get_hw_power_status(hw,
                                    outPayload,
                                    maxOutLength,
                                    actualOutLength);
            break;
        case HW_FAN_EN:
            result =
                get_hw_fan_enabled(hw,
                                   outPayload,
                                   maxOutLength,
                                   actualOutLength);
            break;
#endif

        default:
            result = TSYNC_DRV_INVALID_INDEX;
            *actualOutLength = 0;
            break;
    };

    return result;

} /* end of hw_nonkts_get_trans */


TSYNC_ERROR hw_nonkts_set_trans(uint16_t iid,
                                TPRO_INSTANCE_T *hw,
                                uint8_t *inPayload,
                                uint32_t inLength,
                                uint8_t *outPayload,
                                uint32_t maxOutLength,
                                uint32_t *actualOutLength) {

    TSYNC_ERROR result = TSYNC_DRV_INVALID_INDEX;

    switch(iid) {

#ifdef LAFAYETTE
        case HW_NONKTS_LCD:
            result = set_hw_lcd_value(hw,
                                      inPayload,
                                      inLength);
            break;
        case HW_FAN_EN:
            result = set_hw_fan_enabled(hw, inPayload, inLength);
            break;
#endif

        default:
            result = TSYNC_DRV_INVALID_INDEX;
            break;

    };

    *actualOutLength = 0;

    return result;

} /* end of hw_nonkts_set_trans */
