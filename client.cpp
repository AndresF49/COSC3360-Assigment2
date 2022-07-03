#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <vector>
#include <strings.h>

/* 
client takes the compressed message from stdin

- client needs to ask the num bits from server

*/

struct Data {
    char character;
    char binaryRep[256];
    int sockID;
    int portnum;
    struct hostent * server;
    struct sockaddr_in addr;
    socklen_t addr_size;

    // char messageBit[256];
};
// i think we have to put new socket in this function
void * getCharacter (void * vec) {
    // std::cout << "In getCharacter\n";

    char bin[256];
    int whatDo = 1;
    struct Data * dat = (struct Data *) vec; // send binary representation
    // create a new socketd per thread

    int sockID = socket(AF_INET,SOCK_STREAM,0);
    bcopy((char *) dat->server->h_addr, (char *) &dat->addr.sin_addr.s_addr, dat->server->h_length);
     
    if (sockID < 0) {
        std::cout << "Error in creating a socket in thread\n";
        exit(1);
    }

    if (connect(sockID, (struct sockaddr *) &dat->addr, sizeof(dat->addr)) < 0) {
        std::cout << "Error in connecting to socket in thread\n";
        return nullptr;
    }
    // std::cout << "Connected to socket in thread\n";
    if (write(sockID, &whatDo, sizeof(whatDo)) < 0) {
        std::cout << "Error in telling server that we want to send a binary rep\n";
    }
    if (write(sockID,&dat->binaryRep, sizeof(dat->binaryRep)) < 0) {
        std::cout << "Error writing binaryRep to server\n";
    }
    // now we wait for decompressed character from server
    bzero(&bin, sizeof(bin));
    if (read(sockID, &bin, sizeof(bin)) < 0) {
        std::cout << "Error in reading decompressed character from server\n";
    }
    
    dat->character = bin[0];
    close(sockID);
    return nullptr;
}

int main(int argc, char* argv[]) {

    // lets grab the compressed message first
    std::string message;

    std::getline(std::cin, message);

    int sock, portnum;
    // char buffer[256];
    int socket_int;
    struct sockaddr_in addr;
    int n;
    socklen_t addr_size;
    struct hostent *server; // translate the name into an ip address

    if (argc < 2) {
        std::cout << "ERROR: No port num provided.\n";
        exit(1);
    }

    sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock < 0) {
        perror("Socket error\n");
        exit(1);
    }
    // socket creation success

    portnum = atoi(argv[2]);
    server = gethostbyname(argv[1]); // first argument is the hostname, translates name into an address which is stored into a pointer

    if (server == NULL) { // hostname that we recieved from user is not right
        std::cout << "ERROR, no such host\n";
        exit(0);
    }

    bzero((char*)&addr, sizeof(addr));
    // initialize structs
    addr.sin_family = AF_INET;
    addr.sin_port = htons(portnum);
    bcopy((char *) server->h_addr, (char *) &addr.sin_addr.s_addr, server->h_length);
    
    // now we gotta connect to the server
    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        std::cout << "Error in connecting to server.\n";
        exit(1);
    }
    // we are connected

    // ask for numBits
    int numBits_server;
    bzero(&numBits_server, sizeof(numBits_server));
    // send 0 to server saying that we are doing a numBits request
    int whatDo = 0;
    if (write(sock, &whatDo, sizeof(whatDo)) < 0) {
        std::cout << "Error in telling server that we want to do a numBits request\n";
    }
    // then read numBits from server
    if (read(sock, &numBits_server, sizeof(numBits_server)) < 0) {
        std::cout << "Error reading numBits from server\n";
    }
    // now that we have the numBits, parse the message by the number of bits to get m amount of threads
    int m = message.size() / numBits_server;
    std::vector<Data> vecData;
    for (int i = 0; i < message.size(); i+=numBits_server) {
        Data temp;
        strcpy(temp.binaryRep,message.substr(i,numBits_server).c_str());

        temp.sockID = sock;
        temp.addr.sin_family = AF_INET;
        temp.addr.sin_port = htons(portnum);
        temp.server = server;
        vecData.push_back(temp); // add struct to vector
    }


    // make array to hold pthreads // make it a dynamic or static array instead //
    pthread_t* pthread = new pthread_t[m];

    for (int i = 0; i < m; i++) {
        if (pthread_create(&pthread[i], nullptr, getCharacter, &vecData[i]))
		{
			fprintf(stderr, "Error creating thread\n");
			return 1;
		} 
    }
    for (int i = 0; i < m; i++) {
            pthread_join(pthread[i], nullptr);
    }

    std::cout << "Decompressed message: ";
    for (int i = 0; i < vecData.size(); i++) {
        std::cout << vecData[i].character;
    }
    std::cout << std::endl;

    delete[] pthread;
    return 0;
}
