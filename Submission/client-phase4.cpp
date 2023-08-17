

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
#include <regex.h>
#include <thread>
#include <iomanip>
#include <openssl/md5.h>
#include <filesystem>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
using namespace std;
//send files to neighbours
vector <string> neighbourfiles;
vector <int>    neighbourfiles_port;
vector <int>    neighbourfiles_uniqueid;
int valid=0;

int connectedthread(int theirsideconnection_sockfd,vector <string> myfiles,int my_uniqueid,string directory_path){
    bool enquireyourself=0,sendfile=0,enquireneighbour=0,enquiredp2=0;
        while(true){
            char buf[1024];
            memset(buf, '\0', 1024);
            int received=recv(theirsideconnection_sockfd, buf, 1024, 0);
           
            char closes[6]="CLOSE";
            if(strncmp(buf,closes, 5)==0){
                close(theirsideconnection_sockfd);    
                break;
            }
            char enquire[17]="ENQUIRE YOURSELF";
            if(strncmp(buf,enquire, 16)==0){
                enquireyourself=true;
                sendfile=false;
                enquireneighbour=false;
                 enquiredp2=false;
                 char buff[1024];
                memset(buff, '\0', 1024);
                strcat(buff, "MODE CHANGED TO ENQUIRE");
                send(theirsideconnection_sockfd, buff, 1024, 0);
                continue;
            }
            char sendf[9]="SENDFILE";
            if(strncmp(buf,sendf, 8)==0){
                sendfile=true;
                enquireyourself=false;
                enquireneighbour=false;
                 enquiredp2=false;
                char buff[1024];
                memset(buff, '\0', 1024);
                strcat(buff, "MODE CHANGED TO SENDFILE");
               
                send(theirsideconnection_sockfd, buff, 1024, 0);
                continue;
            }
            char enqneigh[18]="ENQUIRE NEIGHBOUR";
            if(strncmp(buf,enqneigh, 17)==0){
                enquireyourself=false;
                sendfile=false;
                enquireneighbour=true;
                enquiredp2=false;
                char buff[1024];
                memset(buff, '\0', 1024);
                strcat(buff,to_string(my_uniqueid).c_str());
                send(theirsideconnection_sockfd, buff, 1024, 0);
                continue;
            }
            char enqneighdp[11]="ENQUIREDP2";
            if(strncmp(buf,enqneighdp, 10)==0){
               
                enquireyourself=false;
                sendfile=false;
                enquireneighbour=false;
                enquiredp2=true;
                char buff[1024];
                memset(buff, '\0', 1024);
                strcat(buff,"MODE CHANGED TO ENQUIREDP2");
                send(theirsideconnection_sockfd, buff, 1024, 0);
                continue;
            }
           if(enquireyourself){
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
                    continue;
           }
           else if(sendfile){
                    
                   
                   
                    //receivedfile name
                    char filename[1024];
                    memset(filename, '\0', 1024);
                    strcat(filename, directory_path.c_str());
                    strcat(filename, buf);
                    ifstream in_file(filename, ios::binary);
                    in_file.seekg(0, ios::end);
                    int file_size = in_file.tellg();
                    char buff[1024];
                    memset(buff, '\0', 1024);
                    strcat(buff,to_string(file_size).c_str());
                   
                    send(theirsideconnection_sockfd, buff, 1024, 0);
                
                    //sentfilesize
                    memset(buff, '\0', 1024);
                    received=recv(theirsideconnection_sockfd, buff, 1024, 0);
                    //startingtosendfile
                    memset(buff, '\0', 1024);
                    int sent=0;
                    in_file.close();
                    FILE *i_file = fopen(filename, "rb");
                    while (fread(buff, sizeof(char), 1024,i_file))
                    {   
                       
                        sent+=send(theirsideconnection_sockfd, buff, 1024, 0);
                        
                        memset(buff, '\0', 1024);
                    }
                    sendfile=0;
                    
                    continue;
                    //file sent successfully
            }
            else if(enquireneighbour){
               
                char buff[1024];
                for(int i=0;i<myfiles.size();i++){
                    memset(buff, '\0', 1024);
                    strcat(buff,myfiles[i].c_str());
                    int sent=send(theirsideconnection_sockfd, buff, 1024, 0);
                    memset(buff, '\0', 1024);
                    int received=recv(theirsideconnection_sockfd, buff, 1024, 0);
                }
                memset(buff, '\0', 1024);
                strcat(buff,"NOTHINGMOREDONE");
                int sent=send(theirsideconnection_sockfd, buff, 1024, 0);
                
                enquireneighbour=false;
                continue;
            }
           else if (enquiredp2){
               
               bool found=false;
               int buff_port = 0, buff_unique_id =99999;
               while(valid!=1){
                   sleep(2);
               }
               for(int i=0; i<neighbourfiles.size(); i++){
                   if(strncmp(neighbourfiles[i].c_str(),buf,neighbourfiles[i].size())==0){
                       found=true;
                       if(neighbourfiles_uniqueid[i]<buff_unique_id){
                           buff_port = neighbourfiles_port[i];
                           buff_unique_id = neighbourfiles_uniqueid[i];
                       }
                   }
               }
             
               char buff[1024];
               memset(buff, '\0', 1024);
               if(!found){
                     strcat(buff,"NO");
                     send(theirsideconnection_sockfd,buff,1024,0);
               }
               else {
                    strcat(buff,to_string(buff_port).c_str());
                    send(theirsideconnection_sockfd,buff,1024,0);
                    memset(buff, '\0', 1024);
                    recv(theirsideconnection_sockfd,buff,1024,0);
                    memset(buff, '\0', 1024);
                    strcat(buff,to_string(buff_unique_id).c_str());
                    send(theirsideconnection_sockfd,buff,1024,0);
                }
                enquiredp2=false;
                continue;
           }
        }
        return 0;


}

int listenthread(int listeningport,int my_uniqueid,int BACKLOG,vector <string> myfiles,string directory_path){
    
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
            theirconnectionthread.push_back(thread(connectedthread,newallotedsoc_fd,myfiles,my_uniqueid,directory_path));
  
        }   
        else                            break;
    }
    for(int i=0;i<theirconnectionthread.size();i++){
        theirconnectionthread[i].join();
    }
    return 0;
}
//receive files from neighbours
int requestconnectionthread(int no_neighbours,vector <int> neighbourid,vector <int> neighbourport,vector <string> search_filenames,string directory_path){
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
        cout<<"Connected to "<<neighbourid[i]<<" with unique-ID "<<buf<<" on port "<<neighbourport[i]<<endl;
        neighbouruniqueid.push_back(stoi(buf));
      
    }
    for(int i=0;i<no_neighbours;i++){
        char buff[1024];
        memset(buff, '\0', 1024);
        strcat(buff,"ENQUIRE YOURSELF");
        int sent=send(mysideconnection_sockfd[i],buff,1024,0);
         memset(buff, '\0', 1024);
        int received=recv(mysideconnection_sockfd[i],buff,1024,0);
    }
    vector <int> fileowneruniqueid;
    vector <int>    fileownerdepth;
    vector <int> fileownerneighbourno;
    for(int i=0;i<search_filenames.size();i++){
        bool found=false;
        string currentfile=search_filenames[i];
        fileowneruniqueid.push_back(0);
        fileownerdepth.push_back(0);
        unsigned char q[MD5_DIGEST_LENGTH]="0";
        fileownerneighbourno.push_back(-1);
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
                        fileownerneighbourno[fileownerneighbourno.size()-1]=j;
                    }
            }
            else{
                found=true;
                fileownerdepth[fileownerdepth.size()-1]=1;
                fileowneruniqueid[fileowneruniqueid.size()-1]=neighbouruniqueid[j];
                fileownerneighbourno[fileownerneighbourno.size()-1]=j;
            }

        }  
    }
    for(int i=0;i<no_neighbours;i++){
        char buff[1024];
        memset(buff, '\0', 1024);
        strcat(buff, "ENQUIRE NEIGHBOUR");
       
        send(mysideconnection_sockfd[i], buff, 1024, 0);
        memset(buff, '\0', 1024);
        recv(mysideconnection_sockfd[i], buff, 1024, 0);
        int relevevant_id=atoi(buff);
        
        while(true){
            
            memset(buff, '\0', 1024);
            strcat(buff, "SEND NEXT");
            send(mysideconnection_sockfd[i], buff, 1024, 0);
            memset(buff, '\0', 1024);
            recv(mysideconnection_sockfd[i], buff, 1024, 0);

            if(strncmp(buff,"NOTHINGMOREDONE", 15)==0){
               break;
            }
           
            neighbourfiles.push_back(buff);
            neighbourfiles_port.push_back(neighbourport[i]);
            neighbourfiles_uniqueid.push_back(relevevant_id);
        }
       
    }
  
    valid=1;
    for(int i=0;i<search_filenames.size();i++){
        if(fileownerdepth[i]==1) continue;
        bool found=false;
        string currentfile=search_filenames[i];
      
        for(int j=0;j<no_neighbours;j++){
            char buf[1024];
            memset(buf, '\0', 1024);
            strcat(buf,"ENQUIREDP2");
            int sent=send(mysideconnection_sockfd[j],buf,1024,0);
            memset(buf, '\0', 1024);
            recv(mysideconnection_sockfd[j],buf,1024,0);
           
            memset(buf, '\0', 1024);
             
            strcat(buf, currentfile.c_str());
            sent=send(mysideconnection_sockfd[j],buf,1024,0);
            memset(buf, '\0', 1024); 
      
            recv(mysideconnection_sockfd[j], buf, 1024, 0);
       
            char not_found[3]="NO";
            int cur_port;
            
            int cur_uniqueid;
           
            if(strncmp(buf,not_found,2)==0) continue;
            else{
                cur_port=atoi(buf);
             
                memset(buf, '\0', 1024);
                strcat(buf,"SEND UNIQUE ID");
                send(mysideconnection_sockfd[j],buf,1024,0);
                memset(buf, '\0', 1024); 
                recv(mysideconnection_sockfd[j], buf, 1024, 0);
                cur_uniqueid=atoi(buf);
                
            }
            if(found){
                    if(cur_uniqueid<fileowneruniqueid[i]){
                        fileowneruniqueid[i]=cur_uniqueid;
                        fileownerneighbourno[i]=cur_port;
                    }
            }
            else{
                found=true;
                fileownerdepth[i]=2;
                fileowneruniqueid[i]=cur_uniqueid;
                fileownerneighbourno[i]=cur_port;
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

int main(int argc, char** argv){
    string client_configurationfilename=argv[1];
    string directory_path=argv[2];
    if(int(directory_path[directory_path.size()-1]) != 47)
    directory_path=directory_path+'/';
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
    thread thread1(listenthread,client_portno,client_uniqueid,MAX_QUEUE,client_files,directory_path);
    thread thread2(requestconnectionthread,no_neighbours,neighbour_id,neighbour_port,search_filenames,directory_path);
    thread1.join();
    thread2.join();
    
}
