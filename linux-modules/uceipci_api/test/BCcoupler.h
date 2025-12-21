#ifndef __BCCOUPLER_H__
#define __BCCOUPLER_H__

//#include "drast/uceipci_api/busapi.h"
#include <cstring>
#include <iostream>
#include "Coupler.h"

#define MAX_NUM_OF_MSGS 1000

extern "C" {
BT_INT _stdcall bc_intFunction(BT_UINT cardnum, struct api_int_fifo* sIntFIFO);
}
namespace test {
using namespace std;
// ----------------------------------------------------------
class BCcoupler : public Coupler {
private:
    struct api_int_fifo _interruptStack;
    int _bus;
    bool _running;

public:
    // Message Counter
    int msgCount[2][32][32];

    BCcoupler(int bus, int minor_frame_duration, int no_response_time = 20, bool use_internal_bus = false,
              bool use_hw_interrupts = true, bool use_sw_interrupts = true)
        : Coupler(bus, use_hw_interrupts, use_sw_interrupts) {
        BT_UINT status;
        _running = false;
        _bus = bus;
        if (_handler != Coupler::INVALID_HANDLER) {
            memset(&msgCount, 0, sizeof(msgCount));
            if (not use_internal_bus) {
                status = BusTools_SetInternalBus(_handler, EXTERNAL_BUS);
                if (status != API_SUCCESS) {
                    cerr << "Warning during BusTools_SetInternalBus : " << BusTools_StatusGetString(status) << endl;
                }
            }
            // TODO : check what timing mode is the bst for us (or if it has to be a param)
            // TODO : if we want a generic Bus Controller, we should handle error ITs
            // TODO : check what are the retry policies in our applications - here, no retry
            // TODO : what is exactly the role of the double buff here?
            //@note :  the late_response_time is useless
            status = BusTools_BC_Init(_handler, REL_GAP, BT1553_INT_END_OF_MESS, 0, no_response_time,
                                      no_response_time - 2, minor_frame_duration, 1);
            if (status) {
                cerr << "Error during BusTools_BC_Init : " << BusTools_StatusGetString(status) << endl;
                _handler = Coupler::INVALID_HANDLER;
            }
            else {
                // Documentation recommends to preallocate enough space for
                // handling all of  the periodic frames and aperiodics msgs
                status = BusTools_BC_MessageAlloc(_handler, MAX_NUM_OF_MSGS);

                if (status) {
                    cerr << "Error during BusTools_BC_MessageAlloc : " << BusTools_StatusGetString(status) << endl;
                    _handler = Coupler::INVALID_HANDLER;
                }
                API_BC_MBUF bc_msg;
                memset((char*)&bc_msg, 0, sizeof(bc_msg));
                bc_msg.messno = 0;  // Message number
                bc_msg.control = BC_CONTROL_MESSAGE;  // This is a standard BC message.
                bc_msg.control |= BC_CONTROL_CHANNELA;  // Send message on bus A (primary).
                bc_msg.control |= BC_CONTROL_BUFFERA;  // Only/ using one buffer, buffer A
                bc_msg.control |= BC_CONTROL_MFRAME_BEG;  // Beginning of minor frame.
                bc_msg.control |= BC_CONTROL_INTERRUPT;
                bc_msg.messno_next = 1;  // Next message number, go to msg #1
                bc_msg.mess_command1.rtaddr = 1;  // Command word 1, RT address
                bc_msg.mess_command1.subaddr = 1;  // Command word 1, Subaddress
                bc_msg.mess_command1.tran_rec = 0;  // Command word 1, transmit (1) or receive (0)
                bc_msg.mess_command1.wcount = 0;  // Command word 1, word count, 0-31, 0=32 words
                bc_msg.errorid = 0;  // Error injection buffer ID, no err inj
                bc_msg.gap_time = 32;  // Intermessage gap time in microseconds

                // Fill data buffer
                for (int i = 0; i < 32; i++) {
                    bc_msg.data[0][i] = 0xCD00 + i;
                }

                // Write the message to board memory
                status = BusTools_BC_MessageWrite(_handler, bc_msg.messno, &bc_msg);
                if (status != API_SUCCESS)
                    cerr << "ERROR during BusTools_BC_MessageWrite : " << BusTools_StatusGetString(status) << endl;
                memset((char*)&bc_msg, 0, sizeof(bc_msg));  // Clear the BC Message Buffer
                bc_msg.messno = 1;  // Message number
                bc_msg.control = BC_CONTROL_MESSAGE;  // This is a standard BC message.
                bc_msg.control |= BC_CONTROL_CHANNELA;  // Send message on bus A (primary).
                bc_msg.control |= BC_CONTROL_BUFFERA;  // Only using one buffer, buffer A.
                bc_msg.control |= BC_CONTROL_MFRAME_END;  // End of minor frame.
                bc_msg.control |= BC_CONTROL_INTERRUPT;
                bc_msg.messno_next = 0;  // Next message number, go to msg #0.
                bc_msg.mess_command1.rtaddr = 1;  // Command word 1, RT address
                bc_msg.mess_command1.subaddr = 2;  // Command word 1, Subaddress
                bc_msg.mess_command1.tran_rec = 1;  // Command word 1, transmit (1) or receive (0)
                bc_msg.mess_command1.wcount = 0;  // Command word 1, word count, 0-31, 0=32 words
                bc_msg.errorid = 0;  // Error injection buffer ID, no err inj
                bc_msg.gap_time = 10;  // Intermessage gap time in microseconds

                // Write the message to board memory
                status = BusTools_BC_MessageWrite(_handler, 1, &bc_msg);
                if (status != API_SUCCESS)
                    cerr << "ERROR during BusTools_BC_MessageWrite : " << BusTools_StatusGetString(status) << endl;
                if (use_hw_interrupts || use_sw_interrupts) {
                    memset(&_interruptStack, 0, sizeof(_interruptStack));
                    _interruptStack.function = bc_intFunction;  // callback function
                    _interruptStack.pUser[0] = (void*)this;  // reference object address for external C callback
                    _interruptStack.iPriority = THREAD_PRIORITY_ABOVE_NORMAL;  // thread's priority level
                    _interruptStack.dwMilliseconds = INFINITE;  // deadline
                    _interruptStack.iNotification = 0;
                    _interruptStack.FilterType = EVENT_BC_MESSAGE;  // Events to trigger on : here new message received
                    // Enable IT on all messages
                    _interruptStack.FilterMask[1][1][2] = 0xffffffff;
                    _interruptStack.FilterMask[1][0][1] = 0xffffffff;
                }
            }
        }
    };

    ~BCcoupler() {
        if (_running)
            stop();
    };

    virtual void dispatch(API_BC_MBUF bc_mbuf) {
        msgCount[bc_mbuf.mess_command1.tran_rec][bc_mbuf.mess_command1.rtaddr][bc_mbuf.mess_command1.subaddr]++;
    };

    int start() {
        int status = -1;
        if (_handler != INVALID_HANDLER) {
            if (not _running) {
                status = BusTools_RegisterFunction(_handler, &_interruptStack, 1);  // 1=register, 0=unregister
                if (status != API_SUCCESS) {
                    cerr << "Warning : While  registering callback for bus " << _bus
                         << "API says : " << BusTools_StatusGetString(status) << endl;
                }
                status = BusTools_BC_StartStop(_handler, BC_START);
                if (status != API_SUCCESS) {
                    cerr << "Error : While starting RT operations on bus " << _bus
                         << "API says : " << BusTools_StatusGetString(status) << endl;
                }
                else {
                    _running = true;
                }
            }
            else {
                cerr << "Warning : Attempt to start a running bus (" << _bus << ")" << endl;
                status = 0;
            }
        }
        return status;
    };

    int stop() {
        int status = -1;
        if (_handler != INVALID_HANDLER) {
            if (_running) {
                status = BusTools_BC_StartStop(_handler, BC_STOP);
                if (status != API_SUCCESS) {
                    cerr << "Error : While stopping BC operations on bus " << _bus
                         << "API says : " << BusTools_StatusGetString(status) << endl;
                }
                else
                    _running = false;
                int status2 = BusTools_RegisterFunction(_handler, &_interruptStack, 0);  // 1=register, 0=unregister
                if (status2 != API_SUCCESS) {
                    cerr << "Warning : While  stopping callback for bus " << _bus
                         << "API says : " << BusTools_StatusGetString(status2) << endl;
                }
                status += status2;
            }
            else  // nothing to do
                status = 0;
        }
        return status;
    };
};

class BcErrCoupler : public BCcoupler {
public:
    int nbBusyMsgs;
    int nbNoresponse;
    virtual void dispatch(API_BC_MBUF bc_mbuf) {
        if (bc_mbuf.mess_status1.busy)
            nbBusyMsgs++;
        if (bc_mbuf.status & (BT1553_INT_NO_RESP)) {
            nbNoresponse++;
        }
        // BCcoupler::dispatch(bc_mbuf);
    };

    BcErrCoupler(int bus, int minor_frame_duration, int no_response_time = 20, bool use_internal_bus = false,
                 bool use_hw_interrupts = true, bool use_sw_interrupts = true)
        : BCcoupler(bus, minor_frame_duration, no_response_time, use_internal_bus, use_hw_interrupts,
                    use_sw_interrupts) {
        nbBusyMsgs = 0;
        nbNoresponse = 0;
    };

private:
};

}  // namespace test

extern "C" {
#include <cstdio>
using namespace std;
using namespace test;
BT_INT _stdcall bc_intFunction(BT_UINT cardnum, struct api_int_fifo* sIntFifo) {
    API_BC_MBUF bc_mbuf;
    BT_INT tail, status;
    tail = sIntFifo->tail_index;
    while (tail != sIntFifo->head_index) {
        // Get the message
        status = BusTools_BC_MessageRead(cardnum, sIntFifo->fifo[tail].bufferID, &bc_mbuf);
        // TODO : stack msgs and thread this call?
        if (status == API_SUCCESS)
            ((test::BCcoupler*)(sIntFifo->pUser[0]))->dispatch(bc_mbuf);
        tail++;
        tail &= sIntFifo->mask_index;
        sIntFifo->tail_index = tail;
    }
    return 0;
};
};

#endif
