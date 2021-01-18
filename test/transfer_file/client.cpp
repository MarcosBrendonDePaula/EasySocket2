#include <iostream>
#include "../../include/easysocket.hpp"
using namespace std;

void percent(long x,long y){
    cout<<x<<" ------- "<<y<<endl;
}

int main(){
    easy::client::TCP::Tcp_Client Client;
    Client.Connect("192.168.1.114",80,2000,IPV4);
    auto res = easy::utilities::tcp_recive_file(Client.sock_data,"recebido.msi",0L,20000L,percent);
    cout<<res<<endl;
    return 0;
}