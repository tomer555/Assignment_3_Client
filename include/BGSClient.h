//
// Created by tomer on 19/12/18.
//

#ifndef ASSIGNMENT_3_CLIENT_BGSCLIENT_H
#define ASSIGNMENT_3_CLIENT_BGSCLIENT_H

#include "connectionHandler.h"

void ReadFromSocket(ConnectionHandler & connectionHandler);
static short StringToOpcode(const std::string & s);




#endif //ASSIGNMENT_3_CLIENT_BGSCLIENT_H
