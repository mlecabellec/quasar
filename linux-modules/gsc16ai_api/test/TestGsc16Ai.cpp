/*
 * @file TestGsc16Ai.cpp
 *
 * Copyright 2020 ************. All rights reserved.
 * Use is subject to license terms.
 *
 * $Id$
 * $Date$
 */
#include <cppunit/extensions/HelperMacros.h>

#include <mastol/util/Logger.hpp>
#include <mastol/util/Runnable.hpp>
#include <mastol/util/Synchro.hpp>
#include <mastol/util/Thread.hpp>

extern "C" {
#include "drast_lkm/gsc16ai_api/16ai64ssa.h"
#include "drast_lkm/gsc16ai_api/16ai64ssa_utils.h"
}

#include <cmath>
using namespace mastol::util;

namespace test {

LOGGER_CREATE("test::TestGsc16Ai", "");
#define BOARD_EXISTS                                                           \
  (ai64ssa_init_util(0) == 0 &&                                                \
   ai64ssa_count_boards(0, &TestGsc16Ai::_BoardCount) == 0 &&                  \
   TestGsc16Ai::_BoardCount > 0)

// ----------------------------------------------------------
// test fixture implementation
class TestGsc16Ai : public CppUnit::TestFixture, private virtual Runnable {
  CPPUNIT_TEST_SUITE(TestGsc16Ai);
  CPPUNIT_CONDITIONAL_TEST(IS_INTERACTIVE &&BOARD_EXISTS, testRead);
  CPPUNIT_CONDITIONAL_TEST(IS_INTERACTIVE &&BOARD_EXISTS, testWait);
  CPPUNIT_TEST_SUITE_END();

private:
  static s32 _BoardCount;
  int _fileDesc = 0;
  gsc_wait_t _waitSrc;
  Mutex _waitStartMutex;
  Mutex _waitEndMutex;
  Monitor _waitStartMonitor;
  Monitor _waitEndMonitor;
  bool _running = true;
  Thread *_currentThread = nullptr;

  s32 _src_a = 0, _src_b = 0;
  s32 _nrate_a = 0, _nrate_b = 0;
  s32 _enable_a = 0, _enable_b = 0;

  int32_t _dmaValues[64];

public:
  void setUp() { _fileDesc = 0; }

  void tearDown() {
    _running = false;
    if (_currentThread != nullptr) {
      _currentThread->join(1E4);
    }
    if (_fileDesc != 0)
      ai64ssa_close_util(_fileDesc, 1, 1);
  }

  int ai64ssa_config_ai5V(int _fileDesc, int index, int verbose, s32 fsamp) {
    s32 chans;
    int errs = 0;
    s32 qty = 32;
    s32 range;

    gsc_label_index("Input Configuration", index);
    errs += ai64ssa_query(_fileDesc, -1, 0, AI64SSA_QUERY_CHANNEL_QTY, &qty);
    errs += ai64ssa_query(_fileDesc, -1, 0, AI64SSA_QUERY_CHAN_RANGE, &range);

    if (range)
      chans = AI64SSA_CHAN_ACTIVE_RANGE;
    else if (qty >= 64)
      chans = AI64SSA_CHAN_ACTIVE_0_63;
    else
      chans = AI64SSA_CHAN_ACTIVE_0_31;

    if (verbose)
      printf("\n");

    gsc_label_level_inc();

    errs += ai64ssa_initialize(_fileDesc, index, verbose);
    errs += ai64ssa_rx_io_mode(_fileDesc, index, verbose, GSC_IO_MODE_BMDMA,
                               nullptr);
    errs += ai64ssa_rx_io_overflow(_fileDesc, index, verbose,
                                   AI64SSA_IO_OVERFLOW_CHECK, nullptr);
    errs += ai64ssa_rx_io_timeout(_fileDesc, index, verbose,
                                  AI64SSA_IO_TIMEOUT_DEFAULT, nullptr);
    errs += ai64ssa_rx_io_underflow(_fileDesc, index, verbose,
                                    AI64SSA_IO_UNDERFLOW_CHECK, nullptr);
    errs += ai64ssa_ai_range(_fileDesc, index, verbose, AI64SSA_AI_RANGE_0_5V,
                             nullptr);
    errs += ai64ssa_ai_buf_thr_lvl(_fileDesc, index, verbose, 250000, nullptr);
    errs += ai64ssa_ai_mode(_fileDesc, index, verbose, AI64SSA_AI_MODE_SINGLE,
                            nullptr);
    errs += ai64ssa_burst_size(_fileDesc, index, verbose, 1, nullptr);
    errs += ai64ssa_burst_src(_fileDesc, index, verbose,
                              AI64SSA_BURST_SRC_DISABLE, nullptr);
    errs += ai64ssa_chan_active(_fileDesc, index, verbose, chans, nullptr);
    errs += ai64ssa_chan_first(_fileDesc, index, verbose, 0, nullptr);
    errs += ai64ssa_chan_last(_fileDesc, index, verbose, qty - 1, nullptr);
    errs += ai64ssa_chan_single(_fileDesc, index, verbose, 0, nullptr);
    errs += ai64ssa_data_format(_fileDesc, index, verbose,
                                AI64SSA_DATA_FORMAT_OFF_BIN, nullptr);
    errs += ai64ssa_data_packing(_fileDesc, index, verbose,
                                 AI64SSA_DATA_PACKING_DISABLE, nullptr);
    errs += ai64ssa_irq0_sel(_fileDesc, index, verbose, AI64SSA_IRQ0_INIT_DONE,
                             nullptr);
    errs +=
        ai64ssa_irq1_sel(_fileDesc, index, verbose, AI64SSA_IRQ1_NONE, nullptr);
    errs += ai64ssa_fsamp_ai_compute(_fileDesc, index, verbose, fsamp, &_src_a,
                                     &_src_b, &_nrate_a, &_nrate_b, &_enable_a,
                                     &_enable_b, nullptr);
    errs += ai64ssa_samp_clk_src(_fileDesc, index, verbose, _src_a, nullptr);
    errs += ai64ssa_rbg_clk_src(_fileDesc, index, verbose, _src_b, nullptr);
    errs += ai64ssa_rag_nrate(_fileDesc, index, verbose, _nrate_a, nullptr);
    errs += ai64ssa_rbg_nrate(_fileDesc, index, verbose, _nrate_b, nullptr);
    errs += ai64ssa_rag_enable(_fileDesc, index, verbose, _enable_a, nullptr);
    errs += ai64ssa_rbg_enable(_fileDesc, index, verbose, _enable_b, nullptr);

    errs += ai64ssa_scan_marker(_fileDesc, index, verbose,
                                AI64SSA_SCAN_MARKER_DISABLE, nullptr);
    errs += ai64ssa_scan_marker_set(_fileDesc, index, verbose, 0);
    errs += ai64ssa_fsamp_ai_report_all(_fileDesc, index, verbose, nullptr);
    errs += ai64ssa_auto_calibrate(_fileDesc, index, verbose);
    errs += ai64ssa_ai_buf_clear(_fileDesc, index, verbose);

    gsc_label_level_dec();

    if (verbose == 0)
      printf("%s\n", errs ? "FAIL <---" : "PASS");

    return (errs);
  }

  virtual void run() override {
    while (_running) {
      {
        Synchro sync(&_waitStartMutex);
        _waitStartMonitor.notifyAll();
      }
      gsc_wait_t waitDest = _waitSrc;
      ai64ssa_ioctl(_fileDesc, AI64SSA_IOCTL_WAIT_EVENT, &waitDest);
    }
  }

  void testWait() {
    int deviceIndex = 0;
    int fileDesc = 0;
    int verbose = 0;

    // open board
    ai64ssa_init_util(verbose);
    if (ai64ssa_open_util(deviceIndex, 0, -1, 1, &fileDesc) != 0 ||
        fileDesc < 0) {
      CPPUNIT_FAIL("DevGsc16ai failed to open board");
    }

    _fileDesc = fileDesc;
    LOG(info, "DevGsc16ai successfully opened board at index " << deviceIndex);
    ai64ssa_id_device(fileDesc, -1, verbose);

    CPPUNIT_ASSERT(ai64ssa_chan_active(fileDesc, -1, verbose,
                                       AI64SSA_CHAN_ACTIVE_0_63, nullptr) == 0);

    CPPUNIT_ASSERT(ai64ssa_config_ai5V(fileDesc, -1, verbose, 20000) == 0);

    gsc_wait_t waitSrc;
    gsc_wait_t wait;
    memset(&waitSrc, 0, sizeof(gsc_wait_t));
    waitSrc.main = GSC_WAIT_MAIN_DMA0 | GSC_WAIT_MAIN_DMA1;
    waitSrc.timeout_ms = 15000;
    _waitSrc = waitSrc;

    Synchro syncStart(&_waitStartMutex);
    Synchro syncEnd(&_waitEndMutex);
    _currentThread = new Thread("waitThread", this);
    _currentThread->start();
    double prevValue = 0;
    while (true) {
      syncStart.rWait_s(&_waitStartMonitor, 10);

      wait = waitSrc;

      while (true) {
        int ret = ai64ssa_ioctl(_fileDesc, AI64SSA_IOCTL_WAIT_STATUS, &wait);

        if (ret) {
          CPPUNIT_FAIL("FAIL !!");
        }

        if (wait.count == 0) {
          usleep(1E3);
        } else if (wait.count == 1) {
          double value = readDMA(8);
          if (std::abs((double)(value - prevValue)) > 2) {
            LOG_INFO("Value read.......: %lf", value);
            prevValue = value;
          }
          break;
        } else {
          CPPUNIT_FAIL(String::Format("FAIL count %d", wait.count));
        }
      }
    }
  }

  void testRead() {
    int deviceIndex = 0;
    int fileDesc = 0;
    int verbose = 0;
    int prevValue = 0;
    int fsamp = 20000;

    // open board
    ai64ssa_init_util(verbose);
    if (ai64ssa_open_util(deviceIndex, 0, -1, 1, &fileDesc) != 0 ||
        fileDesc < 0) {
      CPPUNIT_FAIL("DevGsc16ai failed to open board");
    }
    _fileDesc = fileDesc;
    LOG(info, "DevGsc16ai successfully opened board at index " << deviceIndex);

    CPPUNIT_ASSERT(ai64ssa_chan_active(fileDesc, -1, verbose,
                                       AI64SSA_CHAN_ACTIVE_0_15, nullptr) == 0);

    int qty = 0;
    CPPUNIT_ASSERT(
        ai64ssa_query(fileDesc, -1, 0, AI64SSA_QUERY_CHANNEL_QTY, &qty) == 0);
    LOG_INFO("Nombre de channels: %d", qty);

    CPPUNIT_ASSERT(ai64ssa_config_ai5V(fileDesc, -1, verbose, fsamp) == 0);
    int values[128];
    while (true) { // for (uint32_t idx = 0; idx < 2000; idx++) {
      // LOG_INFO("Reda1");
      // LOG_INFO("Read2");

      // attente au moins 2 samples
      // attente pour eviter de bloquer dans read.

      double value = readDMA(8);
      if (std::abs((double)(value - prevValue)) > 2) {
        LOG_INFO("Value read.......: %lf", value);
        prevValue = value;
      }

      continue;

      int length = ai64ssa_read(fileDesc, reinterpret_cast<uint8_t *>(values),
                                sizeof(values));
      LOG_INFO("Reda2");
      if (length < 0) {
        CPPUNIT_ASSERT(ai64ssa_ai_buf_clear(fileDesc, -1, verbose) == 0);
      } else {
        double value =
            getDoubleValueFromInt16(values[8], AI64SSA_AI_RANGE_0_5V);
        if (std::abs((double)(value - prevValue)) > 2) {
          LOG_INFO("Value read.......: %lf", value);
          prevValue = value;
        }
      }
      LOG_INFO("Reda3");
      ai64ssa_ai_buf_clear(fileDesc, -1, verbose);
      LOG_INFO("Reda4");
      usleep(2E6 / (double)fsamp);
    }
  }

  // ..........................................................
  double getDoubleValueFromInt16(int receivedInt, int range) {
    receivedInt &= 0xffff;
    double voltage = 0;
    switch (range) {
    case (AI64SSA_AI_RANGE_0_5V):
      voltage = ((double)receivedInt) * 5.0 / 65535.0;
      break;
    case (AI64SSA_AI_RANGE_0_10V):
      voltage = ((double)receivedInt) * 10.0 / 65535.0;
      break;
    case (AI64SSA_AI_RANGE_2_5V):
      voltage = (((double)receivedInt) * 5.0 / 65535.0) - 2.5;
      break;
    case (AI64SSA_AI_RANGE_5V):
      voltage = (((double)receivedInt) * 10.0 / 65535.0) - 5.0;
      break;
    case (AI64SSA_AI_RANGE_10V):
      voltage = (((double)receivedInt) * 20.0 / 65535.0) - 10.0;
      break;
    default:
      LOG(error, "DevGsc16ai  tried to convert voltage from an unknown voltage "
                 "range.");
      break;
    }
    return voltage;
  }

  double readDMA(int index) {
    int errs = 0;
    s32 fsamp = 20000;
    int ret = 0;

    // errs	+= ai64ssa_query(_fileDesc, -1, 0, AI64SSA_QUERY_CHANNEL_QTY,
    // &qty);

    // LOG_INFO("Read1");
    errs += ai64ssa_initialize(_fileDesc, -1, 0);
    // LOG_INFO("Read1_1");
    errs += ai64ssa_rx_io_mode(_fileDesc, -1, 0, GSC_IO_MODE_BMDMA, nullptr);
    errs += ai64ssa_rx_io_overflow(_fileDesc, -1, 0, AI64SSA_IO_OVERFLOW_IGNORE,
                                   nullptr);
    errs += ai64ssa_rx_io_timeout(_fileDesc, -1, 0, AI64SSA_IO_TIMEOUT_DEFAULT,
                                  nullptr);
    errs += ai64ssa_rx_io_underflow(_fileDesc, -1, 0,
                                    AI64SSA_IO_UNDERFLOW_IGNORE, nullptr);
    errs += ai64ssa_ai_range(_fileDesc, -1, 0, AI64SSA_AI_RANGE_0_5V, nullptr);
    errs += ai64ssa_ai_buf_thr_lvl(_fileDesc, -1, 0, 250000, nullptr);
    errs += ai64ssa_ai_mode(_fileDesc, -1, 0, AI64SSA_AI_MODE_SINGLE, nullptr);
    errs += ai64ssa_burst_size(_fileDesc, -1, 0, 1, nullptr);
    errs +=
        ai64ssa_burst_src(_fileDesc, -1, 0, AI64SSA_BURST_SRC_DISABLE, nullptr);
    errs += ai64ssa_data_packing(_fileDesc, -1, 0, AI64SSA_DATA_PACKING_DISABLE,
                                 nullptr);
    // LOG_INFO("Read1_2");
    // errs	+= ai64ssa_fsamp_ai_compute	(_fileDesc, -1, 0, fsamp,
    // &_src_a, &_src_b, &_nrate_a, &_nrate_b,
    // &_enable_a, &_enable_b, nullptr);
    errs += ai64ssa_samp_clk_src(_fileDesc, -1, 0, _src_a, nullptr);
    errs += ai64ssa_rbg_clk_src(_fileDesc, -1, 0, _src_b, nullptr);
    errs += ai64ssa_rag_nrate(_fileDesc, -1, 0, _nrate_a, nullptr);
    errs += ai64ssa_rbg_nrate(_fileDesc, -1, 0, _nrate_b, nullptr);

    // LOG_INFO("Read1_3");
    errs += ai64ssa_rag_enable(_fileDesc, -1, 0, _enable_a, nullptr);
    errs += ai64ssa_rbg_enable(_fileDesc, -1, 0, _enable_b, nullptr);
    errs += ai64ssa_scan_marker(_fileDesc, -1, 0, AI64SSA_SCAN_MARKER_DISABLE,
                                nullptr);

    // LOG_INFO("Read2");

    // attente au moins 2 samples
    // attente pour eviter de bloquer dans read.
    usleep(2E6 / (double)fsamp);

    ret = ai64ssa_read(_fileDesc, _dmaValues, sizeof(_dmaValues));

    // LOG_INFO("Read3");
    if (ret < 0) {
      errs++;
      CPPUNIT_FAIL(
          String::Format("ERROR: ai64ssa_read() failure, returned %d", ret));
    }

    return getDoubleValueFromInt16(_dmaValues[8], AI64SSA_AI_RANGE_0_5V);
  }
};

s32 TestGsc16Ai::_BoardCount = 0;

CPPUNIT_TEST_SUITE_REGISTRATION(TestGsc16Ai);
} // namespace test
