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
char filename[30], send_buff[buff_max_size];
char fbuff1[buff_max_size], fbuff2[buff_max_size], fbuff3[buff_max_size], fbuff4[buff_max_size];
int port[4];
int sockfd1, sockfd2, sockfd3, sockfd4 ;
struct sockaddr_in servaddr;
size_t max = 200;


void extract_ports(char *buff1, int i)
{

        char *tok;
        tok = strtok(buff1, ":");
        tok = strtok(NULL, " \t\n");
        port[i] = atoi(tok);
        printf("port %d : %d\n", i,port[i]);

}


void sendto_server(int socketfd, int server_no, char *filename, char *buf1, int suffix1, char *buf2, int suffix2, char *Username, char *Password, char *req_type)
{
        int rec, len1 = strlen(buf1);
        int len2 = strlen(buf2);
        char fname1[30], fname2[30], rec_buff[90];
        bzero(rec_buff, sizeof(rec_buff));

        sprintf(fname1, ".%s.%d", filename, suffix1);
        sprintf(fname2, ".%s.%d", filename, suffix2);
        sprintf(send_buff,"%s %s %d %s %d %s %s %d ", req_type, fname1, len1, fname2, len2, Username, Password, server_no);
        //puts(send_buff);
        send(socketfd, send_buff, strlen(send_buff), 0);
        


        rec = recv(socketfd, rec_buff, 90, 0);
        printf("\n\n After Command send,Server says : ");
        puts(rec_buff);


        if(strncmp(rec_buff, "ACK",3) == 0)
        {
                send(socketfd, buf1, strlen(buf1), 0);

                bzero(rec_buff, sizeof(rec_buff));
                rec = recv(socketfd, rec_buff, 90, 0);
                printf("\n\n After sending 1st PART Server says : ");
                puts(rec_buff);

                if(strncmp(rec_buff, "ACK",3) == 0)
                {
                        bzero(rec_buff, sizeof(rec_buff));
                        
                        send(socketfd, buf2, strlen(buf2), 0);
                        rec = recv(socketfd, rec_buff, 90, 0);
                        printf("\n\n After sending 2nd PART Server says : ");
                        puts(rec_buff);

                }

        }

        else printf("Invalid Username/Password. Could not proceed");


}


int main(int argc, char **argv)
{
       
        char sendline[MAXLINE], recvline[MAXLINE];
        strncpy(sendline, "Send this please work", 21);


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
                printf("                1. LIST : lists files stored at the server ofr a given username \n ");
                printf("               2. PUT filename: Sends a file to the distributed servers\n");
                printf("                3. GET filename: Retrieves a file from the distributed servers\n\n\r ");

                fgets(cmd, MAXLINE, stdin);
                puts(cmd);

                if( (strncmp(cmd, "PUT", 3) == 0) || (strncmp(cmd, "put", 3) == 0))
                {
                        char *tok2;
                        tok2 = strtok(cmd, " ");
                        strncpy(req_method, tok2, strlen(tok2));
                        puts(req_method);
                        tok2 = strtok(NULL, " \t\n");
                        strncpy(filename, tok2, strlen(tok2));
                        puts(filename);

                        int mod = mod_from_md5(filename);
                        printf("mod : %d\n", mod);

                        file_divide(filename, fbuff1, fbuff2, fbuff3, fbuff4);

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
                
                                        sendto_server(sockfd1, 1, filename, fbuff1, 1, fbuff2, 2, username, password, req_method); 
                                        sendto_server(sockfd2, 2, filename, fbuff2, 2, fbuff3, 3, username, password, req_method); 
                                        sendto_server(sockfd3, 3, filename, fbuff3, 3, fbuff4, 4, username, password, req_method); 
                                        sendto_server(sockfd4, 4, filename, fbuff4, 4, fbuff1, 1, username, password, req_method);
                                        break; 

                                case 1:

                                        sendto_server(sockfd1, 1, filename, fbuff4, 4, fbuff1, 1, username, password, req_method);
                                        sendto_server(sockfd2, 2, filename, fbuff1, 1, fbuff2, 2, username, password, req_method); 
                                        sendto_server(sockfd3, 3, filename, fbuff2, 2, fbuff3, 3, username, password, req_method); 
                                        sendto_server(sockfd4, 4, filename, fbuff3, 3, fbuff4, 4, username, password, req_method); 
 

                                        break; 


                                case 2:

                                        sendto_server(sockfd1, 1, filename, fbuff3, 3, fbuff4, 4, username, password, req_method); 
                                        sendto_server(sockfd2, 2, filename, fbuff4, 4, fbuff1, 1, username, password, req_method);
                                        sendto_server(sockfd3, 3, filename, fbuff1, 1, fbuff2, 2, username, password, req_method);
                                        sendto_server(sockfd4, 4, filename, fbuff2, 2, fbuff3, 3, username, password, req_method); 
 
                                        break; 


                                case 3:

                                        sendto_server(sockfd1, 1, filename, fbuff2, 2, fbuff3, 3, username, password, req_method); 
                                        sendto_server(sockfd2, 2, filename, fbuff3, 3, fbuff4, 4, username, password, req_method); 
                                        sendto_server(sockfd3, 3, filename, fbuff4, 4, fbuff1, 1, username, password, req_method);
                                        sendto_server(sockfd4, 4, filename, fbuff1, 1, fbuff2, 2, username, password, req_method); 

                                        break; 

                        }
                
                }


        }




         
        
 //exit(0);
}
