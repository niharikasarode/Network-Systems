#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include<sys/types.h>
#include<dirent.h>

#define REMOTE  
#define LARGEFILE
int Samepacket_flag =0;
int Timeout_flag= 0;

void Server_Get(char *filename, int MAXBUFSIZE, struct sockaddr_in client, int socket );
void Server_Put(char *filename, int MAXBUFSIZE, struct sockaddr_in client, int socket );
void Get_Command(struct sockaddr_in client, int socket);


unsigned int remote_length;         //length of the sockaddr_in structure
int FILESIZE;
int sock, nbytes;
int buff_size[30];
char Received_Command[70];

char a[5] = "ls"; 
char b[10] = "exit";
char *ret, *ret1;
char cmd1[10] = "put";
char cmd2[10] = "get";
char cmd3[20] = "delete";

FILE *fs;
struct sockaddr_in my_addr, cli_addr;     //"Internet socket address structure"


void Server_Get(char *filename, int MAXBUFSIZE, struct sockaddr_in client, int socket )
{

        fs = fopen(filename,"wb");
        if( fs != NULL)
	
	{

                 char buffer1[MAXBUFSIZE];             //a buffer to store our received message
               char recv_buffer1[MAXBUFSIZE];
                int d =0;
                int ack =0;
                unsigned char sd=0;
                int prev=0;
                char rec[10];        
                remote_length = sizeof(client);

	        if(d == 0)
                {
	                bzero(buff_size,sizeof(buff_size));
	                nbytes = recvfrom(sock, buff_size, 20, 0, (struct sockaddr*)&client, &remote_length);
                        printf("Client has sent file size : %d\n", buff_size[0]);
	                
                                char msg[] = "Received file size";
	                        nbytes = sendto(sock, msg, strlen(msg), 0,(struct sockaddr*)&client, remote_length);
	                
                }       


                FILESIZE = buff_size[0];
                printf("Size of file that is expected : %d\n",FILESIZE);
                int div = FILESIZE/(MAXBUFSIZE-2);
                int mod = FILESIZE%(MAXBUFSIZE-2);
                Samepacket_flag = 0;
                
                        struct timeval tv;
                        tv.tv_sec = 0;
                        tv.tv_usec = 000000;            //400 ms
                        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
               

                 for(d = 1; d <= div; d++)
                {
                        
                        //bzero(buffer1,sizeof(buffer1));
                        memset(buffer1, sd, MAXBUFSIZE);
	                nbytes = recvfrom(sock, buffer1, MAXBUFSIZE, 0, (struct sockaddr*)&client, &remote_length);
                        

                        rec[0] = buffer1[MAXBUFSIZE-2];
                        printf("\nLast toggling bit in received packet : %d\n",rec[0]);
                        for(int g =0; g < (MAXBUFSIZE); g++)
                        {
                                if(recv_buffer1[g] != buffer1[g])
                                {
                                printf("Not a Same packet : %d\n",g);
                                Samepacket_flag = 0;
                                break;
                                }
                                else 
                                {
                                    Samepacket_flag = 1;   
                                }
                        }
                        
                        if(Samepacket_flag == 0)

                        {       
                                ack++;
                                
                                for(int n =0 ; n<MAXBUFSIZE; n++)
                                {
                                recv_buffer1[n] = buffer1[n];
                                
                                }
	
	                        int n = fwrite(recv_buffer1, (nbytes - 2), 1 , fs);
		                if(n < 0)
		                {
			        printf("Cannot write\n");
		                }
                                else printf("Written %d th packet\n", prev++);
                                /*if(d == 2)
                                {
                                usleep(200000);
                                }*/
                               
                                char msg2[30];
                                msg2[0] = ack%100;
                                printf("Value of ACK : %d\n", msg2[0]);
	                        nbytes = sendto(sock, msg2, 30, 0,(struct sockaddr*)&client, remote_length);
		         }
	        
                        else printf("Same packet: discarded\n");
                        
                        if(Samepacket_flag == 1)

                        {
                                #ifdef REMOTE
                                
                                char msg2[30];
                                msg2[0] = ack%100;
	                        nbytes = sendto(sock, msg2, 30, 0,(struct sockaddr*)&client, remote_length);
                                
                                #endif
                                d=d-1;
                        }
                }                                               //for loop ends here

                //struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 000000;            //400 ms
                setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));


        	if(mod != 0)
                {
                          int re = 1;
                           rec[0] =1;
                          Samepacket_flag =0;
                          char final[] = "Done";
	                //waits for an incoming message
                        
                                

                       while(1){
                              bzero(buffer1,sizeof(buffer1));  
                              nbytes = recvfrom(sock, buffer1, (mod), 0, (struct sockaddr*)&client, &remote_length);
	                        

                              if(strcmp(buffer1,final) != 0)
                              {
                                for(int g =0; g < mod; g++)
                                {
                                        if(recv_buffer1[g] != buffer1[g])
                                        {
                                        printf("Not a Same packet : %d\n",g);
                                        Samepacket_flag = 0;
                                         break;
                                        }
                                        else Samepacket_flag = 1;
                                        
                                }
                                //prev[0] = rec[0];
                                
                                if(Samepacket_flag == 1)
                                {
                                printf("Same packet found \n");
                                }
                                

                                if(Samepacket_flag == 0)

                                {
                                        ack++;
                                        for(int n =0 ; n<MAXBUFSIZE; n++)
                                        {
                                        recv_buffer1[n] = buffer1[n];
                                        //printf("%s", recv_buffer1);
                                        }

	                                int n = fwrite(recv_buffer1, mod, 1 , fs);
                                        if(n < 0)
                                        {
                                        printf("Cannot write\n");
                                        }
                                        else printf("\nMod Written %d th packet\n", prev++);
                                         
                                        if(re == 0)
                                        {
                                        //usleep(200000);
                                        printf("Sleeping\n");
                                        re = 1;
                                        }

                                        char msg2[30];
                                        msg2[0] = ack%100;
                                        nbytes = sendto(sock, msg2, 30, 0,(struct sockaddr*)&client, remote_length);

                                }

                                if(Samepacket_flag == 1)
                                {
                                        #ifdef REMOTE
                                        char msg2[30];
                                        msg2[0] = ack%100;
                                        nbytes = sendto(sock, msg2, 20, 0,(struct sockaddr*)&client, remote_length);
                                        #endif
                                }
                              
                              }  // if strcmp closes here
                              else if(strcmp(buffer1,final) == 0)
                              {
                           
                                printf("Client says %s\n",buffer1);
                                /*char msg2[] = "Terminating\n";
                                nbytes = sendto(sock, msg2, strlen(msg2), 0,(struct sockaddr*)&client, remote_length);*/
                                break;
                              }
                     } // while 1 closes

                       
                }                               //mod closes
	//nbytes = recvfrom(sock, buffer1, MAXBUFSIZE, 0, (struct sockaddr*)&client, &remote_length);        
	}
	
	else printf("File not open");
        fclose(fs);

}

void Server_Put(char *filename, int MAXBUFSIZE, struct sockaddr_in client, int socket )
{
        fs = fopen(filename,"rb");

        if(fs != NULL)
        {
                fseek(fs, 0, SEEK_END);
                FILESIZE = ftell(fs);
                fseek(fs, 0, SEEK_SET);
                
                int d=0;
	        int div = FILESIZE / (MAXBUFSIZE - 2);
	        int mod = FILESIZE % (MAXBUFSIZE - 2);
                int reading = 1;
                unsigned char buff1[MAXBUFSIZE];
                char buffer[MAXBUFSIZE];
                int ACK_rec;
                int ACK_prev;
                int addr_length = sizeof(client);

                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 200000;            //400 ms
                setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                Timeout_flag = 0;
                char v[10];
                for( d =0; d <= div; d++)
	        {

                        if( d == 0)
                        {
                                bzero(buffer,sizeof(buffer));
	                        nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&client,&addr_length );  

	                        printf("Client says :%s\n",buffer);

                                addr_length = sizeof(client);
                               buff_size[0]  = FILESIZE;
                                printf("Sending file size : %d\n",buff_size[0]);
                               nbytes = sendto(sock, buff_size, 20, 0, (struct sockaddr*)&client, sizeof(client)); 

                                bzero(buffer,sizeof(buffer));
	                        nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&client,&addr_length );  

	                        printf("Client says : %s\n",buffer);
                                d++;

                                v[0] = (uintptr_t)1;
                        }
                
                        if(Timeout_flag == 0)
                        {
                                int j =fread(buff1, (MAXBUFSIZE-2), 1, fs);
			        if(j < 0)
			        {
				        printf("File not read");
			        }
                                else 
                                {
                                       if(v[0] == 1)
				        {
                                                printf("no of bytes :%d\n",j);
                                                v[0] = 0;
                                                buff1[MAXBUFSIZE - 2] =v[0] ;
                                                //buff1[j+1] = '\0';
                                        }

                                        else 
                                        {
                                                v[0] = 1;
                                                buff1[MAXBUFSIZE - 2] = v[0];
                                                //buff1[j+1]='\0';
                
                                        }
                                printf("\nRead :%d   Append :%d\n",reading,buff1[MAXBUFSIZE - 2]);
                                reading++;
                                }
                       }   
                        
                        //printf("\nRead :%d   Append :%d\n",reading,buff1[MAXBUFSIZE - 2]);
                        nbytes = sendto(sock, buff1, MAXBUFSIZE, 0, (struct sockaddr*)&client, sizeof(client));
                        

	                addr_length = sizeof(client);
	                bzero(buffer,sizeof(buffer));
	                nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&client,&addr_length );  

	                printf("Received ACK : %d\n",buffer[0]);
                        int ACK_rec = buffer[0];

                       if(nbytes < 0)
                       {
                                if(errno == EWOULDBLOCK)
                                {
                                        perror("Socket Timeout");
                                        Timeout_flag = 1;
                                        d = d-1;
                       
                                } else perror("recvfrom error");
                       }

                        else if(ACK_prev == ACK_rec)  //change to reading pointer
                        {
                        Timeout_flag =1; 		//should be 1
	                printf("It was a retransmission for %d\n", buffer[0]);
                        d = d - 1;
                        }

                        else 
                        {
                        Timeout_flag =0;
                        ACK_prev = ACK_rec;
                        }
                       
                }

                
                tv.tv_sec = 0;
                tv.tv_usec = 200000;            //400 ms
                setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

                if( mod != 0)
	        {	
	                int jj =fread(buff1, mod, 1, fs);
	                nbytes = sendto(sock, buff1, mod, 0, (struct sockaddr*)&client, sizeof(client));
                        printf("\nLast %d bytes sent to client\n",mod);

	                bzero(buffer,sizeof(buffer));
	                int addr_length = sizeof(client);
                        nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&client,&addr_length );
                        printf("Client says (mod) :  %d\n",buffer[0]);

                        if(nbytes < 0)
                        {
                                if(errno == EWOULDBLOCK)
                                {
                                perror("Socket Timeout");
                                Timeout_flag = 1;
                        
                                } else perror("recvfrom error");

                                do{

                                        nbytes = sendto(sock, buff1, mod, 0, (struct sockaddr*)&client, sizeof(client));
                                        printf("\nLast %d bytes sent again to server\n",mod);
	

                                        bzero(buffer,sizeof(buffer));
	                                int addr_length = sizeof(client);
                                        nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&client,&addr_length );

                                        if(nbytes > 0)
                                        {
                                        ACK_prev = ACK_rec;
                                        break;
                                        }

                                } while(nbytes < 0);

                                char msg1[] = "Done";
                                nbytes = sendto(sock, msg1, strlen(msg1), 0, (struct sockaddr*)&client, sizeof(client));
                                printf("\nSent done\n");

	                        bzero(buffer,sizeof(buffer));
	                        int addr_length = sizeof(client);
                                nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&client,&addr_length );
                                printf("%s\n",buffer);


                        } // if(nbytes <0 loop closes here)

                        else if(ACK_prev == ACK_rec) 
                        {
                        Timeout_flag =1;
	                printf("It was a retransmission for %d\n", buffer[0]);
                        d = d - 1;
                        }

                        else 
                        {
                           char msg1[] = "Done";
                                nbytes = sendto(sock, msg1, strlen(msg1), 0, (struct sockaddr*)&client, sizeof(client));
                                printf("\nSent done\n");

	                        bzero(buffer,sizeof(buffer));
	                        int addr_length = sizeof(client);
                                nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&client,&addr_length );
                                printf("%s\n",buffer);
     

                        }

                
	        }   //mod closes

                
        }
        
        else printf("File not open");
        fclose(fs);
}

void Get_Command(struct sockaddr_in client, int socket)
{
        int remote_length = sizeof(client);
        bzero(Received_Command,sizeof(Received_Command));
	                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 000000;            //400 ms
                setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        nbytes = recvfrom(sock, Received_Command, 70, 0, (struct sockaddr*)&client, &remote_length);
        printf("\n\nReceived command : %s\n",Received_Command);

        if(((ret = strstr(Received_Command, cmd1)) != NULL) ||((ret = strstr(Received_Command, cmd2)) != NULL)  ||((ret = strstr(Received_Command, cmd3)) != NULL) ||((ret = strstr(Received_Command, a)) != NULL)  ||((ret = strstr(Received_Command, b)) != NULL) )

        {
          char msg[] = "Command Received\n";
	nbytes = sendto(sock, msg, strlen(msg), 0,(struct sockaddr*)&client, remote_length);
        }
        else
        {
                char msg12[70] = "----Incorrect Command Received. Send again\n";
                //msg[] = "Incorrect Command Received. Send again\n";
                strcat(Received_Command,msg12);
	nbytes = sendto(sock, Received_Command, strlen(Received_Command), 0,(struct sockaddr*)&client, remote_length);
        }
}



int main (int argc, char * argv[] )
{

	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&my_addr,sizeof(my_addr));                    //zero the struct
	my_addr.sin_family = AF_INET;                   //address family
	my_addr.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine


	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create socket");
	}


	/******************
	  Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
	{
		printf("unable to bind socket\n");
	}



        int MAXBUFSIZE;
        
        while(1)
        {
        char file[20];
        char command[20];
        char file1[30] ="foo1",  file2[30]="foo2", file3[30]="foo3";

        char file1_put[80] ="./ServerFiles/foo1",  file2_put[80]="./ServerFiles/foo2", file3_put[80]="./ServerFiles/foo3";
        char file1_get[50] ="./ServerFiles/foo1",file2_get[50]="./ServerFiles/foo2", file3_get[50]="./ServerFiles/foo3";
        #ifdef LARGEFILE
        MAXBUFSIZE = 2048;
        #else
        MAXBUFSIZE = 512;
        #endif
        
 
        Get_Command(cli_addr, sock);

        if((ret = strstr(Received_Command,a)) != NULL )
        {
                printf("Command:%s\n",Received_Command);                                //just print ls command
                int count;
                struct dirent **ep1;
                char *path="./ServerFiles/";
                char * dir_contents[500];
                int dir_contentcount[30];
        
                count = scandir(path, &ep1, NULL, alphasort);
                dir_contentcount[0] = count;
                printf("Count: %d\n",count);


                for(char i=1 ; i <= count; i++)
                {
                //printf("%s\n",ep1[i-1]->d_name);
                *(dir_contents + i - 1) = ep1[i-1]->d_name;
                }
                int MAXBUFSIZE = 1000;
                int addr_length = sizeof(cli_addr);
                char buffer[MAXBUFSIZE];
                bzero(buffer,sizeof(buffer));
	        nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&cli_addr,&addr_length );  
                printf("Client says: %s\n",buffer);

                
                nbytes = sendto(sock,dir_contentcount, 20, 0, (struct sockaddr*)&cli_addr, addr_length); // Send count of dir contents
                
                bzero(buffer,sizeof(buffer));
	        nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&cli_addr,&addr_length );  
                printf("Client says: %s\n",buffer);

                for(int h = 0; h < count; h++)
                {
                nbytes = sendto(sock,dir_contents[h] , 70, 0, (struct sockaddr*)&cli_addr, addr_length); 
                bzero(buffer,sizeof(buffer));
	        nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&cli_addr,&addr_length );  
                printf("Client says: %s\n",buffer);
                }
                
       }



        
        else if((ret = strstr(Received_Command,b)) != NULL )
        {
                printf("Command:%s\n\n",Received_Command);                                //just print exit command
                 break;
                printf("Exit command encountered. Closing sock");
        }

        else if(((ret = strstr(Received_Command, cmd1)) != NULL) ||((ret = strstr(Received_Command, cmd2)) != NULL)  ||((ret = strstr(Received_Command, cmd3)) != NULL) )
        {
        
        sscanf(Received_Command,"%[^[]%*c%[^]]%*c%s",command,file);                     //seperates out command and file by format specifier"[]"
        printf("Command : %s and File name is: %s\n",command,file);                     // e.g., seperates "get" from "foo1"
        
     
        	if(strcmp(command,cmd1) == 0)                                           //when command received from client is put, 
        {                                                                               //server will do "get"
                
                if(strcmp(file,file1) == 0)                                         //receive packets and write
                {
  
                     Server_Get(file1_get, MAXBUFSIZE, cli_addr, sock);          //compares if file is f001 and write in new file foo1_fromcli
                }

                else if(strcmp(file,file2) == 0)
                {

                     Server_Get(file2_get, MAXBUFSIZE, cli_addr, sock);   
                }
                else if(strcmp(file,file3) == 0)
                {
                     Server_Get(file3_get, MAXBUFSIZE, cli_addr, sock);   
                }
                
        
        }

        else if(strcmp(command,cmd2) == 0)                                      //when command received from client is get
        {                                                                       // server will do put action        
                

                if(strcmp(file,file1) == 0)                                 //read from file and send packets
                {
                     Server_Put(file1_put, MAXBUFSIZE, cli_addr, sock);  
                }

                else if(strcmp(file,file2) == 0)
                {
                     Server_Put(file2_put, MAXBUFSIZE, cli_addr, sock);
                }
                else if(strcmp(file,file3) == 0)
                {
                     Server_Put(file3_put, MAXBUFSIZE, cli_addr, sock);   
                }
         
        }

       else if(strcmp(command,cmd3) == 0)                                      //when command received from client is delete
        {
                char file_path[70] ="./ServerFiles/";
                strcat(file_path, file);
                int status;
                status = remove(file_path);

                if (status == 0)
                {
                        printf("deleted successfully\n");
                }

                else printf("File doesn't exist / Couldn't delete\n");
        }
        
        }               //elseif loop closed

       

                //while(1);
        }      //while loop closes
     close(sock);  
}



