#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#define MAXLINE 4096 /*max text line length*/
//#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections */
#define buff_max_size 99999


int SERV_PORT,listenfd, connfd, n;
socklen_t clilen;
char fbuff1[buff_max_size], *fbuff2, fbuff3[buff_max_size], *fbuff4;
char recv_buff[buff_max_size], username[30], username_rec[30], password[30], password_rec[30], *conf_buffer, req_method[30];
char fname1[30], fname2[30], root_dir[200], subfolder[60];
int f1_size, f2_size, server_no, req_version, found_version;
struct sockaddr_in cliaddr, servaddr;
size_t max = 200;
int file_presentflag, version_presentflag;



void handle_request(int socketfd)
{
                
        while(1)
        {
                bzero(recv_buff, sizeof(recv_buff));
                n = recv(socketfd, recv_buff, MAXLINE,0);  
                if(n > 0)
                {
                       
                        char decrypt[buff_max_size];
                        bzero(decrypt, sizeof(decrypt));
                        bzero(fbuff1, sizeof(fbuff1));
                        bzero(fbuff3, sizeof(fbuff3));
                        strncpy(fbuff1, recv_buff, strlen(recv_buff));
                        bzero(recv_buff, sizeof(recv_buff));
                        bzero(root_dir, sizeof(root_dir));

                        bzero(fname1, sizeof(fname1));
                        bzero(fname2, sizeof(fname2));
                        bzero(req_method, sizeof(req_method));
                        bzero(username_rec, sizeof(username_rec));
                        bzero(password_rec, sizeof(password_rec));
                        char key[30];

                        if( (strncmp(fbuff1,"PUT",3) ==0 ) || (strncmp(fbuff1,"put",3) ==0 ) )
                        { 
                                
                                printf("\n\n");
                                sscanf(fbuff1,"%s %s %s %d %s %d %s %s %d", req_method, subfolder, fname1, &f1_size, fname2, &f2_size, username_rec, password_rec, &server_no );
                                printf("\n %s %s %s %d %s %d %s %s %d\n", req_method, subfolder, fname1, f1_size, fname2, f2_size, username_rec, password_rec, server_no );
                                
                                if( (strcmp(username, username_rec) == 0) && (strcmp(password, password_rec) == 0))
                                {               
                                        send(socketfd, "ACK", 3, 0);


                /*******************************   WRITING 1ST FILE PART   *************************************/

                                        int len2 = strlen(password);
                                        strncpy(key, password, len2);
                                        fbuff2 = (char*)calloc(buff_max_size,sizeof(char));
                                        if(fbuff2 == NULL) printf("fbuff2 mem not allocated\n\n");
                                        n = recv(socketfd, fbuff2, buff_max_size, 0);
                                        
                                        char usr_dir[40];
                                        sprintf(usr_dir, "./DFS%d/%s", server_no, username);

                                        strncpy(root_dir, usr_dir, strlen(usr_dir));

                                /*** Checking if directory for a Username Already Exists, if not create one ***/

                                        DIR* dir = opendir(usr_dir);
                                        if(ENOENT == errno)
                                        {
                                                printf("\n Directory does not exist. Making one....\n");
                                                mkdir(usr_dir, 0777);

                                        }

                                        sprintf(usr_dir, "%s/%s", root_dir, subfolder);
                                        bzero(root_dir, sizeof(root_dir));
                                        strncpy(root_dir, usr_dir, strlen(usr_dir));            // root_dir = ./DFS*/Username/Subfolder
   
                                        dir = opendir(usr_dir);
                                        if(ENOENT == errno)
                                        {
                                                printf("\n Directory does not exist. Making one....\n");
                                                mkdir(usr_dir, 0777);

                                        }

                                        sprintf(usr_dir,"%s/%s",root_dir, fname1);
                                        FILE *fp1;
                                        fp1 = fopen(usr_dir, "wb");
                                        
                                        if(fp1 != NULL)
                                        {
                                                //printf("\n\n buff size is %d\n", f1_size);

                                                /*for(int l=0; l<f1_size; l++)
                                                {
                                                        decrypt[l] = recv_buff[l]^key[l%len2];

                                                }
                                                printf("******************************************\n");
                                                puts(decrypt);*/
                                                fwrite(fbuff2, f1_size, 1, fp1);

                                                fclose(fp1);
                                                free(fbuff2);
                                                send(socketfd, "ACK", 3, 0);
                                        }

                                        else
                                        {
                                                printf("Unable to write\n");
                                                send(socketfd,"FILE NOT WRITTEN", 16, 0);

                                        }

                /*****************************   WRITING 2ND FILE PART   *************************************/

                                        fbuff4 = (char*)calloc(buff_max_size,sizeof(char));
                                        if(fbuff4 == NULL) printf("fbuff2 mem not allocated\n\n");

                                        n = recv(socketfd, fbuff4, buff_max_size, 0);
                                        //strncpy(fbuff3, recv_buff, f2_size);

                                        sprintf(usr_dir,"%s/%s",root_dir, fname2);
                                        FILE *fp2;
                                        fp2 = fopen(usr_dir, "wb");
                                        
                                        if(fp2 != NULL)
                                        {
                                                /*bzero(decrypt, sizeof(decrypt));
                                                for(int l=0; l<f2_size; l++)
                                                {
                                                        decrypt[l] = recv_buff[l]^key[l%len2];

                                                }

                                                
                                                //printf("\n\n buff size is %d\n", f1_size);*/
                                                fwrite(fbuff4, f2_size, 1, fp2);

                                                fclose(fp2);
                                                free(fbuff4);
                                                send(socketfd, "ACK", 3, 0);
                                        }

                                        else
                                        {
                                                printf("Unable to write\n");
                                                send(socketfd,"FILE NOT WRITTEN", 16, 0);

                                        }


                                        
                                }

                                else            //if username/password is invalid
                                {
                                        printf("Invalid Username/Password received ! \n");
                                        send(socketfd, "Invalid Username/Password", 25, 0);
                                }
                        
                        
                        }

                        else if( (strncmp(fbuff1,"MKDIR",5) ==0 ) || (strncmp(fbuff1,"mkdir",5) ==0 ))
                        {
                                sscanf(fbuff1, "%s %s %s %s %d", req_method, subfolder, username_rec, password_rec, &server_no);
                                printf("Request : %s %s %s %s %d", req_method, subfolder, username_rec, password_rec, server_no);

                                if( (strcmp(username, username_rec) == 0) && (strcmp(password, password_rec) == 0))
                                {               
                                        send(socketfd, "ACK", 3, 0);
                                        
                                        char usr_dir[40];
                                        
                                        sprintf(usr_dir, "./DFS%d/%s", server_no, username);
                                        
                                        
                                        strncpy(root_dir, usr_dir, strlen(usr_dir));

                                /*** Checking if subfolder for a Username/subfolder Already Exists, if not create one ***/

                                        DIR* dir = opendir(usr_dir);
                                        if(ENOENT == errno)
                                        {
                                                printf("\n subfolder does not exist. Making one....\n");
                                                mkdir(usr_dir, 0777);

                                        }
                                        

                                        sprintf(usr_dir, "%s/%s", root_dir, subfolder);
                                        DIR* dir1 = opendir(usr_dir);
                                        if(ENOENT == errno)
                                        {
                                                printf("\n subfolder does not exist. Making one....\n");
                                                mkdir(usr_dir, 0777);

                                        }
                                        

                                }

                                
                                else
                                {
                                        
                                        printf("Invalid Username/Password received ! \n");
                                        send(socketfd, "Invalid Username/Password", 25, 0);
                                
                                }

                        }

                        else if( (strncmp(fbuff1,"LIST",4) ==0 ) || (strncmp(fbuff1,"list",4) ==0 ) )
                        {

                                sscanf(fbuff1, "%s %s %s %s %d", req_method, subfolder, username_rec, password_rec, &server_no);
                                printf("Request : %s %s %s %s %d\n", req_method, subfolder, username_rec, password_rec, server_no);       

                                if( (strcmp(username, username_rec) == 0) && (strcmp(password, password_rec) == 0))
                                {               
                                        send(socketfd, "ACK", 3, 0);
                                        char usr_dir[40];

                                        bzero(recv_buff, sizeof(recv_buff));
                                        n = recv(socketfd, recv_buff, buff_max_size, 0);
                                        printf("Client says : ");
                                        puts(recv_buff);

                           /***************** Check if folders exists, if not create **************************/

                                        sprintf(usr_dir, "./DFS%d/%s", server_no, username);
                                        strncpy(root_dir, usr_dir, strlen(usr_dir));

                                        DIR* dir = opendir(usr_dir);
                                        if(ENOENT == errno)
                                        {
                                                printf("\n username folder does not exist. Making one....\n");
                                                mkdir(usr_dir, 0777);

                                        }
                                        

                                        sprintf(usr_dir, "%s/%s", root_dir, subfolder);
                                        bzero(root_dir, sizeof(root_dir));
                                        strncpy(root_dir, usr_dir, strlen(usr_dir));

                                        DIR* dir1 = opendir(usr_dir);
                                        if(ENOENT == errno)
                                        {
                                                printf("\n subfolder does not exist. Making one....\n");
                                                mkdir(usr_dir, 0777);

                                        }
                                        /********************************************************/

                                        struct dirent **ep1;
                                        char* dir_contents[500];
                                        int dir_contentcount[30], count;
                                        count = scandir(root_dir, &ep1, NULL, alphasort);
                                        dir_contentcount[0] = count;
                                        printf("\n Content count : %d\n", dir_contentcount[0] );

                                        //sending number of contents of subfolder
                                        send(socketfd, dir_contentcount, 20, 0);
                                        printf("Directory content-Count sent\n");
                                        
                                        bzero(recv_buff, sizeof(recv_buff));
                                        n = recv(socketfd, recv_buff, buff_max_size, 0);
                                        printf("Client says : ");
                                        puts(recv_buff); 

                                        for(int i =1 ; i <= dir_contentcount[0]; i++)
                                        {
                                                //printf("%s\n",ep1[i-1]->d_name);
                                                *(dir_contents + i - 1) = ep1[i-1]->d_name;
                                        }
                                        printf("Contents of the subfolder :\n ");
                                        
                                        
                                        for(int cnt =0 ; cnt < dir_contentcount[0]; cnt++)
                                        {
                                                 
                                                send(socketfd, dir_contents[cnt], 90, 0);

                                                bzero(recv_buff, sizeof(recv_buff));
                                                n = recv(socketfd, recv_buff, buff_max_size, 0);
                                                printf("Client says : ");
                                                puts(recv_buff); 


                                        }



                                        

                                }

                                
                                else
                                {
                                        
                                        printf("Invalid Username/Password received ! \n");
                                        send(socketfd, "Invalid Username/Password", 25, 0);
                                
                                }
                
                        }

                        
                        else if( (strncmp(fbuff1,"GET",3) ==0 ) || (strncmp(fbuff1,"get",3) ==0 ) )
                        {
                                
                                sscanf(fbuff1, "%s %s %s %s %s %d %d", req_method, fname1, subfolder, username_rec, password_rec, &server_no, &req_version);
                                printf("Request : %s %s %s %s %s %d %d\n", req_method, fname1, subfolder, username_rec, password_rec, server_no, req_version);

                                if( (strcmp(username, username_rec) == 0) && (strcmp(password, password_rec) == 0))
                                {

                                        char usr_dir[200], dotfile[100];
                                        file_presentflag = 0;
                                        version_presentflag =0;
                                        sprintf(usr_dir, "./DFS%d/%s/%s", server_no, username, subfolder);
                                        strncpy(root_dir, usr_dir, strlen(usr_dir));
                                        DIR* dir = opendir(usr_dir);                    //No folders on DFS
                                        if(ENOENT == errno)
                                        {
                                                send(socketfd, "Requested Path to file does not exist",37, 0);

                                        }

                                        else
                                        {
                                                struct dirent **ep1;
                                                char* dir_contents[500];
                                                bzero(dir_contents, sizeof(dir_contents));
                                                int dir_contentcount[30], count;
                                                count = scandir(root_dir, &ep1, NULL, alphasort);
                                                dir_contentcount[0] = count;

                                                if(dir_contentcount[0] > 2)
                                                {
                                                        for(int g =0 ; g < dir_contentcount[0]; g++)
                                                        {
                                                                //printf("%s\n",ep1[i-1]->d_name);
                                                                if( (strcmp(ep1[g]->d_name,".") != 0) || (strcmp(ep1[g]->d_name,"..") != 0) )
                                                                {
                                                                        *(dir_contents + g) = ep1[g]->d_name;
                                                
                                                                        if(strstr(*(dir_contents + g), fname1) != NULL)
                                                                        {
                                                                                file_presentflag = 1;
                                                
                                                                                printf("Requested file exists\n");
                                                                                char *ret;
                                                                                ret = strrchr(*(dir_contents + g),'.');
                                                                                char str1[20];
                                                                                strcpy(str1, ret+1);
                                                                                found_version = atoi(str1);
                                                                                printf("Found version : %d\n", found_version);
                                                                                if(found_version == req_version)
                                                                                {
                                                                                        version_presentflag=1;
                                                                                        printf("Requested version exists\n");
                                                                                
                                                                                        sprintf(dotfile,"/.%s.%d", fname1, req_version);
                                                                                        strcat(root_dir,dotfile);
                                                                                        FILE *ft;

                                                                                        ft = fopen(root_dir,"rb");
                                                                                        if(ft != NULL){
                                                                                        fseek(ft, 0 , SEEK_END);
                                                                                        f2_size = ftell(ft);
                                                                                        fseek(ft, 0 , SEEK_SET);
                                                                                        fclose(ft);
                                                                                        }
                                                                                        bzero(fbuff3, sizeof(fbuff3));
                                                                                        sprintf(fbuff3, "Available %d", f2_size);
                                                                                        send(socketfd, fbuff3, strlen(fbuff3),0);

                                                                                        bzero(recv_buff, sizeof(recv_buff));
                                                                                        int re = recv(socketfd, recv_buff, 100, 0);

                                                                                        printf("Client says : ");
                                                                                        puts(recv_buff);

                                                                                        fbuff2 = (char*)calloc(buff_max_size,sizeof(char));
                                                                                        if(fbuff2 == NULL) printf("fbuff2 mem not allocated\n\n");
                                                                                        ft = fopen(root_dir,"rb");
                                                                                        if(ft != NULL){
                                                                                        int arr= fread(fbuff2, sizeof(char), f2_size, ft);
                                                                                        if(arr < 0)
                                                                                        {
                                                                                                printf("File not read");
                                                                                                send(socketfd,"File not read",13,0);
                                                                                        }
                                                                                        else send(socketfd,fbuff2,f2_size,0);
                                                                                        }
                                                                                        free(fbuff2);
                                                                                        break;
                                                                                                                                                   
                                                                                }

                                                                        }

                                                                     

                                                                } // if
                                                
                                                        } // for

                                                        if(file_presentflag == 0)
                                                        {
                                                                send(socketfd, "File Unavailable at DFS",22,0);
                                                        }

                                                        else if(version_presentflag == 0)
                                                        {
                                                                send(socketfd, "Version Unavailable at DFS",25,0);
                                                        }

                                                }

                                                else
                                                {
                                                        send(socketfd, "No files on Server", 18, 0);

                                                }


                                        } // else condition for path exists
                                        
                                } // if username & password matches

                                else
                                {
                                        
                                        printf("Invalid Username/Password received ! \n");
                                        send(socketfd, "Invalid Username/Password", 25, 0);
                                
                                }
                                

                        }                        
                        
                        else            //if req_method is not put/get/list
                        {
                                send(socketfd, "Invalid Request Method", 22, 0);
                        }

                }

                else
                {
                        perror("Read error");
                        close(connfd);
                        exit(1);
                }


        }


}

int main (int argc, char **argv)
{
 

        if (argc !=2) 
        {
                perror("Usage: TCPClient <PORT of the server");
                exit(1);
        }

        SERV_PORT = atoi(argv[1]);

        FILE *conf = fopen("dfs.conf", "r");
        if(conf == NULL)
        {
                printf("File not open");
                exit(1);
        }
        else
        {
                conf_buffer = (char *)malloc(max*sizeof(char));
                while(!feof(conf))
                {
                        getline(&conf_buffer,&max,conf);
                        
                        if(strncmp(conf_buffer,"Username",8) == 0)
                        {
                                char *tok1;
                                tok1 = strtok(conf_buffer,":");
                                tok1 = strtok(NULL, " \n\r");
                                //tok1 = strtok(NULL, " \t\n");
                                strncpy(username, tok1, strlen(tok1));
                                puts(username);
                                
                        }
                        else if(strncmp(conf_buffer,"Password",8) == 0)
                        {
                                char *tok1;
                                tok1 = strtok(conf_buffer,":");
                                tok1 = strtok(NULL, " \t\n");
                                int str_len = strlen(tok1);
                                strncpy(password, tok1, str_len);
                                puts(password);

                        }


                } 


        }
 
        //creation of the socket
        listenfd = socket (AF_INET, SOCK_STREAM, 0);

        //preparation of the socket address
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(SERV_PORT);

        bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

        listen (listenfd, LISTENQ);

        printf("%s\n","Server running...waiting for connections.");

        for ( ; ; )
        {
                clilen = sizeof(cliaddr);
                connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
                printf("%s\n","Received request...");
                if(connfd != 0)
                {

                        handle_request(connfd);
                }
                else  close(connfd);
                

        }
 //close listening socket
 close (listenfd);
}
