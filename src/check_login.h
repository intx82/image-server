#ifndef __CHECK_LOGIN_H__
#define __CHECK_LOGIN_H__

#include <spdlog/spdlog.h>

std::string http_get_with_cookies(const std::string& url, const std::string& cookies);
bool check_login(const std::string& cookies);

#endif
