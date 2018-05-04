#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>

#define MAX_CONNECTIONS 1000
#define BYTES 1024
//#define PORT_NUM 10000
#define MAXLEN 50*1024
#define wsMaxSize 1000

// Errors
#define GETADDRINFOERROR 1
#define BINDERROR        1
#define LISTENERROR      1
#define SLOTERROR        0
#define PORTERROR        1
#define WSCONFIGERROR    1
#define SOCKETCLOSE	 0

#define TRUE             1
#define FALSE            0
#define SUCCESS          1
#define FAIL             0
//#define TIMEOUT          10

#define LINEBUFLENGTH    2000

unsigned char TIMEOUT=0;
char *ROOT_DIR, *path;
int listenfd, sockfd_connect[MAX_CONNECTIONS];
void error(char *);
void startWebServer();
void handle_request(int);
char * checkContentType(char *);
//char *directoryroot;
char PORT[10];
char def_page[100];
struct itimerval timeout;

/* Handles Time Out */
int global_connNo;


struct format
{
        char exten[20];
        char content_type[30];

};
struct format frm[10], *frm_ptr;

void store_contenttype(char *src)
{
        char *tok;
        tok = strtok(src, " \t\n");
        strcpy(frm_ptr->exten, tok);
        
        tok = strtok(NULL, " \t\n");
        strcpy(frm_ptr->content_type, tok);
        frm_ptr++;
}

void alarm_handler(void)
{  
	printf("Entering alarm alarm_handler -- closing sock : %d\n\n", global_connNo);
	
	shutdown (sockfd_connect[global_connNo], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(sockfd_connect[global_connNo]);
	sockfd_connect[global_connNo]=-1;
   	exit(SOCKETCLOSE);
}


char *get_port(char *port_stmt)
{
        char *tok;
	char port_delim[5] = " ";
	tok = strtok(port_stmt, port_delim );
	tok = strtok( NULL, port_delim );
	return tok;
}

void get_rootdir(char *root_stmt)
{

	char *root_dir;
	root_dir = strstr(root_stmt, " ");
	root_dir = root_dir+1;
        root_dir = strtok(root_dir, " \t\n");
        printf("Check : %s",root_dir );
        ROOT_DIR=(char*)malloc(100);
	strcpy(ROOT_DIR, root_dir);
	puts(ROOT_DIR);
	//return path;
}

static unsigned int get_file_size (FILE * fileDescriptor)
{
    unsigned int size;

    fseek(fileDescriptor, 0L, SEEK_END);
    size = ftell(fileDescriptor);
    fseek(fileDescriptor, 0L, SEEK_SET);

    return size;
}

char *getExtension (char *filename) {
    char *extension = strrchr (filename, '.');
    return extension;
}

int check_formatexists(char *filetype)
{

    int j=0;
       // printf("\n Came in extract format loop \n");
    while( (strcmp(frm[j].exten,filetype) != 0) && (j < 9) )
        {
                j++;
        }
       // printf("\n\n Matched at : %d \n\n ", j);
    if(j < 9)
    {
        
       // printf("\n file type : %s so returning : %d\n", frm[j].content_type, j);
        return j;
    }

    else return 9;

}


void read_configfile()
{
    FILE *fp;
    char wsBuffer[wsMaxSize];
    char *val1;
    char *wsConffile = getenv("PWD");
	strncat(wsConffile,"/ws.conf", 8);
    printf("Path to ws.conf file is %s: \n", wsConffile);
    
    frm_ptr = &frm[0];

    fp=fopen(wsConffile,"r");	//open ws.conf as read only

    if (fp == NULL)
    {

        perror("ws.conf");
        printf("Exiting the program\n");
        exit(WSCONFIGERROR);
    }

    else
    {
        unsigned int wsConfFileSize = get_file_size (fp);

        //printf("ws.conf size n = %d, filename = %s\n", wsConfFileSize, wsConffile);


        while(fgets(wsBuffer,wsConfFileSize,fp)!=NULL) {

            /**********************
            * Finds Root directory
            /**********************/
            
             if(strstr(wsBuffer, "#document root") != NULL)
	     {
		fgets(wsBuffer, 80, fp);
		get_rootdir(wsBuffer);
                //strncpy(ROOT_DIR,path,sizeof(path));
		printf("Path extracted : ");
		puts(ROOT_DIR);
	     }

            /****************
            * Finds PORT NUM 
            /****************/
            
             else if(strstr(wsBuffer,"#serviceport number") != NULL)
	     {
                fgets(wsBuffer, 20, fp);
                strncpy(PORT,get_port(wsBuffer),4);
                printf("Port number extracted : %s\n", PORT);
	     }

            /****************
            * Finds Default page 
            /****************/
            else if(strncmp(wsBuffer,"DirectoryIndex",14)==0) {
                printf("wsBuffer: %s",wsBuffer);
                val1=strtok(wsBuffer," \t\n");	//checks for space in wsBuffer
                val1 = strtok(NULL, " \t\n");
                strcpy(def_page, val1);
                printf("Directory Index: %s\n", def_page);
            	bzero(wsBuffer, sizeof(wsBuffer));
            }

            /****************
            * Finds timeout 
            /****************/
            else if(strncmp(wsBuffer,"Keep-Alive time",15)==0) {
                printf("wsBuffer: %s",wsBuffer);
                val1=strtok(wsBuffer," \t\n");	//checks for space in wsBuffer
                val1 = strtok(NULL, " \t\n");
		val1 = strtok(NULL, " \t\n");
                TIMEOUT = atoi(val1);
                printf("TIMEOUT: %d\n\n", TIMEOUT);
            	bzero(wsBuffer, sizeof(wsBuffer));
            }

            else if(strstr(wsBuffer, ".") != NULL)
            {

                store_contenttype(wsBuffer);
            }	

        }

        fclose(fp);
    }

        /*for(int dn =0; dn <9; dn++)
        {
                printf("%s : %s\n",frm[dn].exten,frm[dn].content_type);
        }

        while (1);*/

}

int main(int argc, char* argv[])
{

    int connNo=0;
    int i;

    struct sockaddr_in clientAddr;
    socklen_t addrlen;
    char c;    

    read_configfile();     // reads the wsConfigFile
    for (i=0; i<MAX_CONNECTIONS; i++)
    {
        sockfd_connect[i]=-1;
    }

    int port_num = atoi(PORT);
    if (port_num < 1024)
    {
        fprintf(stderr, "The port number chosen is %d and is INVALID\n", port_num);
        exit(PORTERROR);
    }
    //if ()
    startWebServer();

    // ACCEPT connections
    while (1)
    {
        addrlen = sizeof(clientAddr);
        sockfd_connect[connNo] = accept (listenfd, (struct sockaddr *) &clientAddr, &addrlen);
        if (sockfd_connect[connNo]<0)
            error ("accept() error");
        else
        {
            if ( fork()==0 )
            {
                handle_request(connNo);
                //exit(SLOTERROR);
            }
        }

        while (sockfd_connect[connNo]!=-1) 
            {
                connNo = (connNo+1)%MAX_CONNECTIONS;
            }
    }

    return 0;
}

//start server
void startWebServer(void)
{
    struct addrinfo webServerHints, *result, *rp;
    memset (&webServerHints, 0, sizeof(webServerHints)); // Making sure the struct is empty
    webServerHints.ai_family = AF_INET;                  // IPv4
    webServerHints.ai_socktype = SOCK_STREAM;            // TCP stream sockets
    webServerHints.ai_flags = AI_PASSIVE;


   

    if (getaddrinfo( NULL, PORT, &webServerHints, &result) != 0)
    {
        perror ("ERROR in getaddrinfo()");
        exit(GETADDRINFOERROR);
    }
    
    for (rp = result; rp!=NULL; rp=rp->ai_next)
    {
        if ((listenfd = socket (rp->ai_family, rp->ai_socktype, 0)) == -1)	//socket() failed, try next address
        {
            continue;
        }
        if (bind(listenfd, rp->ai_addr, rp->ai_addrlen) == 0) break;		//bind() success
    }
    /* No address succeedded */
    if (rp==NULL)
    {
        perror ("socket() or bind() creation failed");
        exit(BINDERROR);
    }

    freeaddrinfo(result); //No longer needed

    // listen for incoming connections
    if ( listen (listenfd, MAX_CONNECTIONS) != 0 )
    {
        perror("listen() error");
        exit(LISTENERROR);
    }
}


//client connection
int COUNT = 0;
void handle_request(int n)
{

        char *req_method, *req_file, *req_version, *http_ver, *req_conn, server_resp[800];
	char request[MAXLEN], *requestline[3], data_to_send[BYTES], path[MAXLEN];
	int rcv, fd, bytes_read;
	FILE *fp;
	char connection_type[50];
        char status_line[LINEBUFLENGTH];
	global_connNo = n;
	int post_req_check=0;
	signal(SIGALRM, (void(*) (int)) alarm_handler);  /* Creating SIGALRM interrupt to implement timeout */
	timeout.it_value.tv_sec = TIMEOUT;
	timeout.it_value.tv_usec = 0;
	timeout.it_interval = timeout.it_value;

	printf("\n\r    ******************** ClientNumber : %d Socket %d ********************       \n\n",global_connNo, sockfd_connect[global_connNo]);
	while(1)
	{
		post_req_check = 0;  // make it zero before every post request
		/* Initialize arrays to zero */
		bzero(request, sizeof(request));
		bzero(data_to_send, sizeof(data_to_send));
		bzero(path, sizeof(path));
		bzero(connection_type, sizeof(connection_type));
                bzero(status_line, sizeof(status_line));
                bzero(server_resp, sizeof(server_resp));
		/* initialize request to all null characters */
		memset( (void*)request, (int)'\0', MAXLEN );
		rcv=recv(sockfd_connect[n], request, MAXLEN, 0);	//receive message from client
		
		char filename[50] = "storeMsg";
		char count_str[50];
		char line_copy[99999];
		sprintf(count_str,"%d", COUNT);
		strcat(filename, count_str);
		FILE *fp_clientMsg = fopen(filename, "w");
		if (fp_clientMsg != NULL)
		{
			fputs(request, fp_clientMsg);
			fclose(fp_clientMsg);
		}
                char *str;

                if( !strstr(request,"Connection: Close") )
                {
                        strncpy(connection_type, "Connection: keep-alive", strlen("Connection: keep-alive"));
                }

                else strncpy(connection_type, "Connection: Close", strlen("Connection: Close"));
                /*str = strstr(request,"Connection: keep-alive");
                if(strcmp(str,"Connection: keep-alive") == 0)
                {
                        strncpy(connection_type, "Connection: keep-alive", strlen("Connection: keep-alive"));
                }
                else if(str == NULL) strncpy(connection_type, "Connection: Close", strlen("Connection: Close"));*/

                printf("connection status : %s\n", connection_type);
                

		if (rcv<0)    // receive error
			fprintf(stderr,("recv() error\n"));
		else if (rcv==0)    // receive socket closed
			rcv = 0;
		//fprintf(stderr,"Client disconnected upexpectedly.\n");

		else    // message received
		{
		// if another request occpurs pertaining to the same socket then TIMEOUT value has to be reset
			timeout.it_value.tv_sec = TIMEOUT;
			timeout.it_value.tv_usec = 0;
			timeout.it_interval = timeout.it_value;
			if (!strstr(request,"Connection: Close"))    // capturing the last string from the received message
			{
				//setitimer(ITIMER_REAL,&timeout,NULL);
				strncpy(connection_type, "Connection: Keep-alive", strlen("Connection: Keep-alive"));
			}
			else    /* -- If Keep-alive is not found, close the connection --- */
			{
				strncpy(connection_type, "Connection: Close",strlen("Connection: Close"));
			}

			printf("\n## Printing request\n %s \n", request);

			// Now breaking the incoming strng into three different paths
			req_method = strtok (request, " \t\n");
                        //printf("-------------------------------%s\n", req_method);
                        
			if ((strncmp(req_method, "GET\0", 4)==0) || (strncmp(req_method, "POST\0", 5)==0))
			{
			    if (strncmp(req_method, "POST\0", 5)==0)
			    {
			    	printf("\n\r&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&Com\n");
			    	post_req_check = TRUE;
			    }

			    req_file = strtok (NULL, " \t");
			    req_version = strtok (NULL, " \t\n");
			    

			    char http_ver[8];
			    if (strncmp(req_version, "HTTP/1.1", 8) == 0)
				strcpy(http_ver, "HTTP/1.1");
			    else
				strcpy(http_ver, "HTTP/1.0");


			    if ( strncmp( req_version, "HTTP/1.0", 8)!=0 && strncmp( req_version, "HTTP/1.1", 8)!=0 || strstr(req_file, " ") != NULL)
			    {


			        char invalid_version[8000];
			        strncat(invalid_version,"HTTP/1.1",strlen("HTTP/1.1"));
			        strncat(invalid_version," 400 Bad Request", strlen(" 400 Bad Request"));
			        strncat(invalid_version,"\n", strlen("\n"));
			        strncat(invalid_version,"Content-Type: Invalid", strlen("Content-Type: Invalid"));
			        strncat(invalid_version,"\n", strlen("\n"));
			        strncat(invalid_version,"Content-Length: Invalid", strlen("Content-Length: Invalid"));
			        strncat(invalid_version,"\r\n", strlen("\r\n"));
			        strncat(invalid_version,"\r\n", strlen("\r\n")); 
			        strncat(invalid_version,"<head><b>400 BAD REQUEST</b><br> </head>", strlen("<head><b>400 BAD REQUEST</b><br> </head>"));
			        strncat(invalid_version,"<title> 400 Bad Request </title>", strlen("<title> 400 Bad Request </title>"));
			        strncat(invalid_version,"<html><body> Reason Invalid HTTP Version:", strlen("<html><body> Reason Invalid HTTP Version:"));
			        strncat(invalid_version,req_version,strlen(req_version));
			        strncat(invalid_version,"</body></html>",strlen("</body></html>"));
			        strncat(invalid_version,"\r\n", strlen("\r\n"));
			
				write(sockfd_connect[n], invalid_version, strlen(invalid_version));
			    }
			    else
			    {
				if ( strncmp(req_file, "/\0", 2)==0 )
				    strcat(req_file, def_page);
				   // req_file = "/index.html";//Because if no file is specified, index.html will be opened by default
				strcpy(path, ROOT_DIR);
				strcpy(&path[strlen(ROOT_DIR)], req_file);
				printf("file: %s\n", path);
				char* retrn_format;
				char *ext = getExtension(path);
				if (ext == NULL)
				{
				    retrn_format = NULL;
				}
				else
				{

                                    //printf("fnvjnjfnvfvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv ext : %s\n",ext);
				    
                                    int dn = check_formatexists(ext);
                                    retrn_format = (char*)malloc(33);
                                    memset(retrn_format,'\0',sizeof(retrn_format));
                                    strcat(retrn_format, frm[dn].content_type);
				    printf("\n\rformat checked returns = %s\n\r", retrn_format);
                                       
				}
				char size_array[20];
				char tempp = 0;
				if (retrn_format != NULL)
				{
				    if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
				    {
					fp = fopen(path,"r");
					int size= get_file_size(fp);
					sprintf(size_array,"%d",size);
					char msgRecvPost[99999];
					if (post_req_check)
					{
					    FILE * fp_chck_storeMsg;
					    char * line = NULL;
					    size_t len = 0;
					    ssize_t read;
					    int emptyLineFound = FALSE;
					    unsigned int con_len=0;
					    fp_chck_storeMsg = fopen(filename, "r");
					    if (fp_chck_storeMsg == NULL)
						exit(EXIT_FAILURE);
					    while ((read = getline(&line, &len, fp_chck_storeMsg)) != -1) {
					    	if(strncmp(line, "Content-Length", 14) == 0){
							char *temp = strtok(line, " \n\r");
							temp = strtok(NULL, " \n\r");
							con_len = atoi(temp);
							printf("\n\rCon len = %d", con_len);
							break;
						}
					    }
					    while ((read = getline(&line, &len, fp_chck_storeMsg)) != -1) {
						if(read>=2){
							printf("\n\rRead bytes = %d", read);
							printf("\n\rLine read: %s", line);
							tempp++;
							if(tempp>1) strncat(msgRecvPost, line, strlen(line));					
						}
					    }
					    
					    if(tempp<2){
						memset( (void*)request, (int)'\0', MAXLEN );
						rcv=recv(sockfd_connect[n], request, MAXLEN, 0);	//receive message from client
					    	*(request + con_len) = '\0';
					    //printf("\n\rreceived message = %s", request);
					   	fp_clientMsg = fopen(filename, "w");
					    	if (fp_clientMsg != NULL)
					    	{
					     		fputs(request, fp_clientMsg);
					    		fclose(fp_clientMsg);
					   	}
					    	fp_chck_storeMsg = fopen(filename, "r");
					    	if (fp_chck_storeMsg == NULL)
							exit(EXIT_FAILURE);


					   	 while ((read = getline(&line, &len, fp_chck_storeMsg)) != -1) {
					    	
							strncat(msgRecvPost, line, strlen(line));
	
					  	  }

					    }
					    COUNT++;
					    // add some thing to data_to_send
					    printf("\n\rcoming into post loop\n");
					    printf("						%s\n", msgRecvPost);	
					    fclose(fp_chck_storeMsg);
					    remove(filename);
					    if (line)
					    free(line);
					}                               /** if (post _req check closes here) **/

		
			                char msg[99999];


		                     /* If valid request, send this response and file */
					
					strncat(server_resp,http_ver,strlen(http_ver));
					strncat(server_resp," 200 OK",strlen(" 200 OK"));
					strncat(server_resp,"\n",strlen("\n"));
					strncat(server_resp,"Content-Type:",strlen("Content-type:"));
					strncat(server_resp,retrn_format,strlen(retrn_format));
					strncat(server_resp,"\n",strlen("\n"));
					strncat(server_resp,"Content-Length:",strlen("Content-Length:"));
					strncat(server_resp,size_array,strlen(size_array));
					strncat(server_resp,"\n",strlen("\n"));
					strncat(server_resp,connection_type,strlen(connection_type));
					if (post_req_check){
						strncat(server_resp,"\r\n\r\n",strlen("\r\n\r\n"));

						sprintf(msg,"<html><body><pre><h1>%s</h1></pre>",msgRecvPost);
						
					}
					else{
						strncat(server_resp,"\r\n",strlen("\r\n"));
						strncat(server_resp,"\r\n",strlen("\r\n"));
					}
					printf("Printing server response \n%s\n",server_resp);
					send(sockfd_connect[n], server_resp, strlen(server_resp), 0);
					if(post_req_check) write(sockfd_connect[n], msg, strlen(msg));
					while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
					    write (sockfd_connect[n], data_to_send, bytes_read);

					fclose(fp);
					bzero(msgRecvPost,sizeof(msgRecvPost));
					bzero(msg,sizeof(msg));
				    }
		
				    else{  // file not found loop

				    	
                                        char notfound[8000];
			                strncat(notfound,http_ver,strlen(http_ver));
			                strncat(notfound," 404 Not Found", strlen(" 404 Not Found"));
			                strncat(notfound,"\n", strlen("\n"));
			                strncat(notfound,"Content-Type: Invalid", strlen("Content-Type: Invalid"));
			                strncat(notfound,"\n", strlen("\n"));
			                strncat(notfound,"Content-Length: Invalid", strlen("Content-Length: Invalid"));
			                strncat(notfound,"\r\n", strlen("\r\n"));
			                strncat(notfound,"\r\n", strlen("\r\n")); 
			                strncat(notfound,"<head> <b>404 NOT FOUND</b><br> </head>", strlen("<head> <b>404 NOT FOUND</b><br> </head>"));
			                strncat(notfound,"<title> 404 NOT FOUND </title>", strlen("<title> 404 NOT FOUND </title>"));
			                strncat(notfound,"<html><body> Reason URL does not exist :", strlen("<html><body> Reason URL does not exist :"));
                                        strncat(notfound,path,strlen(path));
			                strncat(notfound,"</body></html>",strlen("</body></html>"));
			                strncat(notfound,"\r\n", strlen("\r\n"));

			                write(sockfd_connect[n], notfound, strlen(notfound));
			                //exit(0);
					    
				    }    
				
				}                       /* else closes here for version 1.0 or 1.1*/

				else            /* file not supported */
				{
				   
                                char format_notsupported[8000];
			        strncat(format_notsupported,http_ver,strlen(http_ver));
			        strncat(format_notsupported," 501 Not Supported", strlen(" 501 Not Supported"));
			        strncat(format_notsupported,"\n", strlen("\n"));
			        strncat(format_notsupported,"Content-Type: Invalid", strlen("Content-Type: Invalid"));
			        strncat(format_notsupported,"\n", strlen("\n"));
			        strncat(format_notsupported,"Content-Length: Invalid", strlen("Content-Length: Invalid"));
			        strncat(format_notsupported,"\r\n", strlen("\r\n"));
			        strncat(format_notsupported,"\r\n", strlen("\r\n")); 
			        strncat(format_notsupported,"<head> <b>501 FORMAT NOT SUPPORTED</b><br> </head>", strlen("<head> <b>501 FORMAT NOT SUPPORTED</b><br> </head>"));
			        strncat(format_notsupported,"<title> 501 Not Supported </title>", strlen("<title> 501 Not Supported </title>"));
			        strncat(format_notsupported,"<html><body> Reason Following format is not Supported :<br>", strlen("<html><body> Reason Following format is not Supported :<br>"));
			        strncat(format_notsupported,ext,strlen(ext));
			        strncat(format_notsupported,"</body></html>",strlen("</body></html>"));
			        strncat(format_notsupported,"\r\n", strlen("\r\n"));
			        //printf("Not found Error : %s",notfound);
			        write(sockfd_connect[n], format_notsupported, strlen(format_notsupported));   
				
                                }

			    }

			}                                       /* if ends */
			                /* HEAD, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH not implemented */
			else if ((strncmp(req_method, "HEAD\0", 4)==0) || (strncmp(req_method, "PUT\0", 3)==0) || (strncmp(req_method, "DELETE\0", 6)==0)|| (strncmp(req_method, "CONNECT\0", 7)==0)|| (strncmp(req_method, "OPTIONS\0", 7)==0)|| (strncmp(req_method, "TRACE\0", 5)==0)|| (strncmp(req_method, "PATCH\0", 5)==0))
			{

                               printf("jrongnrgnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnjjjjjjjjjnjksanbbbbbfrkb\n");

				strncat(status_line,"HTTP/1.1",strlen("HTTP/1.1"));
				strncat(status_line,"\n",strlen("\n"));//strncat(status_line,"\r\n",strlen("\r\n"));
				strncat(status_line,"Content-Type:",strlen("Content-type:"));
				strncat(status_line,"NONE",strlen("NONE"));
				strncat(status_line,"\n",strlen("\n"));
				strncat(status_line,"Content-Length:",strlen("Content-Length:"));
				strncat(status_line,"NONE",strlen("NONE"));
				strncat(status_line,"\r\n",strlen("\r\n"));
				strncat(status_line,"\r\n",strlen("\r\n"));
				strncat(status_line,"<HEAD><TITLE>501 Not Implemented</TITLE></HEAD>",strlen("<HEAD><TITLE>501 Not Implemented</TITLE></HEAD>"));
				//strncat(status_line,"\r\n",strlen("\r\n"));
				strncat(status_line,"<BODY>501 Not Implemented: File format not supported:",strlen("<BODY>501 Not Implemented: File format not supported:"));
				strncat(status_line,"HTTP/1.1",strlen("HTTP/1.1"));
				strncat(status_line,"</BODY></html>",strlen("</BODY></html>"));
				// strncat(status_line,"\r\n",strlen("\r\n"));
				strncat(status_line,"\r\n",strlen("\r\n"));
				write(sockfd_connect[n], status_line, strlen(status_line)); //FILE NOT FOUND */
                                


  
			 }
			/* Bad request */
			else
			{
				strncat(status_line,"HTTP/1.1",strlen("HTTP/1.1"));
				strncat(status_line,"\n",strlen("\n"));//strncat(status_line,"\r\n",strlen("\r\n"));
				strncat(status_line,"Content-Type:",strlen("Content-type:"));
				strncat(status_line,"NONE",strlen("NONE"));
				strncat(status_line,"\n",strlen("\n"));
				strncat(status_line,"Content-Length:",strlen("Content-Length:"));
				strncat(status_line,"NONE",strlen("NONE"));
				strncat(status_line,"\r\n",strlen("\r\n"));
				strncat(status_line,"\r\n",strlen("\r\n"));
				strncat(status_line,"<HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>",strlen("<HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>"));
				//strncat(status_line,"\r\n",strlen("\r\n"));
				strncat(status_line,"<BODY>500 Internal Server Error:cannot allocate memory",strlen("<BODY>500 Internal Server Error:cannot allocate memory"));
				strncat(status_line,"HTTP/1.1",strlen("HTTP/1.1"));
				strncat(status_line,"</BODY></html>",strlen("</BODY></html>"));
				// strncat(status_line,"\r\n",strlen("\r\n"));
				strncat(status_line,"\r\n",strlen("\r\n"));
				write(sockfd_connect[n], status_line, strlen(status_line)); //FILE NOT FOUND   
			 }

		}//message received end
                printf("----- niche conn stat : %s\n", connection_type);
		if (!strstr(connection_type,"Connection: Close"))    // capturing the last string from the received message
		{
                    printf("\ntick resetting the timer\n");
		    setitimer(ITIMER_REAL,&timeout,NULL);

		}
		else    /* -- If Keep-alive is not found, close the connection --- */
		{
			
		    shutdown (sockfd_connect[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
		    close(sockfd_connect[n]);
		    sockfd_connect[n]=-1;
		    exit(0);
		}
    }

    
}
