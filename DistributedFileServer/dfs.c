#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/syscall.h>

#define MAXLINE 4096 /*max text line length*/
//#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections */
#define buff_max_size 99999


int SERV_PORT,listenfd, connfd, n;
socklen_t clilen;
char fbuff1[buff_max_size], fbuff2[buff_max_size], fbuff3[buff_max_size], fbuff4[buff_max_size];
char recv_buf[MAXLINE], username[30], username_rec[30], password[30], password_rec[30], *conf_buffer, req_method[30];
char fname1[30], fname2[30];
int f1_size, f2_size;
struct sockaddr_in cliaddr, servaddr;
size_t max = 200;



void handle_request(int socketfd)
{

                n = recv(socketfd, recv_buf, MAXLINE,0);  
                if(n > 0)
                {
                        printf("Received buf size : %d\n", n);

                        strncpy(fbuff1, recv_buf, strlen(recv_buf));

                        if( (strncmp(fbuff1,"PUT",3) ==0 ) || (strncmp(fbuff1,"GET",3) ==0 ) ||  (strncmp(fbuff1,"LIST",4) ==0 )  )
                        { 
                                printf("%s\n","String received from client:");
                                puts(fbuff1);
                                printf("\n\n");
                                sscanf(fbuff1,"%s %s %d %s %d %s %s", req_method, fname1, &f1_size, fname2, &f2_size, username_rec, password_rec );
                                printf("\n %s %s %d %s %d %s %s \n", req_method, fname1, f1_size, fname2, f2_size, username_rec, password_rec );
                                
                                if(strcmp(username, username_rec) == 0)
                                {               
                                        send(socketfd, "ACK", 3, 0);
                                }

                                else
                                {

                                        send(socketfd, "Invalid Username/Password", 25, 0);
                                }
                        
                        //send(connfd, buf, n, 0);
                        }
                }

                else
                {
                        perror("Read error");
                        close(connfd);
                        exit(1);
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
