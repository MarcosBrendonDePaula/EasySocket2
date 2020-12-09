#include <iostream>
#include "../include/easysocket.hpp"
#include "./modules/response.hpp"

using namespace std;
using namespace easy;
//using namespace easy::server;

void response(map<string,void*>& inputs) {
    client::Assync_Tcp_client* client = ((client::Assync_Tcp_client*)inputs["client"]);
    cout<<client->buffers.front().to_string()<<endl;
    client->buffers.erase(client->buffers.begin());
}

Assync_Tcp_Module response_module(response);

int main() {
    server::Assync_Tcp_Server s1;
    s1.Listen(25565,1500,IPV4);
    //s1.use(cifra_cesar::module_encriptar,ONSEND);
    //s1.use(cifra_cesar::module_decriptar,ONRECEIVE);
    s1.use(response_module_server::module,ONRECEIVE);

    easy::client::Assync_Tcp_client novo;
    novo.use(response_module,ONRECEIVE);
    cout<<novo.Connect("127.0.0.1",25565,1500,IPV4)<<endl;
    while(true) {
        string msg;
        cin>>msg;
        cout<<novo.Send(Tcp_client_Buffer(msg,1500))<<endl;
    }

    return 0;
}