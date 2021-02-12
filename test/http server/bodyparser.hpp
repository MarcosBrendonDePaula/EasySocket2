#include "../../include/easysocket.h"
#include "http/Header.hpp"
#include "http/json.hpp"

#include <iostream>
#include <map>
#include <regex>

namespace BodyParser {
    using string = std::string;
    using json = nlohmann::json;

    void function(std::map<string,void*>& Args) {
        easy::Header::Request req(((easy::MSG_Buffer*)Args["message"])->to_str());

        std::regex varsregex("[^&]+");
        std::sregex_iterator vars(req.body.begin(), req.body.end(), varsregex),rxend;
        auto body = new json();

        while(vars != rxend) {
            string vA = vars->str();
            std::regex varf("[^=]+");
            std::sregex_iterator var(vA.begin(),vA.end(),varf);
            string name = var->str(),value;
            var++;
            if(var->str()[0]=='[' || var->str()[0]=='{') {
                (*body)[name] = json::parse(var->str());
            }else
                (*body)[name] = var->str();
            vars++;
        }
        Args["body"] = body;
    }
    easy::Assync_Tcp_Module ModuleJson(function);
}