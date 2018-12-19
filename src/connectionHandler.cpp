#include "../include/connectionHandler.h"
#include "../include/BGSClient.h"

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
    char *OpByteArr = new char[2];
    bool resultOpcode=getFrameTwoByte(OpByteArr);
    if(!resultOpcode)
        return false;
    short Opcode=bytesToShort(OpByteArr);
    switch (Opcode) {
        case 9:
            return getNotificationFrame(line);
        case 10:
            return getAckFrame(line);
        case 11:
            return getErrorFrame(line);
        default:
            return false;
    }

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




bool ConnectionHandler::getFrameTwoByte(char* bytesArr){
    char ch1;
    char ch2;
    try {
        bytesArr[0]=getBytes(&ch1,1);
        bytesArr[1]=getBytes(&ch2,1);
    }
    catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
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
    //sending
    bool resultUSR=sendFrameAscii(username,'\0');
    if(!resultUSR) return false;
    bool resultCNT=sendFrameAscii(content,'\0');
    return resultCNT;
}

bool ConnectionHandler::sendPostFrame(const std::string& line){
    return sendFrameAscii(line,'\0');
}


bool ConnectionHandler::sendFollowUnfollowFrame(const std::string& line){
    //parsing
    std::string fopcodeString = line.substr(0, line.find(' '));//Follow Opcode
    std::string restWithNum=line.substr(line.find(' '));
    std::string usrNumString=restWithNum.substr(0,line.find(' '));
    std::string usrNameList=restWithNum.substr(line.find(' '));
    short followOpcode =StringToOpcode(fopcodeString);
    short NumOfUsers =StringToOpcode(usrNumString);
    char *OpByteArr1 = new char[2];
    char *OpByteArr2 = new char[2];

    shortToBytes(followOpcode, OpByteArr1);
    shortToBytes(NumOfUsers, OpByteArr2);

    bool resultOp = sendBytes(OpByteArr1, 2);
    if(!resultOp){
        delete []OpByteArr1;
        delete []OpByteArr2;
        return false;
    }
    bool resultNum = sendBytes(OpByteArr2, 2);
    if(!resultNum){
        delete []OpByteArr1;
        delete []OpByteArr2;
        return false;
    }
    return sendFrameAscii(usrNameList,'\0');
}

bool ConnectionHandler::sendStatFrame(const std::string &line) {
    return sendFrameAscii(line,'\0');
}

bool ConnectionHandler::getNotificationFrame(std::string &frame) {
    frame.append("NOTIFICATION ");
    char *OpByteArr = new char[2];
    bool resultOpcode=getFrameTwoByte(OpByteArr);
    delete []OpByteArr;
    if(!resultOpcode)return false;
    short Opcode=bytesToShort(OpByteArr);
    if (Opcode==0)
        frame.append("PM ");
    else
        frame.append("Public ");
    std::string postingUser;
    bool resultPosting=getFrameAscii(postingUser,'\0');
    if(!resultPosting)return false;
    frame.append(postingUser+" ");
    std::string content;
    bool resultContent=getFrameAscii(content,'\0');
    if(!resultContent)return false;
    frame.append(content);
    return true;
}

bool ConnectionHandler::getAckFrame(std::string &frame) {
    frame.append("ACK ");
    char *OpByteArr = new char[2];
    bool resultOpcode=getFrameTwoByte(OpByteArr);
    delete []OpByteArr;
    if(!resultOpcode)return false;
    short Opcode=bytesToShort(OpByteArr);
    frame.append(boost::lexical_cast<std::string>(Opcode));
    std::string optional;
    bool resultOptinal=getFrameAscii(optional,'\0');
    if(!resultOptinal)return false;
    frame.append(optional);
    return true;
}

bool ConnectionHandler::getErrorFrame(std::string &frame) {
    frame.append("Error ");
    char *OpByteArr = new char[2];
    bool resultOpcode=getFrameTwoByte(OpByteArr);
    delete []OpByteArr;
    if(!resultOpcode)return false;
    short Opcode=bytesToShort(OpByteArr);
    frame.append(boost::lexical_cast<std::string>(Opcode));
    return true;
}


