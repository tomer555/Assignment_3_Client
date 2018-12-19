#include "../include/connectionHandler.h"
using boost::asio::ip::tcp;

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
 
ConnectionHandler::ConnectionHandler(string host, short port): host_(host), port_(port), io_service_(), socket_(io_service_){}
    
ConnectionHandler::~ConnectionHandler() {
    close();
}
 
bool ConnectionHandler::connect() {
    std::cout << "Starting connect to " 
        << host_ << ":" << port_ << std::endl;
    try {
		tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
		boost::system::error_code error;
		socket_.connect(endpoint, error);
		if (error)
			throw boost::system::system_error(error);
    }
    catch (std::exception& e) {
        std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}
 
bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
    size_t tmp = 0;
	boost::system::error_code error;
    try {
        while (!error && bytesToRead > tmp ) {
			tmp += socket_.read_some(boost::asio::buffer(bytes+tmp, bytesToRead-tmp), error);			
        }
		if(error)
			throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
    int tmp = 0;
	boost::system::error_code error;
    try {
        while (!error && bytesToWrite > tmp ) {
			tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
        }
		if(error)
			throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}
 
bool ConnectionHandler::getLine(std::string& line) {
    return getFrameAscii(line, '\n');
}

bool ConnectionHandler::sendLine(std::string& line,short Opcode) {
    char *OpByteArr = new char[2];
    shortToBytes(Opcode, OpByteArr);
    bool resultOp = sendBytes(OpByteArr, 2);
    delete[]OpByteArr;
    if (resultOp) {
        switch (Opcode) {
            case 1:
            case 2:
                return sendRegisterLoginFrame(line);
            case 3:
            case 7:
                return true;
            case 4:
                return sendFollowUnfollowFrame(line);
            case 5:
                return sendPostFrame(line);
            case 6:
                return sendPmFrame(line);
            case 8:
                return sendStatFrame(line);
            default:
                return false;
        }
    }
    return false;
}
 
bool ConnectionHandler::getFrameAscii(std::string& frame, char delimiter) {
    char ch;
    // Stop when we encounter the null character. 
    // Notice that the null character is not appended to the frame string.
    try {
		do{
			getBytes(&ch, 1);
            frame.append(1, ch);
        }while (delimiter != ch);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}
 
bool ConnectionHandler::sendFrameAscii(const std::string& frame, char delimiter) {
	bool result=sendBytes(frame.c_str(),frame.length());
	if(!result) return false;
	return sendBytes(&delimiter,1);
}
 
// Close down the connection properly.
void ConnectionHandler::close() {
    try{
        socket_.close();
    } catch (...) {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}

short ConnectionHandler::bytesToShort(char* bytesArr)
{
    short result = (short)((bytesArr[0] & 0xff) << 8);
    result += (short)(bytesArr[1] & 0xff);
    return result;
}


void ConnectionHandler:: shortToBytes(short num, char* bytesArr)
{
    bytesArr[0] = ((num >> 8) & 0xFF);
    bytesArr[1] = (num & 0xFF);
}

bool ConnectionHandler::sendRegisterLoginFrame(const std::string& line) {
    //parsing
    std::string username=line.substr(0,line.find(' '));
    std::string password=line.substr(line.find(' '));
    //sending
    bool resultUSR=sendFrameAscii(username,'\0');
    if(!resultUSR)
        return false;
    bool resultPSW=sendFrameAscii(password,'\0');
    return resultPSW;
}

bool ConnectionHandler::sendPmFrame(const std::string& line){
    //parsing
    std::string username=line.substr(0,line.find(' '));
    std::string content=line.substr(line.find(' '));

    bool resultUSR=sendFrameAscii(username,'\0');
    if(!resultUSR) return false;
    bool resultCNT=sendFrameAscii(content,'\0');
    return resultCNT;
}

bool ConnectionHandler::sendPostFrame(const std::string& line){
    //parsing


    return sendFrameAscii(content,'\0');
}


bool ConnectionHandler::sendFollowUnfollowFrame(const std::string& line){
    //parsing



}

bool ConnectionHandler::sendStatFrame(const std::string &line) {
    //parsing

    return sendFrameAscii(username,'\0');

}


