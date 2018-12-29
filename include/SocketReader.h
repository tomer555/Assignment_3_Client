
#ifndef ASSIGNMENT_3_CLIENT_SOCKETREADER_H
#define ASSIGNMENT_3_CLIENT_SOCKETREADER_H

#include <mutex>
#include <condition_variable>
#include "connectionHandler.h"

class SocketReader {
private:
    std::condition_variable & cond;
    std::mutex& mutex;
    ConnectionHandler * handler;
    bool * terminate;
    bool * falseTerminate;


public:
    SocketReader(ConnectionHandler * handler,bool * falseTerminate, bool * terminate,std::condition_variable &cond, std::mutex& mutex);
    void run();
};


#endif //ASSIGNMENT_3_CLIENT_SOCKETREADER_H
