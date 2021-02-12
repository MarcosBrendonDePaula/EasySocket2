#ifndef INCLUDE_EASY_HTTP_WEBSOCKET
#define INCLUDE_EASY_HTTP_WEBSOCKET
#pragma once

#include <iostream>
#include <map>
#include "../../../include/easysocket.h"
#include "Header.hpp"

#include "sha1/sha1.hpp"
#include "base64/base64.hpp"

namespace easy::http::websocket {
    void websocketHandshake(std::map<std::string,void *>&Args) {
        auto client = (easy::server::TCP::Assync_Tcp_client*)Args["client"];
        easy::Header::Request req(((easy::MSG_Buffer*)Args["message"])->to_str());
        bool *jmp = (bool*)Args["exit"];
        std::cout<<req.to_str()<<std::endl;
        if(!req.Header["Sec-WebSocket-Key"].empty()) {
            easy::Header::Response res;
            res.version = "HTTP/2";

            *jmp = true;
            std::string key = req.Header["Sec-WebSocket-Key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

            unsigned char message_digest[20];
            websocketpp::sha1::calc(key.c_str(),key.length(),message_digest);
            key = websocketpp::base64_encode(message_digest,20);

            
            res.stats = 101;
            res.Headers["Sec-WebSocket-Accept"] = key;
            std::cout<<res.Headers["Sec-WebSocket-Accept"]<<std::endl;
            res.Headers["Upgrade"] = "websocket";
            res.Headers["Connection"] = "Upgrade";
            res.body.clear();
            auto buf = easy::MSG_Buffer(res.to_str()+"  ",res.to_str().size()+2);
            ((easy::server::TCP::Assync_Tcp_client*)Args["client"])->Send(buf);
        }

    }
    easy::Assync_Tcp_Module Module(websocketHandshake);
}






#endif