#ifndef INCLUDE_EASY_HTTP_HEADER
#define INCLUDE_EASY_HTTP_HEADER
#pragma once

#include <map>
#include <iostream>
#include <vector>
#include <regex>

#include "Utility.hpp"

namespace easy::Header {
    using string = std::string;

    class Response {
        public:
        string version;
        unsigned stats;
        string body;
        std::map<string,string> Headers;

        Response();
        string to_str();
        private:
    };

    Response::Response() {
        body = "";
        version = "HTTP/1.1";
        stats = 0;
    }
    
    string Response::to_str() {
        string header = (version+" "+std::to_string(stats)+"\r\n");
        
        for(auto i:Headers) {
            header += (i.first+": "+i.second+"\r\n");
        }

        header +="\r\n";
        header += body;
        return header;
    }

    class Request{
        public:
            Request(string req);
            string to_str();
            string path;
            string method;
            string version;
            string body;
            std::map<string,string> Header;
    };

    Request::Request(string req="") {
        
        if(req.empty()){
            return;
        }

        std::vector<string> Lines;
        std::regex rx("[^\r\n]+\r\n");

        std::sregex_iterator FormatedFileList(req.begin(), req.end(), rx), rxend;
        while(FormatedFileList != rxend)
        {
            Lines.push_back(FormatedFileList->str().c_str());
            ++FormatedFileList;
            
        }

        std::regex fline("[^ ]+");
        std::sregex_iterator FormatedL1(Lines[0].begin(), Lines[0].end(), fline);
        method = FormatedL1->str();
        FormatedL1++;
        path =(easy::http::utility::IsUrlEncoded(FormatedL1->str()))?easy::http::utility::UrlDecode(FormatedL1->str()):FormatedL1->str();
        FormatedL1++;
        version = FormatedL1->str();

        //read headers
        std::regex headerex("(\\S+)");
        for(int i=1;i<Lines.size();i++) {
            std::sregex_iterator headerf(Lines[i].begin(), Lines[i].end(), headerex);
            int pos=0;
            string headerName;
            string headerValue="";
            while(headerf != rxend) {
                if(!pos) {
                    headerName = headerf->str();
                    headerName.pop_back();
                } else {
                    headerValue+=headerf->str()+" ";
                }
                headerf++;
                pos++;
            }

            if(headerValue.back()==' ')
                headerValue.pop_back();
            
            Header[headerName]=headerValue;
        }

        if(Header.find("Content-Length")!=Header.end()) {
            body = (easy::http::utility::IsUrlEncoded(req.substr(req.find("\r\n\r\n")+4)))?easy::http::utility::UrlDecode(req.substr(req.find("\r\n\r\n")+4)):req.substr(req.find("\r\n\r\n")+4);
        } else {
            body = "";
        }
    }

    string Request::to_str() {
        string strheader = "";
        strheader+=(method+" "+path+" "+version);
        for(auto hd:Header) {
            strheader+=(hd.first+":"+hd.second+"\r\n");
        }
        return strheader;
    }
}
#endif