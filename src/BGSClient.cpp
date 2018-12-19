#include <stdlib.h>
#include <thread>
#include "../include/connectionHandler.h"
#include "../include/BGSClient.h"

/**
* This code assumes that the server replies the exact text the client sent it (as opposed to the practical session example)
*/

int main (int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port = atoi(argv[2]);
    
    ConnectionHandler *connectionHandler=new ConnectionHandler (host, port);
    if (!connectionHandler->connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }

    std::thread th1(ReadFromSocket,connectionHandler);

	//From here we will see the rest of the ehco client implementation:
	std::string line;
    while (line.find("LOGOUT")== std::string::npos) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        line=buf;//Copy assignment
        std::string first_token = line.substr(0, line.find(' '));
        std::string message = line.substr (line.find(' '));
        short Opcode =StringToOpcode(first_token);
        if(Opcode!=0) {
            int len = line.length();
            if (!connectionHandler->sendLine(message,Opcode)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            // connectionHandler.sendLine(line) appends '\n' to the message. Therefor we send len+1 bytes.
            std::cout << "Sent " << len + 1 << " bytes to server" << std::endl;
        } else
            std::cout << "No such Command exists: " << first_token << ", please try again" << std::endl;
    }
    return 0;
}



void ReadFromSocket(ConnectionHandler & connectionHandler){
    while (true){
        // We can use one of three options to read data from the server:
        // 1. Read a fixed number of characters
        // 2. Read a line (up to the newline character using the getline() buffered reader
        // 3. Read up to the null character
        std::string answer;
        int len;
        // Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
        // We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end
        if (!connectionHandler.getLine(answer)) {
            std::cout << "Disconnected. Exiting...\n" << std::endl;
            break;
        }

        len=answer.length();
        // A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
        // we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.
        answer.resize(len-1);
        std::cout << answer << std::endl;
        if (answer.find("ACK 3")!= std::string::npos) {
            std::cout << "Exiting...\n" << std::endl;
            break;
        }

    }


}

short StringToOpcode(const std::string & s){
    if(s=="REGISTER")
        return 1;
    else if(s=="LOGIN")
        return 2;
    else if(s=="LOGOUT")
        return 3;
    else if(s=="FOLLOW")
        return 4;
    else if(s=="POST")
        return 5;
    else if(s=="PM")
        return 6;
    else if(s=="USERLIST")
        return 7;
    else if(s=="STAT")
        return 8;
    else if(s=="NOTIFICATION")
        return 9;
    else if(s=="ACK")
        return 10;
    else if(s=="ERROR")
        return 11;
    return 0;
}



