#include<iostream>
#include<fstream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if(argc!=4)
    {
        std::cerr<<"Incorrect number of arguments!\n"
                    "1. ServerIP:Port \n"
                    "2. name of file to recieve\n"
                    "3. Recieve Interval\n";
        
        exit(1);

    }

    string tmp = argv[1];
    int delim = tmp.find(":");
    string serverIP = tmp.substr(0,delim);
    int portNum = stoi(tmp.substr(delim+1,tmp.size()));

    string filename = argv[2];
    int recieveInterval = std::stoi(argv[3]);

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
    int bytes_read,file_size=0,bytes_sent;

    strcpy(buff,"get ");
    strcat(buff,filename.c_str());

    // strcat(buff,"\0");
    // if(buff[strlen(buff)-1]=="\0")cout<<"OK\n";
    // std::cout << "buff is null terminated: " << (strcmp(buff, "") == 0) << std::endl;

    do
    {
        bytes_sent = send(mySocketID,buff,BUFFER_SIZE,0);
        // cout<<bytes_sent<<endl;
        bytes_read+=bytes_sent;
    } while (bytes_read<=strlen(buff));

    // cout<<"Sent filname\n"<<buff<<endl;
    



    std::ofstream outfile(filename, std::ios::app);
    int i=0;
    do
    {
        // cout<<"receiving... "<<i<<endl;
        bytes_read = recv(mySocketID, buff, BUFFER_SIZE,0);
        outfile.write(buff,bytes_read);
        file_size+=bytes_read;
        i++;
        usleep(recieveInterval*1000);
    } while (bytes_read>0);
    
    cout<<"FileWritten: "<<file_size<<" bytes\n";

    close(mySocketID);
    outfile.close();

    return 0;
    
}