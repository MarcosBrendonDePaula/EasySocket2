#include "../../../include/easysocket.hpp"
#include <iostream>
#include <iomanip>
#include <string>
using namespace std;


#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


int tick=0;

void percent(unsigned  long x,unsigned long y) {
    tick++;
    if(tick==3000 || x==y){
        tick=0;
        cout<<x<<"--------"<<y<<endl;
    }
}

bool *connected;
vector<string> erros;
vector<string> success;

void menu() {
    for(auto i:erros) {
        cout<<BOLDRED;
        cout<<"AVISO:"<<i<<endl;
    }
    for(auto s:success){
        cout<<GREEN;
        cout<<"SUCESSO:"<<s<<endl;
    }
    
    erros.clear();
    success.clear();

    if((*connected)) {
        cout<<GREEN;
        cout<<"CONECTADO!"<<endl;    
    }else {
        cout<<BOLDRED;
        cout<<"DISCONECTADO!"<<endl;
    }

    cout<<WHITE;
    cout<<"====================MENU====================" << endl;
    cout<<"---"<<setw(20)<<"enviar"<<setw(21)<<"---"<<endl;
    cout<<"---"<<setw(20)<<"receber"<<setw(21)<<"---"<<endl;
    cout<<"---"<<setw(20)<<"lpastas"<<setw(21)<<"---"<<endl;
    cout<<"---"<<setw(20)<<"larquivos"<<setw(21)<<"---"<<endl;
    if(!(*connected)){
        cout<<"---"<<setw(20)<<"reconectar"<<setw(21)<<"---"<<endl;
    }
    cout << "====================MENU====================" << endl;
}

int main() {
    cout<<"Informe o Ip para conexao"<<endl;
    string ip;
    cin>>ip;
    
    easy::client::TCP::Tcp_Client Client;
    connected = &Client.connected;
    Client.Connect(ip,25567,1500,IPV4);
    string comando;
    menu();
    do{
        system("cls");
        menu();
        cin>>comando;
        if(comando=="sair")
            break;
        if(comando=="enviar") {
            
            Client.send_buffer(easy::MSG_Buffer("RecvFile"));
            
            cout<<"Em qual pasta voce quer que o servidor guarde o arquivo (OBS:Nao permitido espaco no nome)"<<endl;
            string pasta;
            cin>>pasta;
            cin.ignore();
            
            Client.send_buffer(easy::MSG_Buffer(pasta));
            
            cout<<"Informe o nome do arquivo a ser transferido com seu formato"<<endl;
            string arq_name;
            getline(cin,arq_name);
            
            Client.send_buffer(easy::MSG_Buffer(arq_name));

            fstream file(arq_name,ios::in);
            if(!file.is_open()){
                erros.push_back("Acao cancelada arquivo nao existe no diretorio onde se encontra o executavel");
                Client.send_buffer(easy::MSG_Buffer("-1"));
                file.close();
            } else {
                file.close();
                Client.send_buffer(easy::MSG_Buffer("0"));
                auto res = Client.recv_buffer();
                easy::utilities::tcp_send_file(Client.sock_data,arq_name,0L,10000UL,percent);
                success.push_back("Arquivo Enviado!");
            }
        }
        if(comando=="receber") {
            Client.send_buffer(easy::MSG_Buffer("SendFile"));

            cout<<"Em qual pasta o arquivo se encontra "<<endl;
            string pasta;
            cin>>pasta;

            cin.ignore();
            Client.send_buffer(easy::MSG_Buffer(pasta));
            auto resposta = Client.recv_buffer();
            
            if(resposta.to_string()=="-1"){
                erros.push_back("Pasta inexistente");
            }else {
                cout<<"Informe o nome do arquivo a ser transferido com seu formato"<<endl;
                string arq_name;
                getline(cin,arq_name);

                Client.send_buffer(easy::MSG_Buffer(arq_name));
                resposta = Client.recv_buffer();
                if(resposta.to_string()=="-2") {
                    erros.push_back("Arquivo inexistente");
                }else {
                    if(easy::utilities::tcp_recive_file(Client.sock_data,arq_name,0L,10000UL,percent))
                        success.push_back("Arquivo Recebido!");
                    else
                        erros.push_back("Erro ao receber o arquivo");
                }
            }
        }else
        if(comando=="lpastas"){
            Client.send_buffer(easy::MSG_Buffer("ListarPastas"));
            auto res = Client.recv_buffer();
            int size = stoi(res.to_string());
            for(int i=0;i<size;i++) {
                success.push_back(to_string(i)+": "+Client.recv_buffer().to_string());
            }
        }else
        if(comando=="larquivos"){
            Client.send_buffer(easy::MSG_Buffer("ListarArquivos"));
            cout<<"De qual pasta voce deseja listar os arquivos"<<endl;
            string pasta;
            cin>>pasta;
            
            Client.send_buffer(easy::MSG_Buffer(pasta));
            auto res = Client.recv_buffer();
            int size = stoi(res.to_string());
            for(int i=0;i<size;i++) {
                success.push_back(to_string(i)+": "+Client.recv_buffer().to_string());
            }
            
        }else 
        if(comando=="reconectar") {
            Client.Connect(ip,25567,1500,IPV4);
        }
    }while(true);
    return 0;
}