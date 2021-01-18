# from EasySocket.Server.TCP import *


# def conectou(args):
#     print("Conectou!", args[0].Addr)

# def onreceive(args):
    
#     print(args[2].Connections)
#     for i in args[2].Connections:
#         i.send(args[0])
    
#     print("mandou",args[0])

# def dc(args):
#     print("dc",args)

# server = Assync_Server()
# server.use(conectou,ONCONNECT)
# server.use(onreceive,ONRECEIVE)
# server.use(dc,ONDISCONNECT)
# server.listen(Port=80,Join=True)

from EasySocket.Client.TCP import *

def conectou(args):
    print("Conectou!", args)

def onreceive(args):
    print(args)
    args[1].send(args[0])

def dc(args):
    print("dc",args)

client = Assync_Client()

client.use(conectou,ONCONNECT)
client.use(onreceive,ONRECEIVE)
client.use(dc,ONDISCONNECT)

client.connect()
