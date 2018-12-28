#include <stdlib.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../include/connectionHandler.h"
#include "../include/SocketReader.h"

/**
 * BGS Client:
* This code uses 2 threads:
 * thread0: running the main, establish connection with the server and getting input from user.
 * thread1: running ReadFromSocket, print all the input that comes from the server.
*/

int main (int argc, char *argv[]) {
    //Checking args : host and port
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port =boost::lexical_cast<short>(argv[2]);

    //Establishing connection with a server
    ConnectionHandler *connectionHandler=new ConnectionHandler (host, port);
    if (!connectionHandler->connect()) {
        return 1;
    }
    //Creating thread1 : reading from Socket and bool flag to know ACK 3 got accepted
    bool * terminate= new bool(false);
    bool * falseTerminate= new bool(false);
    std::mutex mutex;

    std::condition_variable cond;
    SocketReader reader(connectionHandler,falseTerminate,terminate,cond,mutex);
    std::thread th1(&SocketReader::run,&reader);


	//Accepting input string from user. Exit on command: LOGOUT
	std::string line;
    while (!(*terminate) || (line!=("LOGOUT")))  {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        line=buf;//Copy assignment
        std::string message;
        std::string first_token = line.substr(0, line.find(' '));
        if(line.find(' ')!= std::string::npos)
            message = line.substr (line.find(' ')+1);
        short Opcode =connectionHandler->StringToOpcode(first_token);
        if(Opcode!=0) {
            if (!connectionHandler->sendLine(message,Opcode)) {
                break;
            }
        } else
            std::cout << "No such Command exists: " << first_token << ", please try again" << std::endl;
       if(line=="LOGOUT") {
           std::unique_lock<std::mutex> lock {mutex};
           while (!(*terminate) && !(*falseTerminate)) {
               cond.wait(lock);
           }
           (*falseTerminate)= false;
       }
    }
    //waiting for thread1 to finish: waiting server response : ACK 3(LOGOUT)
    th1.join();
    //Closing the connection with the server
    delete connectionHandler;
    delete terminate;
    return 0;
}












