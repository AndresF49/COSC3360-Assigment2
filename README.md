## For more details, view pdf
Simple server and client programs to send and recieve data using sockets.
Reuses some of the code from Homework 1.
Client program will:
1. Create a socket to communicate with the server program.
2. Send the binary code of the symbol to decompress to the server program using sockets. 
3. Wait for the decompressed character from the server program.
4. Write the received information into a memory location accessible by the main thread.

Server program will take 2 types of inputs:
1. A request from the main thread of the client program asking for the number of bits of the fixed-length codes. 
2. One request per child thread of the client program. These threads send the binary code of a symbol in the compressed
message, and the server returns the character from the alphabet assigned to that binary sequence.