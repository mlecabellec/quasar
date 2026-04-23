#include <gtest/gtest.h>
#include "quasar/webui/WebUIService.hpp"

using namespace quasar::webui;

TEST(TestWebUI, Creation) {
    WebUIService service("MyWebUI", 8080);
    EXPECT_EQ(service.getName(), "MyWebUI");
}
