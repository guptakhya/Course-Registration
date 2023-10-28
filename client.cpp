/*
The data structures and scoket programming functions of the following code has been referenced from
https://beej.us/guide/bgnet/html/, https://www.youtube.com/watch?v=cNdlrbZSkyQ&list=RDCMUC4LMPKWdhfFlJrJ1BHmRhMQ&start_radio=1&rv=cNdlrbZSkyQ&t=2559,
https://www.youtube.com/watch?v=fmn-pRvNaho&list=RDCMUC4LMPKWdhfFlJrJ1BHmRhMQ&index=2
*/

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <vector>

using namespace std;

void sendToServer(string userInput, int sock);
string receiveFromServer(int sock);
string getUserInput(bool isUName); // inputType true = username, false = password
void error(string msg);
void displayAuthMsg(string authMsg, int &numAttempts, int portNum);
int getSockNum(int TCP_Connect_Sock);
void handleCourseDetails(int socketNum, int portNum);
void processSingleInput(vector<string> course, int socketNum, int portNum);
void processMultipleInput(vector<string> course, int socketNum, int portNum);

//global scope since used across the code and can have only 1 user at a time
string username;

int main(){
    
    cout << "The client is up and running." << endl;
    int numAttempts = 3;
    //create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket == -1){
        error("Could not create socket.");
    }

    //creating a hint server to connect with the server
    int serverPort = 25887; //25000 + 887 (last 3 digits of my USC ID)
    string localIP = "127.0.0.1"; //loopback IP Address

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(serverPort);
    inet_pton(AF_INET, localIP.c_str(), &hint.sin_addr);

    //Connecting with server
    int connectToServer = connect(clientSocket, (sockaddr*)&hint, sizeof(hint));
    if(connectToServer == -1){
        error("Could not connect to server");
    }

    int myPort = getSockNum(clientSocket);
    string password;

    while(numAttempts > 0){
        username = getUserInput(true);
        password = getUserInput(false);
        //send input to the server as comma separated strings
        //to distinguish requests for different servers, using 'Type1:' to indicate for serverC
        string creds = "Type1:" + username + "," + password;
        sendToServer(creds, clientSocket);
        cout << username << " sent an authentication request to the main server." << endl;
        //get response from server
        string authCheck = receiveFromServer(clientSocket);
        displayAuthMsg(authCheck, numAttempts, myPort);
        if(authCheck == "Auth Passed"){
            break; // break if authorisation was successful
        }
    }
    if(numAttempts == 0){
        cout << "Authentication Failed for 3 attempts. Client will shut down." << endl;
        close(clientSocket);
        return 0;
    }

    //to get details about the courses 
    handleCourseDetails(clientSocket, myPort);

    //close the socket
    close(clientSocket);

    return 0;
}

//sends data to serverM
void sendToServer(string userInput, int sock){
    int sendRes = send(sock, userInput.c_str(), userInput.size()+1, 0); // +1 to account for the trailing 0 of c_str()
    if(sendRes == -1){
        error("Sending to server failed.");
    }
}

//receive data from server
string receiveFromServer(int sock){
    char buff[4096];
    memset(buff, 0, 4096);
    int receivedBytes = recv(sock, buff, 4096, 0);
    if(receivedBytes == -1){
        error("Error getting response from the main server");
    }
    return string(buff,0,receivedBytes);
}

/*
    takes in the username/password as the input from the user
    validates if the username/password are wof length b/w 5 and 50 characters
    validates if the username is all lower case
    if invalid, gives 3 attempts to the user to enter valid credentials
    returns the username/password if valid in 3 attempts
    returns "inv" otherwise
*/
string getUserInput(bool isUName){
    int attempts = 3;
    bool isValid = true;
    string userInput;
    string inputType;
    if(isUName){
        inputType = "username";
    }
    else{
        inputType = "password";
    }

    while(attempts > 0){
        cout << "Please enter the " << inputType << ": ";
        cin >> userInput;
        isValid = true;
        if(userInput.size() < 5 || userInput.size() > 50){
            isValid = false;
            cout<< "Invalid input: length should be between 5 and 50. Attempts remaining: " << --attempts << endl;
        }
        else if(isUName){
            for(int i = 0; i < userInput.size(); i++){
                if(!islower(userInput[i])){
                    isValid = false;
                    break;
                }
            }
            if(!isValid){
                cout<< "Invalid input: username should be lowercase. Attempts remaining: " << --attempts << endl;
            }
        }
        if(isValid){
            break;
        }
    }
    if(isValid){
        return userInput;
    }
    else{
        error("Too many failed attempts. Ending program");
    }
}

/*
    function to display the text after credential verification from server
    reduces numAttempts if not successful
*/
void displayAuthMsg(string authMsg, int &numAttempts, int portNum){
    if(authMsg == "Auth Passed"){
        cout << username << " received the result of authentication using TCP over port " << portNum << ". Authentication is successful" << endl;
    }
    else if(authMsg == "Invalid Username"){
        cout << username << " received the result of authentication using TCP over port " << portNum << ". Authentication failed: Username Does not exist\n" << endl;
        cout << "Attempts remaining: " << --numAttempts << endl; 
    }
    else{
        cout << username << " received the result of authentication using TCP over port " << portNum << ". Authentication failed: Password does not match\n" << endl;
        cout << "Attempts remaining: " << --numAttempts << endl;
    }
}

//fetch the dynamically created port number for the client
int getSockNum(int TCP_Connect_Sock){
    sockaddr_in my_addr;
    socklen_t addrlen = sizeof(my_addr);
    //Code for getting port number used from the Project description file provided
    /*Retrieve the locally-bound name of the specified socket and store it in the
    sockaddr structure*/
    int getsock_check = getsockname(TCP_Connect_Sock, (struct sockaddr*)&my_addr,(socklen_t *)&addrlen);
    //Error checking
    if (getsock_check== -1) {
        error("getsockname");
    }
    return my_addr.sin_port;
}

/*
Takes in the user input for the course they want to get information for and the category (if single courseCode entered)
Any input with leading or trailing spaces is invalid and user has to enter the course details again
Based on single/multiple input, process the input accordingly
continues the same process for more inputs
*/
void handleCourseDetails(int socketNum, int portNum){
    cin.ignore(); //clear any previous input charachters
    while(true){
        cout << "Please enter the course code to query: ";
        char courseCodeArr[60]; //60 is the max input size since user can enter upto max 10 courses (10*5 + 9 spaces, 1 additional)
        
        cin.getline(courseCodeArr, 60); //60 since max 10 codes can be entered

        string courseCode(courseCodeArr); // converting char array to string
        if(courseCode[0] == ' ' || courseCode[courseCode.size()-1] == ' '){
            cout << "Invalid entry, there should not be any leading or trailing spaces." << endl;
            continue; // prompts user to enter the course code again
        }
        else{
            vector<string> courseList;
            //the following code was referred from https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
            int start = 0, end = 0;
            while(end != -1){
                end = courseCode.find(" ", start);
                courseList.push_back(courseCode.substr(start, end-start));
                start = end+1;
            }
            if(courseList.size() == 1){
                processSingleInput(courseList, socketNum, portNum);
            }
            else{
                processMultipleInput(courseList, socketNum, portNum);
            }
        }
    }
}

//processing the single entry for course
void processSingleInput(vector<string> course, int socketNum, int portNum){
    string courseType = course[0].substr(0,2);
    string toSend;
    if(courseType == "EE"){
        toSend += "Type2:" + course[0] += ","; // to indicate single EE course
    }
    else if(courseType == "CS"){
        toSend += "Type3:" + course[0] += ","; // to indicate single CS course
    }
    else{
        cout << "Invalid course code, should begin with EE or CS." << endl;
        return;
    }
    cout << "Please enter the category (Credit / Professor / Days / CourseName):";
    string category;
    getline(cin,category);
    //Category is case sensitive and will only be valid if matches as is.
    if(category != "Credit" && category != "Professor" && category != "Days" && category != "CourseName"){
        cout << "Invalid Category." << endl; // will prompt user to re-enter details from the course code
        return;
    }
    toSend += category;
    sendToServer(toSend, socketNum);
    cout << username << " sent a request to the main server." << endl;
    //recieve response from server
    string courseDetails = receiveFromServer(socketNum);
    cout << "The client received the response from the Main server using TCP over port " << portNum << "." << endl;
    cout << courseDetails << "\n" << endl;

    cout << "-----Start a new request-----" << endl;
}

//processing the single entry for course
void processMultipleInput(vector<string> course, int socketNum, int portNum){
    
    string toSend;
    toSend += "Type4:"; //to identify multiple course request
    for(int i=0; i<course.size()-1; i++){
        toSend += course[i];
        toSend += ",";
    }
    toSend += course[course.size()-1];

    sendToServer(toSend, socketNum);
    cout << username << " sent a request with multiple CourseCode to the main server." << endl;
    //recieve response from server
    string courseDetails = receiveFromServer(socketNum);
    cout << "The client received the response from the Main server using TCP over port " << portNum << "." << endl;
    cout << "CourseCode: Credits, Professor, Days, Course Name" << endl;
    //following logic referred to from https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    int start = 0, end = 0;
    string delim = "\n";
    while(end != -1){
        end = courseDetails.find("\n", start);
        cout << courseDetails.substr(start, end-start) << endl;
        start = end + delim.length();
    }

    cout << "\n" << "-----Start a new request-----" << endl;
}

//prints error message and exits the program
void error(string msg){
    cout << msg << endl;
    exit(0);
}