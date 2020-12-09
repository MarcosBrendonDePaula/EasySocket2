#include "../../include/easysocket.hpp"

namespace game{
    #include <map>
    using namespace easy;
    namespace config {
        map<Assync_Tcp_client,bool> players;
    }

    void response(map<string,void*>& inputs) {
        Assync_Tcp_client *client = (Assync_Tcp_client*)inputs["client"];
        Tcp_client_Buffer *buffer = client->Buffers.back();
    }

    Assync_Tcp_Module module_encriptar();
    Assync_Tcp_Module module_decriptar(main_decriptar);
}

