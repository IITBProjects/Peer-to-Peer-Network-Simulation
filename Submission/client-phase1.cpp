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

vector<int> theirsideconnection_sockfd;

int connectedthread(){
        while(true){
            int size=theirsideconnection_sockfd.size();
            if(size>=1){
                int socfd=theirsideconnection_sockfd[0];
                close(socfd);
                vector<int>::iterator it;
                it= theirsideconnection_sockfd.begin();
                theirsideconnection_sockfd.erase(it);

            }
            if(size==0){
                sleep(20);
                size=theirsideconnection_sockfd.size();
                if(size==0){  
                   return 0;
                }
                
            }
        }


}

int listenthread(int listeningport,int my_id,int BACKLOG){
    
    int listeningsoc_fd,newallotedsoc_fd;
    struct sockaddr_in my_addr,their_addr;
    socklen_t sin_size;
    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(listeningport);
    my_addr.sin_addr.s_addr=INADDR_ANY;
    memset(&(my_addr.sin_zero),'\0',8);
    bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
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
            theirsideconnection_sockfd.push_back(newallotedsoc_fd);
            char buf[1024];
            memset(buf, '\0', 1024);
            string myid=to_string(my_id);
            strcat(buf, myid.c_str());
            int sent=send(newallotedsoc_fd,buf,1024,0);
            continue;
        }   
        else                            break;
    }
    return 0;
}
//receive files from neighbours
int requestconnectionthread(int no_neighbours,vector <int> neighbourid,vector <int> neighbourport){
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
        close(mysideconnection_sockfd[i]);
    }
    return 0;
}
int main(int argc, char** argv)
{
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
    ifstream MyReadFile(client_configurationfilename);
    string file_text;
    vector<string> search_filenames;
    int phase=1;
    while (getline (MyReadFile,file_text)) {
            vector<string> words;
           	string temp = "";
            for(int i=0;i<file_text.length();++i){
                
                if(file_text[i]==' '){
                    words.push_back(temp);
                    temp = "";
                }
                else{
                    temp.push_back(file_text[i]);
                }
                
            }
	        words.push_back(temp);

            if(phase==1){
                client_id=stoi(words.at(0));
                client_portno=stoi(words.at(1));
                client_uniqueid=stoi(words[2]);
                phase++;
                continue; 
            }
            else if(phase==2){
                no_neighbours=stoi(words.at(0));
                phase++;
                continue;
               
            }
            else if(phase==3){
                for (int i=0; i<no_neighbours; i++){
                    neighbour_id.push_back(stoi(words.at(2*i)));
                    neighbour_port.push_back(stoi(words.at(2*i+1)));
                }
                phase++;
                continue;
                
            }
            else if(phase==4){
                no_filestobesearched=stoi(words.at(0));
                phase++;
                continue;
                
            }
            else{
                string r=words.at(0);
                int len=r.size();
                if(int(r[len-1])==13 || int(r[len-1])==10){
                    r=r.substr(0,len-1);
                }
                
                search_filenames.push_back(r);
            }
    } 
    DIR *dir; struct dirent *diread;
    vector<char *> files;
    vector<string> client_files;
    const char * path = directory_path.c_str();
    if ((dir = opendir(path)) != nullptr) {
        while ((diread = readdir(dir)) != nullptr) {
            files.push_back(diread->d_name);
        }
        closedir (dir);
    } else {
        perror ("opendir");
        return EXIT_FAILURE;
    }

    for (auto file : files){ 
        string a=file;
        if(a!="." && a!=".."){
            if(a=="Downloaded") continue;
            client_files.push_back(a);
        }
    }
    sort(client_files.begin(),client_files.end());
    for(int i=0;i<client_files.size();i++){
        cout<<client_files[i]<<endl;
    }
    sort(search_filenames.begin(),search_filenames.end());
    thread thread1(listenthread,client_portno,client_uniqueid,MAX_QUEUE);
    thread thread2(requestconnectionthread,no_neighbours,neighbour_id,neighbour_port);
    thread thread3(connectedthread);
    thread1.join();
    thread2.join();
    thread3.join();


    

}
