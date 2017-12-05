#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <openssl/md5.h>
#include <dirent.h>
#include <errno.h>
#include "md5_cal.h"

#define MAXLINE 400 /*max text line length*/
#define SERV_PORT 3000
#define buff_max_size 99999

FILE *conf;
char dfc_conf[30], *conf_buffer, username[20], password[20], cmd[60], req_method[30];
char filename[30], send_buff[buff_max_size], subfolder[60], rec_buff1[buff_max_size];
char fbuff1[buff_max_size], fbuff2[buff_max_size], fbuff3[buff_max_size], fbuff4[buff_max_size];
char *DFS1_list[99],*DFS2_list[99], *DFS3_list[99], *DFS4_list[99];
int DFS1_count[10], DFS2_count[10], DFS3_count[10], DFS4_count[10];
int port[4], b_size[4], chunk_size,rem_size;
int sockfd1, sockfd2, sockfd3, sockfd4 ;
struct sockaddr_in servaddr;
size_t max = 200;
int svar=0, ver_num=0;

struct valid_file{
char fl[100];
int vr;
};

struct fileversions{

char file_name[100];
int version;

};

struct fileversions fv[1000], fv_ptr;
struct valid_file vf[1000];

void store_version(char *str)
{

        char file1[100], dn[100];
        int vers;
        char *tok;

        bzero(file1, sizeof(file1));
        strncpy(dn, str, strlen(str));
        tok = strtok(dn, ".");
        strcat(file1, tok);
        strcat(file1, ".");
        tok = strtok(NULL, ".");
        strcat(file1, tok);

        strncpy(fv[svar].file_name, file1, strlen(file1));
        //printf(" %d file :%s\n ", svar, fv[svar].file_name);
        tok = strtok(NULL, ".");
        vers = atoi(tok);
        fv[svar].version = vers;
        //printf(" %d file :%d\n ", svar, fv[svar].version);
        svar++;
}


void extract_ports(char *buff1, int i)
{

        char *tok;
        tok = strtok(buff1, ":");
        tok = strtok(NULL, " \t\n");
        port[i] = atoi(tok);
        printf("port %d : %d\n", i,port[i]);

}


void sendto_server(int socketfd, int server_no, char *filename, char *buf1, int suffix1, char *buf2, int suffix2, char *Username, char *Password, char *req_type, char *sub_f)
{
        int rec, len1, len2;
        char fname1[30], fname2[30], rec_buff[90];
        bzero(rec_buff, sizeof(rec_buff));

        sprintf(fname1, ".%s.%d", filename, suffix1);
        sprintf(fname2, ".%s.%d", filename, suffix2);

        len1 = b_size[suffix1 - 1];
        len2 = b_size[suffix2 - 1];



        sprintf(send_buff,"%s %s %s %d %s %d %s %s %d ", req_type, sub_f, fname1, len1, fname2, len2, Username, Password, server_no);
        //puts(send_buff);
        send(socketfd, send_buff, strlen(send_buff), 0);
        


        rec = recv(socketfd, rec_buff, 90, 0);
        printf("\n\n After Command send,Server says : ");
        puts(rec_buff);


        if(strncmp(rec_buff, "ACK",3) == 0)
        {
                /***********************      SEND  buf1       **********************/                
        
                send(socketfd, buf1, len1, 0);

                bzero(rec_buff, sizeof(rec_buff));
                rec = recv(socketfd, rec_buff, 90, 0);
                printf("\n\n After sending 1st PART Server says : ");
                puts(rec_buff);

                if(strncmp(rec_buff, "ACK",3) == 0)
                {
                        bzero(rec_buff, sizeof(rec_buff));
                      
                /***********************      SEND  buf2       **********************/   

                        send(socketfd, buf2, len2, 0);
                        rec = recv(socketfd, rec_buff, 90, 0);
                        printf("\n\n After sending 2nd PART Server says : ");
                        puts(rec_buff);

                }

        }

        else printf("Invalid Username/Password. Could not proceed");


}

void get_dircontents(int socketfd, char *req_type, char *sub_folder, char *Username, char *Password, int server_no)
{

                        sprintf(send_buff, "%s %s %s %s %d", req_type, sub_folder, Username, Password, server_no);
                        send(socketfd, send_buff, strlen(send_buff), 0 );
                        
                        bzero(rec_buff1, sizeof(rec_buff1));
                        int rec = recv(socketfd, rec_buff1, 90, 0);
                        
                        

                       
                        if(strncmp(rec_buff1, "ACK", 3) == 0)
                        {
                                sprintf(send_buff, "Send directory contents COUNT");
                                send(socketfd, send_buff, strlen(send_buff), 0 );

                                if(server_no == 1)
                                {
                                        int rec = recv(socketfd, DFS1_count, 90, 0);
                                        printf("\n\n Directory contents count under DFS %d 's subfolder :\n ", server_no);
                                        printf("Count received : %d\n",DFS1_count[0]);

                                        sprintf(send_buff, "Now Send the contents");
                                        send(socketfd, send_buff, strlen(send_buff),0);
                                
                                        for(int h=0; h<DFS1_count[0]; h++)
                                        {
                                                bzero(rec_buff1, sizeof(rec_buff1));
                                                int rec = recv(socketfd, rec_buff1, 90, 0);
                                                
                                                if(h>1)
                                                {
                                                        *(DFS1_list + h) = rec_buff1;                                                
                                                        //printf("%s\n", DFS1_list[h] );
                                                        store_version(*(DFS1_list + h));
                                                }                                                
                                                send(socketfd, "ACK", 3,0);
                                               
                                        }

                                        




                                }

                                else if(server_no == 2)
                                {
                                        int rec = recv(socketfd, DFS2_count, 90, 0);
                                        printf("\n\n Directory contents count under DFS %d 's subfolder :\n ", server_no);
                                        printf("Count received : %d\n",DFS2_count[0]);


                                        sprintf(send_buff, "Now Send the contents");
                                        send(socketfd, send_buff, strlen(send_buff),0);

                                        for(int h=0; h<DFS2_count[0]; h++)
                                        {
                                                bzero(rec_buff1, sizeof(rec_buff1));
                                                int rec = recv(socketfd, rec_buff1, 90, 0);
                                                
                                                if(h>1)
                                                {  
                                                        *(DFS2_list + h) = rec_buff1;                                              
                                                        //printf("%s\n", DFS2_list[h] );
                                                        store_version(*(DFS2_list + h));
                                                }
                                                send(socketfd, "ACK", 3,0); 
                                        }
                                }

                                else if(server_no == 3)
                                {
                                        int rec = recv(socketfd, DFS3_count, 90, 0);
                                        printf("\n\n Directory contents count under DFS %d 's subfolder :\n ", server_no);
                                       printf("Count received : %d\n",DFS3_count[0]);

                                        sprintf(send_buff, "Now Send the contents");
                                        send(socketfd, send_buff, strlen(send_buff),0);

                                        for(int h=0; h<DFS3_count[0]; h++)
                                        {
                                                bzero(rec_buff1, sizeof(rec_buff1));
                                                int rec = recv(socketfd, rec_buff1, 90, 0);
                                                
                                                if(h>1)
                                                {
                                                        *(DFS3_list + h) = rec_buff1;
                                                        //printf("%s\n", DFS3_list[h] );
                                                        store_version(*(DFS3_list + h));
                                                }
                                                send(socketfd, "ACK", 3,0);
                                        }
                                }

                                else if(server_no == 4)
                                {
                                        int rec = recv(socketfd, DFS4_count, 90, 0);
                                        printf("\n\n Directory contents count under DFS %d 's subfolder :\n ", server_no);
                                        printf("Count received : %d\n",DFS4_count[0]);

                                        sprintf(send_buff, "Now Send the contents");
                                        send(socketfd, send_buff, strlen(send_buff),0);

                                        for(int h=0; h<DFS4_count[0]; h++)
                                        {
                                                bzero(rec_buff1, sizeof(rec_buff1));
                                                int rec = recv(socketfd, rec_buff1, 90, 0);
                                               
                                                if(h>1)
                                                {
                                                        *(DFS4_list + h) = rec_buff1;
                                                        //printf("%s\n", DFS4_list[h] );
                                                        store_version(*(DFS4_list + h));
                                                }
                                                send(socketfd, "ACK", 3,0); 
                                        }
                                } // server_no = 4
                        
                                
                                

                        } // if ACK is received on sending main command

                        



}

int main(int argc, char **argv)
{
       
        char sendline[MAXLINE], recvline[MAXLINE];
        strncpy(sendline, "Send this please work", 21);
        //int svar =0;

        //basic check of the arguments
        //additional checks can be inserted
        if (argc !=2) 
        {
                perror("Usage: Name of dfc.conf file to be passed as an argument while running..");
                exit(1);
        }

        
 
        strcpy(dfc_conf, argv[1]);
        printf("argv1 : %s\n", dfc_conf);
        conf = fopen(dfc_conf, "r");
        if(conf == NULL)
        {
                printf(" Configuration file not open");
                exit(1);
        }

        else
        {
                int port_num=0;
                conf_buffer = (char *)malloc(max*sizeof(char));
                while(!feof(conf))
                {
                        getline(&conf_buffer,&max,conf);
                        if(strncmp(conf_buffer,"Server",6) == 0)
                        {
                                extract_ports(conf_buffer, port_num);
                                port_num++;
                        }

                        else if(strncmp(conf_buffer,"Username",8) == 0)
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
                                strncpy(password, tok1, strlen(tok1));
                                puts(password);

                        }


                } 
 
        }



        
        //Creation of the socket
       if ((sockfd1 = socket (AF_INET, SOCK_STREAM, 0)) <0) {
        perror("Problem in creating the socket");
        exit(2);
        }

        //Creation of the socket
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
        servaddr.sin_port =  htons(port[0]); //convert to big-endian order

        //Connection of the client to the socket
        if (connect(sockfd1, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
        perror("Problem in connecting to the server");
        exit(3);
        }
        else printf("Connected to server \n\n" );

        servaddr.sin_port =  htons(port[1]); //convert to big-endian order

        if ((sockfd2 = socket (AF_INET, SOCK_STREAM, 0)) <0)
        {
                perror("Problem in creating the socket");
                exit(2);
        }
       

        if (connect(sockfd2, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
                perror("Problem in connecting to the server");
                exit(3);
        }
        else printf("Connected to server \n\n" );


        servaddr.sin_port =  htons(port[2]); //convert to big-endian order
        if ((sockfd3 = socket (AF_INET, SOCK_STREAM, 0)) <0)
        {
                perror("Problem in creating the socket");
                exit(2);
        }


        
        if (connect(sockfd3, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
                perror("Problem in connecting to the server");
                exit(3);
        }
        else printf("Connected to server \n\n" );


        servaddr.sin_port =  htons(port[3]); //convert to big-endian order
        if ((sockfd4 = socket (AF_INET, SOCK_STREAM, 0)) <0)
        {
                perror("Problem in creating the socket");
                exit(2);
        }


        if (connect(sockfd4, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
                perror("Problem in connecting to the server");
                exit(3);
        }
        else printf("Connected to server \n\n" );

       


                /*send(sockfd1, sendline, strlen(sendline), 0);
                send(sockfd2, sendline, strlen(sendline), 0);
                send(sockfd3, sendline, strlen(sendline), 0);
                send(sockfd4, sendline, strlen(sendline), 0);*/
       while(1)
       {
                printf("\n\n\n");
                printf(" *-*-*-*-*-*-*-  The following commands are handled by the DFC :    *-*-*-*-*-*-*-\n");
                printf("                1. LIST subfolder : lists files stored at the server ofr a given username \n ");
                printf("               2. PUT filename subfolder: Sends a file to the distributed servers\n");
                printf("                3. GET filename subfolder: Retrieves a file from the distributed servers\n\n\r ");
                printf("                4. MKDIR subfolder : Makes a subfolder on all servers\n\n\r ");


                bzero(cmd, sizeof(cmd));
                bzero(filename, sizeof(filename));
                bzero(req_method, sizeof(req_method));


                fgets(cmd, MAXLINE, stdin);
                puts(cmd);

                if(strncmp(cmd, "PUT", 3) == 0) 
                {
                        char *tok2;
                        tok2 = strtok(cmd, " ");
                        strncpy(req_method, tok2, strlen(tok2));
                        puts(req_method);
                        tok2 = strtok(NULL, " ");
                        strncpy(filename, tok2, strlen(tok2));
                        puts(filename);
                        tok2 = strtok(NULL, " \t\n");
                        strncpy(subfolder, tok2, strlen(tok2));
                        puts(subfolder);


                        int mod = mod_from_md5(filename);
                        printf("mod : %d\n", mod);
                        chunk_size = 0;
                        rem_size =0;
                        file_divide(filename, fbuff1, fbuff2, fbuff3, fbuff4, b_size, password);

                        /*printf(" ********************************** Part 1 ************************************** \n");
                        puts(fbuff1);
                        printf(" ********************************** Part 2 ************************************** \n");
                        puts(fbuff2);
                        printf(" ********************************** Part 3 ************************************** \n");
                        puts(fbuff3);
                        printf(" ********************************** Part 4 ************************************** \n");
                        puts(fbuff4);*/               

                        switch(mod)
                        {
                                case 0:
                
                                        sendto_server(sockfd1, 1, filename, fbuff1, 1, fbuff2, 2, username, password, req_method, subfolder); 
                                        sendto_server(sockfd2, 2, filename, fbuff2, 2, fbuff3, 3, username, password, req_method, subfolder); 
                                        sendto_server(sockfd3, 3, filename, fbuff3, 3, fbuff4, 4, username, password, req_method, subfolder); 
                                        sendto_server(sockfd4, 4, filename, fbuff4, 4, fbuff1, 1, username, password, req_method, subfolder);
                                        break; 

                                case 1:

                                        sendto_server(sockfd1, 1, filename, fbuff4, 4, fbuff1, 1, username, password, req_method, subfolder);
                                        sendto_server(sockfd2, 2, filename, fbuff1, 1, fbuff2, 2, username, password, req_method, subfolder); 
                                        sendto_server(sockfd3, 3, filename, fbuff2, 2, fbuff3, 3, username, password, req_method, subfolder); 
                                        sendto_server(sockfd4, 4, filename, fbuff3, 3, fbuff4, 4, username, password, req_method, subfolder); 
 

                                        break; 


                                case 2:

                                        sendto_server(sockfd1, 1, filename, fbuff3, 3, fbuff4, 4, username, password, req_method, subfolder); 
                                        sendto_server(sockfd2, 2, filename, fbuff4, 4, fbuff1, 1, username, password, req_method, subfolder);
                                        sendto_server(sockfd3, 3, filename, fbuff1, 1, fbuff2, 2, username, password, req_method, subfolder);
                                        sendto_server(sockfd4, 4, filename, fbuff2, 2, fbuff3, 3, username, password, req_method, subfolder); 
 
                                        break; 


                                case 3:

                                        sendto_server(sockfd1, 1, filename, fbuff2, 2, fbuff3, 3, username, password, req_method, subfolder); 
                                        sendto_server(sockfd2, 2, filename, fbuff3, 3, fbuff4, 4, username, password, req_method, subfolder); 
                                        sendto_server(sockfd3, 3, filename, fbuff4, 4, fbuff1, 1, username, password, req_method, subfolder);
                                        sendto_server(sockfd4, 4, filename, fbuff1, 1, fbuff2, 2, username, password, req_method, subfolder); 

                                        break; 

                        }
                
                } // if command put


                else if( strncmp(cmd, "GET", 3) == 0 )
                {
                        char *tok2;
                        tok2 = strtok(cmd, " ");
                        strncpy(req_method, tok2, strlen(tok2));
                        puts(req_method);
                        tok2 = strtok(NULL, " \t\n");
                        strncpy(filename, tok2, strlen(tok2));
                        puts(filename);

                        

                }

                else if( strncmp(cmd, "LIST", 4) == 0)
                {
                        char *tok;
                        tok = strtok(cmd, " ");
                        strncpy(req_method, tok, strlen(tok));
                        puts(req_method);
                        tok = strtok(NULL, " \t\n");
                        strncpy(subfolder, tok, strlen(tok));
                        puts(subfolder);

                        bzero(DFS1_count, sizeof(DFS1_count));
                        bzero(DFS2_count, sizeof(DFS2_count));
                        bzero(DFS3_count, sizeof(DFS3_count));
                        bzero(DFS4_count, sizeof(DFS4_count));
                        memset(fv, 0, 100*sizeof(struct fileversions));
                        memset(vf, 0, 100*sizeof(struct fileversions));

                        svar = 0;
                        get_dircontents(sockfd1, req_method, subfolder, username, password, 1);
                        get_dircontents(sockfd2, req_method, subfolder, username, password, 2);
                        get_dircontents(sockfd3, req_method, subfolder, username, password, 3);
                        get_dircontents(sockfd4, req_method, subfolder, username, password, 4);


                        int yu = DFS1_count[0] +  DFS2_count[0] +  DFS3_count[0] +  DFS4_count[0];
                            yu = yu - 8;      
                                /*for(int m=0; m<yu; m++)             // 2 dirs escaped in each DFS : . & ..
                                {
                                        printf(" Structur %d has file %s and version %d\n", m, fv[m].file_name, fv[m].version);

                                }*/ 
                                int m,a;
                
                        /****  From the array of structures, each struct where all filenames received from DFSs are save, 
                                compare each with a valid file structure, which is btw initially set to 0, if no filename is 
                                present in valid file struct, then populate it. if matching filename, then update the version number
                                for version 1,2,3,4 -------   011110 denotes all versions present  ******/

                                for( m=0; m<yu; m++)             
                                {
                                        for( a=0; a<yu; a++)
                                        {            
                                                if(strcmp(fv[m].file_name , vf[a].fl) == 0) break;
                                                else if(vf[a].fl[0] == 0) break;
                                        }

                                        if(vf[a].fl[0] == 0)
                                        {
                                                strcpy(vf[a].fl,fv[m].file_name);
                                                vf[a].vr |= 1 << fv[m].version;
                                        }

                                        else if(strcmp(fv[m].file_name , vf[a].fl) == 0)
                                        {
                                                vf[a].vr |= 1 << fv[m].version;
                                        }
                                }
                                
                                printf("\n\n **************************  LIST RESULTS  ***************************\n\n");
                                for(int d=0; d<100; d++)
                                {
                                        if(vf[d].vr == 30) printf(" %d. %s Full File\n", d+1, vf[d].fl);
                                        else if( (vf[d].vr > 0) &&(vf[d].vr < 30)) printf(" %d. %s Full File\n", d+1, vf[d].fl);
                                        else if( vf[0].fl[0] == 0)
                                        {
                                                 printf("\n THE DFS SERVERS DONT HAVE ANY FILES :( \n");
                                                 break;
                                        }
                                        else if(vf[d].fl[0] == 0) break;

                                }



                        svar=0;

                }


                else if( strncmp(cmd, "MKDIR", 5) == 0)
                {
                        char *tok3;
                        int rec;
                        tok3 = strtok(cmd, " ");
                        strncpy(req_method, tok3, strlen(tok3));
                        tok3 = strtok(NULL, " \t\n");
                        strncpy(subfolder, tok3, strlen(tok3));
                        printf("Making the subfolder on all servers : ");
                        puts(subfolder);

                                                /**************************/
                        sprintf(send_buff, "%s %s %s %s %d", req_method, subfolder, username, password, 1);
                        send(sockfd1, send_buff, strlen(send_buff), 0 );

                        bzero(rec_buff1, sizeof(rec_buff1));
                        rec = recv(sockfd1, rec_buff1, 90, 0);
                        printf("\n\n After sending command to mkdir,Server1 says : ");
                        puts(rec_buff1);

                                                /**************************/                       
                        sprintf(send_buff, "%s %s %s %s %d", req_method, subfolder, username, password, 2);
                        send(sockfd2, send_buff, strlen(send_buff), 0 );

                        bzero(rec_buff1, sizeof(rec_buff1));
                        rec = recv(sockfd2, rec_buff1, 90, 0);
                        printf("\n\n After sending command to mkdir,Server2 says : ");
                        puts(rec_buff1);

                                                /**************************/
                        sprintf(send_buff, "%s %s %s %s %d", req_method, subfolder, username, password, 3);
                        send(sockfd3, send_buff, strlen(send_buff), 0 );

                        bzero(rec_buff1, sizeof(rec_buff1));
                        rec = recv(sockfd3, rec_buff1, 90, 0);
                        printf("\n\n After sending command to mkdir,Server1 says : ");
                        puts(rec_buff1);

                                                /**************************/
                        sprintf(send_buff, "%s %s %s %s %d", req_method, subfolder, username, password, 4);
                        send(sockfd4, send_buff, strlen(send_buff), 0 );

                        bzero(rec_buff1, sizeof(rec_buff1));
                        rec = recv(sockfd4, rec_buff1, 90, 0);
                        printf("\n\n After sending command to mkdir,Server1 says : ");
                        puts(rec_buff1);



                }


        } // while




         
        
 //exit(0);
} // main
