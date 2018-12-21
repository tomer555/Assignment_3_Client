#ifndef CONNECTION_HANDLER__
#define CONNECTION_HANDLER__
                                           
#include <string>
#include <iostream>
#include <regex>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

using boost::asio::ip::tcp;

//class ConnectionHandler
class ConnectionHandler {
private:
	const std::string host_;
	const short port_;
	boost::asio::io_service io_service_;   // Provides core I/O functionality
	tcp::socket socket_; 
 
public:
    ConnectionHandler(std::string host, short port);
    virtual ~ConnectionHandler();
    short StringToOpcode(const std::string & s);
    // Connect to the remote machine
    bool connect();
 
    // Read a fixed number of bytes from the server - blocking.
    // Returns false in case the connection is closed before bytesToRead bytes can be read.
    bool getBytes(char bytes[], unsigned int bytesToRead);
 
	// Send a fixed number of bytes from the client - blocking.
    // Returns false in case the connection is closed before all the data is sent.
    bool sendBytes(const char bytes[], int bytesToWrite);
	
    // Read an ascii line from the server
    // Returns false in case connection closed before a newline can be read.
    bool getLine(std::string& line);
	
	// Send an ascii line from the server
    // Returns false in case connection closed before all the data is sent.
    bool sendLine(std::string& line,short Opcode);
 
    // Get Ascii data from the server until the delimiter character
    // Returns false in case connection closed before null can be read.
    bool getFrameAscii(std::string& frame, char delimiter);



    bool getNotificationFrame(std::string& frame);

    bool getAckFrame(std::string& frame,char *OpByteArr);

    bool getErrorFrame(std::string& frame,char *OpByteArr);
 
    // Send a message to the remote host.
    // Returns false in case connection is closed before all the data is sent.
    bool sendFrameAscii(const std::string& frame, char delimiter);


    short bytesToShort(char* bytesArr);


    void shortToBytes(short num, char* bytesArr);

    bool sendPmRegisterLoginFrame(const std::string& line);

    bool getShortAndAppend(bool prefix,bool suffix,char* bytesArr,std::string);

    bool sendFollowUnfollowFrame(const std::string& line);

    // Close down the connection properly.
    void close();
    std::vector<std::string> splitString(const std::string& stringToSplit, const std::string& regexPattern);
};
 
#endif