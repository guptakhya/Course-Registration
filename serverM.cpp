/*
The data structures and scoket programming functions of the following code has been referenced from
https://beej.us/guide/bgnet/html/, https://www.youtube.com/watch?v=cNdlrbZSkyQ&list=RDCMUC4LMPKWdhfFlJrJ1BHmRhMQ&start_radio=1&rv=cNdlrbZSkyQ&t=2559,
https://www.youtube.com/watch?v=fmn-pRvNaho&list=RDCMUC4LMPKWdhfFlJrJ1BHmRhMQ&index=2, https://www.linuxhowtos.org/data/6/server_udp.c and
https://www.linuxhowtos.org/data/6/client_udp.c
*/

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <vector>
#include <set>

using namespace std;

string encryptData(string input);
int bindSocket(sockaddr_in hint, int sock);
void error(string msg);
void executeServerPath(int sockM, string credsToCheck, int clientSock, int destPort, string serverName);
vector<string> getAllCoursesList(string input);
void executeMultipleServersPath(int sockM, vector<string> courseList, int clSocket);
void sendToUDPServer(int sockM, string data, int destPort);
string receiveFromUDPServer(int sockM);
void sendToClient(int clientSock, string data);
vector<string> convertToList(string courses);

const int SERVEREE_PORT = 23887; //23000 + 887 (last 3 digits of my USC ID)
const int SERVERCS_PORT = 22887; // 22000 + 887 
const int SERVERC_PORT = 21887; // 21000 + 887

int main(){

    cout << "The main server is up and running." << endl;
    
    //creating TCP socket
    int tcpsocketM = socket(AF_INET, SOCK_STREAM, 0);
    //in case of failure of creating socket
    if(tcpsocketM == -1){
        error("Could not create then socket.");
    }

    sockaddr_in tcpServerAddr;
    memset(&tcpServerAddr, 0, sizeof(tcpServerAddr));
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons(25887); //serverM TCP port number (25000 + 887)
    inet_pton(AF_INET, "0.0.0.0", &tcpServerAddr.sin_addr); //"0.0.0.0" to listen to any
    //Binding tcp socket:
    int bindTcp = bindSocket(tcpServerAddr, tcpsocketM);
    
    while(true){
        
        //start listening on the tcp socket:
        if(listen(tcpsocketM, 1)){ //listen to max 1 client at a time
            error("Failed to listen");
        }
    
        //accept a tcp client:
        sockaddr_in client;
        memset(&client,0,sizeof(client));
        socklen_t clSize = sizeof(client);
        char host[NI_MAXHOST];
        char service[NI_MAXSERV];

        int clSocket = accept(tcpsocketM, (sockaddr*)&client, &clSize);
        //if accept failed:
        if(clSocket == -1){
            error("Could not connect to client.");
        }

        //clean up any garbage before
        memset(host, 0, NI_MAXHOST);
        memset(service, 0, NI_MAXSERV);

        //create UDP client socket
        int udpSocketM = socket(AF_INET, SOCK_DGRAM, 0);
        if(udpSocketM == -1){
            error("Could not create the UDP socket.");
        }
        
        //source/client udp server address struct
        sockaddr_in srcAddr;
        memset(&srcAddr, 0, sizeof(srcAddr));
        srcAddr.sin_family = AF_INET;
        srcAddr.sin_port = htons(24887); //24000 + 887 (last 3 digits of my ID)
        srcAddr.sin_addr.s_addr = INADDR_ANY;

        //need to bind UDP client to use static port number instead of dynamic
        int bindUdpSrc = bindSocket(srcAddr, udpSocketM);
        
        //receiving - display message
        char buff[4096];
        string username;

        while(true){
            memset(buff, 0, 4096);
            //wait for a message from tcp client
            int recCreds = recv(clSocket, buff, 4096, 0);
            if(recCreds <= 0){
                close(clSocket); //close child socket when disconnected
                break; //break from this loop to go back to listening 
            }
            //process input and display respective on screen messages
            string inputFromClient = string(buff, 0, recCreds); // converts char array to string
            string trimmed = inputFromClient.substr(6);//removes the "TypeX:" part of the string to fetch the actual input
            if(inputFromClient.substr(0,6) == "Type1:"){//Type1 request - Main should contact serverC to verify credentials
                username = trimmed.substr(0, trimmed.find(","));
                cout << "The main server received the authentication for " << username << " using TCP over port 25887." << endl;
                //cout << "Received: " << trimmed << endl;
                string encryptedCreds = encryptData(trimmed);
                executeServerPath(udpSocketM, encryptedCreds, clSocket, SERVERC_PORT, "serverC");
                cout << "The main server sent the authentication result to the client." << endl;
            }
            else if(inputFromClient.substr(0,6) == "Type2:"){ //Type2 represents single course request to serverEE
                int nextEnd = trimmed.find(",");
                string courseCode = trimmed.substr(0,nextEnd);
                string category = trimmed.substr(nextEnd+1);
                string data = "0" + trimmed; //appending 0 for server to recognise single course request
                cout << "The main server received from " << username << " to query course " << courseCode << " about " << category << " using TCP over port 25887." << endl;
                //cout << "Received: " << trimmed << endl;
                executeServerPath(udpSocketM, data, clSocket, SERVEREE_PORT, "serverEE");
                cout << "The main server sent the query information to the client." << endl;
            }
            else if(inputFromClient.substr(0,6) == "Type3:"){ //Type3 represents single course request to serverCS
                int nextEnd = trimmed.find(",");
                string courseCode = trimmed.substr(0,nextEnd);
                string category = trimmed.substr(nextEnd+1);
                string data = "0" + trimmed; //appending 0 for server to recognise single course request
                cout << "The main server received from " << username << " to query course " << courseCode << " about " << category << " using TCP over port 25887." << endl;
                //cout << "Received: " << trimmed << endl;
                executeServerPath(udpSocketM, data, clSocket, SERVERCS_PORT, "serverCS");
                cout << "The main server sent the query information to the client." << endl;
            }
            else if(inputFromClient.substr(0,6) == "Type4:"){ //Type4 represents multiple courses request
                
                vector<string> courseList = getAllCoursesList(trimmed); //converts the string input to a list of all courseCodes
                cout << "The main server received from " << username << " a request with multiple CourseCodes using TCP over port 25887." << endl; //modified slightly for multiple courses
                executeMultipleServersPath(udpSocketM, courseList, clSocket);
                cout << "The main server sent the query information to the client." << endl;
            }
        }
        
    }
    close(tcpsocketM);
    return 0;
}

//function to encrypt credentials before sending to serverC
string encryptData(string a){
    char arr[a.size()];
	for(int i=0; i<a.size(); i++){
		if((a[i] >=97 && a[i] <=118) || (a[i] >=65 && a[i] <=86) || (a[i] >=48 && a[i] <=53)){
			arr[i] = (a[i]+4); //add 4 if not cyclic addition
		}
		else if((a[i] >118 && a[i] <=122) || (a[i] >86 && a[i] <=90) || (a[i] >53 && a[i] <=57)){
			int startVal = a[i] < 58 ? 47 : (a[i] < 91 ? 64 : 96);
			int endVal = a[i] < 58 ? 57 : (a[i] < 91 ? 90 : 122);
			arr[i] = startVal + 4 - (endVal-a[i]); //moving by 4 for cyclic addition
		}
		else{
			arr[i] = a[i]; // not changing anything for special characters
		}
	}
	return string(arr);
}

//function to bind a socket with the port
int bindSocket(sockaddr_in hint, int sock){
    int bound = bind(sock, (sockaddr*)&hint, sizeof(hint));
    //verifying tcp binding was successful:
    if(bound == -1){
        error("Could not bind to the port/IP");
    }
    return bound;
}

/*
    Connects to server (C, EE or CS)
    Sends data to that server
    Receives response from that server
    Forwards response to client
    Displays some messages on the main server terminal
*/
void executeServerPath(int sockM, string data, int clientSock, int destPort, string serverName){

    //send data to the UDP server based on the Port number destPort
    sendToUDPServer(sockM, data, destPort);
    //print on screen Message
    if(serverName == "serverC"){
        cout << "The main server sent an authentication request to serverC." << endl;
    }
    else{
        cout << "The main server sent a request to " << serverName << "." << endl;
    }

    //receive from UDP from server
    string respMessage = receiveFromUDPServer(sockM);
    //print on screen Message
    if(serverName == "serverC"){
        cout << "The main server received the result of the authentication request from " << serverName << " using UDP over port 24887." << endl;
    }
    else{
        cout << "The main server received the response from " << serverName << " using UDP over port 24887." << endl;
    }

    //send response from server to client
    sendToClient(clientSock, respMessage);
}

/*
Takes in the multiple courses input string and converts it into a vector of course codes
There will be atleast 2 courses in such a request
Returns a vector of unique course codes
*/
vector<string> getAllCoursesList(string input){
    vector<string> allCoursesInOrder;
    set<string> uniqueCourses;
    pair<set<string>::iterator,bool> res;
    //following 'splitting' on comma logic inspired by https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    int start = 0, end = 0;
    while(end != -1){
        end = input.find(",", start);
        string temp = input.substr(start, end-start); //get the courseCode
        res = uniqueCourses.insert(temp); //store in set to remove any duplicates
        if(res.second == true){ //if courseCode got added to set (i.e. not already present in it), add to the list
            allCoursesInOrder.push_back(temp);
        }
        start = end+1;
    }
    return allCoursesInOrder;
}

//send data to UDP server based on portNumber
void sendToUDPServer(int sockM, string data, int destPort){
    //destination udp server address struct based on port number destPort in the function parameters
    sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(destPort); 
    inet_pton(AF_INET, "127.0.0.1", &destAddr.sin_addr); //loopback IP address

    //send to UDP server
    unsigned int lenAddr = sizeof(struct sockaddr_in);
    int sentToServer = sendto(sockM, data.c_str(), data.size()+1, 0, (sockaddr *)&destAddr, lenAddr); 
    if (sentToServer < 0) {
       error("Sending data to server failed.");
    }
}

//receive data to UDP server
string receiveFromUDPServer(int sockM){
    sockaddr_in from;
    unsigned int lenAddr = sizeof(struct sockaddr_in);
    char buffUdpServer[4096]; 
    memset(buffUdpServer, 0, 4096);
    int respFromServer = recvfrom(sockM, buffUdpServer, 4096, 0, (sockaddr *)&from, &lenAddr);
    if (respFromServer < 0) {
        error("Error getting response from the server");
    }
    string respMessage = string(buffUdpServer, 0, respFromServer); //converts response from char array to string
    return respMessage;
}

//send data to client
void sendToClient(int clientSock, string data){
    //send response from server to client
    int sendToClient = send(clientSock, data.c_str(), data.size()+1, 0); //send to client using TCP
    if(sendToClient == -1){
        error("Sending result to client failed.");
    }
}

/*
processes input to separate EE and CS courses
send to each server respectively and fetch result
reassemble result in same sequence as original order
send result to client
*/
void executeMultipleServersPath(int sockM, vector<string> courseList, int clSocket){
    string sendToServerEE = "1", sendToServerCS = "1"; //1 to indicate Multiple courses request
    string eeCoursesResp = "", csCoursesResp = "";
    vector<string> eeResult, csResult;
    for(int i=0; i<courseList.size(); i++){
        if(courseList[i].substr(0,2) == "EE"){
            sendToServerEE += courseList[i] + ",";
        }
        else{
            sendToServerCS += courseList[i] + ",";
        }
    }
    if(sendToServerEE != "1"){
        sendToUDPServer(sockM, sendToServerEE.substr(0,sendToServerEE.size()-1), SERVEREE_PORT); //-1 to remove the trailing comma
        cout << "The main server sent a multiple course request to serverEE." << endl; //modified slightly for multiple courses
        //receive from UDP from server
        eeCoursesResp = receiveFromUDPServer(sockM);
        eeResult = convertToList(eeCoursesResp);
        cout << "The main server received the response from serverEE using UDP over port 24887." << endl;
    }
    if(sendToServerCS != "1"){
        sendToUDPServer(sockM, sendToServerCS.substr(0,sendToServerCS.size()-1), SERVERCS_PORT); //-1 to remove the trailing comma
        cout << "The main server sent a multiple course request to serverCS." << endl; //modified slightly for multiple courses
        //receive from UDP from server
        csCoursesResp = receiveFromUDPServer(sockM);
        csResult = convertToList(csCoursesResp);
        cout << "The main server received the response from serverCS using UDP over port 24887." << endl;
    }

    string toSendCombined;
    if(eeCoursesResp == ""){
        sendToClient(clSocket, csCoursesResp);
    }
    else if(csCoursesResp == ""){
        sendToClient(clSocket, eeCoursesResp);
    }
    else{
        int indEE = 0, indCS = 0;
        for(int i=0; i<courseList.size(); i++){
            if(indEE < eeResult.size() && eeResult[indEE].substr(0,5) == courseList[i]){
                toSendCombined += eeResult[indEE++] + "\n";
            }
            else{          
                toSendCombined += csResult[indCS++] + "\n";
            }
        }
        sendToClient(clSocket, toSendCombined);
    }
}

//converts a single string of course details of 1 or more courses to a vector of course details
vector<string> convertToList(string courses){
    vector<string> listOfCourses;
    //following logic referred to from https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    int start = 0, end = 0;
    string delim = "\n";
    while(end != -1){
        end = courses.find("\n", start);
        listOfCourses.push_back(courses.substr(start, end-start));
        start = end + delim.length();
    }
    listOfCourses.pop_back(); //remove the last emtpy element because of the trailing \n
    return listOfCourses;
}

//prints error and exists program
void error(string msg){
    cout << msg << endl;
    exit(0);
}