#include "easysocket.h"



namespace easy {
    #ifdef WIN32
        bool Win_init_Sock = false;
        WSADATA SockDll;
        int dll_version[] = {2,2};
    #endif

    Assync_Tcp_Module::Assync_Tcp_Module(void(*run)(std::map<std::string,void*>&)) {
        this->run=run;
    }

    MSG_Buffer::MSG_Buffer(std::string str,int max_buffer) {
        this->max_buffer = max_buffer;
        Buffer = new char[this->max_buffer];
        this->actual_buffer = str.size();
        memcpy(Buffer,str.c_str(),this->actual_buffer);
    }
    

    MSG_Buffer::MSG_Buffer(int max_buffer) {
        this->max_buffer = max_buffer;
        this->Buffer = new char[this->max_buffer];
        memset(this->Buffer,0x0,sizeof(char) * max_buffer);
    }

    const std::string MSG_Buffer::to_string() {
        if(this->actual_buffer<1)
            return std::string("");
        return std::string(this->Buffer,(size_t)this->actual_buffer);
    }

    namespace server {

        namespace TCP {
            Assync_Tcp_client::~Assync_Tcp_client() {
                this->Disconnect();
            }

            Assync_Tcp_client::Assync_Tcp_client(Assync_Tcp_Server *server,std::map<uint32_t,std::vector<Assync_Tcp_Module>> &modules,uint32_t id) {
                this->server = server;
                this->modules = modules;
                this->id = id;
            }
            
            void Assync_Tcp_client::init_communication() {
                loop_recv = std::thread(recv_loop,this);
                loop_recv.detach();
            }

            bool Assync_Tcp_client::Send(MSG_Buffer data) {
                MSG_Buffer buffer = data;
                std::map<std::string,void*> temp;
                temp["message"] = &buffer;
                temp["client"] = this;
                temp["server"] = this->server;
                for(auto i:this->modules[ONSEND]) {
                    i.run(temp);
                }
                if(send(this->addr.client_socket,buffer.Buffer,buffer.actual_buffer,0)!=data.actual_buffer)
                    return false;
                return true;
            }

            MSG_Buffer Assync_Tcp_client::recv_buffer() {
                MSG_Buffer temp(this->max_buffer);
                temp.actual_buffer = recv(this->addr.client_socket,temp.Buffer,temp.max_buffer,0);
                return temp;
            }

            void Assync_Tcp_client::Disconnect() {
                closesocket(this->addr.client_socket);
            }

            void Assync_Tcp_client::recv_loop(Assync_Tcp_client *Client) {
                while(true) {

                    auto novo = Client->recv_buffer();
                    if(novo.actual_buffer<=0) {
                        delete novo.Buffer;
                        break;
                    }
                        

                    Client->buffers.push_back(novo);
                    std::map<std::string,void*> temp;
                    temp["client"] = Client;
                    temp["server"] = Client->server;
                    temp["message"] = &Client->buffers.back();
                    for(auto i:Client->modules[ONRECEIVE]) {
                        i.run(temp);
                    }
                    delete novo.Buffer; 
                }
                
                for(auto i:Client->modules[ONDISCONNECT]) {
                    std::map<std::string,void*> temp;
                    temp["client"] = Client;
                    i.run(temp);
                }
            }




            Assync_Tcp_Server::Assync_Tcp_Server() {
                id_count = 0;
                listening = false;
                #ifdef WIN32
                if(!Win_init_Sock) {
                    WSAStartup(MAKEWORD(dll_version[0],dll_version[1]),&SockDll);
                    Win_init_Sock = true;
                }
                #endif
            }

            void Assync_Tcp_Server::use(Assync_Tcp_Module& module,int on_action) {
                modules[on_action].push_back(module);
            }

            void Assync_Tcp_Server::Listen(int port,int max_buffer,int domain) {
                this->port = port;
                this->max_buffer = max_buffer;
                this->domain = domain;
                this->listening = true;
                this->listen_socket = socket(domain,SOCK_STREAM,0);
                
                if(this->listen_socket==0)
                    std::cout<<"ERR: Socket failed"<<std::endl;
                
                server_address.sin_family = domain;
                server_address.sin_addr.s_addr = INADDR_ANY;
                server_address.sin_port = htons(port);

                if(bind(this->listen_socket ,(struct sockaddr *)&server_address ,sizeof(server_address))<0) {
                    std::cout<<"ERR: bind"<<std::endl;
                    this->listening = false;
                }

                if (listen(this->listen_socket, 20) < 0) {
                    std::cout<<"ERR: listen"<<std::endl;
                    this->listening = false;
                }

                accept_loop = std::thread(accept_function,this);
                this->accept_loop.detach();
            }


            //static functions
            void Assync_Tcp_Server::accept_function(Assync_Tcp_Server* Server) {
                while (Server->listening) {
                    auto novo = new Assync_Tcp_client(Server,Server->modules,Server->id_count);
                    novo->addr.address_size = sizeof(novo->addr.address);
                    novo->addr.client_socket = accept(Server->listen_socket,(struct sockaddr *)&novo->addr.address,&novo->addr.address_size);
                    if(novo->addr.client_socket > 0) {
                        Server->clients.push_back(novo);
                        Server->clients.back()->max_buffer = Server->max_buffer;
                        Server->clients.back()->init_communication();
                        Server->id_count++;
                        std::map<std::string,void*> temp;
                        temp["client"] = Server->clients.back();
                        for(auto i:Server->modules[ONCONNECT]) {
                            i.run(temp);
                        }
                    }
                }
            }


            void Tcp_Client::send_buffer(MSG_Buffer buffer) {
                send(this->sock_data.client_socket,buffer.Buffer,buffer.actual_buffer,0);
                return;
            }

            MSG_Buffer Tcp_Client::recv_buffer() {
                MSG_Buffer temp(this->max_buffer);
                temp.actual_buffer = recv(this->sock_data.client_socket,temp.Buffer,temp.max_buffer,0);
                return temp;
            }
            Tcp_Server::Tcp_Server() {
                id_count = 0;
                listening = false;
                #ifdef WIN32
                if(!Win_init_Sock) {
                    WSAStartup(MAKEWORD(dll_version[0],dll_version[1]),&SockDll);
                    Win_init_Sock = true;
                }
                #endif
            }
            
            void Tcp_Server::Listen(int port,int max_buffer,int domain) {
                this->port = port;
                this->max_buffer = max_buffer;
                this->domain = domain;
                this->listening = true;
                this->listen_socket = socket(domain,SOCK_STREAM,0);
                
                if(this->listen_socket==0)
                    std::cout<<"ERR: Socket failed"<<std::endl;
                
                server_address.sin_family = domain;
                server_address.sin_addr.s_addr = INADDR_ANY;
                server_address.sin_port = htons(port);

                if(bind(this->listen_socket ,(struct sockaddr *)&server_address ,sizeof(server_address))<0) {
                    std::cout<<"ERR: bind"<<std::endl;
                    this->listening = false;
                }

                if (listen(this->listen_socket, 20) < 0) {
                    std::cout<<"ERR: listen"<<std::endl;
                    this->listening = false;
                }
            }

            Tcp_Client Tcp_Server::accept_connection() {
                Tcp_Client novo;
                novo.sock_data.client_socket = accept(this->listen_socket,(sockaddr*)&novo.sock_data.address,&novo.sock_data.address_size);
                return novo;
            }

        }
        namespace UDP {
        //falta_cmd
            UDP_Server::UDP_Server() {
                #ifdef WIN32
                if(!Win_init_Sock) {
                    WSAStartup(MAKEWORD(dll_version[0],dll_version[1]),&SockDll);
                    Win_init_Sock = true;
                }
                #endif
            }
            
            //falta_cmd
            bool UDP_Server::Listen(int port,int domain) {
                
                Sock_Data.client_socket = socket(domain, SOCK_DGRAM, 0);

                Sock_Data.address.sin_family = domain;
                Sock_Data.address.sin_addr.s_addr = htonl(INADDR_ANY);
                Sock_Data.address.sin_port = htons(port);

                if(bind(Sock_Data.client_socket,(const struct sockaddr *)&Sock_Data.address,sizeof(Sock_Data.address))<0)
                    return false;
                return true;

            }

            //falta_cmd
            void UDP_Server::SendTo(Basic_Socket& Client,MSG_Buffer& Buffer) {
                sendto(Sock_Data.client_socket,Buffer.Buffer,Buffer.actual_buffer,0,(const struct sockaddr *) &Client.address,Client.address_size);
            }
            
            //falta_cmd
            Basic_Socket UDP_Server::RecvData(MSG_Buffer& Buffer) {
                Basic_Socket CLIENT;
                Buffer.actual_buffer = recvfrom(Sock_Data.client_socket,
                                                Buffer.Buffer,
                                                Buffer.max_buffer,
                                                0,
                                                (struct sockaddr*) &CLIENT.address,
                                                &CLIENT.address_size);
                return CLIENT;
            }

        }
    }

    namespace client {
        namespace TCP {
            Assync_Tcp_client::Assync_Tcp_client() {
                #ifdef WIN32
                if(!Win_init_Sock) {
                    WSAStartup(MAKEWORD(dll_version[0],dll_version[1]),&SockDll);
                    Win_init_Sock = true;
                }
                #endif
            }
            
            void Assync_Tcp_client::use(Assync_Tcp_Module& module,int on_action) {
                modules[on_action].push_back(module);
            }

            bool Assync_Tcp_client::Connect(std::string ip,uint64_t port,uint64_t max_buffer,int domain) {
                memset(&this->server_address,0x0,sizeof(this->server_address));
                this->max_buffer = max_buffer;
                server_address.sin_family = domain;
                server_address.sin_addr.s_addr=inet_addr(ip.c_str());
                server_address.sin_port = htons(port);
                this->listen_socket = socket(domain, SOCK_STREAM, 0);

                if(connect(this->listen_socket,(struct sockaddr*)&this->server_address,sizeof(this->server_address))==-1) {
                    return false;
                }
                
                this->thread_loop = std::thread(response_loop,this);
                this->thread_loop.detach();

                std::map<std::string,void*> temp;
                temp["client"] = this;
                for(auto i:modules[ONCONNECT]) {
                    i.run(temp);
                }


                return true;
            }

            bool Assync_Tcp_client::Send(MSG_Buffer buffer) {
                std::map<std::string,void*> temp;
                temp["message"] = &buffer;
                temp["client"] = this;
                for(auto i:this->modules[ONSEND]) {
                    i.run(temp);
                }
                uint64_t result = send(this->listen_socket,buffer.Buffer,buffer.actual_buffer,0);
                if(result==(uint64_t)buffer.actual_buffer)
                    return true;
                return false;
            }

            void Assync_Tcp_client::response_loop(Assync_Tcp_client* Client) {
                uint64_t sock = Client->listen_socket;
                uint64_t max_buffer = Client->max_buffer;
                while(true) {
                    auto _new=MSG_Buffer(max_buffer);
                    _new.actual_buffer = recv(sock,_new.Buffer,max_buffer,0);
                    if(_new.actual_buffer<0) return;
                    Client->buffers.push_back(_new);
                    std::map<std::string,void*> temp;
                    temp["message"] = &Client->buffers.back();
                    temp["client"] = Client;
                    for(auto i:Client->modules[ONRECEIVE]) {
                        i.run(temp);
                    }
                }
            }
            Tcp_Client::Tcp_Client() {
                #ifdef WIN32
                if(!Win_init_Sock) {
                    WSAStartup(MAKEWORD(dll_version[0],dll_version[1]),&SockDll);
                    Win_init_Sock = true;
                }
                #endif
                connected = false;
            }

            bool Tcp_Client::Connect(std::string ip,uint64_t port,uint64_t max_buffer,int domain) {
                memset(&this->sock_data.address,0x0,sizeof(this->sock_data.address));
                this->max_buffer = max_buffer;
                sock_data.address.sin_family = domain;
                sock_data.address.sin_addr.s_addr=inet_addr(ip.c_str());
                sock_data.address.sin_port = htons(port);
                this->sock_data.client_socket = socket(domain, SOCK_STREAM, 0);

                if(connect(this->sock_data.client_socket,(struct sockaddr*)&this->sock_data.address,sizeof(this->sock_data.address))==-1) {
                    return false;
                }
                connected = true;
                return true;
            }

            void Tcp_Client::send_buffer(MSG_Buffer buffer) {
                send(this->sock_data.client_socket,buffer.Buffer,buffer.actual_buffer,0);
                return;
            }

            MSG_Buffer Tcp_Client::recv_buffer() {
                MSG_Buffer temp(this->max_buffer);
                temp.actual_buffer = recv(this->sock_data.client_socket,temp.Buffer,temp.max_buffer,0);
                if(temp.actual_buffer<0)
                    connected = false;
                return temp;
            }
        }
        namespace UDP {
            UDP_Client::UDP_Client() {
                #ifdef WIN32
                if(!Win_init_Sock) {
                    WSAStartup(MAKEWORD(dll_version[0],dll_version[1]),&SockDll);
                    Win_init_Sock = true;
                }
                #endif
            }
            
            bool UDP_Client::prepare(std::string ip,int port,int max_buffer,int domain) {
                this->max_buffer = max_buffer; 
                Client_Data.client_socket = socket(domain, SOCK_DGRAM, 0);
                memset(&Client_Data.address, 0, sizeof(Client_Data.address));

                Client_Data.address.sin_family = IPV4;
                Client_Data.address.sin_port = htons(port);
                Client_Data.address.sin_addr.s_addr = inet_addr(ip.c_str());

                return true;
            }

            //falta_cmd
            void UDP_Client::SendTo(Basic_Socket& Client,MSG_Buffer& Buffer) {
                sendto(Client_Data.client_socket,Buffer.Buffer,Buffer.actual_buffer,0,(const struct sockaddr *) &Client.address,Client.address_size);
            }
            
            //falta_cmd
            Basic_Socket UDP_Client::RecvData(MSG_Buffer& Buffer) {
                Basic_Socket CLIENT;
                Buffer.actual_buffer = recvfrom(Client_Data.client_socket,
                                                Buffer.Buffer,
                                                Buffer.max_buffer,
                                                0,
                                                (struct sockaddr*) &CLIENT.address,
                                                &CLIENT.address_size);
                return CLIENT;
            }


        }
    }
    
    namespace utilities {
        
        template<typename type> 
        bool tcp_send_var(Basic_Socket Socket,type* var) {
        return (send(Socket.client_socket,(char*)var,sizeof(type),0)==sizeof(type))?true:false;
        }
        
        template<typename type> 
        bool tcp_recv_var(Basic_Socket Socket,type* var) {
            return (recv(Socket.client_socket,(char*)var,sizeof(type),0) == sizeof(type))?true:false;
        }

        long tcp_send_file(Basic_Socket Socket, std::string path, long i_Bytes = 0L, long Packet_Size = 2000, void(*callback)(long,long) = [](long a, long b){a=a*1;b=b*1;}) {
            std::fstream file(path,std::ios::in | std::ios::binary);
            file.seekg(0,std::ios::end);
            long bytes = file.tellg();

            tcp_send_var<long>(Socket,&bytes);
            //send(Socket.client_socket,(char*)&bytes,sizeof(bytes),0);

            file.seekg(i_Bytes,std::ios::beg);

            long A_bytes =  i_Bytes;
            
            int resto = (bytes - A_bytes)%Packet_Size;
            int parts = ((bytes - A_bytes)-resto)/Packet_Size;

            char *Buffer = new char[Packet_Size];

            for(int i=0;i<parts;i++) {
                memset(Buffer,0x0,sizeof(char)*Packet_Size);
                file.read(Buffer,Packet_Size);
                send(Socket.client_socket,Buffer,Packet_Size,0);
                A_bytes+=Packet_Size;
                
                callback(A_bytes,bytes);
            }
            
            memset(Buffer,0x0,sizeof(char)*Packet_Size);
            file.read(Buffer,resto);
            send(Socket.client_socket,Buffer,resto,0);
            A_bytes+=resto;
            callback(A_bytes,bytes);

            file.close();
            return A_bytes;
        }

        long tcp_recive_file(Basic_Socket Socket,std::string path, long i_Bytes = 0L, long Packet_Size = 2000, void(*callback)(long,long) = [](long a, long b){a=a*1;b=b*1;}) {
            
            long bytes = 0L;
            long A_bytes =  i_Bytes;

            // int r = recv(Socket.client_socket,(char*)&bytes,sizeof(bytes),0);
            // if(r < 0)
            //     return 0L;
            
            if(!tcp_recv_var<long>(Socket,&bytes))
                return 0L;
            
            std::fstream file(path,std::ios::binary | std::ios::out);

            file.seekg(i_Bytes,std::ios::beg);

            char *Buffer = new char[Packet_Size];
            long recived_bytes = 0L;
            
            while(bytes - A_bytes) {
                memset(Buffer,0x0,sizeof(char)*Packet_Size);
                recived_bytes = recv(Socket.client_socket,Buffer,Packet_Size,0);
                
                if(recived_bytes < 0)
                    break;
                A_bytes += recived_bytes;
                callback(A_bytes,bytes);

                file.write(Buffer,recived_bytes);
                
            }

            file.close();
            return A_bytes;
        }
    }
}
