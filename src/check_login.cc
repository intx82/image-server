#include "check_login.h"
#include "config.h"

#include <iostream>
#include <jsoncpp/json/json.h>
#include <regex>
#include <string>
#include <optional>

#include "httplib.h"

struct parsed_url_t {
    std::string scheme;
    std::string host;
    int port;
    std::string target;
};

static std::optional<parsed_url_t> parse_url(const std::string& url)
{
    // scheme://host[:port][/path][?query][#fragment]
    static const std::regex re(R"(^([Hh][Tt][Tt][Pp][Ss]?)://([^/:?#]+)(?::(\d+))?([^?#]*)?(\?[^#]*)?(#.*)?$)");
    std::smatch m;
    if (!std::regex_match(url, m, re))
        return std::nullopt;

    parsed_url_t out;
    out.scheme = m[1].str();
    out.host = m[2].str();
    out.port = m[3].matched ? std::stoi(m[3].str()) : ((out.scheme == "https" || out.scheme == "HTTPS") ? 443 : 80);

    std::string path = m[4].matched ? m[4].str() : "/";
    if (path.empty())
        path = "/";
    std::string query = m[5].matched ? m[5].str() : "";
    out.target = path + query;
    return out;
}

static void setup_common(httplib::Client& cli)
{
    cli.set_follow_location(true); // follow 30x
    cli.set_connection_timeout(10, 0);
    cli.set_read_timeout(20, 0);
    cli.set_write_timeout(20, 0);
}

std::string http_get_with_cookies(const std::string& url, const std::string& cookies)
{
    auto parsed_opt = parse_url(url);
    if (!parsed_opt) {
        return "";
    }
    const auto& u = *parsed_opt;

    httplib::Headers headers = { { "Cookie", cookies } };

    if (u.scheme == "http" || u.scheme == "HTTP") {
        httplib::Client cli(u.host.c_str(), u.port);
        setup_common(cli);
        auto res = cli.Get(u.target.c_str(), headers);

        if (res && (res->status >= 200 && res->status < 300)) {
            return res->body;
        }
    }
    return "";
}

bool check_login(const std::string& cookies)
{
    Json::Reader reader;
    Json::Value root;
    const std::string url = config_t::get_config()->auth_url();
    const std::string ret = http_get_with_cookies(url, cookies);
    bool done = reader.parse(ret, root);
    if (!done) {
        spdlog::warn("Can't parse response for {} -> {}", url, ret);
        return false;
    }

    if (!root.isMember("status") || !root["status"].isBool()) {
        spdlog::warn("Response doesn't contains 'status' or wrong have a wrong type", ret);
        return false;
    }

    spdlog::info("Checklogin: {}", root["status"].asBool());
    return root["status"].asBool();
}

