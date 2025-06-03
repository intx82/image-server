#ifdef TEST_BUILD

#include <fstream>
#include <cstdio>
#include "gtest/gtest.h"
#include "fmt/format.h"
#include "config.h"

TEST(config_tests, AllInOne)
{
    std::remove("config.json");

    config_t *cfg1 = config_t::get_config();
    ASSERT_TRUE(cfg1);
    config_t *cfg2 = config_t::get_config();
    ASSERT_TRUE(cfg1 == cfg2);
    cfg2->refresh();
    ASSERT_TRUE(cfg1->session_timeout() == 1800);
    ASSERT_TRUE(cfg1->http_port() == 8080);
    ASSERT_TRUE(cfg1->db_filename() == "db.sqlite");
    ASSERT_TRUE(cfg1->login_redirect() == "/activity");

    std::ofstream test_cfg("config.json");
    test_cfg << "{ \"session_timeout\": 3600, \
        \"db_filename\": \"test.sqlite\",\
        \"http_port\": 8081, \
        \"login_redirect\": \"/user_page\", \
        }";

    test_cfg.flush();
    cfg1->refresh();
    ASSERT_TRUE(cfg1->session_timeout() == 3600);
    ASSERT_TRUE(cfg1->db_filename() == "test.sqlite");
    ASSERT_TRUE(cfg1->http_port() == 8081);
    ASSERT_TRUE(cfg1->login_redirect() == "/user_page");

    std::remove("config.json");
}

#endif