#include "../../include/easysocket.hpp"

namespace cifra_cesar_server{
    using namespace easy;
    using namespace easy::server;
    using namespace std;
    int cifracesar_deslizamento = 1;
    
    void main_encriptar(map<string,void*>& inputs) {
        Tcp_client_Buffer *data = (Tcp_client_Buffer*)inputs["message"];
        for(int i=0;i<data->actual_buffer;i++) {
            data->Buffer[i]+=cifracesar_deslizamento;
        }
    }
    
    void main_decriptar(map<string,void*>& inputs) {
        Assync_Tcp_client *Client = (Assync_Tcp_client*)inputs["client"];
        char *Buffer = Client->buffers.back().Buffer;
        int actual = Client->buffers.back().actual_buffer;
        cout<<Buffer<<endl;
        for(int i=0;i<actual;i++) {
            Buffer[i]-=cifracesar_deslizamento;
            
        }
    }

    Assync_Tcp_Module module_encriptar(main_encriptar);
    Assync_Tcp_Module module_decriptar(main_decriptar);
}

namespace cifra_cesar_client{
    using namespace easy;
    using namespace easy::client;
    using namespace std;
    int cifracesar_deslizamento = 1;
    
    void main_encriptar(map<string,void*>& inputs) {
        Tcp_client_Buffer *data = (Tcp_client_Buffer*)inputs["message"];
        for(int i=0;i<data->actual_buffer;i++) {
            data->Buffer[i]+=cifracesar_deslizamento;
        }
    }
    
    void main_decriptar(map<string,void*>& inputs) {
        Assync_Tcp_client *Client = (Assync_Tcp_client*)inputs["client"];
        char *Buffer = Client->buffers.back().Buffer;
        int actual = Client->buffers.back().actual_buffer;
        for(int i=0;i<actual;i++) {
            Buffer[i]-=cifracesar_deslizamento;
        }
    }

    Assync_Tcp_Module module_encriptar(main_encriptar);
    Assync_Tcp_Module module_decriptar(main_decriptar);
}

