#include <iostream>
#include <bits/stdc++.h>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iomanip>
#include <openssl/md5.h>
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
            bool found = false;
            for(int i = 0; i< myfiles.size() ; i++){
                if(strncmp(myfiles[i].c_str(), buf, myfiles[i].size()) == 0){
                    char buff[1024];
                    memset(buff, '\0', 1024);
                    strcat(buff,"YES");
                    send(theirsideconnection_sockfd, buff, 1024, 0);
                    found = true;
                    break;
                }
            }
            if(!found) {
                char buff[1024];
                memset(buff, '\0', 1024);
                strcat(buff, "NO");
                send(theirsideconnection_sockfd, buff, 1024, 0);
            }
        }
        return 0;


}

int listenthread(int listeningport,int my_uniqueid,int BACKLOG,vector <string> myfiles){
    
    int listeningsoc_fd,newallotedsoc_fd;
    struct sockaddr_in my_addr,their_addr;
    socklen_t sin_size;
    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(listeningport);
    my_addr.sin_addr.s_addr=INADDR_ANY;
    memset(&(my_addr.sin_zero),'\0',8);
    vector <thread> theirconnectionthread;
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
    vector <int> neighbouruniqueid;
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
        neighbouruniqueid.push_back(stoi(buf));
        cout<<"Connected to "<<neighbourid[i]<<" with unique-ID "<<buf<<" on port "<<neighbourport[i]<<endl;
      
    }
    vector <int> fileowneruniqueid;
    vector <int>    fileownerdepth;
    
    for(int i=0;i<search_filenames.size();i++){
        bool found=false;
        string currentfile=search_filenames[i];
        fileowneruniqueid.push_back(0);
        fileownerdepth.push_back(0);
        for(int j=0;j<no_neighbours;j++){
            char buf[1024];
            memset(buf, '\0', 1024);
            strcat(buf, currentfile.c_str());
            int sent=send(mysideconnection_sockfd[j],buf,1024,0);
            memset(buf, '\0', 1024); 
            int received=recv(mysideconnection_sockfd[j], buf, 1024, 0);
            char not_found[3]="NO";
            if(strncmp(buf,not_found,2)==0) continue;
            
            if(found){
                    if(neighbouruniqueid[j]<fileowneruniqueid[fileowneruniqueid.size()-1]){
                        fileowneruniqueid[fileowneruniqueid.size()-1]=neighbouruniqueid[j];
                    }
            }
            else{
                found=true;
                fileownerdepth[fileownerdepth.size()-1]=1;
                fileowneruniqueid[fileowneruniqueid.size()-1]=neighbouruniqueid[j];
            }

        }  
    }
    for(int i = 0; i<search_filenames.size(); i++){     
        cout<<"Found "<<search_filenames[i]<<" at "<<fileowneruniqueid[i]<<" with MD5 0 at depth "<<fileownerdepth[i]<<endl;
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
    thread thread1(listenthread,client_portno,client_uniqueid,MAX_QUEUE,client_files);
    thread thread2(requestconnectionthread,no_neighbours,neighbour_id,neighbour_port,search_filenames);
    thread1.join();
    thread2.join();
    
}
