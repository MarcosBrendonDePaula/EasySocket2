#include <iostream>
#include <fstream>
#include <map>

#include "../../include/easysocket.h"
#include "bodyparser.hpp"
#include "http/json.hpp"
#include "http/Utility.hpp"
// #include "http/WebSocket.hpp"

using namespace std;
using json = nlohmann::json;
using Memi = easy::http::utility::Memi;

string Path = "public";

auto MeMiList = Memi::get();

void HttpFunction(map<string,void *>&Args) {
    auto client = (easy::server::TCP::Assync_Tcp_client*)Args["client"];

    easy::Header::Request req(((easy::MSG_Buffer*)Args["message"])->to_str());
    uint64_t size=0;
    if(req.method=="GET"){
        easy::Header::Response res;
        //req.path.erase(req.path.begin());
        fstream File;
        File.open(Path+req.path,ios::in|ios::ate|ios::binary);
        if(!File.is_open()) {
            res.body =  "  ";
            res.stats = 404;
            res.version = "HTTP/1.1";
            res.Headers["content-length"] = "0";
        }
        else 
        {
            size = File.tellp();
            File.seekp(0,ios::beg);
            char *file = new char[size];
            File.read(file,size);
            string StrFile(file,size);
            delete[] file;
            File.close();
            
            res.Headers["Content-Transfer-Encoding"] = "binary";
            res.Headers["charset"] = "ISO-8859-4";
            res.Headers["content-length"] = to_string(size);
            res.body = StrFile+"  ";
            res.stats = (unsigned int)easy::http::utility::status::ok;
            res.version = "HTTP/2";
            res.Headers["Content-Type"] = MeMiList->ftm(req.path.substr(req.path.find(".")+1));
        }
        cout<<req.path<<" "<<res.Headers["Content-Type"]<<" "<<size<<endl;
        auto buf = easy::MSG_Buffer(res.to_str(),res.to_str().size()+2);
        ((easy::server::TCP::Assync_Tcp_client*)Args["client"])->Send(buf);
    }else 
    if(req.method=="POST") {
        easy::Header::Response res;
        string body = ((json*)Args["body"])->dump();
        cout<<((json*)Args["body"])->dump()<<endl;
        res.Headers["Content-Transfer-Encoding"] = "binary";
        res.Headers["charset"] = "ISO-8859-4";
        res.Headers["content-length"] = to_string(body.size());
        res.body = body+"  ";
        res.stats = (unsigned int)easy::http::utility::status::ok;
        res.version = "HTTP/2";
        res.Headers["Content-Type"] = MeMiList->ftm("json");
        auto buf = easy::MSG_Buffer(res.to_str(),res.to_str().size()+2);
        ((easy::server::TCP::Assync_Tcp_client*)Args["client"])->Send(buf);
    }
    //remove actual buffer with client
    //client->buffers.pop_back();
    //remove Body Parser pointer
    delete ((json*)Args["body"]);
}
easy::Assync_Tcp_Module SimpleHttp(HttpFunction);

int main() {
    easy::server::TCP::Assync_Tcp_Server server;
    // server.use(easy::http::websocket::Module,ONRECEIVE);
    server.use(BodyParser::ModuleJson,ONRECEIVE);
    server.use(SimpleHttp,ONRECEIVE);
    server.Listen(25569,2000000,IPV4,true);

    return 0;
}