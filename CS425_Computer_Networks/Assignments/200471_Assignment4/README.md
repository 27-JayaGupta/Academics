# Execution Instructions

## To run the server

```
python3 server.py
```

## To run the client
```
python3 client.py
```

## Implementation

* The Server is running on ```localhost``` and port ```12000```.
* ```sendto``` and ```recvfrom``` are used for communication.
* ```SOCK_DGRAM``` flag is used to initialise the socket.
* The Buffer size to receive/send the message is ```2048```.
* Client enters a lowecase sentence and send it to the server.
* The server capitalizes the sentence and sends it back to the client.