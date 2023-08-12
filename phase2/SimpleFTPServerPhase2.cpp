#include<iostream>
#include<fstream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include<string.h>
#include <unistd.h>
#include <regex>
#include <filesystem>

using namespace std;

#define MYPORT 3469
#define BUFFER_SIZE 1024

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr<<"The program accepts only 2 Arguments.\n"
                     "portNum: The server listens on this port\n";
        exit(1);
    }

    int portNum = stoi(argv[1]);
    // string filename = argv[2];

    struct sockaddr_in my_addr;
    int sockfd = socket(AF_INET,SOCK_STREAM,0);

    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(portNum);
    my_addr.sin_addr.s_addr= inet_addr("127.0.0.1");
    memset(&(my_addr.sin_zero),'\0',8);

    int check = bind(sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr));
    if(check==-1)
    {
        std::cerr<<"Failed to bind socket to port"<<endl;
        exit(2);
    }
    cout<<"BindDone: "<<portNum<<endl;
    if(listen(sockfd, 10)==-1){
        std::cerr<<"Failed to listen incoming connections"<<endl;
        exit(0);
    }
    cout<<"ListenDone: "<<portNum<<endl;

    while(true)
    {

        struct sockaddr_in their_addr;
        socklen_t sin_size = sizeof(their_addr);


        int new_fd = accept(sockfd,(struct sockaddr *)&their_addr,&sin_size);
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&their_addr.sin_addr,client_ip,INET_ADDRSTRLEN);
        cout<<"Client: "<<client_ip<<":"<<ntohs(their_addr.sin_port)<<endl;
        // cout<<"Client: "<<their_addr.sin_addr.s_addr<<" port: "<<their_addr.sin_port<<endl;

        int bytes_read,bytes_sent,file_size=0;

        string fileToSend="";
        char buff[BUFFER_SIZE];

        
        // cout<<"recieving..\n";
        bytes_read = recv(new_fd,buff,BUFFER_SIZE,0);
        fileToSend+=buff;
            // if(fileToSend.back()== '\0'){cout<<"YES\n";break;}
        // } while (fileToSend.back()!= '\0');
        
        // cout<<"recieved!\n";
        // cout<<fileToSend<<endl;
        // regex pattern("get (\\w+)");
        // if(!regex_match(fileToSend,pattern))
        // {
        //     std::cout<<"UnknownCmd\n";
        //     std::cerr<<"The Format must be \"get FileToSend\"\n";
        //     close(new_fd);
        //     continue;
        // }
        int delim = fileToSend.find(' ');

        string action = fileToSend.substr(0,delim);
        string filename = fileToSend.substr(delim+1,fileToSend.size());

        cout<<"FileRequested: "<<filename<<endl;
        if(action!="get" || filename.size()==0){
            std::cout<<"UnknownCmd\n";
            std::cerr<<"The Format must be \"get FileToSend\"\n";
        }


        
        std::ifstream file(filename,ios::binary);
        if(!file.is_open())
        {
            cout<<"FileTransferFail\n";
            if(!std::filesystem::exists(filename)){
                std::cerr<<"File does not exist!\n";
                
            }
            else
            {
                std::cerr<<"File is not readable...\n";
                close(new_fd);
            }
                continue;
        }

        // cout<<"file is open\n";
        // file.open(filename,ios::binary);
        do
        {
            // cout<<"sending....\n";
            file.read(buff,BUFFER_SIZE);
            bytes_read=file.gcount();
            bytes_sent = send(new_fd,buff,bytes_read,0);
            file_size+=bytes_read;
            // if(bytes_read!=bytes_sent){cout<<"ERROR! Not sending properly...\n";}

        } while (bytes_read>0);
        
        cout<<"TeansferDone: "<<file_size<<" bytes\n";

        close(new_fd);
    }

    return 0;
    
}