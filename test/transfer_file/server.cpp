#include <iostream>
#include "../../include/easysocket.hpp"
using namespace std;



int main(){
    easy::server::TCP::Tcp_Server Server;
    Server.Listen(80,200,IPV4);
    
    while(true) {
        auto con = Server.accept_connection();
        auto size = easy::utilities::tcp_send_file(con.sock_data,"arq_env.msi",0L,20000L);
        cout<<size<<endl;
    }

    return 0;
}