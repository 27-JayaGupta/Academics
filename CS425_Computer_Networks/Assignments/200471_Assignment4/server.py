from socket import *

serverPort = 12000
serverSocket = socket(AF_INET, SOCK_DGRAM)
serverSocket.bind(("localhost", serverPort))
print ("The server is ready to receive")

while True:
    sentence, clientAddress = serverSocket.recvfrom(2048)
    print("Received message from", clientAddress)
    capitalizedSentence = sentence.upper()
    serverSocket.sendto(capitalizedSentence, clientAddress)