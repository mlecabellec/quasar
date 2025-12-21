
#ifndef __COUPLER_H__
#define __COUPLER_H__

extern "C" {
#include "drast_lkm/uceipci_api/busapi.h"
}
#include <iostream>
namespace test {
using namespace std;

// ----------------------------------------------------------
class Coupler {
public:  // types and constants
    static const BT_UINT INVALID_HANDLER = 5;

private:
    // Types
    struct Versions {
        BT_UINT apiRevision;
        BT_INT fwBuildNumber;
        float fwLpuRevision;
        float fwWcsRevision;
        BT_U32BIT hwSerialNum;
    };

    // Properties
    Versions* _version;
    DEVICE_INFO _features;

    // Methods
    // This method and the public getters associated should be used as member functions
    int getBoardInfo() {
        int status = -1;
        if (_handler != INVALID_HANDLER) {
            if (_version == NULL) {
                _version = new Versions;

                if (_version != NULL) {
                    _version->hwSerialNum = 0x0;
                    BusTools_GetSerialNumber(_handler, &(_version->hwSerialNum));
                    BusTools_GetFWRevision(_handler, &(_version->fwWcsRevision), &(_version->fwLpuRevision),
                                           &(_version->fwBuildNumber));
                    {
                        BT_UINT useless;
                        BusTools_GetRevision(_handler, &useless, &(_version->apiRevision));
                    }
                    status = 0;
                }
            }
            else {
                // nothing to do
                status = 0;
            }
        }
        return status;
    };

protected:
    BT_UINT _handler;

public:
    BT_UINT handler() {
        return _handler;
    };

    int apiVersion() {
        if (getBoardInfo() == 0)
            return _version->apiRevision;
        else
            return -1;
    };

    int serialNumber() {
        if (getBoardInfo() == 0)
            return _version->hwSerialNum;
        else
            return -1;
    };

    int numOfBuses() {
        return _features.nchan;
    };

    bool boardHasIrig() {
        return _features.irig;
    };

    bool boardIsMultiFunction() {
        return (_features.mode == API_MULTI_FUNCTION);
    };
    bool boardIsDualFunction() {
        return (_features.mode == API_DUAL_FUNCTION);
    };

    Coupler(int plug, bool use_hw_interrupts, bool use_sw_interrupts) {
        _version = NULL;
        int status, mode;
        // set the board operating mode

        _handler = INVALID_HANDLER;
        mode = API_B_MODE;  // we don't plan to use 1553-A
        if (use_hw_interrupts && use_sw_interrupts) {
            mode |= API_HW_INTERRUPT;
        }
        else if (use_hw_interrupts && !use_sw_interrupts) {
            mode |= API_HW_ONLY_INT;
        }
        else if (!use_hw_interrupts && use_sw_interrupts) {
            mode |= API_SW_INTERRUPT;
        }
        else {
            mode |= API_MANUAL_INT;
        }
        // check if the "plug" number is valid
        if (plug >= 0 && plug <= 3) {
            // If so, perform channel opening
            // TODO : In this test, only one card (numer 0 is used) - the final implementation should take the card num
            // into account
            status = BusTools_API_OpenChannel(&_handler, mode, 0, plug);
            //_opened=true;
            if (status) {
                cerr << "Coupler::Coupler - Error during BusTools_API_OpenChannel : "
                     << BusTools_StatusGetString(status) << endl;
                ;
                cerr << "BusTools_API_OpenChannel returns handler " << _handler << endl;
                //_opened=false;
                _handler = INVALID_HANDLER;
            }
            else {
                BusTools_GetDevInfo(0, &_features);
                // The mode member doesn't trat DualFunction card
                _features.mode = BusTools_BoardIsMultiFunction(_handler);
            }
        }
    };

    ~Coupler() {
        if (_handler != INVALID_HANDLER)
            BusTools_API_Close(_handler);
        if (_version != NULL)
            delete _version;
    };
};
};  // namespace test
#endif
