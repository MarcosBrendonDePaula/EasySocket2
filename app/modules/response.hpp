#include "../../include/easysocket.hpp"
#include <map>

using namespace easy;
using namespace easy::server;

namespace response_module_server {

    void main(map<string,void*>& inputs) {
        cout<<((Assync_Tcp_client*)inputs["client"])->buffers.back().to_string()<<endl;
    }
    
    easy::Assync_Tcp_Module module(main);
}


