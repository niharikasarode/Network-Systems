#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define buff_max_size 99999
#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/


int listenfd, connfd, site_sock, sock1, n, k, PORT, Cache_Timeout;
int port_specified, port;
pid_t childpid;
socklen_t clilen;
char buf[MAXLINE];
struct sockaddr_in cliaddr, servaddr, mainserver_addr;
char recv_buff[buff_max_size], req_method[10], URL_slash[1024], http_version[10], *url, site[1024], *path;
char send_request[buff_max_size], rec_buf[buff_max_size], *temp, send_buff[buff_max_size];
char method_error[2048];
char version_error[] = "<html><body><h1>ERROR: Invalid HTTP Version</h1></body></html>";
struct hostent *hp;
FILE *fp;

void handle_request(int socketfd)
{

        printf("\n\n*********************      Handling request listen sock : %d        ******************\n\n", socketfd);
        
        bzero(recv_buff, sizeof(recv_buff));
        int n = recv(socketfd, recv_buff, buff_max_size,0);                 

        if(n<0)
        {
                printf("ERROR: Error in receiving message\n");
        }

        /*************  Parsing client request message   **************/
        sscanf(recv_buff, "%s %s %s", req_method, URL_slash, http_version);
        printf("Message received : %s, %s, %s\n", req_method, URL_slash, http_version);



        if(strncmp(req_method, "GET", 3) != 0)
        {
                                strncat(method_error,"HTTP/1.1",strlen("HTTP/1.1"));
			        strncat(method_error," 400 Bad Request", strlen(" 400 Bad Request"));
			        strncat(method_error,"\n", strlen("\n"));
			        strncat(method_error,"Content-Type: Invalid", strlen("Content-Type: Invalid"));
			        strncat(method_error,"\n", strlen("\n"));
			        strncat(method_error,"Content-Length: Invalid", strlen("Content-Length: Invalid"));
			        strncat(method_error,"\r\n", strlen("\r\n"));
			        strncat(method_error,"\r\n", strlen("\r\n")); 
			        strncat(method_error,"<head><b>400 BAD REQUEST</b><br> </head>", strlen("<head><b>400 BAD REQUEST</b><br> </head>"));
			        strncat(method_error,"<title> 400 Bad Request </title>", strlen("<title> 400 Bad Request </title>"));
			        strncat(method_error,"<html><body> Reason Invalid HTTP Method:", strlen("<html><body> Reason Invalid HTTP Method:"));
			        strncat(method_error,req_method,strlen(req_method));
			        strncat(method_error,"</body></html>",strlen("</body></html>"));
			        strncat(method_error,"\r\n", strlen("\r\n"));
                

                                n = send(socketfd, method_error, strlen(method_error), 0);

                                printf("Sent method error message to client\n");
                                exit(0);
        }
        
        else
        {
                if(strstr(URL_slash, "//") == NULL)
                {
                strncpy(url, URL_slash, strlen(URL_slash));
                }
                else
                {
                url = strstr(URL_slash, "//");
                url = url+2;
                }
                port_specified = 0;
        

                path = strstr(url, "/");

                for(int i=0; i<strlen(url); i++)
                {
                        if(url[i] == '/')
                        break;
                        else site[i] = url[i];                

                }

                for(int m=0; m<strlen(site); m++)
                {
                        if(site[m] == ':')
                        {
                        port_specified = 1;
                        break;
                        }
                }

                if(port_specified == 1)
                {
                        temp = strtok(site, ":");
                        printf("Berfore colon - %s\n", temp);
                        strncpy(site, temp, strlen(temp));

                        temp = strtok(NULL, "/");
                        printf("Port specified in request : %s\n", temp);
                        port = atoi(temp);

                }

                else port = 80;

                hp = gethostbyname(site);
                if(hp == NULL)
                {
                        printf("Host :");
                        puts(site);
                        printf("ERROR: gethostbyname failed\n");
                        exit(1);
                }
                bzero((char*)&mainserver_addr, sizeof(mainserver_addr));

                printf("actual name - %s\n", hp->h_name);
                printf("address - %s\n", inet_ntoa(*((struct in_addr *)hp->h_addr)));


                mainserver_addr.sin_family = AF_INET;
                mainserver_addr.sin_port = htons(80);
                bcopy((char*)hp->h_addr, (char*)&mainserver_addr.sin_addr.s_addr,hp->h_length );

                if ((site_sock = socket (AF_INET, SOCK_STREAM, 0)) <0) 
                {
                        perror("Problem in creating the main server to site socket");
                        exit(1);
                }

                sock1 = connect(site_sock, (struct sockaddr*)&mainserver_addr, sizeof(struct sockaddr));
                if(sock1 < 0)
                {
                printf("ERROR in connecting to mainserver\n");
                exit(1);
                }

                printf("Connected to %s on IP : %s\n\n", url, inet_ntoa(*((struct in_addr *)hp->h_addr)));

                if(strlen(path) != 0)
                sprintf(send_request, "GET %s %s\r\nHost: %s\r\nConnection: close\r\n\r\n", path, http_version, site);
                else sprintf(send_request, "GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n", http_version, site);

                printf("Request sent to main server : %s\n\n", send_request);

                n = send(site_sock, send_request, sizeof(send_request), 0);

                if(n<0)
                printf("Message not sent to main server\n");
        
                fp = fopen("page.html", "ab");
                do
                {
                        bzero((char*)rec_buf, buff_max_size);
                        n = recv(site_sock, rec_buf, buff_max_size, 0);
                        k = k+n;
                        fwrite(rec_buf, 1, n, fp);
                        if(n>0)
                        {
                        int sn = send(socketfd, rec_buf, n, 0 );
                        }
        
                }while(n>0);

                printf("Done sending to client\n");
        }
        
}






int main (int argc, char **argv)
{
 
        if (argc !=3) 
        {
                perror("Usage: ./webproxy_executable TCP/IP_PORT Cache_Timeout ");
                exit(1);
        }

        PORT = atoi(argv[1]);
        Cache_Timeout = atoi(argv[2]);
        printf("Arguments rec: %d %d\n\n", PORT, Cache_Timeout);


        //Create a socket for the soclet
        //If sockfd<0 there was an error in the creation of the socket
        if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) 
        {
        perror("Problem in creating the socket");
        exit(2);
        }


        //preparation of the socket address
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(PORT);

        //bind the socket
        bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

        //listen to the socket by creating a connection queue, then wait for clients
        listen (listenfd, LISTENQ);

        printf("%s\n","Server running...waiting for connections.");

        while(1) 
        {

        clilen = sizeof(cliaddr);
        //accept a connection
        connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

        if(connfd < 0)
        {
                printf("Accept Error\n");
                close(connfd);
        }
        else 
        {
        printf("%s\n","Received request...");

                if ( fork () == 0 ) 
                {                                                       //if it’s 0, it’s child process
                        printf ("\n%s\n","Child created for dealing with client requests");
                        //close listening socket
                        
                        handle_request(connfd);
                }
        }
        close(connfd);
  

        } // while(1) closes
 //close socket of the server
 close(connfd);

}
