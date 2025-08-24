#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <string>
#include <cstdint>


/**
 * @brief Configuration singletone
*/
class config_t
{
    public:
        /**
         * @brief Config accessor
        */
        static config_t* get_config();
        
        /**
         * @brief returns DB filename
        */
        const std::string& storage_path() const {
            return _storage_path;
        }

        /**
         * @brief HTTP server port
        */
        uint16_t http_port() const {
            return _http_port;
        }

        /**
         * @brief Serve static files or not
         */
        const std::string& use_static() const {
            return _use_static;
        }

        const std::string& auth_url() const {
            return _auth_url;
        }
        
        /**
         * @brief Fetches data from config file
        */
        void refresh();
    private:
        config_t();
        std::string _storage_path;
        uint16_t    _http_port;
        std::string _use_static;
        std::string _auth_url;

        static config_t _cfg;

};

#endif
