#include<iostream>
#include<fstream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include<string.h>
#include <unistd.h>
using namespace std;

#define MYPORT 3469
#define BUFFER_SIZE 512

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cerr<<"The program accepts only 2 Arguments.\n"
                     "portNum: The server listens on this port\n"
                     "fileToTransfer: The file you want to transfer"<<endl;
        exit(1);
    }

    int portNum = stoi(argv[1]);
    string filename = argv[2];

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

    struct sockaddr_in their_addr;
    socklen_t sin_size = sizeof(their_addr);

    int new_fd = accept(sockfd,(struct sockaddr *)&their_addr,&sin_size);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&their_addr.sin_addr,client_ip,INET_ADDRSTRLEN);
    cout<<"Client: "<<client_ip<<":"<<ntohs(their_addr.sin_port)<<endl;

    int bytes_read,bytes_sent,file_size=0;
    std::ifstream file(filename,ios::binary);
    // file.open(filename,ios::binary);
    char buff[BUFFER_SIZE];
    do
    {
        file.read(buff,BUFFER_SIZE);
        bytes_read=file.gcount();
        bytes_sent = send(new_fd,buff,bytes_read,0);
        file_size+=bytes_read;
        // if(bytes_read!=bytes_sent){cout<<"ERROR! Not sending properly...\n";}

    } while (bytes_read>0);
    
    cout<<"TeansferDone: "<<file_size<<" bytes\n";

    close(sockfd);
    close(new_fd);
    
    file.close();

    return 0;
    
}