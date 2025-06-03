#ifndef __ENDPOINTS_H__
#define __ENDPOINTS_H__

#include <magic.h>
#include <spdlog/spdlog.h>

#include "config.h"
#include "httplib.h"

/**
 * @brief HTTP endpoints class
 */
class endpoints_t
{
   private:
    template <void (endpoints_t::*cb)(const httplib::Request &,
                                      httplib::Response &)>
    struct _http_cb_t {
        _http_cb_t(endpoints_t *parrent) : _parrent(parrent) {}
        void operator()(const httplib::Request &req, httplib::Response &res)
        {
            if (_parrent != nullptr) {
                (_parrent->*cb)(req, res);
            }
        }

       private:
        endpoints_t *_parrent;
    };

    /**
     * @brief Split string
     */
    std::vector<std::string> split(const std::string &s, char delimiter);

    /**
     * @brief Trim leading whitespaces
     */
    std::string trim(const std::string &str, const std::string &whitespace = " \t");

   public:
    /**
     * @brief ctor
     * @param srv HTTP Server (@see httplib)
     * @param db DB connection
     */
    endpoints_t(httplib::Server &srv)
        : _srv(srv)
    {
        _srv.Get("/ping", _http_cb_t<&endpoints_t::on_ping>(this));
        _srv.Post("/upload", _http_cb_t<&endpoints_t::on_upload>(this));
        _magic = magic_open(MAGIC_MIME_TYPE);
        if (!_magic) {
            throw std::runtime_error("Failed to initialize libmagic");
        }
        magic_load(_magic, nullptr);
    }

    /**
     * @brief Handle '/ping' GET request
     * @param req Request
     * @param res Response
     */
    void on_ping(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief Handle '/upload' POST request
     * @param req Request
     * @param res Response
     */
    void on_upload(const httplib::Request &req, httplib::Response &res);

    /**
     * @brief Detect mime type
     * @param data Incoming data;
     * @param size Incoming data size
     * @return Mime type
     */
    std::string detect_mime_type(const char *data, size_t size);

    ~endpoints_t()
    {
        if (_magic) {
            magic_close(_magic);
        }
    }

   private:
    httplib::Server &_srv;
    magic_t _magic;
};

#endif