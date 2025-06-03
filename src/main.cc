
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/utsname.h>
#include <spdlog/spdlog.h>

#include "httplib.h"
#include "endpoints.h"
#include "config.h"


void __start_func__()
{
    spdlog::info("Starting. http://localhost:{}/", config_t::get_config()->http_port());

    struct utsname _hostname;
    uname(&_hostname);

    httplib::Server srv;
    endpoints_t api(srv);

    if (!config_t::get_config()->use_static().empty()) {
        srv.set_mount_point("/img", config_t::get_config()->storage_path());
        srv.set_mount_point("/", config_t::get_config()->use_static());
    }

    srv.listen("0.0.0.0", config_t::get_config()->http_port());
}

#ifdef TEST_BUILD
#include "gtest/gtest.h"

GTEST_API_ int main(int argc, char **argv)
{
    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#else
int main()
{
    __start_func__();
    return 0;
}
#endif