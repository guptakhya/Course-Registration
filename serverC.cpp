/*
The data structures and scoket programming functions of the following code has been referenced from
https://beej.us/guide/bgnet/html/, https://www.linuxhowtos.org/data/6/server_udp.c and
https://www.linuxhowtos.org/data/6/client_udp.c
*/

#include <iostream>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <fstream>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <vector>

using namespace std;

string getUser(string inputCreds);
void error(string msg);
void processFile();
vector<string> credentials;

int main(int argc, char *argv[])
{
    int credSock, serverLen, n;
    socklen_t fromLen;
    sockaddr_in serverC;
    sockaddr_in fromM;
    char buff[1024];

    credSock = socket(AF_INET, SOCK_DGRAM, 0); // creating a UDP socket
    if (credSock < 0){
        error("Error opening sokcet");
    } 
    serverLen = sizeof(serverC);
    memset(&serverC, 0, serverLen);
    serverC.sin_family=AF_INET;
    serverC.sin_addr.s_addr=INADDR_ANY;
    serverC.sin_port=htons(21887); // 21000 + 887 (last 3 digits of my ID)
    if (bind(credSock, (sockaddr *)&serverC, serverLen)<0) {
         error("Binding failed.");
    }
    cout << "The ServerC is up and running using UDP on port 21887." << endl;
    processFile(); //read cred.txt and store its data in the vector
    fromLen = sizeof(struct sockaddr_in);
    while(true){
        memset(buff,0,1024);
        int credsReceived = recvfrom(credSock, buff, 1024, 0, (sockaddr *)&fromM, &fromLen);
        if (credsReceived < 0){
            error("Error receiving data");
        }

        cout << "The ServerC received an authentication request from the Main Server." << endl;
        string creds = string(buff, 0, credsReceived);

        string toSend = getUser(creds);

        int sendAuthResult = sendto(credSock, toSend.c_str(), toSend.size()+1, 0, (sockaddr *)&fromM, fromLen);
        if (sendAuthResult < 0) {
            error("Failed to send");
        }
        else{
            cout << "The ServerC finished sending the response to the Main Server." << endl;
        }
    }
    return 0;
}

//loads file cred.txt and stores each line in the vector courses
void processFile(){
    string checkLine;
    ifstream credFile ("cred.txt");
    if(credFile.is_open()){
        while(getline(credFile, checkLine)){
            credentials.push_back(checkLine);
        }
        credFile.close();
    }
    else{
        error("Could not open file cred.txt");
    }

}

//check if username, password match the list of credentials in cred.txt
string getUser(string inputCreds){
    bool nameMatch = false;
    for(int i=0; i< credentials.size(); i++){
        string username = credentials[i].substr(0, credentials[i].find(","));
        string password = credentials[i].substr(credentials[i].find(","), credentials[i].size());
        if(username == inputCreds.substr(0, inputCreds.find(","))){
            nameMatch = true;
            if(password == inputCreds.substr(inputCreds.find(","), inputCreds.size())){
                return "Auth Passed";
            }
        }
    }
    if(!nameMatch){
        return "Invalid Username";
    }
    else{
        return "Invalid Password";
    }
}

void error(string msg){
    cout << msg << endl;
    exit(0);
}