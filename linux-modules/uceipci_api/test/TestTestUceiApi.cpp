#include <cppunit/extensions/HelperMacros.h>
#include <unistd.h>
#include <fstream>
//#include "Coupler.h"
#include "BCcoupler.h"
#include "RTcoupler.h"

//#include "drast/uceipci_api/TestUceiApi.h"

namespace test {
// using namespace drast::uceipci_api;
using namespace std;

// ----------------------------------------------------------
// test fixture implementation
class TestTestUceiApi : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TestTestUceiApi);
    // TODO for each test method:
    CPPUNIT_CONDITIONAL_TEST(BusTools_FindDevice(RPCIe1553, 1) != -1, testBoardMgMt);
    CPPUNIT_CONDITIONAL_TEST(BusTools_FindDevice(RPCIe1553, 1) != -1, testSimpleDialog);
    CPPUNIT_CONDITIONAL_TEST(BusTools_FindDevice(RPCIe1553, 1) != -1, testErrors);
    CPPUNIT_TEST_SUITE_END();

private:
public:
    void setUp() {}

    void tearDown() {}
    //@warning  : this test assumes we're using the XOVG12 DUALFunction board with 4 couplers embedded, with IRIG
    void testBoardMgMt() {
        Coupler* cpl = new Coupler(4, false, false);
        CPPUNIT_ASSERT(cpl->handler() == Coupler::INVALID_HANDLER);
        delete cpl;

        Coupler* cpls[4];

        cpls[0] = new Coupler(3, true, false);
        CPPUNIT_ASSERT(cpls[0]->handler() == 0);

        cpls[1] = new Coupler(2, true, true);
        CPPUNIT_ASSERT(cpls[1]->handler() == 1);

        cpls[2] = new Coupler(1, true, true);
        CPPUNIT_ASSERT(cpls[2]->handler() == 2);

        cpls[3] = new Coupler(0, true, true);
        CPPUNIT_ASSERT(cpls[3]->handler() == 3);

        // Only one open at a time
        cpl = new Coupler(3, true, true);
        CPPUNIT_ASSERT(cpl->handler() == Coupler::INVALID_HANDLER);

        delete cpl;
        for (int i = 0; i < 4; i++)
            delete cpls[i];

        cpl = new Coupler(3, false, false);

        // Display board and API info
        CPPUNIT_ASSERT(cpl->boardHasIrig());

        CPPUNIT_ASSERT(not cpl->boardIsMultiFunction());
        CPPUNIT_ASSERT(cpl->boardIsDualFunction());

        CPPUNIT_ASSERT(cpl->numOfBuses() == 4);

        cerr << "SN# " << cpl->serialNumber() << endl;

        CPPUNIT_ASSERT(cpl->apiVersion() == 816);
    }

    void testSimpleDialog() {
        RTcoupler rtl(0);
        BCcoupler bfm(2, 1000000);

        CPPUNIT_ASSERT(rtl.start() == 0);
        CPPUNIT_ASSERT(bfm.start() == 0);

        usleep(700000);
        CPPUNIT_ASSERT(bfm.stop() == 0);
        CPPUNIT_ASSERT(rtl.stop() == 0);

        CPPUNIT_ASSERT(bfm.msgCount[0][1][1] > 0);
        CPPUNIT_ASSERT(bfm.msgCount[1][1][2] > 0);
        CPPUNIT_ASSERT(rtl.msgCount[0][1][1] > 0);
        CPPUNIT_ASSERT(rtl.msgCount[1][1][2] > 0);
    }

    void testErrors() {
        RtErrCoupler rtl(0);
        BcErrCoupler bfm(2, 10000);
        rtl.start();
        bfm.start();
        usleep(600000);
        CPPUNIT_ASSERT(bfm.nbNoresponse == 26);
        CPPUNIT_ASSERT(bfm.nbBusyMsgs == 5);
        CPPUNIT_ASSERT(rtl.msgCount[1][1][2] - rtl.msgCount[0][1][1] == 27);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTestUceiApi);
}  // namespace test
