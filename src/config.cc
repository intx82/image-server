#include <fstream>
#include "config.h"
#include <jsoncpp/json/json.h>

config_t config_t::_cfg;

config_t::config_t()
{
    refresh();
}

void config_t::refresh()
{
    std::ifstream f("config.json");
    if (f.fail())
    {
        _storage_path = "storage";
        _http_port = 8080;
        _use_static = "./static";
    }
    else
    {
        Json::Value cfg;
        f >> cfg;
        _storage_path = cfg.isMember("storage_path") ? cfg["storage_path"].asString() : "storage";
        _http_port = cfg.isMember("http_port") ? cfg["http_port"].asInt() : 8080;
        _use_static = cfg.isMember("use_static") ? cfg["use_static"].asString() : "";
    }
}

config_t *config_t::get_config()
{
    _cfg.refresh();
    return &_cfg;
}
