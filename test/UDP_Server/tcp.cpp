#include "../../include/easysocket.hpp"
#include <iostream>
#include <vector>
#include <map>

using namespace std;
using namespace easy;

void request(map<string,void*>& args) {
    MSG_Buffer *msg = (MSG_Buffer *)args["message"];
    cout<<msg->Buffer<<endl;
}

int main () {
    server::TCP::Assync_Tcp_Server servidor;
    Assync_Tcp_Module modulo(request);
    servidor.Listen();
    servidor.use(modulo,ONRECEIVE);
    int x;
    cin>>x;


    return 0;
}