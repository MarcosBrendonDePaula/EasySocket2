#include "../include/easysocket.hpp"
#include "../app/modules/cifra_de_cesar.hpp"
#include <iostream>

using namespace std;
using namespace easy;

namespace receber {
    void main(map<string,void*>& args) {
        client::Assync_Tcp_client *Client = (client::Assync_Tcp_client *)args["client"];
        cout<<Client->buffers.front().to_string()<<endl;
        Client->buffers.erase(Client->buffers.begin());
    }
    Assync_Tcp_Module module(main);
}

int main() {
    std::any ok = 1;
    cout<< ok.type() <<endl;
    client::Assync_Tcp_client client;
    client.Connect("127.0.0.1",25565,1500,IPV4);
    client.use(cifra_cesar_client::module_decriptar,ONRECEIVE);
    client.use(receber::module,ONRECEIVE);

    client.use(cifra_cesar_client::module_encriptar,ONSEND);
    
    while(true) {
        string input;
        cin>>input;
        client.Send(Tcp_client_Buffer(input));
        
    }
    return 0;
}