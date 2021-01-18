#include "../../include/easysocket.hpp"
#include <iostream>
#include <vector>
#include <map>

using namespace std;
using namespace easy;

int main () {
    int count = 1;
    map<int,Basic_Socket> client_sock;
    map<int,int> clients;
    server::UDP::UDP_Server server;
    server.Listen(2000);
    server.max_buffer=1500;
    while(true) {
        MSG_Buffer buffer(1500);
        Basic_Socket client = server.RecvData(buffer);
        if(clients[client.address.sin_port]!=0) {
            cout<<"Client:"<<clients[client.address.sin_port]<<endl;
        } else {
            clients[client.address.sin_port] = count;
            client_sock[count] = client;  
            count++;
            
            cout<<"new client"<<endl;
        }
        for(auto i:clients) {
            server.SendTo(client_sock[i.second],buffer);
        }
    }
    return 0;
}

