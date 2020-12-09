#pragma once
//include libs
#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <memory>
#include <fstream>
#include <WinSock2.h>

namespace easy {

    #ifdef WIN32
        bool Win_init = false;
        WSADATA SockDll;
        int dll_version[] = {2,2};
    #endif
    
    #define ONCONNECT 1
    #define ONDISCONNECT 2
    #define ONRECEIVE 3
    #define ONSEND 4
    #define IPV4 2
    #define IPV6 23


    class Tcp_sock {
        public:
            uint32_t client_socket;
            struct sockaddr_in address;
            int address_size;
    };
    
    class Udp_sock {
        public:
            uint32_t client_socket;
            struct sockaddr_in address; 
            int address_size;
    };

    class Assync_Tcp_Module {
    public:
        void(*run)(std::map<std::string,void*>&);
        
        /*!
        * \fn Assync_Tcp_Module(void(*run)(std::map<std::string,void*>&)
        * \brief method constructor.
        * \param run function to be performed.
        */
        Assync_Tcp_Module(void(*run)(std::map<std::string,void*>&));
    };

    Assync_Tcp_Module::Assync_Tcp_Module(void(*run)(std::map<std::string,void*>&)) {
        this->run=run;
    }

    class Tcp_client_Buffer {
        public:
            /*!
            * \fn Tcp_client_Buffer(int max_buffer=1500)
            * \brief method constructor.
            * \param max_buffer maximum characters in buffer std::vector.
            */
            Tcp_client_Buffer(int max_buffer=1500);
            
            /*!
            * \fn ~Tcp_client_Buffer
            * \brief method Destructor.
            */
            ~Tcp_client_Buffer();

            /*!
            * \fn Tcp_client_Buffer(std::string str,int max_buffer=1500)
            * \brief method constructor.
            * \param str auto allocator std::string to Tcp_client_Buffer
            * \param max_buffer maximum characters in buffer std::vector
            */
            Tcp_client_Buffer(std::string str,int max_buffer=1500);
            
            /**
             * @brief parse buffer to std::string
             * 
             * @return const std::string 
             */
            const std::string to_string();

            int max_buffer;
            int actual_buffer;
            char *Buffer;
        private:

    };

    Tcp_client_Buffer::Tcp_client_Buffer(std::string str,int max_buffer) {
        this->max_buffer = max_buffer;
        Buffer = new char[this->max_buffer];
        this->actual_buffer = str.size();
        memcpy(Buffer,str.c_str(),this->actual_buffer);
    }
    
    Tcp_client_Buffer::~Tcp_client_Buffer() {
        delete this->Buffer;
    }

    Tcp_client_Buffer::Tcp_client_Buffer(int max_buffer) {
        this->max_buffer = max_buffer;
        Buffer = new char[max_buffer];
        memset(Buffer,0x0,sizeof(char)*max_buffer);
    }

    const std::string Tcp_client_Buffer::to_string() {
        std::string msg;
        msg.clear();
        for(int i=0;i<this->actual_buffer;i++) {
            msg.push_back(this->Buffer[i]);
        }
        return msg;
    }

    namespace server {
        class Assync_Tcp_client;

        class Assync_Tcp_Server;

        class Assync_Tcp_client {
            public:

                /*!
                * \fn Assync_Tcp_client(std::map<uint32_t,std::vector<Assync_Tcp_Module>> &modules,uint32_t id)
                * \brief method constructor.
                * \param modules list of modules to be executed.
                */
                Assync_Tcp_client(Assync_Tcp_Server *server,std::map<uint32_t,std::vector<Assync_Tcp_Module>> &modules,uint32_t id);
                std::map<uint32_t,std::vector<Assync_Tcp_Module>> modules;

                ~Assync_Tcp_client();

                uint32_t client_socket;
                struct sockaddr_in address;
                //functions

                Assync_Tcp_Server *server;
                /*!
                * \fn init_comunication();
                * \brief receiving std::thread initiator.
                */
                void init_comunication();

                /*!
                * \fn Tcp_client_Buffer(int max_buffer=1500);
                * \brief send buffer to client.
                * \param input Buffer to send to client.
                */
                bool Send(Tcp_client_Buffer input);
                void Disconnect();
                //static functions
                
                /*!
                * \fn recv_loop(Assync_Tcp_client *Client);
                * \brief recv loop to current client.
                * \param Client current client address.
                */
                static void recv_loop(Assync_Tcp_client *Client);



                uint32_t id;
                int none;
                int max_buffer;
                std::vector<Tcp_client_Buffer> buffers;
            private:
                bool connected;
                std::thread loop_recv;
        };
        
        Assync_Tcp_client::~Assync_Tcp_client() {
            this->Disconnect();
        }

        Assync_Tcp_client::Assync_Tcp_client(Assync_Tcp_Server *server,std::map<uint32_t,std::vector<Assync_Tcp_Module>> &modules,uint32_t id) {
            this->server = server;
            this->modules = modules;
            this->id = id;
        }
        
        void Assync_Tcp_client::init_comunication() {
            loop_recv = std::thread(Assync_Tcp_client::recv_loop,this); 
        }

        bool Assync_Tcp_client::Send(Tcp_client_Buffer data) {
            Tcp_client_Buffer buffer = data;
            std::map<std::string,void*> temp;
            temp["message"] = &buffer;
            temp["client"] = this;
            temp["server"] = this->server;
            for(auto i:this->modules[ONSEND]) {
                i.run(temp);
            }
            if(send(this->client_socket,buffer.Buffer,buffer.actual_buffer,0)!=data.actual_buffer)
                return false;
            return true;
        }

        void Assync_Tcp_client::Disconnect() {
            closesocket(this->client_socket);
        }

        void Assync_Tcp_client::recv_loop(Assync_Tcp_client *Client) {
            int size;
            while(true) {
                auto novo = Tcp_client_Buffer(Client->max_buffer);
                novo.actual_buffer = recv(Client->client_socket,novo.Buffer,Client->max_buffer,0);
                
                if(novo.actual_buffer<=0)
                    return;

                Client->buffers.push_back(novo);
                std::map<std::string,void*> temp;
                temp["client"] = Client;
                temp["server"] = Client->server;
                temp["message"] = &Client->buffers.back();
                for(auto i:Client->modules[ONRECEIVE]) {
                    i.run(temp);
                }
            }
        }


        class Assync_Tcp_Server {
            public:
                /*!
                * \fn Assync_Tcp_Server()
                * \brief method constructor.
                */
                Assync_Tcp_Server();

                /*!
                * \fn Assync_Tcp_Server()
                * \brief method constructor.
                * \param new_module module to be added.
                * \param on_action action to which the module fits.
                */
                void use(Assync_Tcp_Module& new_module,int on_action);
                
                /*!
                * \fn Listen(int port = 3333 , int max_connections = -1, int max_buffer = 1500,int domain = IPV4)
                * \brief starts listening mode.
                * \param port port on listening.
                * \param max_buffer maximum buffer size.
                * \param domain ip type.
                */
                void Listen(int port = 3333 , int max_buffer = 1500,int domain = IPV4);

                //static functions

                /*!
                * \fn accept_function(Assync_Tcp_Server* Server)
                * \brief accept loop.
                * \param Server current adress Server.
                */
                static void accept_function(Assync_Tcp_Server* Server);
            private:
                bool listening;
                uint32_t listen_socket;
                struct sockaddr_in server_address;

                int domain;
                int port;

                uint32_t max_connection;
                uint32_t max_buffer;

                std::vector<Assync_Tcp_client*> clients;
                std::thread accept_loop;
                std::map<uint32_t,std::vector<Assync_Tcp_Module>> modules;
                uint32_t id_count;
        };

        Assync_Tcp_Server::Assync_Tcp_Server() {
            id_count = 0;
            listening = false;
            #ifdef WIN32
            if(!Win_init) {
                WSAStartup(MAKEWORD(dll_version[0],dll_version[1]),&SockDll);
                Win_init = true;
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
                novo->none = sizeof(novo->address);
                novo->client_socket = accept(Server->listen_socket,(struct sockaddr *)&novo->address,&novo->none);
                if(novo->client_socket > 0) {
                    Server->clients.push_back(novo);
                    Server->clients.back()->max_buffer = Server->max_buffer;
                    Server->clients.back()->init_comunication();
                    Server->id_count++;
                    std::map<std::string,void*> temp;
                    temp["client"] = Server->clients.back();
                    for(auto i:Server->modules[ONCONNECT]) {
                        i.run(temp);
                    }
                }
            }
        }

        
        class Tcp_Client {
            public:
            
            Tcp_sock sock_data;
            uint64_t max_buffer;
            
            void send_buffer(Tcp_client_Buffer buffer); 
            Tcp_client_Buffer recv_buffer();
        };
        
        void Tcp_Client::send_buffer(Tcp_client_Buffer buffer) {
            send(this->sock_data.client_socket,buffer.Buffer,buffer.actual_buffer,0);
            return;
        }

        Tcp_client_Buffer Tcp_Client::recv_buffer() {
            Tcp_client_Buffer temp(this->max_buffer);
            temp.actual_buffer = recv(this->sock_data.client_socket,temp.Buffer,temp.max_buffer,0);
            return temp;
        }

        class Tcp_Server{
            public:
                Tcp_Server();
                void use(Assync_Tcp_Module& module,int on_action);
                void Listen(int port,int max_buffer,int domain);
                Tcp_Client accept_connection();
            private:
                uint64_t id_count;
                bool listening;
                
                uint32_t listen_socket;
                struct sockaddr_in server_address;

                int domain;
                int port;

                uint32_t max_buffer;
        };

        Tcp_Server::Tcp_Server() {
            id_count = 0;
            listening = false;
            #ifdef WIN32
            if(!Win_init) {
                WSAStartup(MAKEWORD(dll_version[0],dll_version[1]),&SockDll);
                Win_init = true;
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

    namespace client {
        class Assync_Tcp_client {
            public:
                Assync_Tcp_client();
                bool Connect(std::string ip="127.0.0.1",uint64_t port=3333,uint64_t max_buffer=1500,int domain=IPV4);
                bool Disconnect(){closesocket(this->listen_socket);return true;};
                bool Send(Tcp_client_Buffer buffer);

                void use(Assync_Tcp_Module& module,int on_action);
                static void response_loop(Assync_Tcp_client* Client);

                std::vector<Tcp_client_Buffer> buffers;
            private:
                
                std::thread thread_loop;
                std::string ip;
                uint64_t port;
                uint32_t listen_socket;
                
                uint64_t max_buffer;
                std::map<uint32_t,std::vector<Assync_Tcp_Module>> modules;

                struct sockaddr_in server_address;
        };

        Assync_Tcp_client::Assync_Tcp_client() {
            #ifdef WIN32
            if(!Win_init) {
                WSAStartup(MAKEWORD(dll_version[0],dll_version[1]),&SockDll);
                Win_init = true;
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
            
            this->thread_loop = std::thread(Assync_Tcp_client::response_loop,this);
            this->thread_loop.detach();

            std::map<std::string,void*> temp;
            temp["client"] = this;
            for(auto i:modules[ONCONNECT]) {
                i.run(temp);
            }


            return true;
        }

        bool Assync_Tcp_client::Send(Tcp_client_Buffer buffer) {
            std::map<std::string,void*> temp;
            temp["message"] = &buffer;
            temp["client"] = this;
            for(auto i:this->modules[ONSEND]) {
                i.run(temp);
            }
            uint64_t result = send(this->listen_socket,buffer.Buffer,buffer.actual_buffer,0);
            if(result==buffer.actual_buffer)
                return true;
            return false;
        }

        void Assync_Tcp_client::response_loop(Assync_Tcp_client* Client) {
            uint64_t sock = Client->listen_socket;
            uint64_t max_buffer = Client->max_buffer;
            while(true) {
                auto _new=Tcp_client_Buffer(max_buffer);
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

        class Tcp_Client {
            public:
            Tcp_Client();
            Tcp_sock sock_data;
            uint64_t max_buffer;
            bool Connect(std::string ip,uint64_t port,uint64_t max_buffer,int domain);
            void send_buffer(Tcp_client_Buffer buffer); 
            Tcp_client_Buffer recv_buffer();
        };
        
        Tcp_Client::Tcp_Client() {
            #ifdef WIN32
            if(!Win_init) {
                WSAStartup(MAKEWORD(dll_version[0],dll_version[1]),&SockDll);
                Win_init = true;
            }
            #endif
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
            return true;
        }

        void Tcp_Client::send_buffer(Tcp_client_Buffer buffer) {
            send(this->sock_data.client_socket,buffer.Buffer,buffer.actual_buffer,0);
            return;
        }

        Tcp_client_Buffer Tcp_Client::recv_buffer() {
            Tcp_client_Buffer temp(this->max_buffer);
            temp.actual_buffer = recv(this->sock_data.client_socket,temp.Buffer,temp.max_buffer,0);
            return temp;
        }
    }
    
    namespace utilities {
        
        template<typename type> bool tcp_send_var(uint64_t socket,type* var) {
           return (send(socket,var,sizeof(type),0)==sizeof(type))?true:false;
        }
        
        template<typename type> bool tcp_recv_var(uint64_t socket,type* var) {
            return (recv(socket,var,sizeof(type),0)==sizeof(type))?true:false;
        }

        class Binary_File {
            public:
                bool Open(std::string file="",uint64_t max_buffer=1500);
                bool Save(std::string file="");
                std::vector<Tcp_client_Buffer> parts;
            private:
                std::ifstream r_file;
                std::ofstream w_file;
        };

        bool Binary_File::Open(std::string file,uint64_t max_buffer) {
            this->r_file.open(file,std::ios::in|std::ios::binary);
            if(!this->r_file.is_open())
                return false;
            this->r_file.seekg(0,std::ios::end);
            uint64_t file_size = this->r_file.tellg();
            this->r_file.seekg(0,std::ios::beg);
            uint64_t resto = file_size % max_buffer;
            uint64_t parts = (file_size-resto)/max_buffer;
            
            for(auto i=0;i<parts;i++) {
                Tcp_client_Buffer temp(max_buffer);
                this->r_file.read(temp.Buffer,max_buffer);
                temp.actual_buffer = max_buffer;
                this->parts.push_back(temp);
            }

            Tcp_client_Buffer temp(max_buffer);
            this->r_file.read(temp.Buffer,resto);
            temp.actual_buffer = resto;
            this->parts.push_back(temp);
            this->r_file.close();
            return true;
        }

        bool Binary_File::Save(std::string file) {
            this->w_file.open(file,std::ios::out|std::ios::binary);
            for(auto i:this->parts) {
                this->w_file.write(i.Buffer,i.actual_buffer);
            }
            this->w_file.close();
            return true;
        }

    }
}
