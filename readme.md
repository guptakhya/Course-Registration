Name: Khyati Gupta
USC ID: 8031075887
USC email: khyatigu@usc.edu

Completed parts: Phases 1,2,3,4 as well as the extra credit portion.

The project works on 5 files:
1. client.cpp: Takes input from the client for username and password, and later for courses they want the details of. The client interacts only with the serverM using a TCP connection. Client adds the following identifiers to strings for the main server to differentiate easily between the different types of request, followed by comma separated input values:
    - "Type1:" is used for requests to serverC for verfiying credentials
    - "Type2:" is used for single course code and category request for a EE course
    - "Type3:" is used for single course code and category request for a CS course
    - "Type4:" is used for multiple course codess request (the extra credit part)
Assumptions:
    i. Clients are given 3 attempts if the username or password they entered do not match respective constraints (length and characters)
    ii. CourseCodes cannot have leading or trailing spaces, they will need to reenter these details
    iii. 'Category' is case sensitive i.e. Credit is not same as credit and the users need to enter it correctly
    iv. CourseCodes are all 5 characters each
    v. For extra credit portion: all courses entered either begin with 'EE' or 'CS'

2. serverM.cpp: The main server, connects with client through TCP and UDP for the other 3 servers. Processes the input based on the above 'Type's and sends request to respective servers and get responses that it further sends to client. In case of courses, it appends in front a "0" for single course and category requests for serverEE and serverCS, while adds a "1" for multiple course code request. It also maintains the order of the original course codes in a vector of strings, and uses a set to check that there are no duplicate course codes. Upon receiving responses from serverEE and serverCS for the multiple course codes, it also reassembles them in the same order as requested before passing information on to the client.
Assumptions: Input from the client and responses from the servers are coming in the expected formats.

3. serverC.cpp: the credentials server loads the cred.txt file, processes the data and stores each line in a vector. Each element of the vector is a string which contains both username and password, separated by a comma. It verifies the request from serverM and sends out a response to it as follows:
    - "Auth Passed" sent if the username and password match an entry
    - "Invalid Username" if the username does not match any usernames in the list
    - "Invalid Password" if the username matches but the password does not

4. serverEE.cpp: Loads ee.txt ans stores the course details as a vector of strings - every element being 1 row of the file. It processes input from serverM to search for the information in the vector for ee.txt and send it back. For extra credit - each entry in response in separated by "\n" in the end.

5. serverCS.cpp: Loads cs.txt ans stores the course details as a vector of strings - every element being 1 row of the file. It processes input from serverM to search for the information in the vector for cs.txt and send it back. For extra credit - each entry in response in separated by "\n" in the end.

References:
1. Beej's Guide to Network Programming (https://beej.us/guide/bgnet/html/): used for overall reference of understanding socket programming as well as creating some structures including socket, sockaddr_in, socklen_t, inet_pton and some particulars like using AF_INET to represent IPv4 and htons() for port number conversions.
2. Sloan Kelly's youtube videos on creating a TCP server (https://www.youtube.com/watch?v=cNdlrbZSkyQ&list=RDCMUC4LMPKWdhfFlJrJ1BHmRhMQ&start_radio=1&rv=cNdlrbZSkyQ&t=2559) and client (https://www.youtube.com/watch?v=fmn-pRvNaho&list=RDCMUC4LMPKWdhfFlJrJ1BHmRhMQ&index=2) were also referenced while understanding how to establish connections using bind, connect, listen, send, recv and close.
3. https://www.linuxhowtos.org/data/6/server_udp.c and https://www.linuxhowtos.org/data/6/client_udp.c were referred to understand communicating using UDP for Linux
4. Stack overflow (https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c), was used to understand the logic for 'splitting' words for an input
5. https://cplusplus.com/, https://www.geeksforgeeks.org/ and https://stackoverflow.com/ were also referred to time and again to verify C++ various commands/syntaxes, their functionailities and nuances.

The make file created has been referenced from https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html. It contains compiling and executing commands for the above 5 files.