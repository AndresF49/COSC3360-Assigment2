#include <iostream>
#include <string>
#include <unistd.h>
#include <map>
#include <iterator>
#include <vector>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <strings.h>


// start putting together the server!!!!!

/*
The server program calculates the number of bits of the fixed-length codes based on the value of the greatest 
base 10 code in the alphabet.
Additionally, this program determines the binary representation of each of the symbols in the alphabet 

the server receives 2 types of request from the client
1. request the num of bits of fixed-length codes
2. one request per child thread that sends the binary code of the symbol in the compresssed mes
 -> server sends back the character from the alphabet assigned to that sequence

the server will create a child process every time a request is made
*/

// beginning of assignment 1

// create a struct that has integer value, numBits, pointer to the compressed message
struct Data {
    int base10Val;
    std::string binary;
    char letter;
    int numBits;
    int frequency;
    std::string message;
};

// struct Data2 {
//     // std::string binaryRep;
//     int base10Val, numBits;
//     char letter;
// };

struct Data3 {
    char let;
    int base10;
};

void fireman(int)
{
   while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

std::string toBinary (int base10Val, int numBits) {
    std::string base2 = "";
    std::string newBinary = "";
    std::string bstr = "", bstr2 = "";
    int bNums[numBits];
    int remainder = 0;
    int j = 0;
    while (base10Val != 0) {
        remainder = base10Val % 2;
        base10Val /= 2;
        bNums[j] = remainder;
        j++;
    }
    for (int i = j-1; i >= 0; i--) {
        bstr.append(std::to_string(bNums[i]));
    }
    if (bstr.length() < numBits) {
        bstr2.append(numBits - (int)bstr.length(), '0');
        bstr = bstr2 + bstr;
    } 

    return bstr;
}

int main(int argc, char * argv[]) {

    int server_sock, client_sock, portnum;
    char buffer[256];
    struct sockaddr_in server_addr, client_addr;
    
    socklen_t addr_size;

    if (argc < 2) {
        std::cout << "ERROR: No port num provided.\n";
        exit(1);
    }

    server_sock = socket(AF_INET,SOCK_STREAM,0);
    if (server_sock < 0) {
        perror("Socket error\n");
        exit(1);
    }
    // socket creation success
    std::cout << "Socket Created in Server.\n";

    bzero((char*)&server_addr, sizeof(server_addr));
    portnum = atoi(argv[1]);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portnum);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        std::cout << "Binding Error :(\n";
        exit(1);
    }

    // binding is success
    std::cout << "Binded to port: " << portnum << "\n";

    listen(server_sock,5);
    std::cout << "Listening...\n";
    // waiting for client now

    // from PA 1
    std::string line;
    std::string n_str; // n lines and n number of symbols in this alphabet
    int n, maxNum = 0;
    // std::vector<Data> vecData; // VECTOR OF STRUCT

    std::getline(std::cin, n_str);
    n = stoi(n_str);

    // get all letters and their code value //

    std::map<std::string,Data3> *alphabet = new std::map<std::string,Data3>; // start grabbing the inputs
    std::vector<Data3> charBase10;

    for (int k = 0; k < n; k++) {
        std::getline(std::cin,line); // this is input redirection
        Data3 temp3;
        temp3.base10 = stoi(line.substr(2));
        temp3.let = line[0];
 
        charBase10.push_back(temp3);


        if (maxNum < stoi(line.substr(2))) { // get the max code value
            maxNum = stoi(line.substr(2));
        }
    } 

    // calculate the # of bits used per symbol //
    int numBits = (int)ceil(log2(maxNum + 1));
    // fill map with needed info
    for (int i = 0; i < charBase10.size(); i++) {
        Data3 temp;
        temp.base10 = charBase10[i].base10;
        temp.let = charBase10[i].let;
        std::string binaryRep = toBinary(temp.base10, numBits);
        alphabet->insert(std::pair<std::string, Data3>(binaryRep, temp)); // insert binaryRep as Key and a struct
        // containing relevant info as the Value
    }


    // we need to send the amt of bits to the client 

    // threads will convert the value to binary then count the # of occurences in the message
    
    // now we will recieve a binary representation of the compressed letter
    // we'll use buffer to get the binary rep and send back a character
    // this will have to start the infinite loop so that we can get multiple requests simultaneously
    // we need to differentiate between sending numbit request or decompress letter request

    int whatDo;
    addr_size = sizeof(client_addr);
    // if ((client_sock = accept(server_sock, (struct sockaddr *) &client_addr, (socklen_t *) &addr_size)) < 0) {
    //     std::cout << "Error in accepting socket from client" << std::endl;
    // }
    // std::cout << "Client connected\n";
    std::cout << "We are about to fork\n";
    signal(SIGCHLD, fireman);
    while((client_sock = accept(server_sock, (struct sockaddr *) &client_addr, (socklen_t *) &addr_size)) >= 0) {
        if (fork() == 0) {
            // std::cout << "In fork\n";
            
            bzero(&whatDo, sizeof(whatDo));
            if (read(client_sock, &whatDo, sizeof(whatDo)) < 0) {
                std::cout << "Error in processing what type of request needed\n";
            }
            // std::cout << "At iteration: " << i+1 << " socket recieved whatDo of " << whatDo << std::endl;
            // whatDo = 0 if sending numBits, 1 if dealing with decompression
            if (whatDo == 0) {
                std::cout << "In whatDo = 0\n";

                if (write(client_sock, &numBits, sizeof(numBits)) < 0) {
                    std::cout << "Error in sending numBits to client\n";
                }
            }
            else if (whatDo == 1) {
                // std::cout << "In whatDo = 1\n";

                bzero(&buffer, sizeof(buffer));
                if (read(client_sock, &buffer, sizeof(buffer)) < 0) {
                    std::cout << "Error in recieving binary rep from client\n";
                }
                // we have the binary rep, so go through map and send back the letter
                // std::cout << "This is the binary rep that was recieved: " << buffer << std::endl;

                if (alphabet->find(buffer) == alphabet->end()) {
                    std::cout << "Binary string not found in map" << std::endl;
                    exit(1);
                } 
                // this means that the binary rep is in the map, so we send it back to client
                if (write(client_sock, &(alphabet->find(buffer)->second.let), sizeof(alphabet->find(buffer)->second.let)) < 0) {
                    std::cout << "Error in sending letter to client\n";
                }
                // we have now sent the decompressed letter back to client
            }
            else {
                std::cout << "Uhhhhhhhh whatDo is not a 0 or 1 :/\n";
            }
            close(client_sock);
            _exit(0);
            }
        }
    wait(0);
    close(server_sock);
    delete alphabet;
    return 0;
}