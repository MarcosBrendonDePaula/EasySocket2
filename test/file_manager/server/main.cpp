#include <iostream>
#include <fstream>
#include <map>

#include "../../../include/easysocket.h"
#include <windows.h>

#include "models/folder.hpp"


using namespace std;

map<string,Folder> folders;

void loadFolders() {
    fstream folderList("folders/folders.list",ios::in);
    string linha;
    while(getline(folderList,linha)) {
        folders[linha] = Folder("folders/"+linha);
    }
}

void addFolder(string folder) {
    cout<<("mkdir folders/"+ folder)<<endl<<("type NUL > folders/"+folder+"/files.list")<<endl;
    CreateDirectory(("folders/"+ folder).c_str(),NULL);

    HANDLE h = CreateFile(
                ("folders/"+folder+"/files.list").c_str(),
                GENERIC_WRITE, // open for writing
                0,             // sharing mode, none in this case
                0,             // use default security descriptor
                CREATE_ALWAYS, // overwrite if exists
                FILE_ATTRIBUTE_NORMAL,
                0);
    CloseHandle(h);

    fstream folderList("folders/folders.list",ios::out|ios::app);
    folderList<<folder<<"\n";
    folderList.close();
    folders[folder] = Folder("folders/"+folder);
    return;
}

void Response(map<string,void*>& args) {
    auto Client = (easy::server::TCP::Assync_Tcp_client*)args["client"];
    auto MSG = (easy::MSG_Buffer*)args["message"];
    
    std::cout<<"Resposta: "<<MSG->to_string()<<std::endl;
    
    if(MSG->to_string() == "SendFile") {
        string folder = Client->recv_buffer().to_string();
        if(folders.find(folder)!=folders.end()) {
            Client->Send(easy::MSG_Buffer("1"));
            string file_name = Client->recv_buffer().to_string();
            cout<<file_name<<endl;
            if(folders[folder].file_exist(file_name)) {
                Client->Send(easy::MSG_Buffer("2"));
                easy::utilities::tcp_send_file(Client->addr,folders[folder].getFilePath(file_name));
                return;
            } else {
                Client->Send(easy::MSG_Buffer("-2"));
            }
        } else {
            Client->Send(easy::MSG_Buffer("-1"));
        }
    }else
    if(MSG->to_string() == "RecvFile") {
        auto folder = Client->recv_buffer().to_string();
        if(folders.find(folder) == folders.end()) {
            addFolder(folder);
        }

        string file_name = Client->recv_buffer().to_string();
        easy::utilities::tcp_recive_file(Client->addr,folders[folder].getFilePath(file_name));
        folders[folder].reg_file(file_name);
    }else
    if(MSG->to_string() == "ListarPastas") {
        std::string size = to_string(folders.size());
        Client->Send(easy::MSG_Buffer(size));
        for(auto it:folders) {
            Client->Send(easy::MSG_Buffer(it.first));
        }
    }else
    if(MSG->to_string() == "ListarArquivos") {
        string pasta = Client->recv_buffer().to_string();
        if(folders.find(pasta) != folders.end()) {
            Client->Send(easy::MSG_Buffer(to_string(folders[pasta].getFiles().size())));
            for(auto it:folders[pasta].getFiles()) {
                Client->Send(easy::MSG_Buffer(it));
            }
        }else
        Client->Send(easy::MSG_Buffer(to_string(0)));
    }
    
}

int main(int argc, char *argv[]) {
    loadFolders();
    cout<<argv[0]<<endl;
    easy::Assync_Tcp_Module Response_Module(Response);
    easy::server::TCP::Assync_Tcp_Server Server;
    Server.Listen(3001,1500);
    Server.use(Response_Module,ONRECEIVE);
    int x;
    cin>>x;
    return 0;
}