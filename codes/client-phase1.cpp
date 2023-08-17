#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <regex.h>
#include <thread>
#include <filesystem>
#include <vector>
#include <dirent.h>
using namespace std;
//send files to neighbours

int connectedthread(int theirsideconnection_sockfd,vector <string> myfiles,int my_uniqueid){
        while(true){
            char buf[1024];
            memset(buf, '\0', 1024);
            int received=recv(theirsideconnection_sockfd, buf, 1024, 0);
            char closes[6]="CLOSE";
            if(strncmp(buf,closes, 5)==0){
                close(theirsideconnection_sockfd);
                break;
            }
        }
    return 0;

}

int listenthread(int listeningport,int my_uniqueid,int BACKLOG,vector <string> myfiles){
    
    int listeningsoc_fd,newallotedsoc_fd;
    struct sockaddr_in my_addr,their_addr;
    socklen_t sin_size;
    vector <thread> theirconnectionthread;
    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(listeningport);
    my_addr.sin_addr.s_addr=INADDR_ANY;
    memset(&(my_addr.sin_zero),'\0',8);
    int Option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&Option, sizeof(Option));
    while(bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))<0) {
      sleep(5);
    };
    listen(sockfd,BACKLOG);
    struct timeval tv;
    fd_set readfds;
    tv.tv_sec=20;
    tv.tv_usec=5;
    FD_ZERO(&readfds);
    FD_SET(sockfd,&readfds);
    sin_size=sizeof(struct sockaddr_in);
    while(true){
        select(sockfd+1,&readfds,NULL,NULL,&tv);
        
        if(FD_ISSET(sockfd,&readfds)){
            newallotedsoc_fd=accept(sockfd, (struct sockaddr *)&their_addr,&sin_size);
            char buf[1024];
            memset(buf, '\0', 1024);
            string myid=to_string(my_uniqueid);
            strcat(buf, myid.c_str());
            int sent=send(newallotedsoc_fd,buf,1024,0);
            theirconnectionthread.push_back(thread(connectedthread,newallotedsoc_fd,myfiles,my_uniqueid));
          
        }   
        else                            break;
    }
     for(int i=0;i<theirconnectionthread.size();i++){
        theirconnectionthread[i].join();
    }
    return 0;
}
//receive files from neighbours
int requestconnectionthread(int no_neighbours,vector <int> neighbourid,vector <int> neighbourport,vector <string> search_filenames){
    int mysideconnection_sockfd[no_neighbours];
    char buf[1024];
    for(int i = 0; i < no_neighbours; i++){
        mysideconnection_sockfd[i] = socket(PF_INET, SOCK_STREAM, 0);
        struct sockaddr_in their_addr;
        their_addr.sin_family = AF_INET;
        their_addr.sin_port=htons(neighbourport[i]);
        their_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        memset(&(their_addr.sin_zero),'\0',8);
        while(true){
            if(connect(mysideconnection_sockfd[i], (struct sockaddr *) &their_addr, sizeof(struct sockaddr)) == 0) 
                break;
            sleep(2);
        }
        memset(buf, '\0', 1024);
        int received=recv(mysideconnection_sockfd[i], buf, 1024, 0);
        cout<<"Connected to "<<neighbourid[i]<<" with unique-ID "<<buf<<" on port "<<neighbourport[i]<<endl;
    }

    for(int i = 0; i < no_neighbours; i++){
        memset(buf, '\0', 1024);
        strcat(buf, "CLOSE");
        send(mysideconnection_sockfd[i], buf, 1024, 0);
        close(mysideconnection_sockfd[i]);
    }
    return 0;
}
int main(int argc, char** argv)
{
    ifstream inputs;
    string client_configurationfilename=argv[1];
    string directory_path=argv[2];
    int MAX_QUEUE = 10;
    int client_id;
    int client_portno;
    int client_uniqueid;
    int no_neighbours;
    int no_filestobesearched;
    vector <int>neighbour_id;
    vector <int>neighbour_port;
    vector<string> search_filenames;

    inputs.open(argv[1]);
    inputs>>client_id>>client_portno>>client_uniqueid;
    inputs>>no_neighbours;

    for(int i=0; i<no_neighbours; i++){
        int id,port;
        inputs>>id>>port;
        neighbour_id.push_back(id);
        neighbour_port.push_back(port);
    }

    inputs>>no_filestobesearched;

    for(int i=0; i<no_filestobesearched; i++){
        string file;
        inputs>>file;
        search_filenames.push_back(file);
    }

    DIR *dir; struct dirent *diread;
    vector<string> client_files;
    const char * path = directory_path.c_str();
    if ((dir = opendir(path)) != nullptr) {
       
        while ((diread = readdir(dir)) != nullptr) {
            if(diread->d_type==DT_DIR)  continue;
            client_files.push_back(diread->d_name);
        }
        closedir (dir);
    } else {
        perror ("opendir");
        return EXIT_FAILURE;
    }

    sort(client_files.begin(),client_files.end());
    for(int i=0;i<client_files.size();i++){
        cout<<client_files[i]<<endl;
    }
    sort(search_filenames.begin(),search_filenames.end());
    thread thread1(listenthread,client_portno,client_uniqueid,MAX_QUEUE,client_files);
    thread thread2(requestconnectionthread,no_neighbours,neighbour_id,neighbour_port,search_filenames);
    thread1.join();
    thread2.join();

    

}
