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
#include<unordered_map>
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

    int portNum = std::stoi(argv[1]);
    fd_set master;
    fd_set readFds;
    fd_set writeFds;
    struct sockaddr_in myAddr;
    struct sockaddr_in clientAddr;
    int fdmax;
    int mySocketID;
    int clientSocketID;
    char buff[BUFFER_SIZE];
    int nbytes;
    int yes=1;
    socklen_t addrLen;
    int i,j;

    FD_ZERO(&master);
    FD_ZERO(&readFds);

    mySocketID = socket(AF_INET,SOCK_STREAM,0);

    //bind
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    myAddr.sin_port = htons(portNum);
    memset(&(myAddr.sin_zero),'\0',8);

    if(bind(mySocketID,(struct sockaddr *)&myAddr, sizeof(myAddr))==-1)
    {
        std::cerr<<"Failed to bind socket to port"<<endl;
        exit(2);
    }
    cout<<"BindDone: "<<portNum<<endl;

    if(listen(mySocketID, 10)==-1){
        std::cerr<<"Failed to listen incoming connections"<<endl;
        exit(0);
    }
    cout<<"ListenDone: "<<portNum<<endl;

    FD_SET(mySocketID,&master);
    fdmax=mySocketID;

    // unordered_map<int, std::ifstream> m;
    auto m = std::unordered_map<int, std::ifstream *>{};

    unordered_map<int, int>file_sizes;
    while(true)
    {
        // cout<<"Start..\n";
        readFds = master;
        writeFds = master;
        // cout<<"selecting...\n";
        select(fdmax+1,&readFds,&writeFds,NULL,NULL);
        
        // cout<<"select done\n";
        for(i=0;i<=fdmax;i++)
        {
            // cout<<"now on top\n";
            
            if(FD_ISSET(i,&readFds) ||FD_ISSET(i,&writeFds))
            {
                // cout<<i<<"  ..here now\n";
                if(i==mySocketID)
                {
                    // cout<<"HIiii\n";
                    addrLen = sizeof(clientAddr);
                    clientSocketID = accept(mySocketID,(struct sockaddr *)&clientAddr,&addrLen);

                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET,&clientAddr.sin_addr,client_ip,INET_ADDRSTRLEN);
                    cout<<"Client: "<<client_ip<<":"<<ntohs(clientAddr.sin_port)<<endl;

                    FD_SET(clientSocketID,&master);
                    if(clientSocketID>fdmax) fdmax = clientSocketID;
                }
                else
                {
                    // cout<<i<<endl;
                    if(m.find(i)==m.end())
                    {
                        
                        // cout<<"hmmmmm\n";
                        std::ifstream *f = new std::ifstream;
                        string fileToSend="";
                        nbytes = recv(i,buff,BUFFER_SIZE,0);

                        fileToSend=buff;

                        int delim = fileToSend.find(' ');

                        string action = fileToSend.substr(0,delim);
                        string filename = fileToSend.substr(delim+1,fileToSend.size());
                        cout<<"FileRequested: "<<filename<<endl;
                        if(action!="get" || filename.size()==0)
                        {
                            std::cout<<"UnknownCmd\n";
                            std::cerr<<"The Format must be \"get FileToSend\"\n";
                        }

                        

                        // std::ifstream file(filename,ios::binary);
                        (*f).open(filename,ios::binary);
                        if(!(*f).is_open())
                        {
                            cout<<"FileTransferFail\n";
                            if(!std::filesystem::exists(filename)){
                                std::cerr<<"File does not exist!\n";
                            }
                            else{std::cerr<<"File is not readable...\n";
                            close(i);}
                            continue;
                        }

                        // cout<<"Adding to umap..\n";
                        m.insert(std::make_pair(i, f));
                        // cout<<"added\n";
                        file_sizes.insert(std::make_pair(i,0));
                        // cout<<"File size done\n";

                    }
                    else
                    {   
                        // cout<<"INide ELSE..\n";
                        std::ifstream *f = m[i];
                        int bytes_read,bytes_sent;
                        (*f).read(buff,BUFFER_SIZE);
                        bytes_read = (*f).gcount();
                        bytes_sent = send(i,buff,bytes_read,0);
                        file_sizes[i]+=bytes_read;

                        // cout<<bytes_read<<endl;

                        if(bytes_read<=0){
                            cout<<"TransferDone: "<<file_sizes[i]<<" bytes\n";
                            close(i);
                            FD_CLR(i,&master);
                            delete m[i];
                            // (*f).close();
                            m.erase(i);
                            file_sizes.erase(i);
                        }

                    }
                    // cout<<"Hi frandssss1\n";
                    // int bytes_read,file_size=0,bytes_sent;    
                    // do
                    // {
                    //     cout<<"sending....\n";
                    //     file.read(buff,BUFFER_SIZE);
                    //     bytes_read=file.gcount();
                    //     bytes_sent = send(i,buff,bytes_read,0);
                    //     file_size+=bytes_read;
                    //     if(bytes_read!=bytes_sent){cout<<"ERROR! Not sending properly...\n";}

                    // } while (bytes_read>0);
                    
                    // cout<<"TeansferDone: "<<file_size<<" bytes\n";

                    

                    
                    // close(i);
                    // FD_CLR(i,&master);
                }
                // cout<<"Hi frandssss2\n";
            }
            // cout<<"Hi frandssss3\n";
        }
        // cout<<"Hi frandssss4\n";
    }

    return 0;
    
}