import logging
from threading import Thread
import socket

logger = logging.getLogger(__name__)

IPV4 = socket.AF_INET  
ONCONNECT = 1
ONDISCONNECT = 2
ONRECEIVE = 3
ONSEND = 4

class Assync_Client(Thread):
    
    def __init__(self,Host = "127.0.0.1", Port = 3333, Buffer_size = 1500, Domain = IPV4):
        Thread.__init__(self)
        super().__init__()
        self.Host = Host
        self.Port = Port
        self.Buffer_size = Buffer_size
        self.Domain = Domain

        self.Modules = {
            ONCONNECT : [],
            ONDISCONNECT : [],
            ONRECEIVE : [],
            ONSEND : []
        }

        self.Addr = (Host,Port)
        self.Socket = socket.socket(self.Domain, socket.SOCK_STREAM)
    
    def use(self,Module,ONACTION):
        self.Modules[ONACTION].append(Module)

    def run(self):
    
        while(True):
            
            Buffer = None
            Buffer = self.Socket.recv(self.Buffer_size)
            
            if not Buffer:
                for i in self.Modules[ONDISCONNECT]:
                    i(args=(self))   
                break
            
            for i in self.Modules[ONRECEIVE]:
                i(args=(Buffer,self))
            pass
    
    def connect(self):
        try:
            
            self.Socket.connect(self.Addr)
            self.start()

            for i in self.Modules[ONCONNECT]:
                i(args=(self))
            
        except:
            logger.exception("Erro to Connect")
        pass

    def disconnect(self):
        try:
            self.Socket.close()
            for i in self.Modules[ONDISCONNECT]:
                i(args=(self))
        except:
            logger.exception("Erro to close socket")
            pass
        pass
    
    def send(self,message):
        try:
            for i in self.Modules[ONSEND]:
                i(args=(self))
            self.Socket.sendall(message)
        except:
            logger.exception("Erro to Send")
        pass

class Client:
    def __init__(self,Host = "127.0.0.1", Port = 3333, Buffer_size = 1500, Domain = IPV4):
        super().__init__()
        self.Host = Host
        self.Port = Port
        self.Buffer_size = Buffer_size
        self.Domain = Domain
        self.Addr = (Host,Port)
        self.Socket = socket.socket(self.Domain, socket.SOCK_STREAM)
    
    def connect(self):
        try:
            self.Socket.connect(self.Addr)
        except:
            logger.exception("Erro to Connect")
        pass
    pass

    def disconnect(self):
        try:
            self.Socket.close()
        except:
            logger.exception("Erro to close socket")
            pass
        pass
    
    def recv(self):
        return self.Socket.recv(self.Buffer_size)
    
    def send(self,message):
        self.Socket.sendall(message)
