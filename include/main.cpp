#include "easysocket.hpp"
#include <iostream>
using namespace std;



int main() {

    // easy::server::UDP::UDP_Server server;
    // server.Listen(25565);

    // server.max_buffer = 1500;
    // cout<<easy::Win_init<<endl;
    
    // while(1) {

    //     easy::MSG_Buffer buffer(1500);
    //     auto nova = server.RecvData(buffer);
    //     server.SendTo(nova,buffer);
    // }

    easy::client::UDP::UDP_Client client;
    client.prepare("127.0.0.1",25565,1500,IPV4);

    while(true) {
        easy::MSG_Buffer buffer("eae mundo");
        easy::MSG_Buffer buffer2("recebido");
        client.SendTo(client.getServer(),buffer);
        auto r=client.RecvData(buffer);
        client.SendTo(r,buffer2);
    }
    
    
    

    return 0;
}