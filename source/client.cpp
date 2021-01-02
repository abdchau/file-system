// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <string>
#include <iostream>
#define PORT 95

using namespace std;

int main(int argc, char const *argv[]) 
{ 
    int sock = 0; 
    string server_ip;
    struct sockaddr_in serv_addr; 
    char buffer[1500] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    
    cout << "Enter Server IP: ";
    getline(cin, server_ip);
    cout << "Connecting to server at " << server_ip << " Port " << PORT << endl;

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    
    while(true) {
        if (read(sock, buffer, 1500) > 0)
            cout << buffer;

        string input;
        getline(cin, input);
        if (!input.compare("exit"))
            break;
        else if (input.length() > 0) {
            const char * str = input.c_str();
            send(sock, str, strlen(str), 0);
            bzero(buffer, sizeof(buffer));
        }
    }
    cout << "Connection closed" << endl;
    return 0; 
} 