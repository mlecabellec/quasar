#ifndef __RTCOUPLER_H__
#define __RTCOUPLER_H__

//#include "drast/uceipci_api/busapi.h"
#include <cstring>
#include <iostream>
#include "Coupler.h"

extern "C" {
BT_INT _stdcall rt_intFunction(BT_UINT cardnum, struct api_int_fifo* sIntFIFO);
}
namespace test {
using namespace std;
// ----------------------------------------------------------
class RTcoupler : public Coupler {
private:
    struct api_int_fifo _interruptStack;
    int _bus;
    bool _running;

public:
    // Message Counter
    int msgCount[2][32][32];

    RTcoupler(int bus, bool use_internal_bus = false, bool use_hw_interrupts = true, bool use_sw_interrupts = true)
        : Coupler(bus, use_hw_interrupts, use_sw_interrupts) {
        int status;
        API_RT_ABUF abuf;  // RT address buffer structure.
        API_RT_CBUF cbuf;  // RT control buffer structures.
        API_RT_MBUF_WRITE msg_buffer_write;  // RT data emission structure
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
            status = BusTools_RT_Init(_handler, 0);  // 2nd arg MUST be 0
            if (status) { /*{{{*/
                cerr << "Error during BusTools_RT_Init : " << BusTools_StatusGetString(status) << endl;
                _handler = Coupler::INVALID_HANDLER;
            } /*}}}*/
            else {
                if (use_hw_interrupts || use_hw_interrupts) {
                    memset(&_interruptStack, 0, sizeof(_interruptStack));
                    _interruptStack.function = rt_intFunction;  // callback function
                    _interruptStack.pUser[0] = (void*)this;  // reference object address for external C callback
                    _interruptStack.iPriority = THREAD_PRIORITY_ABOVE_NORMAL;  // thread's priority level
                    _interruptStack.dwMilliseconds = INFINITE;  // deadline
                    _interruptStack.iNotification = 0;
                    _interruptStack.FilterType = EVENT_RT_MESSAGE;  // Events to trigger on : here new message received
                    // Enable IT on all messages
                    _interruptStack.FilterMask[1][1][2] = 0xffffffff;
                    _interruptStack.FilterMask[1][0][1] = 0xffffffff;
                }
                // Setup RT address buffer for our RT (RT1)
                abuf.enable_a = 1;  // Respond on bus A
                abuf.enable_b = 1;  // Respond on bus B
                abuf.inhibit_term_flag =
                    RT_ABUF_EXT_STATUS;  //  Definit le mode de réponse, de gestion du status et des CC
                abuf.status = 0x0000;  // Set status word no status bits set
                abuf.bit_word = 0x0000;  // Set BIT word (for mode code 19)
                status = BusTools_RT_AbufWrite(_handler, 1, &abuf);
                if (status != API_SUCCESS) {
                    cerr << "Error during  BusTools_RT_AbufWrite : " << BusTools_StatusGetString(status) << endl;
                    _handler = Coupler::INVALID_HANDLER;
                }
                else {
                    // Setup RT1 Subaddresses
                    cbuf.legal_wordcount = 0xFFFFFFFF;  // any word count is legal.

                    // Setup a control buffer - RT1, SA1, Receive (0), 1 buffer.
                    status = BusTools_RT_CbufWrite(_handler, 1, 1, 0, 1, &cbuf);
                    if (status != API_SUCCESS) {
                        cerr << "Warning :  BusTools_RT_CbufWrite declares : " << BusTools_StatusGetString(status)
                             << endl;
                    }
                    else {
                        // Initialize data structure
                        // Select the interrupt event : here valid msg
                        msg_buffer_write.enable = BT1553_INT_END_OF_MESS;
                        msg_buffer_write.error_inj_id = 0;  // No error injection
                                                            // set data
                        for (int i = 0; i < 32; i++)
                            msg_buffer_write.mess_data[i] = 0xA5A5;
                        // send data
                        // RT1, SA1,Receive, buffer 0;
                        status = BusTools_RT_MessageWrite(_handler, 1, 1, 0, 0, &msg_buffer_write);
                        if (status != API_SUCCESS) {
                            cerr << "Warning :  BusTools_RT_MessageWrite declares : "
                                 << BusTools_StatusGetString(status) << endl;
                        }
                    }
                    // Setup a control buffer - RT1, SA2, Transmit (1), 1 buffer.
                    status = BusTools_RT_CbufWrite(_handler, 1, 2, 1, 1, &cbuf);
                    if (status != API_SUCCESS) {
                        cerr << "Warning :  BusTools_RT_CbufWrite declares : " << BusTools_StatusGetString(status)
                             << endl;
                    }
                    else {
                        // Initialize data structure
                        // Select the interrupt event : here valid msg
                        msg_buffer_write.enable = BT1553_INT_END_OF_MESS;
                        msg_buffer_write.error_inj_id = 0;  // No error injection
                                                            /// set data
                        for (int i = 0; i < 32; i++)
                            msg_buffer_write.mess_data[i] = 0xA5A5;
                        // send data
                        // RT1, SA2,Transmit, buffer 0;
                        status = BusTools_RT_MessageWrite(_handler, 1, 2, 1, 0, &msg_buffer_write);
                        if (status != API_SUCCESS) {
                            cerr << "Warning :  BusTools_RT_MessageWrite declares : "
                                 << BusTools_StatusGetString(status) << endl;
                        }
                    }
                }
            }
        }
    };

    ~RTcoupler() {
        if (_running)
            stop();
    };

    virtual void dispatch(API_RT_MBUF_READ rt_mbuf) {
        msgCount[rt_mbuf.mess_command.tran_rec][rt_mbuf.mess_command.rtaddr][rt_mbuf.mess_command.subaddr]++;
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
                status = BusTools_RT_StartStop(_handler, RT_START);
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
                status = BusTools_RT_StartStop(_handler, RT_STOP);
                if (status != API_SUCCESS) {
                    cerr << "Error : While stopping RT operations on bus " << _bus
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

class RtErrCoupler : public RTcoupler {
private:
    int numSwitchBusy;
    int numSwitchRespond;
    int numSwitchNotify;
    bool switchedBusy;
    bool switchedRespond;
    bool switchedNotify;

    void switchBusy() {
        int cr;
        if (not switchedBusy) {
            switchedBusy = true;
            cr = BusTools_RT_MessageWriteStatusWord(_handler, 1, 1, 0, 0, API_1553_STAT_BY, RT_EXT_STATUS);
            if (cr)
                cerr << "Warning during BusTools_RT_MessageWriteStatusWord(1) : " << BusTools_StatusGetString(cr)
                     << endl;
        }
        else {
            cr = BusTools_RT_MessageWriteStatusWord(_handler, 1, 1, 0, 0, 0x0, RT_EXT_STATUS);
            if (cr)
                cerr << "Warinig during BusTools_RT_MessageWriteStatusWord(0) : " << BusTools_StatusGetString(cr)
                     << endl;
        }
    }

    void switchResponse() {
        int cr;
        if (not switchedRespond) {
            switchedRespond = true;
            cr = BusTools_RT_MonitorEnable(_handler, 1, 1);
            if (cr)
                cerr << "Warning during BusTools_RT_MonitorEable(1) : " << BusTools_StatusGetString(cr) << endl;
        }
        else {
            cr = BusTools_RT_MonitorEnable(_handler, 1, 0);
            if (cr)
                cerr << "Warning during BusTools_RT_MonitorEable(0) : " << BusTools_StatusGetString(cr) << endl;
        }
    };

    void switchNotify() {
        int cr;
        API_RT_MBUF_WRITE msg_buffer_write;
        msg_buffer_write.error_inj_id = 0;  // No error injection
                                            // set data
        for (int i = 0; i < 32; i++)
            msg_buffer_write.mess_data[i] = 0xA5A5;
        if (not switchedNotify) {
            switchedNotify = true;
            msg_buffer_write.enable = 0x0;
        }
        else {
            msg_buffer_write.enable = BT1553_INT_END_OF_MESS;
        }
        cr = BusTools_RT_MessageWrite(_handler, 1, 1, 0, 0, &msg_buffer_write);
        if (cr)
            cerr << "Warning during BusTools_RT_MessageWrite(" << msg_buffer_write.enable
                 << ") : " << BusTools_StatusGetString(cr) << endl;
    };

public:
    virtual void dispatch(API_RT_MBUF_READ rt_mbuf) {
        RTcoupler::dispatch(rt_mbuf);
        if (rt_mbuf.mess_command.subaddr == 2) {
            if (msgCount[1][1][2] == numSwitchBusy || msgCount[1][1][2] == 2 * numSwitchBusy) {
                switchBusy();
            }
            else if (msgCount[1][1][2] == numSwitchRespond || msgCount[1][1][2] == 2 * numSwitchRespond)
                switchResponse();
            else if (msgCount[1][1][2] == numSwitchNotify || msgCount[1][1][2] == 2 * numSwitchNotify)
                switchNotify();
        }
    };

    RtErrCoupler(int bus, bool use_internal_bus = false, bool use_hw_interrupts = true, bool use_sw_interrupts = true)
        : RTcoupler(bus, use_internal_bus, use_hw_interrupts, use_sw_interrupts) {
        numSwitchBusy = 5;
        numSwitchRespond = 13;
        numSwitchNotify = 27;
        switchedBusy = false;
        switchedRespond = false;
        switchedNotify = false;
    };
};

}  // namespace test

extern "C" {
BT_INT _stdcall rt_intFunction(BT_UINT cardnum, struct api_int_fifo* sIntFifo) {
    API_RT_MBUF_READ rt_mbuf;
    BT_INT tail, status;
    tail = sIntFifo->tail_index;
    while (tail != sIntFifo->head_index) {
        // Get the message
        status = BusTools_RT_MessageRead(cardnum, sIntFifo->fifo[tail].rtaddress, sIntFifo->fifo[tail].subaddress,
                                         sIntFifo->fifo[tail].transrec, sIntFifo->fifo[tail].bufferID, &rt_mbuf);
        // TODO : stack msgs and thread this call?
        if (status == API_SUCCESS)
            ((test::RTcoupler*)(sIntFifo->pUser[0]))->dispatch(rt_mbuf);
        tail++;
        tail &= sIntFifo->mask_index;
        sIntFifo->tail_index = tail;
    }
    return 0;
};
};

#endif
