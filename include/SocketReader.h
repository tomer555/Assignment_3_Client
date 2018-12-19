

#ifndef ASSIGNMENT_3_CLIENT_SOCKETREADER_H
#define ASSIGNMENT_3_CLIENT_SOCKETREADER_H

#include "connectionHandler.h"

class SocketReader {
private:
    ConnectionHandler * handler;
public:
    SocketReader(ConnectionHandler * handler);
    void run();
};


#endif //ASSIGNMENT_3_CLIENT_SOCKETREADER_H
