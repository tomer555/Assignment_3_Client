

#include "../include/SocketReader.h"
SocketReader::SocketReader(ConnectionHandler *handler,bool * falseTerminate, bool * terminate,std::condition_variable & cond, std::mutex& mutex):cond(cond),mutex(mutex),handler(handler),terminate(terminate),falseTerminate(falseTerminate) {}
void SocketReader::run() {

    while (true){

        // We can use one of three options to read data from the server:
        // 1. Read a fixed number of characters
        // 2. Read a line (up to the newline character using the getline() buffered reader
        // 3. Read up to the null character
        std::string answer;
        // Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
        // We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end
        if (!handler->getLine(answer)) {
            break;
        }

        // A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
        // we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.
        std::cout << answer << std::endl;

        if(answer=="Error 3"){
            std::unique_lock<std::mutex> lock {mutex};
            (*falseTerminate)= true;
            cond.notify_all();
        }
        if (answer=="ACK 3") {
            std::unique_lock<std::mutex> lock {mutex};
            (*terminate) = true;
            cond.notify_all();
            break;
        }
    }
}








