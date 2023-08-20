from socket import *

serverName = 'localhost'
serverPort = 12000
clientSocket = socket(AF_INET, SOCK_DGRAM)
sentence = input('Input lowercase sentence:')
clientSocket.sendto(sentence.encode(), (serverName, serverPort))
modifiedSentence, serverAddress = clientSocket.recvfrom(2048)
print ('From Server:', modifiedSentence.decode())
clientSocket.close()
