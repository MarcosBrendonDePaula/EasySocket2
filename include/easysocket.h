#pragma once
#ifndef EASY_INC
#define EASY_INC

#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <memory>
#include <fstream>
#include <WinSock2.h>

namespace easy {

    #ifdef WIN32
        extern bool Win_init_Sock;
        extern WSADATA SockDll;
        extern int dll_version[2];
    #endif

    #define ONCONNECT 1
    #define ONDISCONNECT 2
    #define ONRECEIVE 3
    #define ONSEND 4
    #define IPV4 2
    #define IPV6 23


    class Basic_Socket {
        public:
            int client_socket;
            struct sockaddr_in address;
            int address_size;
            Basic_Socket() {
                address_size = sizeof(address);
                memset(&address,0x0,sizeof(address));
            }
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
    class MSG_Buffer {
        public:
            /*!
            * \fn MSG_Buffer(int max_buffer=1500)
            * \brief method constructor.
            * \param max_buffer maximum characters in buffer std::vector.
            */
            MSG_Buffer(int max_buffer=1500);
            

            /*!
            * \fn MSG_Buffer(std::string str,int max_buffer=1500)
            * \brief method constructor.
            * \param str auto allocator std::string to MSG_Buffer
            * \param max_buffer maximum characters in buffer std::vector
            */
            MSG_Buffer(std::string str,int max_buffer=1500);
            
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
    namespace server {
        //TCP-----------------------------------------------------
        namespace TCP {
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

                    Basic_Socket addr;

                    Assync_Tcp_Server *server;
                    /*!
                    * \fn init_communication();
                    * \brief receiving std::thread initiator.
                    */
                    void init_communication();

                    /*!
                    * \fn MSG_Buffer(int max_buffer=1500);
                    * \brief send buffer to client.
                    * \param input Buffer to send to client.
                    */
                    bool Send(MSG_Buffer input);
                    
                    MSG_Buffer recv_buffer();

                    void Disconnect();
                    //static functions
                    
                    uint32_t id;
                    int max_buffer;
                    std::vector<MSG_Buffer> buffers;
                    
                    ~Assync_Tcp_client();
                private:

                    /*!
                    * \fn recv_loop(Assync_Tcp_client *Client);
                    * \brief recv loop to current client.
                    * \param Client current client address.
                    */
                    static void recv_loop(Assync_Tcp_client *Client);

                    bool connected;
                    std::thread loop_recv;
            };

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

                private:

                    /*!
                    * \fn accept_function(Assync_Tcp_Server* Server)
                    * \brief accept loop.
                    * \param Server current adress Server.
                    */
                    static void accept_function(Assync_Tcp_Server* Server);

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
 
            class Tcp_Client {
                public:
                
                Basic_Socket sock_data;
                uint64_t max_buffer;
                
                void send_buffer(MSG_Buffer buffer); 
                MSG_Buffer recv_buffer();
            };
            
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
        }
        namespace UDP {

            class UDP_Server {
                public:
                    int max_buffer;
                    UDP_Server();
                    bool Listen(int port,int domain=IPV4);

                    void SendTo(Basic_Socket& Socket,MSG_Buffer& Buffer);
                    Basic_Socket RecvData(MSG_Buffer& Buffer);
                    

                private:
                    Basic_Socket Sock_Data;
            };
        }
    }
    namespace client {
        namespace TCP {
            class Assync_Tcp_client {
                public:
                    Assync_Tcp_client();
                    bool Connect(std::string ip="127.0.0.1",uint64_t port=3333,uint64_t max_buffer=1500,int domain=IPV4);
                    bool Disconnect(){closesocket(this->listen_socket);return true;};
                    bool Send(MSG_Buffer buffer);

                    void use(Assync_Tcp_Module& module,int on_action);
                    
                    std::vector<MSG_Buffer> buffers;
                private:
                    
                    static void response_loop(Assync_Tcp_client* Client);
                    std::thread thread_loop;
                    std::string ip;
                    uint64_t port;
                    uint32_t listen_socket;
                    
                    uint64_t max_buffer;
                    std::map<uint32_t,std::vector<Assync_Tcp_Module>> modules;

                    struct sockaddr_in server_address;
            };

            class Tcp_Client {
                public:
                Tcp_Client();
                Basic_Socket sock_data;
                uint64_t max_buffer;
                bool Connect(std::string ip,uint64_t port,uint64_t max_buffer,int domain);
                void send_buffer(MSG_Buffer buffer); 
                MSG_Buffer recv_buffer();
                bool connected;
            };
            
        }
        namespace UDP {

            class UDP_Client {
                public:
                    UDP_Client();
                    bool prepare(std::string ip,int port,int max_buffer,int domain);
                    
                    void SendTo(Basic_Socket& Socket,MSG_Buffer& Buffer);
                    Basic_Socket RecvData(MSG_Buffer& Buffer);

                    Basic_Socket& getServer(){return Client_Data;}
                private:
                    Basic_Socket Client_Data;
                    int max_buffer;
            };
            
        }
    }
    namespace utilities {
    
    }
}
#endif
