#include <zconf.h>
#include "../include/connectionHandler.h"


using boost::asio::ip::tcp;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

//ConnectionHandler Constructor
ConnectionHandler::ConnectionHandler(string host, short port): host_(host), port_(port), io_service_(), socket_(io_service_){}

//ConnectionHandler Destructor
ConnectionHandler::~ConnectionHandler() {
    close();
}
//--------------Server Connection---------------------------
//connecting to the server using boost package
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

// Close down the connection properly.
void ConnectionHandler::close() {
    try{
        socket_.close();
    } catch (...) {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}


//---------------Sending Message to server------------------------

//Sending line according to its Opcode.
bool ConnectionHandler::sendLine(std::string& line,short Opcode) {
    char *OpByteArr = new char[2];
    shortToBytes(Opcode, OpByteArr);
    bool resultOp = sendBytes(OpByteArr, 2);
    delete[]OpByteArr;
    if (resultOp) {
        switch (Opcode) {
            case 1:
            case 2:
            case 6:
                return sendRegisterLoginFrame(line);
            case 3:
            case 7:
                return true;
            case 4:
                return sendFollowUnfollowFrame(line);
            case 5:
                return sendFrameAscii(line,'\0');


            case 8:
                return sendFrameAscii(line,'\0');
            default:
                return false;
        }
    }
    return false;
}

//Parsing and sending the line according to Register/Login frame (Opcode 1,2)
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

//Parsing and sending the line according to Follow/Unfollow frame (Opcode 4)
bool ConnectionHandler::sendFollowUnfollowFrame(const std::string& line){
    //parsing
    std::string restWithNum=line.substr(line.find(' ')+1);
    std::string usrNumString=restWithNum.substr(0,line.find(' '));
    std::string usrNameList=restWithNum.substr(restWithNum.find(' ')+1);
    const char *followOpcode =line.substr(0, line.find(' ')).c_str();
    short NumOfUsers =boost::lexical_cast<short>(usrNumString);
    char *OpByteArr1 = new char[2];
    shortToBytes(NumOfUsers, OpByteArr1);
    bool charResult =sendBytes(followOpcode,1);
    if (!charResult)return false;
    bool resultNumUsr = sendBytes(OpByteArr1, 2);
    if(!resultNumUsr){
        delete []OpByteArr1;
        return false;
    }
    delete [] OpByteArr1;
    std::vector<std::string> users = splitString(usrNameList, "[ \\s]+");
    for(int i=0;i<users.size();i++){
        bool result=sendFrameAscii(" "+users[i],'\0');
        if(!result)return false;
    }
    return true;
}

//Parsing and sending the line according to PM frame (Opcode 6)
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

// Send a message(UTF-8) to the remote host. separated by the delimiter
// Returns false in case connection is closed before all the data is sent.
bool ConnectionHandler::sendFrameAscii(const std::string& frame, char delimiter) {
    bool result=sendBytes(frame.c_str(),frame.length());
    if(!result) return false;
    return sendBytes(&delimiter,1);
}

// Send a fixed number of bytes from the client - blocking.
// Returns false in case the connection is closed before all the data is sent.
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


//---------------Receiving Message from server-----------------------

//Receiving line according to its Opcode.
bool ConnectionHandler::getLine(std::string& line) {
    char *OpByteArr = new char[2];
    //bool resultOpcode=getFrameTwoByte(OpByteArr);
    bool resultOpcode=getBytes(OpByteArr,2);
    if(!resultOpcode)
        return false;
    short Opcode=bytesToShort(OpByteArr);
    switch (Opcode) {
        case 9:
            return getNotificationFrame(line);
        case 10:
            return getAckFrame(line,OpByteArr);
        case 11:
            return getErrorFrame(line);
        default:
            return false;
    }
}

//Receive and decode the bytes according to Notification frame (Opcode 9)
bool ConnectionHandler::getNotificationFrame(std::string &frame) {
    frame.append("NOTIFICATION ");
    char *OpByteArr = new char[2];
    bool resultOpcode=getBytes(OpByteArr,2);
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

//Receive and decode the bytes according to ACK frame (Opcode 10)
bool ConnectionHandler::getAckFrame(std::string &frame,char *OpByteArr) {
    frame.append("ACK ");
    bool resultOpcode=getBytes(OpByteArr,2);
    short Opcode=bytesToShort(OpByteArr);
    if(!resultOpcode)return false;
    frame.append(boost::lexical_cast<std::string>(Opcode)+" ");
    switch (Opcode){
        case 8: {
            bool numPostsResult = getBytes(OpByteArr, 2);
            if (!numPostsResult)
                return false;
            short numPosts = bytesToShort(OpByteArr);
            frame.append(boost::lexical_cast<std::string>(numPosts) + " ");
            bool numOfFollowersResult = getBytes(OpByteArr, 2);
            if (!numOfFollowersResult)
                return false;
            short numOfFollowers = bytesToShort(OpByteArr);
            frame.append(boost::lexical_cast<std::string>(numOfFollowers) + " ");
            bool numOfFollowingResult = getBytes(OpByteArr, 2);
            if (!numOfFollowingResult)
                return false;
            short numOfFollowing = bytesToShort(OpByteArr);
            frame.append(boost::lexical_cast<std::string>(numOfFollowing));
            return true;
        }
        case 4:
        case 7:
            bool numOfUsersResult=getBytes(OpByteArr,2);
            if(!numOfUsersResult)
                return false;
            short numOfUsers=bytesToShort(OpByteArr);
            frame.append(boost::lexical_cast<std::string>(numOfUsers));
            for(int i=0;i<numOfUsers;i++){
                std::string user;
                bool resultUser=getFrameAscii(user,'\0');
                if(!resultUser) return false;
                frame.append(' '+user);
            }
            return true;
    }
    return false;

}

//Receive and decode the bytes according to Error frame (Opcode 11)
bool ConnectionHandler::getErrorFrame(std::string &frame) {
    frame.append("Error ");
    char *OpByteArr = new char[2];
    bool resultOpcode=getBytes(OpByteArr,2);
    delete []OpByteArr;
    if(!resultOpcode)return false;
    short Opcode=bytesToShort(OpByteArr);
    frame.append(boost::lexical_cast<std::string>(Opcode));
    return true;
}

// Get Ascii data from the server until the delimiter character
// Returns false in case connection closed before null can be read.
bool ConnectionHandler::getFrameAscii(std::string& frame, char delimiter) {
    char ch;
    // Stop when we encounter the null character.
    // Notice that the null character is not appended to the frame string.
    try {
        do{
            getBytes(&ch, 1);
            if(ch!='\0')
                frame.append(1, ch);
        }while (delimiter != ch);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}



// Read a fixed number of bytes from the server - blocking.
// Returns false in case the connection is closed before bytesToRead bytes can be read.
bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
    size_t tmp = 0;
	boost::system::error_code error;
    try {
        while (!error && bytesToRead > tmp )
			tmp += socket_.read_some(boost::asio::buffer(bytes+tmp, bytesToRead-tmp), error);
		if(error)
			throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

//-----------------byteToshort\ShortToByte--------------
short ConnectionHandler::bytesToShort(char* bytesArr){
    short result = (short)((bytesArr[0] & 0xff) << 8);
    result += (short)(bytesArr[1] & 0xff);
    return result;
}

void ConnectionHandler:: shortToBytes(short num, char* bytesArr){
    bytesArr[0] = ((num >> 8) & 0xFF);
    bytesArr[1] = (num & 0xFF);
}


//Static Function to convert String to Opcode
short ConnectionHandler:: StringToOpcode(const std::string & s){
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

//Splits a string on a given regex expression
std::vector<std::string> ConnectionHandler:: splitString(const std::string& stringToSplit, const std::string& regexPattern)
{
    std::vector<std::string> result;
    const std::regex rgx(regexPattern);
    std::sregex_token_iterator iter(stringToSplit.begin(), stringToSplit.end(), rgx, -1);

    for (std::sregex_token_iterator end; iter != end; ++iter)
    {
        result.push_back(iter->str());
    }

    return result;
}






 

 
















