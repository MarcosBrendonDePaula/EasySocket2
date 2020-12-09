#pragma once
#include "../easysocket.hpp"
#include <iostream>
namespace http {
    class Header {
    public:
        std::string path;
        std::string method;
        std::string HttpVersion;
        std::string host;
        std::string User_Agent;
        std::string Accept;
        std::string Accept_Language;
        std::string Accept_Encoding;
        std::string Accept_Charset;
        u_int Keep_Alive;
        std::string Connection;

        Header();
    };

    class Request {
    public:
       Header header;
       Request(std::string methd,std::string url);
    private:
        
    };

}


namespace http {
    Request::Request(std::string methd,std::string url) {

    }
}