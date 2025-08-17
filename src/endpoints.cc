
#include "endpoints.h"

#include <json/json.h>

#include <fstream>
#include <spdlog/spdlog.h>

#include "base64.h"
#include "sha256.h"
#include "check_login.h"

std::string endpoints_t::detect_mime_type(const char *data, size_t size)
{
    if (magic_load(_magic, nullptr) != 0) {
        throw std::runtime_error("Failed to load magic database");
    }

    const char *mime = magic_buffer(_magic, data, size);
    std::string result = mime ? mime : "";

    return result;
}

std::string endpoints_t::trim(const std::string &str, const std::string &whitespace)
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) {
        return "";
    }

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::vector<std::string> endpoints_t::split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void endpoints_t::on_ping(const httplib::Request &, httplib::Response &res)
{
    spdlog::info("Ping request");
    res.set_content("\"pong\"", "application/json");
}

void endpoints_t::on_upload(const httplib::Request &req, httplib::Response &res)
{
    Json::Value result;
    Json::Value file_list(Json::arrayValue);
    std::string cookie = req.get_header_value("Cookie");
    spdlog::info("Request on-upload with cookie: {}", cookie);

    if (!check_login(cookie)) {
        result["status"] = 1;
        result["message"] = "Not authorized";
        res.status = 401;
    } else if (!req.has_file("file")) {
        result["status"] = 1;
        result["message"] = "No file uploaded";
        res.status = 400;
    } else {
        for (const auto &[fieldname, file] : req.files) {
            std::string mime = detect_mime_type(file.content.data(), file.content.size());
            std::vector<std::string> mime_parts = split(mime, '/');

            if (mime_parts.size() < 2) {
                Json::Value file_info;
                spdlog::warn("File upload error, Unknown type; {} {}", file.filename, mime);

                file_info["name"] = file.filename;
                file_info["status"] = 1;
                file_info["error"] = fmt::format("File is not image; Unknown type; ({})", mime);
                file_list.append(file_info);
                continue;
            }

            if (mime_parts[0] == "image") {
                sha256_t hasher;
                hasher.update(reinterpret_cast<const uint8_t *>(file.content.data()), file.content.size());
                std::string hash_str = sha256_t::toString(hasher.digest());

                std::string target_path = config_t::get_config()->storage_path() + "/" + hash_str + "." + mime_parts[1];
                std::ofstream out(target_path, std::ios::binary);
                out.write(file.content.data(), file.content.size());

                // Add to result
                Json::Value file_info;
                file_info["name"] = file.filename;
                file_info["sha256"] = hash_str;
                file_info["status"] = 0;
                file_info["link"] = hash_str + "." + mime_parts[1];
                file_list.append(file_info);
                spdlog::info("Uploaded: {} {}", file.filename, mime);
            } else {
                Json::Value file_info;
                file_info["name"] = file.filename;
                file_info["status"] = 1;
                file_info["error"] = fmt::format("File is not image ({})", mime);
                file_list.append(file_info);
                spdlog::warn("File upload error: {} {}", file.filename, mime);
            }
        }

        result["status"] = 0;
        result["files"] = file_list;
    }

    Json::StreamWriterBuilder builder;
    res.set_content(Json::writeString(builder, result), "application/json");
}
