#include<iostream>
#include<fstream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<filesystem>
using namespace std;

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if(argc!=5)
    {
        std::cerr<<"Incorrect number of arguments!\n"
                    "1. ServerIP:Port \n"
                    "2. operation {get, put}\n"
                    "3. name of file to recieve\n"
                    "4. Recieve Interval\n";
        
        exit(1);

    }

    string tmp = argv[1];
    int delim = tmp.find(":");
    string serverIP = tmp.substr(0,delim);
    int portNum = stoi(tmp.substr(delim+1,tmp.size()));

    string op=argv[2];
    string filename = argv[3];
    int recieveInterval = stoi(argv[4]);

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

    strcpy(buff,op.c_str());
    strcat(buff,(" "+filename).c_str());
    // cout<<buff<<endl;

    // strcat(buff,"\0");
    // if(buff[strlen(buff)-1]=="\0")cout<<"OK\n";
    // std::cout << "buff is null terminated: " << (strcmp(buff, "") == 0) << std::endl;

    do
    {
        bytes_sent = send(mySocketID,buff,BUFFER_SIZE,0);
        // cout<<bytes_sent<<endl;
        bytes_read+=bytes_sent;
    } while (bytes_read<=strlen(buff));

    // cout<<"Sent action and filname\n"<<buff<<endl;
    
    if(op=="put")
    {
        
         std::ifstream file(filename,ios::binary);
        if(!file.is_open())
        {
            cout<<"FileTransferFail\n";
            if(!std::filesystem::exists(filename)){
                std::cerr<<"File does not exist!\n";
            }
            else{std::cerr<<"File is not readable...\n";
            close(mySocketID);}
            exit(0);
        }
        
        
        int bytes_read,file_size=0,bytes_sent;    
        do
        {
            // cout<<"sending....\n";
            file.read(buff,BUFFER_SIZE);
            bytes_read=file.gcount();
            bytes_sent = send(mySocketID,buff,bytes_read,0);
            file_size+=bytes_read;
            usleep(recieveInterval*1000);
            // if(bytes_read!=bytes_sent){cout<<"ERROR! Not sending properly...\n";}

        } while (bytes_read>0);
        
        cout<<"TeansferDone: "<<file_size<<" bytes\n";
        file.close();
    }
    else
    {
        std::ofstream outfile(filename, std::ios::app);
        do
        {
            bytes_read = recv(mySocketID, buff, BUFFER_SIZE,0);
            outfile.write(buff,bytes_read);
            file_size+=bytes_read;
            usleep(recieveInterval*1000);
        } while (bytes_read>0);
        outfile.close();
        
        cout<<"FileWritten: "<<file_size<<" bytes\n";
    }
    close(mySocketID);

    return 0;
    
}