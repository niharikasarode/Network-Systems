#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <time.h>

#define buff_max_size 99999
#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/


int listenfd, connfd, site_sock, sock1, n, k, PORT, Cache_Timeout, cache_present;
int port_specified, port, time_created;
pid_t childpid;
socklen_t clilen;
char buf[MAXLINE];
struct sockaddr_in cliaddr, servaddr, mainserver_addr;
char recv_buff[buff_max_size], req_method[10], URL_slash[1024], http_version[10], *url, site[1024], *path;
char send_request[buff_max_size], rec_buf[buff_max_size], *temp, send_buff[buff_max_size];
char cache_dir[2048], Full_md5[100], dir[100], IP[100];
char method_error[2048], *conf_buffer, fname[100];
struct hostent *hp;
FILE *fp, *f1, *f2, *f3;
time_t time_now;

void calculate_md5(char *string)
{

        unsigned char c[MD5_DIGEST_LENGTH];
    
        MD5_CTX mdContext;
        MD5_Init(&mdContext);
        MD5_Update(&mdContext, string, strlen(string));
        MD5_Final(c,&mdContext);

        for(int i = 0; i < MD5_DIGEST_LENGTH; i++) 
	{

                sprintf(&Full_md5[2*i], "%02x", c[i]);
   	}

        time_now = time(NULL);


}

int check_md5(char *hash_str, int timeout)
{

        size_t max=200;
        int temp_var=0;
        char *tok;
        conf_buffer = (char *)malloc(max*sizeof(char));
        bzero(dir, sizeof(dir));
        strncpy(dir, cache_dir, strlen(cache_dir));
        strcat(dir,"md5list");
        
        f1 = fopen(dir, "rb");        
        if(f1 != NULL)
        {
                 while(!feof(f1))
                {
                        getline(&conf_buffer,&max,f1);
                        if(strncmp(conf_buffer, hash_str, strlen(hash_str)) == 0)
                        {
                                getline(&conf_buffer,&max,f1);
                                tok = strtok(conf_buffer, "\n");
                                time_created = atoi(tok);
                                time_now = time(NULL);
                                if((time_now - time_created) < timeout)
                                {
                                        temp_var = 5;
                                        free(conf_buffer);
                                        fclose(f1);
                                        return 5;
                                }
                                else return 1;   
                        }
                }

        }
        free(conf_buffer);
        fclose(f1);
        if(temp_var != 0)
        {
                return 1;
        }
}


int check_hostname(char *hostname, char *c_dir)
{

        bzero(dir, sizeof(dir));
        size_t max = 200;
        strncpy(dir, c_dir, strlen(c_dir));
        strncat(dir, "host_list", strlen("host_list"));
        conf_buffer = (char *)malloc(max*sizeof(char));
        
        char *tok;
        int u = 3;
        f3 = fopen(dir, "rb");

        if(f3 != NULL)
        {
                        getline(&conf_buffer,&max,f3);
                        if(strncmp(conf_buffer, hostname, strlen(hostname)) == 0)
                        {
                                bzero(conf_buffer, sizeof(conf_buffer));
                                getline(&conf_buffer,&max,f3);
                                tok = strtok(conf_buffer, "\n");
                                puts(tok);                                
                                bzero(IP, sizeof(IP));
                                strncpy(IP, tok, strlen(tok));
                                puts(IP);
                                u=2;
                        }
        fclose(f3);
        }
        free(conf_buffer);
        return u;
}




void handle_request(int socketfd, char *cache_directory, int timeout)
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



        /**************         Check request method (ONLY GET SUPPORTED)      ************/

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

                /****   Removing "//" from http://  ****/

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
        

                /****     Storing path to a file/image (respect to the server) to be sent in GET request  ****/
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

                calculate_md5(url);

                cache_present = check_md5(Full_md5, timeout);


                /** Status 5 returned only if cached copy exists and hasn't timedout **/
                if(cache_present != 5)
                {

                        printf("Cached copy doesn't exist OR Invalid\n\n");

                        char bufer[200];
                        f1 = fopen(dir, "a");        
                        if(f1 != NULL)
                        {
                        
                                fwrite(Full_md5, 1, strlen(Full_md5), f1);
                                fwrite("\n", 1, strlen("\n"), f1);
                                sprintf(bufer, "%ld\n", time_now);
                                fwrite(bufer, 1, strlen(bufer), f1);
                                bzero(bufer, sizeof(bufer));
                                fclose(f1);
                        }


                         /**  Check if site was already accessed to avoid extra DNS query**/
                                        /* Returns 2 on success and 3 on fail*/
                        int st;
                        st = check_hostname(site, cache_directory);

                        if(st == 3)
                        {  
                                printf("Need to do DNS query\n");                      
                                hp = gethostbyname(site);
                                if(hp == NULL)
                                {
                                        printf("Host :");
                                        puts(site);
                                        printf("ERROR: gethostbyname failed\n");
                                        exit(1);
                                }

                                printf("actual name - %s\n", hp->h_name);
                                printf("address - %s\n", inet_ntoa(*((struct in_addr *)hp->h_addr)));

                                /* Save hostname n IP for future reference*/

                                f3 = fopen(dir, "a");
                                char bufer[100];
                                if(f3 != NULL)
                                {
                                        fwrite(site, 1, strlen(site), f3);
                                        fwrite("\n", 1, strlen("\n"), f3);
                                        sprintf(bufer, "%s\n", inet_ntoa(*((struct in_addr *)hp->h_addr)));
                                        fwrite(bufer, 1, strlen(bufer), f3);
                                        bzero(bufer, sizeof(bufer));
                                        fclose(f3);       
                                }
                        }

                        bzero((char*)&mainserver_addr, sizeof(mainserver_addr));

                        mainserver_addr.sin_family = AF_INET;
                        mainserver_addr.sin_port = htons(port);
                        if(st == 3)
                        {
                                bcopy((char*)hp->h_addr, (char*)&mainserver_addr.sin_addr.s_addr,hp->h_length );
                        }
                        else
                        {
                                 printf("No need of DNS query\n");
                                 bcopy(IP, (char*)&mainserver_addr.sin_addr.s_addr, strlen(IP));
                        }

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

                        //printf("Connected to %s on IP : %s\n\n", site, inet_ntoa(*((struct in_addr *)hp->h_addr)));

                        if(strlen(path) != 0)
                        sprintf(send_request, "GET %s %s\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", path, http_version, site);
                        else sprintf(send_request, "GET / %s\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", http_version, site);

                        printf("Request sent to main server : %s\n\n", send_request);

                        n = send(site_sock, send_request, sizeof(send_request), 0);

                        if(n<0)
                        printf("Message not sent to main server\n");

                        /**** Check if a timed-out cached copy exists and delete****/


                        sprintf(fname, "./cache/%s.html",Full_md5);
                        if(access(fname, F_OK) == 0)
                        {
                                printf("Cached but timed out copy exists.. deleting it n creating new .. \n");
                                int status = remove(fname);
                                if(status == 0) 
                                printf("Deleted cached copy.\n");
                        }
                        fp = fopen(fname, "ab");
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
                        fclose(fp);
                        printf("Done sending to client\n");
                }

                /** if status after check is 5, non-timedout copy exists which can be used to send back to client **/
                else if(cache_present == 5)
                {

                        printf("\n*****  Valid cached copy exists  ******\n\n");
                        sprintf(fname, "./cache/%s.html",Full_md5);
                        fp = fopen(fname, "rb");
                        int n,d,fsize=0, size_flag=0;
                        char buff[buff_max_size];
                        if(fp != NULL)
                        {
                                fseek(fp, 0 , SEEK_END);
                                fsize = ftell(fp);
                                fseek(fp, 0 , SEEK_SET);
                                fclose(fp);
                        }
                        
                        if(fsize>buff_max_size) 
                        size_flag=1;

                        fp = fopen(fname, "rb");
                        if(fp != NULL)
                        {
                                if(size_flag == 1)
                                {
                                        int div = fsize/buff_max_size;
                                        int mod = fsize%buff_max_size;
                                        do
                                        {
                                                fread(buff, buff_max_size, sizeof(char), fp);
                                                int dn = send(socketfd, buff, buff_max_size, 0);
                                                bzero(buff, sizeof(buff));
                                                n = n+dn;  
                                                div--;

                                        }while(div>0);

                                        fread(buff, mod, sizeof(char), fp);
                                        int dn = send(socketfd, buff, buff_max_size, 0);
                                        n = n+dn;
                                        printf("size_flag=1, sent %d bytes\n", n);
                                }
                                else
                                {
                                        fread(buff, fsize, sizeof(char), fp);
                                        int ln = send(socketfd, buff, fsize, 0);
                                        bzero(buff, sizeof(buff));
                                        printf("size_flag=o, sent %d bytes\n", ln);
                                }
                        }
     
                }   // else case where valid cached copy exists



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

        sprintf(cache_dir, "./cache/");
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
                        
                        handle_request(connfd, cache_dir, Cache_Timeout);
                }
        }
        close(connfd);
  

        } // while(1) closes
 //close socket of the server
 close(connfd);

}
