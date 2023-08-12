#include<iostream>
#include<fstream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

#define BUFFER_SIZE 512

int main(int argc, char *argv[])
{
    if(argc!=3)
    {
        std::cerr<<"Incorrect number of arguments!\n"
                    "1. ServerIP:Port \n"
                    "2. name of file to recieve\n";
        
        exit(1);

    }

    string tmp = argv[1];
    int delim = tmp.find(":");
    string serverIP = tmp.substr(0,delim);
    int portNum = stoi(tmp.substr(delim+1,tmp.size()));
    // cout<<serverIP<<portNum<<endl;

    string filename = argv[2];

    int mySocketID;
    struct sockaddr_in dest_addr;

    mySocketID=socket(AF_INET,SOCK_STREAM,0);

    dest_addr.sin_family=AF_INET;
    dest_addr.sin_port=htons(portNum);
    dest_addr.sin_addr.s_addr=inet_addr(serverIP.c_str());
    memset(&(dest_addr.sin_zero),'\0',8);


    if(connect(mySocketID,(struct sockaddr *)&dest_addr,sizeof(struct sockaddr))==-1){
        std::cerr<<"Failed to connect to server"<<endl;
        exit(2);
    }
    cout<<"ConnectDone: "<<tmp<<endl;

    char buff[BUFFER_SIZE];
    int bytes_read,file_size=0;

    std::ofstream outfile(filename, std::ios::app);
    if(!outfile.is_open())
    {
        std::cerr<<"Failed to open the file!\n";
        exit(3);
    }
    do
    {
        bytes_read = recv(mySocketID, buff, BUFFER_SIZE,0);
        outfile.write(buff,bytes_read);
        file_size+=bytes_read;
    } while (bytes_read>0);
    
    cout<<"FileWritten: "<<file_size<<" bytes\n";

    close(mySocketID);
    outfile.close();

    return 0;
    
}