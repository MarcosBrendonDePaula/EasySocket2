#include <iostream>
#include "../../include/easysocket.hpp"
using namespace std;


void request(map<string,void*>& args) {
    easy::MSG_Buffer* Buffer = (easy::MSG_Buffer*)args["message"];
    auto client = (easy::server::TCP::Assync_Tcp_client*) args["client"];
    cout<<"Recebido"<<endl<<Buffer->to_string()<<endl;
    string html = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n<title>Teste</title>\n</head>\n<body>\n</body>\n</html>";
    auto msg = easy::MSG_Buffer(html,1500);
    client->Send(msg);
    //delete msg.Buffer;
}
easy::Assync_Tcp_Module req(request);

void response(map<string,void*>& args) {
    easy::MSG_Buffer* Buffer = (easy::MSG_Buffer*)args["message"];
    cout<<"Enviado"<<endl<<Buffer->to_string()<<endl;
}
easy::Assync_Tcp_Module res(response);


int main() {
    easy::server::TCP::Assync_Tcp_Server HttpServer;
    
    HttpServer.Listen(80,100000,IPV4);
    HttpServer.use(req,ONRECEIVE);
    HttpServer.use(res,ONSEND);

    int x;
    cin>>x;

    return 0;
}
