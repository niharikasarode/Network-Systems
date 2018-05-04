#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#define REMOTE  
#define LARGEFILE

FILE *fc;


void Client_Put(char filename[], int MAXBUFSIZE, struct sockaddr_in server, int socket );
void Client_Get(char filename[], int MAXBUFSIZE, struct sockaddr_in server, int socket );
void Send_Command(char arr[], struct sockaddr_in server, int socket);


int nbytes, FILESIZE;                             // number of bytes send by sendto()
int sock;                               //this will be our socket
int buff_size[10];
int reading=1;
struct sockaddr_in remote;              //"Internet socket address structure"

int Timeout_flag = 0;
int Samepacket_flag = 0;
void Client_Put(char *filename, int MAXBUFSIZE, struct sockaddr_in server, int socket )
{
        
	fc = fopen(filename,"rb");

        if(fc != NULL)
        {
                fseek(fc, 0, SEEK_END);
                FILESIZE = ftell(fc);
                fseek(fc, 0, SEEK_SET);

                int d=0;
                unsigned char x=0;
	        int div = FILESIZE / (MAXBUFSIZE - 2);
	        int mod = FILESIZE % (MAXBUFSIZE - 2);
                int reading=1;
                char buff1[MAXBUFSIZE];
                char buffer[MAXBUFSIZE];
                int addr_length = sizeof(server);
                int ACK_rec;
                int ACK_prev;
		unsigned char id[10];
                char v[10];
		int j = 0;
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 200000;            //200 ms
                setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

                int Timeout_flag = 0;
                for( d =0; d <= div; d++)
	        {

                        if(d == 0)
                        {
                                bzero(buff_size,sizeof(buff_size));
                              
                               buff_size[0]  = FILESIZE;
                                printf("Sending file size : %d\n",buff_size[0]);
                               nbytes = sendto(sock, buff_size, 20, 0, (struct sockaddr*)&server, sizeof(server)); 

                                int addr_length = sizeof(server);
	                        bzero(buffer,sizeof(buffer));
	                        nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&server,&addr_length );  

	                        printf("Server says: %s\n",buffer);
                                d++;

                                v[0] = (uintptr_t)1;

                         }
                
                        if(Timeout_flag == 0)
                        {
                                j =fread(buff1, (MAXBUFSIZE - 2), 1, fc);
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
                        nbytes = sendto(sock, buff1, MAXBUFSIZE, 0, (struct sockaddr*)&server, sizeof(server));
                       
	                int addr_length = sizeof(server);
	                bzero(buffer,sizeof(buffer));
	                nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&server,&addr_length );  

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
                
                        else{
                         Timeout_flag = 0;
                         ACK_prev = ACK_rec;
                        }
                        
                }
                
                
                tv.tv_sec = 0;
                tv.tv_usec = 200000;            //400 ms
                setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

                if( mod != 0)
	        {	
	                int jj =fread(buff1, mod, 1, fc);
                        if(jj < 0)
                        {
                        printf("Read error\n");
                        }
                        else
                        {
                        
                                printf("\nRead :%d\n",reading);
                                reading++;
                        }
	                nbytes = sendto(sock, buff1, (mod), 0, (struct sockaddr*)&server, sizeof(server));
                        printf("\nLast %d bytes sent to server\n",(mod+1));

	                bzero(buffer,sizeof(buffer));
	                int addr_length = sizeof(server);
                        nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&server,&addr_length );
                        printf("Received ACK : %d\n",buffer[0]);
                        int ACK_rec = buffer[0];
                        if(nbytes < 0)
                        {
                                if(errno == EWOULDBLOCK)
                                {
                                perror("Socket Timeout");
                                Timeout_flag = 1;
                        
                                } else perror("recvfrom error");

                                do{
                                nbytes = sendto(sock, buff1, mod, 0, (struct sockaddr*)&remote, sizeof(remote));
                                printf("\nLast %d bytes sent again to server\n",mod);
	                        
                                bzero(buffer,sizeof(buffer));
	                        int addr_length = sizeof(remote);
                                nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&remote,&addr_length );

                                if(nbytes > 0)
                                {
                                ACK_prev = ACK_rec;
                                break;
                                }

                                } while(nbytes < 0);

                                char msg1[] = "Done";
                                nbytes = sendto(sock, msg1, strlen(msg1), 0, (struct sockaddr*)&server, sizeof(server));
                                printf("\nSent done\n");

	                        bzero(buffer,sizeof(buffer));
	                        int addr_length = sizeof(server);
                                nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&server,&addr_length );
                                printf("%s\n",buffer);

                        }
                        
                        else if(ACK_prev == ACK_rec) 
                        {
                        Timeout_flag =1;
	                printf("It was a retransmission for %d\n", buffer[0]);
                        d = d - 1;
                        }

                        else 
                        {
                                ACK_prev = ACK_rec;
                           char msg1[] = "Done";
                                nbytes = sendto(sock, msg1, strlen(msg1), 0, (struct sockaddr*)&server, sizeof(server));
                                printf("\nSent done\n");

	                        /*bzero(buffer,sizeof(buffer));
	                        int addr_length = sizeof(server);
                                nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&server,&addr_length );
                                printf("%s\n",buffer);*/
     

                        }

                  } //mod closes  

                //nbytes = sendto(sock, "done", MAXBUFSIZE, 0, (struct sockaddr*)&server, sizeof(server));
        }
        
        else printf("File not open");
        fclose(fc);
        
}

void Client_Get(char filename[], int MAXBUFSIZE, struct sockaddr_in server, int socket )
{
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 000000;            //200 ms
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));        
        int ack=0;
         int prev=0; 
        char rec[10];      
        fc = fopen(filename,"wb");
        if( fc != NULL)
	
	{

                unsigned char buff1[MAXBUFSIZE];   //to read received data
                char buffer[MAXBUFSIZE];
                unsigned char recv_buffer1[MAXBUFSIZE];
                int d =0;
                bzero(recv_buffer1,sizeof(recv_buffer1));
                int remote_length = sizeof(server);
                       
                        if(d == 0)
                        {
	                        char msg[] = "Mention file size";
	                        nbytes = sendto(sock, msg, strlen(msg), 0,(struct sockaddr*)&server, remote_length);
                                
                                nbytes = recvfrom(sock, buff_size, 20, 0, (struct sockaddr*)&server, &remote_length);
                                //d = 1;
                                if(buff_size[0] > 0 && buff_size[0] <10000000)
                                {
                                char msg1[] = "Received file size";
	                        nbytes = sendto(sock, msg1, strlen(msg1), 0,(struct sockaddr*)&server, remote_length);
                                }
                        }


                        FILESIZE = buff_size[0];
                        printf("Size of file that is expected : %d\n\n",FILESIZE);
                        int div = FILESIZE/(MAXBUFSIZE-2);
                        int mod = FILESIZE%(MAXBUFSIZE-2);
                        Samepacket_flag = 0;
                        bzero(recv_buffer1,sizeof(recv_buffer1));
                        char rec[10];
                        struct timeval tv;
                        tv.tv_sec = 0;
                        tv.tv_usec = 000000;            //200 ms
                        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));  
              
                for(d = 1; d <= div; d++)
                {
                        bzero(buff1,sizeof(buff1));
	                nbytes = recvfrom(sock, buff1, MAXBUFSIZE, 0, (struct sockaddr*)&server, &remote_length);
                        
                        rec[0] = buff1[MAXBUFSIZE-2];
                        printf("\nLast toggling bit in received packet : %d\n",rec[0]);
                        for(int g =0; g < (MAXBUFSIZE); g++)
                        {
                                if(recv_buffer1[g] != buff1[g])
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

                                if(Samepacket_flag == 1)
                                 {
                                printf("For loop same : %d\n",d);
                                }
                                else printf("For loop not same : %d and div : %d\n",d, div);
                        
                        if(Samepacket_flag == 0)
                        {

                                ack++;
                                for(int n =0 ; n<MAXBUFSIZE; n++)
                                {
                                recv_buffer1[n] = buff1[n];
                                //printf("%s", recv_buffer1);
                                }
	
	                        int n = fwrite(recv_buffer1, (nbytes-2), 1 , fc);
		                if(n < 0)
		                {
			        printf("Cannot write\n");
		                } 
                                else printf("Written %d th packet\n", prev++);

                                if(d == 2)
                                {
                                //usleep(200000);
                                }

                                char msg2[30];
                                msg2[0] = ack%100;
                                printf("Value of ACK : %d\n", msg2[0]);
	                        nbytes = sendto(sock, msg2, 30, 0,(struct sockaddr*)&server, remote_length);
                        }
	        
                      else printf("Same packet so not writinf in file\n");
                        
                        

                        if(Samepacket_flag == 1)

                        {
                           #ifdef REMOTE
                            
                                char msg2[30];
                                msg2[0] = ack%100;
	                   nbytes = sendto(sock, msg2, 30, 0,(struct sockaddr*)&server, remote_length);
                           #endif
                           d=d-1;
                        }
                }                               //for loop ends here

        	
                tv.tv_sec = 0;
                tv.tv_usec = 000000;
                setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

                if(mod != 0)
                {

                int re = 1;
                rec[0] =1;
                Samepacket_flag =0;
                char final[] = "Done";
	         
                   while(1){       
              
                        bzero(buff1,sizeof(buff1));
                        nbytes = recvfrom(sock, buff1, mod, 0, (struct sockaddr*)&server, &remote_length);
	                
                        if(strcmp(buff1,final) != 0)
                              {

                                printf("Not received Done yet in mod\n");
                                for(int g =0; g < mod; g++)
                                     {
                                        if(recv_buffer1[g] != buff1[g])
                                        {
                                        printf("Not a Same mod packet : %d\n",g);
                                        Samepacket_flag = 0;
                                                
                                        }
                                        else 
                                        {
                                        Samepacket_flag = 1;
                                        // printf("Same packet : ");
                                        }
                                     }

                                 if(Samepacket_flag == 0)
                                        {
                                         printf("Not same in mod\n");
                                        }
                                 else printf("\n Same packet found in mod\n");


                                if(Samepacket_flag == 0)

                                {
                                        ack++;
                                        for(int n =0 ; n<MAXBUFSIZE; n++)
                                        {
                                        recv_buffer1[n] = buff1[n];
                                        //printf("%s", recv_buffer1);
                                        }

	                                int n = fwrite(recv_buffer1, mod, 1 , fc);
                                        if(n < 0)
                                        {
                                        printf("Cannot write\n");
                                        }
                                        else printf("mod write done %d\n",prev++);
                                        
                                        

                                        char msg2[30];
                                        msg2[0] = ack%100;
                                        nbytes = sendto(sock, msg2, 30, 0,(struct sockaddr*)&server, remote_length);

                                 }

                                 if(Samepacket_flag == 1)
                                {
                                        #ifdef REMOTE
                                        char msg2[30];
                                        msg2[0] = ack%100;
                                        nbytes = sendto(sock, msg2, 30, 0,(struct sockaddr*)&server, remote_length);
                                        #endif
                                }
                            }               //if strcmp ends

                            else if(strcmp(buff1,final) == 0)
                            {
                                printf("Server says %s\n",buff1);
                                char msg2[] = "Terminating\n";
                                nbytes = sendto(sock, msg2, strlen(msg2), 0,(struct sockaddr*)&server, remote_length);
                                break;
                            }
                        }                                       //while 1 closes

                     }                       // mod closes
        }
	
	else printf("File not open");
        fclose(fc);
}


void Send_Command(char arr[], struct sockaddr_in server, int socket)
{

                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 000000;            //400 ms
                setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                
	        nbytes = sendto(sock, arr, 70, 0, (struct sockaddr*)&server, sizeof(server));
                
                char buffer[70];
	        bzero(buffer,sizeof(buffer));
	        int addr_length = sizeof(server);
                nbytes = recvfrom(sock, buffer, 70, 0, (struct sockaddr *)&server,&addr_length );
                printf("Server says about command status :%s\n",buffer);
}

int main (int argc, char * argv[])
{
        

	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/
	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address
	

	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create socket");
	}

        
        int MAXBUFSIZE;	

         while(1)

        {

        char str[20];
        char file[20];
        char file1[30] ="foo1",  file2[30]="foo2", file3[30]="foo3";
        char file1_put[30] ="./foo1",  file2_put[30]="./foo2", file3_put[30]="./foo3";
        char file1_get[50] ="./ClientFiles/foo1",file2_get[50]="./ClientFiles/foo2", file3_get[50]="./ClientFiles/foo3";
        char command[20]; 
        char *ret, *ret1;
        char cmd1[10] = "put";
        char cmd2[10] = "get";
        char cmd3[10] = "delete";
        char a[5] = "ls"; 
        char b[10] = "exit";
        #ifdef LARGEFILE
        MAXBUFSIZE = 2048;
        #else
        MAXBUFSIZE = 512;
        #endif
      
        printf("Enter one of these commands : get[file_name]\n put[file_name]\n delete[file_name]\n ls\n exit\n\n\n");
         gets(str);
        
        //printf("You entered : %s\n",str);

        if ((ret = strstr(str, a)) != NULL)
        {
                printf("Command:%s\n",str);                                       //just print ls command
                Send_Command(str, remote, sock);
                
                char buffer[70];
                int dir_contentcount[20];
                 int addr_length = sizeof(remote);
                char msg[] = "Send number of contents of the directory\n";
                nbytes = sendto(sock, msg, strlen(msg), 0,(struct sockaddr*)&remote, addr_length);

                nbytes = recvfrom(sock, dir_contentcount, 20, 0, (struct sockaddr *)&remote,&addr_length ); // receive count of dir contents
                printf("Number of files in directory : %d\n",dir_contentcount[0]);

                char msg1[] = "Count received. Send list of files\n";
                nbytes = sendto(sock, msg1, strlen(msg1), 0,(struct sockaddr*)&remote, addr_length);

	        printf("Result of ls on server:\n\r");
                
                for(int h=0; h< dir_contentcount[0]; h++)
                {
                bzero(buffer,sizeof(buffer));
                nbytes = recvfrom(sock, buffer, 70, 0, (struct sockaddr *)&remote,&addr_length );
                printf("%s\n",buffer);

                char msg2[] = "Received\n";
                nbytes = sendto(sock, msg2, strlen(msg2), 0,(struct sockaddr*)&remote, addr_length);
                }
                printf("\n");
                
        }
        else if ((ret1 = strstr(str, b)) != NULL)
        {
                printf("Command:%s\n",str);                                       //just print exit command
                Send_Command(str, remote, sock);
        }
        else if(((ret = strstr(str, cmd1)) != NULL) ||((ret = strstr(str, cmd2)) != NULL)  ||((ret = strstr(str, cmd3)) != NULL) )
        {
        
        sscanf(str,"%[^[]%*c%[^]]%*c%s",command,file);
        printf("Command : %s and File name is: %s\n",command,file);
        

	if(strcmp(command,cmd1) == 0)                                           //when command is put, cmd1 =put
        {
                if(strcmp(file,file1) == 0)
                {
                     Send_Command(str, remote, sock);
                     Client_Put(file1_put, MAXBUFSIZE, remote, sock);   
                }

                else if(strcmp(file,file2) == 0)
                {
                     Send_Command(str, remote, sock);
                     Client_Put(file2_put, MAXBUFSIZE, remote, sock);   
                }
                else if(strcmp(file,file3) == 0)
                {
                     Send_Command(str, remote, sock);
                     Client_Put(file3_put, MAXBUFSIZE, remote, sock);   
                }
                
        
        }

        else if(strcmp(command,cmd2) == 0)                                      //when command is get, cmd2 =get
        {
                if(strcmp(file,file1) == 0)
                {
                     Send_Command(str, remote, sock);
                     Client_Get(file1_get, MAXBUFSIZE, remote, sock);   
                }

                else if(strcmp(file,file2) == 0)
                {
                     Send_Command(str, remote, sock);
                     Client_Get(file2_get, MAXBUFSIZE, remote, sock);   
                }
                else if(strcmp(file,file3) == 0)
                {
                     Send_Command(str, remote, sock);
                     Client_Get(file3_get, MAXBUFSIZE, remote, sock);   
                }
         
        }

        else if(strcmp(command,cmd3) == 0)                                      //when command is delete, cmd3 =delete
        {
                Send_Command(str, remote, sock);
        }
           
       } 

        else  Send_Command(str, remote, sock);
   }    

	close(sock);

}



