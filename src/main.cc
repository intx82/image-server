
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>


#include "httplib.h"
#include "endpoints.h"
#include "config.h"


void ensure_upload_dir() {
    if (!std::filesystem::exists(config_t::get_config()->storage_path())) {
        std::filesystem::create_directories(config_t::get_config()->storage_path());
        spdlog::warn("Created storage directory: {}", config_t::get_config()->storage_path());
    }
}

void __start_func__()
{
    spdlog::info("Starting. http://localhost:{}/", config_t::get_config()->http_port());
    ensure_upload_dir();

    httplib::Server srv;
    endpoints_t api(srv);

    if (!config_t::get_config()->use_static().empty()) {
        spdlog::info("Using {} as /img", config_t::get_config()->storage_path());
        spdlog::info("Using {} as /", config_t::get_config()->use_static());

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