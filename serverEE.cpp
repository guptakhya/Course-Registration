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

string getCourseDetails(string courseCode, string category);
void error(string msg);
void processFile();
vector<string> convertToList(string courses);
string getMultipleCourseDetails(vector<string> courseList);
vector<string> courses;

int main(int argc, char *argv[])
{
    int serverEESock, serverEELen;
    socklen_t fromLen;
    sockaddr_in serverEE;
    sockaddr_in fromM;
    char buff[1024];

    serverEESock = socket(AF_INET, SOCK_DGRAM, 0); // creating a UDP socket
    if (serverEESock < 0){
        error("Error opening sokcet");
    } 
    serverEELen = sizeof(serverEE);
    memset(&serverEE, 0, serverEELen);
    serverEE.sin_family=AF_INET;
    serverEE.sin_addr.s_addr=INADDR_ANY;
    serverEE.sin_port=htons(23887); // 23000 + 887 (last 3 digits of my ID)
    if (bind(serverEESock, (sockaddr *)&serverEE, serverEELen)<0) {
        error("Binding failed.");
    }
    cout << "The ServerEE is up and running using UDP on port 23887." << endl;
    processFile();
    fromLen = sizeof(struct sockaddr_in);
    while(true){
        memset(buff,0,1024);
        int requestReceived = recvfrom(serverEESock, buff, 1024, 0, (sockaddr *)&fromM, &fromLen);
        if (requestReceived < 0){
            error("Error receiving data");
        }
        
        string courseRequest = string(buff, 0, requestReceived);
        if(courseRequest[0] == '0'){
            courseRequest = courseRequest.substr(1); //removing the 0
            string courseCode = courseRequest.substr(0, courseRequest.find(","));
            string category = courseRequest.substr(courseRequest.find(",")+1);
            cout << "The serverEE received a request from the Main Server about the " << category << " of " << courseCode << "." << endl;

            string toSend = getCourseDetails(courseCode, category);
        
            int sendCourseDetails = sendto(serverEESock, toSend.c_str(), toSend.size()+1, 0, (sockaddr *)&fromM, fromLen);
            if (sendCourseDetails < 0) {
                error("Failed to send");
            }
            else{
                cout << "The serverEE finished sending the response to the Main Server." << endl;
            }
        }
        else{
            courseRequest = courseRequest.substr(1); //removing the 1
            cout << "The serverEE received a request from the Main Server about the multiple courses." << endl; //modified slightly for multiple courses
            vector<string> courseList = convertToList(courseRequest);
            string courseDetails = getMultipleCourseDetails(courseList);
            int sendCourseDetails = sendto(serverEESock, courseDetails.c_str(), courseDetails.size()+1, 0, (sockaddr *)&fromM, fromLen);
            if (sendCourseDetails < 0) {
                error("Failed to send");
            }
            else{
                cout << "The serverEE finished sending the response to the Main Server." << endl;
            }
        }
    }
    return 0;
}

//loads file ee.txt and stores each line in the vector courses
void processFile(){
    string checkLine;
    ifstream eeFile ("ee.txt");
    if(eeFile.is_open()){
        while(getline(eeFile, checkLine)){
            courses.push_back(checkLine);
        }
        eeFile.close();
    }
    else{
        error("Could not open file ee.txt");
    }

}

//check if courseCode matches with any of the courseCodes in the EE list in ee.txt, and return the details based on the category
string getCourseDetails(string courseCode, string category){
    bool foundCourse = false;
    for(int i=0; i< courses.size(); i++){
        //following 'splitting' on comma logic inspired by https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
        int start = 0, end = 0;
        vector<string> details;
        while(end != -1){
            end = courses[i].find(",", start);
            details.push_back(courses[i].substr(start, end-start));
            start = end+1;
        }
        if(details[0] == courseCode){
            foundCourse = true;
            int ind;
            if(category == "Credit"){
                ind = 1;
            }
            else if(category == "Professor"){
                ind = 2;
            }
            else if(category == "Days"){
                ind = 3;
            }
            else if(category == "CourseName"){
                ind = 4;
            }
            else{
                error("Invalid Category");
            }
            string output = "The " + category + " of " + courseCode + " is " + details[ind];
            cout << "The course information has been found: " << output << "." << endl;
            return output; 
        }
    }
    string out = "Didn’t find the course: " + courseCode + ".";
    cout << out << endl;
    return out; 
}

//converts a single string of course details of 1 or more courses to a vector of course details
vector<string> convertToList(string courses){
    vector<string> listOfCourses;
    //following logic referred to from source4 (https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c)
    int start = 0, end = 0;
    string delim = ",";
    while(end != -1){
        end = courses.find(",", start);
        listOfCourses.push_back(courses.substr(start, end-start));
        start = end + delim.length();
    }
    return listOfCourses;
}

//finds details of the courses and appends the result to the string
string getMultipleCourseDetails(vector<string> courseList){
    string allDetails;
    for(int i=0; i<courseList.size(); i++){
        bool courseFound = false;
        for(int j=0; j<courses.size(); j++){
            if(courses[j].substr(0,5) == courseList[i]){
                courseFound = true;
                string toAdd = courses[j]; 
                toAdd[5] = ':';// make the ',' after course code to ':'
                allDetails += toAdd +"\n";
                break;
            }
        }
        if(!courseFound){
            allDetails += courseList[i] + ": Didn’t find the course." +"\n";
        }
    }
    return allDetails;
}

//prints error message and exits program
void error(string msg){
    cout << msg << endl;
    exit(0);
}