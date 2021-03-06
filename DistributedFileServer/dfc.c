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
#define buff_max_size 90000

FILE *conf, *fi, *fop, *fop1, *fr;
char dfc_conf[30], *conf_buffer, username[20], password[20], cmd[60], req_method[30], cmd1[60];
char filename[30], send_buff[buff_max_size], subfolder[60], rec_buff1[buff_max_size], des[buff_max_size];
char *fbuff1, *fbuff2, *fbuff3, *fbuff4, *f1, *f2, *f3, *f4;
char *f_ver1, *f_ver2, *f_ver3, *f_ver4;
char *DFS1_list[99],*DFS2_list[99], *DFS3_list[99], *DFS4_list[99];
int DFS1_count[10], DFS2_count[10], DFS3_count[10], DFS4_count[10];
int port[4], b_size[4], chunk_size,rem_size, req_version;
int fsize_1, fsize_2, fsize_3, fsize_4;
int sockfd1, sockfd2, sockfd3, sockfd4 ;
struct sockaddr_in servaddr;
size_t max = 200;
int svar=0, var=0, ver_num=0, sumofvers=0;
char usr_dir[80];


int server_upflag, server1_down_flag, server2_down_flag, server3_down_flag, server4_down_flag;
int version_obtainedflag;
int req_ver4 = 0;

struct timeval tv;

struct valid_file{
char fl[100];
char vr;
};

struct fileversions{

char file_name[100];
char version;

};

struct fileversions fv[1000], fv_ptr;
struct valid_file vf[1000];

struct srvr_contains{

int available_versions;

};

struct srvr_contains v_number;

void connect_servers(void)
{


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
        if (connect(sockfd1, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
        server1_down_flag =1;
        perror("Problem in connecting to the server");
        //exit(3);
        }
        

        servaddr.sin_port =  htons(port[1]); //convert to big-endian order

        if ((sockfd2 = socket (AF_INET, SOCK_STREAM, 0)) <0)
        {
                perror("Problem in creating the socket");
                exit(2);
        }
       

        if (connect(sockfd2, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
                server2_down_flag =1;
                perror("Problem in connecting to the server");
                //exit(3);
        }
       


        servaddr.sin_port =  htons(port[2]); //convert to big-endian order
        if ((sockfd3 = socket (AF_INET, SOCK_STREAM, 0)) <0)
        {
                perror("Problem in creating the socket");
                exit(2);
        }


        
        if (connect(sockfd3, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
                server3_down_flag =1;
                perror("Problem in connecting to the server");
                //exit(3);
        }



        servaddr.sin_port =  htons(port[3]); //convert to big-endian order
        if ((sockfd4 = socket (AF_INET, SOCK_STREAM, 0)) <0)
        {
                perror("Problem in creating the socket");
                exit(2);
        }


        if (connect(sockfd4, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
                server4_down_flag =1;
                perror("Problem in connecting to the server");
                //exit(3);
        }


}

void connect_server1(void)
{
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
        if (connect(sockfd1, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
        server1_down_flag =1;
        perror("Problem in connecting to the server");
        //exit(3);
        }
}

void connect_server2(void)
{
        if ((sockfd2 = socket (AF_INET, SOCK_STREAM, 0)) <0) {
        perror("Problem in creating the socket");
        exit(2);
        }

        //Creation of the socket
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
        servaddr.sin_port =  htons(port[1]); //convert to big-endian order

        //Connection of the client to the socket
        if (connect(sockfd2, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
        server2_down_flag =1;
        perror("Problem in connecting to the server");
        //exit(3);
        }
}

void connect_server3(void)
{
        if ((sockfd3 = socket (AF_INET, SOCK_STREAM, 0)) <0) {
        perror("Problem in creating the socket");
        exit(2);
        }

        //Creation of the socket
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
        servaddr.sin_port =  htons(port[2]); //convert to big-endian order

        //Connection of the client to the socket
        if (connect(sockfd3, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
        server3_down_flag =1;
        perror("Problem in connecting to the server");
        //exit(3);
        }
}

void connect_server4(void)
{
        if ((sockfd4 = socket (AF_INET, SOCK_STREAM, 0)) <0) {
        perror("Problem in creating the socket");
        exit(2);
        }

        //Creation of the socket
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
        servaddr.sin_port =  htons(port[3]); //convert to big-endian order

        //Connection of the client to the socket
        if (connect(sockfd4, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) 
        {
        server4_down_flag =1;
        perror("Problem in connecting to the server");
        //exit(3);
        }
}





void clear_serverstatus_flags(void)
{

        server1_down_flag =0;
        server2_down_flag =0;
        server3_down_flag =0;
        server4_down_flag =0;
}

void close_sockets(void)
{

        close(sockfd1);
        close(sockfd2);
        close(sockfd3);
        close(sockfd4);

}

void store_version(char *str)
{

        char file1[100], dn[100];
        char vers;
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
        tok = strtok(NULL, "\n");
        vers = atoi(tok);
        //printf(" ********************* vers : %d\n", vers);
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

                       /***********************************      PUT routine        ***********************************/

void sendto_server(int socketfd, int server_no, char *filename, char *buf1, int suffix1, char *buf2, int suffix2, char *Username, char *Password, char *req_type, char *sub_f)
{
        int rec, len1, len2, n, n2;
        char fname1[30], fname2[30], rec_buff[90], key[90], enc[buff_max_size];
        int key_len=strlen(Password);
        strncpy(key, Password, key_len);
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
                                /******    SEND  buf1     *******/                
        
                if (len1 > buff_max_size)
                {
                        fop = fopen(buf1, "rb");
                        if(fop != NULL)
                        {

                                int an =1,div = len1/buff_max_size;
                                int per = len1%buff_max_size;
                                n=0,n2=0;
                                while(n2<len1)
                                {
                                         printf("Coming here 2\n\n");
                                        if(an <= div)
                                        {
                                                int jk = fread(des, sizeof(char), buff_max_size, fop);
                                                for(int l=0; l<buff_max_size; l++)
                                                {
                                                        enc[l] = des[l]^key[l%(key_len)];

                                                }
                                                n = send(socketfd, enc, buff_max_size, 0);
                                                memset(des, 0, sizeof(des));
                                                memset(enc, 0, sizeof(enc));
                                                n2 = n2 +n;
                                                an++;
                                        }
                                        else
                                        {
                                                int jk = fread(des, sizeof(char), per, fop);
                                                for(int l=0; l<per; l++)
                                                {
                                                        enc[l] = des[l]^key[l%(key_len)];

                                                }
                                                n = send(socketfd, enc, per, 0);
                                                memset(des, 0, sizeof(des));
                                                memset(enc, 0, sizeof(enc));
                                                n2 = n2 +n;
                                                an++;
                                        }
                                } 

                        }      
                        printf(" ******** buf1 > 90000, total sent, n2 : %d\n\n", n2);
                }
                else
                {
                        fop = fopen(buf1, "rb");
                        if(fop != NULL)
                        {
                                n=0,n2=0;
                                
                                int jk = fread(des, sizeof(char), len1, fop);
                                                for(int l=0; l<len1; l++)
                                                {
                                                        enc[l] = des[l]^key[l%(key_len)];
                                                       

                                                }
                                n = send(socketfd, enc, len1, 0);
                                memset(des, 0, sizeof(des));
                                memset(enc, 0, sizeof(enc));
                                

                        }

                }
                bzero(rec_buff, sizeof(rec_buff));
                rec = recv(socketfd, rec_buff, 90, 0);
                printf("\n\n After sending 1st PART Server says : ");
                puts(rec_buff);

                if(strncmp(rec_buff, "ACK",3) == 0)
                {

                      
                                /******      SEND  buf2       *****/   

                        if (len2 > buff_max_size)
                        {
                                fop1 = fopen(buf2, "rb");
                                if(fop1 != NULL)
                                {
                                        int an =1,div = len2/buff_max_size;
                                        int per = len2%buff_max_size;
                                        n=0,n2=0;
                                        while(n2<len2)
                                        {
                                                if(an <= div)                                                
                                                {
                                                int jk = fread(des, sizeof(char), buff_max_size, fop1);
                                                for(int l=0; l<buff_max_size; l++)
                                                {
                                                        enc[l] = des[l]^key[l%(key_len)];
                                                  

                                                }
                                                n = send(socketfd, enc, buff_max_size, 0);
                                                memset(des, 0, sizeof(des));
                                                memset(enc, 0, sizeof(enc));
                                                n2 = n2 +n;
                                                an++;
                                                }
                                                else
                                                {
                                                int jk = fread(des, sizeof(char), per, fop1);
                                                for(int l=0; l<per; l++)
                                                {
                                                        enc[l] = des[l]^key[l%(key_len)];


                                                }
                                                n = send(socketfd, enc, per, 0);
                                                memset(des, 0, sizeof(des));
                                                memset(enc, 0, sizeof(enc));
                                                n2 = n2 +n;
                                                }
                                        } 

                                }      
                                printf(" ******** buf1 > 90000, total sent, n2 : %d\n\n", n2);
                        }
                        else
                        {
                                fop1 = fopen(buf2, "rb");
                                if(fop1 != NULL)
                                {
                                n=0,n2=0;
                                       
                                        int jk = fread(des, sizeof(char), len2, fop1);
                                        for(int l=0; l<len2; l++)
                                                {
                                                        enc[l] = des[l]^key[l%(key_len)];

                                                }
                                        n = send(socketfd, enc, len2, 0);
                                        memset(des, 0, sizeof(des));
                                        
                                }

                        }
                        bzero(rec_buff, sizeof(rec_buff));
                        rec = recv(socketfd, rec_buff, 90, 0);
                        printf("\n\n After sending 2nd PART Server says : ");
                        puts(rec_buff);

                }

        }

        else printf("Invalid Username/Password. Could not proceed");
        
}


                        /******************************      LIST routine     *********************************/

void get_dircontents(int socketfd, char *req_type, char *sub_folder, char *Username, char *Password, int server_no)
{


                        tv.tv_sec = 0;
                        tv.tv_usec = 800000;

                        setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                        sprintf(send_buff, "%s %s %s %s %d", req_type, sub_folder, Username, Password, server_no);
                        send(socketfd, send_buff, strlen(send_buff), 0 );
                        
                        bzero(rec_buff1, sizeof(rec_buff1));
                        int rec = recv(socketfd, rec_buff1, 90, 0);
                        
                        if((rec < 0) || rec == 0)
                        {
                                if(errno == EWOULDBLOCK)
                                {
                                        printf(" Socket %d timed out\n", server_no);
                                        perror("Socket Timeout");
                                        
                       
                                } else printf(" Socket %d down\n", server_no);
                        }
                        

                       
                        else
                        {

                                server_upflag++;
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
                                                                int le = strlen(rec_buff1);
                                                                rec_buff1[le] = '\n';
                                                                //*(DFS1_list + h) = rec_buff1;                                                
                                                                //printf("%s\n", DFS1_list[h] );
                                                                store_version(rec_buff1);
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
                                                                int le = strlen(rec_buff1);
                                                                rec_buff1[le] = '\n';  
                                                                //*(DFS2_list + h) = rec_buff1;                                              
                                                                //printf(" ******* In DFS2 %s\n", DFS2_list[h] );
                                                                store_version(rec_buff1);
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
                                                                int le = strlen(rec_buff1);
                                                                rec_buff1[le] = '\n';
                                                                //*(DFS3_list + h) = rec_buff1;
                                                                //printf(" ******* In DFS3 %s\n", DFS3_list[h] );
                                                                store_version(rec_buff1);
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
                                                                int le = strlen(rec_buff1);
                                                                rec_buff1[le] = '\n';
                                                                //*(DFS4_list + h) = rec_buff1;
                                                                //printf("************ In DFS4 %s\n", DFS4_list[h] );
                                                                store_version(rec_buff1);
                                                        }
                                                        send(socketfd, "ACK", 3,0); 
                                                }
                                        } // server_no = 4

                                }
                                else    // if incorrect username/password received
                                {

                                        printf("Invalid Username/Password. Try Again\n\n");
                                }
                                
                                

                        } // if socket did not timeout


}

                        /******************************      GET routine     *********************************/

void get_version(int socketfd, char *file_name1, char *sub_folder, char *Username, char *Password, int version_no, int server_no, char *temp)
{

        char decrypt[buff_max_size];        
        bzero(decrypt, sizeof(decrypt));
        char key1[100];
        int key_len = strlen(Password);
        strncpy(key1, Password, key_len);
        tv.tv_sec = 0;
        tv.tv_usec = 800000;

        setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        memset(send_buff, 0, sizeof(send_buff));
        
        sprintf(usr_dir, "./DFC/%s", file_name1);
        sprintf(send_buff, "%s %s %s %s %s %d %d" , req_method, file_name1, sub_folder, Username, Password, server_no, version_no);
        send(socketfd, send_buff, strlen(send_buff), 0);

        bzero(rec_buff1, sizeof(rec_buff1));
        int rec = recv(socketfd, rec_buff1, 90, 0);

        puts(rec_buff1);
                
        if((rec < 0) || rec == 0)
        {
                if(errno == EWOULDBLOCK)
                {
                        printf(" Socket %d timed out\n", server_no);
                        perror("Socket Timeout");
                                        
                       
                } else printf(" Socket %d down\n", server_no);

                if(server_no == 1) server1_down_flag = 1; 
                else if(server_no == 2) server2_down_flag = 1;
                else if(server_no == 3) server3_down_flag = 1; 
                else if(server_no == 4) server4_down_flag = 1;  
        }

        else
        {
                if(strncmp(rec_buff1,"Available",9)==0)  
                {

                        char status[20];
                        sscanf(rec_buff1, "%s %d", status, &fsize_1);
                        if(version_no == 4)
                        {
                                fsize_4 = fsize_1;
                                sumofvers = (3*fsize_2)+ fsize_4;
                        }
                        else fsize_2 = fsize_1;

                        sprintf(send_buff, "Send file part");
                        send(socketfd, send_buff, strlen(send_buff), 0);


                        bzero(rec_buff1, sizeof(rec_buff1));
                        //int rec= recv(socketfd, rec_buff1, fsize_1, 0);
                        
                        if(version_no != 4)
                        {
                                FILE *fp1;
                                
                                fp1 = fopen(temp, "wb");
                                        
                                if(fp1 != NULL)
                                {
                                        if(fsize_2 > buff_max_size)
                                        {
                                        int an =1,div = fsize_2/buff_max_size;
                                        int per = fsize_2%buff_max_size;
                                        int n=0,n2=0;

                                        while(n2<fsize_2)
                                        {
                                                n = recv(socketfd, rec_buff1, buff_max_size, 0);
                                                for(int l=0; l<buff_max_size; l++)
                                                {
                                                        decrypt[l] = rec_buff1[l]^key1[l%(key_len)];

                                                }
                                                fwrite(decrypt, n, 1, fp1);
                                                bzero(rec_buff1, sizeof(rec_buff1));
                                                bzero(decrypt, sizeof(decrypt));
                                                n2 = n2 + n;

                                        }
                                        printf("Written %d bytes for %d\n", n2, version_no);
                                        }

                                        else
                                        {
                                        int n = recv(socketfd, rec_buff1, fsize_2, 0);
                                                for(int l=0; l<fsize_2; l++)
                                                {
                                                        decrypt[l] = rec_buff1[l]^key1[l%(key_len)];

                                                }       
                                        fwrite(decrypt, n, 1, fp1);
                                        bzero(rec_buff1, sizeof(rec_buff1));
                                        bzero(decrypt, sizeof(decrypt));

                                        }
                                }
                                else printf("GET function file not written");
                                fclose(fp1);
                        }

                        else
                        {

                                FILE *fs1;
                                
                                fs1 = fopen(temp, "wb");
                                        
                                if(fs1 != NULL)
                                {
                                        if(fsize_4 > buff_max_size)
                                        {
                                        int an =1,div = fsize_4/buff_max_size;
                                        int per = fsize_4%buff_max_size;
                                        int n=0,n2=0;

                                        while(n2<fsize_4)
                                        {
                                                n = recv(socketfd, rec_buff1, buff_max_size, 0);
                                                for(int l=0; l<buff_max_size; l++)
                                                {
                                                        decrypt[l] = rec_buff1[l]^key1[l%(key_len)];

                                                }
                                                fwrite(decrypt, n, 1, fs1);
                                                bzero(rec_buff1, sizeof(rec_buff1));
                                                bzero(decrypt, sizeof(decrypt));
                                                n2 = n2 + n;

                                        }
                                        printf("Written %d bytes for %d\n", n2, version_no);
                                        }

                                        else
                                        {
                                        int n = recv(socketfd, rec_buff1, fsize_4, 0);
                                                for(int l=0; l<fsize_4; l++)
                                                {
                                                        decrypt[l] = rec_buff1[l]^key1[l%(key_len)];

                                                }
                                        fwrite(decrypt, n, 1, fs1);
                                        bzero(rec_buff1, sizeof(rec_buff1));
                                                bzero(decrypt, sizeof(decrypt));

                                        }
                                }
                                else printf("GET function file not written");
                                fclose(fs1);



                        }
        


                        version_obtainedflag |= 1 << version_no;
                        
                }

                else if(strncmp(rec_buff1, "Requested",9)==0)                   //Folders not present on DFS
                {
                        puts(rec_buff1);
                }
        
                else if(strncmp(rec_buff1, "Invalid",7 )==0)
                {

                        printf(" Invalid Username/Password. Try Again !");
                }

                else if(strncmp(rec_buff1, "File not",8 )==0)
                {

                       puts(rec_buff1);
                }

                else if(strncmp(rec_buff1, "Version", 7 )==0)
                {

                }
                else puts(rec_buff1);


        }

}


int main(int argc, char **argv)
{
       
        
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

            while(1)
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
                free(conf_buffer); 
 
        

                                                                                                     
        

        

        server_upflag=0;
        server1_down_flag=0; 
        server2_down_flag=0;                                                
        server3_down_flag=0;                                                
        server3_down_flag=0;         


                printf("\n\n\n");
                printf(" *-*-*-*-*-*-*-  The following commands are handled by the DFC :    *-*-*-*-*-*-*-\n");
                printf("                1. LIST/list subfolder : lists files stored at the server ofr a given username \n ");
                printf("               2. PUT/put filename subfolder: Sends a file to the distributed servers\n");
                printf("                3. GET/get filename subfolder: Retrieves a file from the distributed servers\n ");
                printf("                4. MKDIR/mkdir subfolder : Makes a subfolder on all servers\n\n\r ");


                bzero(cmd, sizeof(cmd));
                bzero(filename, sizeof(filename));
                bzero(req_method, sizeof(req_method));
                bzero(subfolder, sizeof(subfolder));


                fgets(cmd, MAXLINE, stdin);
                printf("\n\n You entered : ");
                puts(cmd);
                if( (strncmp(cmd, "PUT", 3) == 0) || (strncmp(cmd, "put", 3) == 0) || ( strncmp(cmd, "GET", 3) == 0) || (strncmp(cmd, "get", 3) == 0) ) 
                {

                        
                        sscanf(cmd, "%s %s %s", req_method, filename, subfolder);
                        if( strlen(filename) > 1)
                        {
                                if(strlen(subfolder) > 1)
                                {
                                        if( (strncmp(req_method, "PUT", 3) == 0) ||   (strncmp(req_method, "put", 3) == 0) )
                                        {
                                                connect_servers();
                                                printf("\n\n Connected ..\n\n");

                        
                                                int mod = mod_from_md5(filename);
                                                printf("mod : %d\n", mod);
                                                chunk_size = 0;
                                                rem_size =0;
                                                
                                                fbuff1 = (char*) calloc(buff_max_size, sizeof(char));
                                                if(fbuff1 == NULL) printf("No memory alloc 1\n");
                                                else printf("Mem alloc 1\n");
                                                 
                                                fbuff2 = (char*) calloc(buff_max_size, sizeof(char));
                                                if(fbuff2 == NULL) printf("No memory alloc 2\n");
                                                else printf("Mem alloc 2\n");
                                                /*strncpy(f1,"f1", 2);
                                                strncpy(f2,"f2", 2);
                                                strncpy(f3,"f3", 2);
                                                strncpy(f4,"f4", 2);*/
                                                //divides a file into 4 files
                                                file_divide(filename, fbuff1, fbuff2, b_size);

                                                switch(mod)
                                                {
                                                        case 0:
                
                                                        sendto_server(sockfd1, 1, filename, "f1", 1, "f2", 2, username, password, req_method, subfolder); 
                                                        sendto_server(sockfd2, 2, filename, "f2", 2, "f3", 3, username, password, req_method, subfolder); 
                                                        sendto_server(sockfd3, 3, filename, "f3", 3, "f4", 4, username, password, req_method, subfolder); 
                                                        sendto_server(sockfd4, 4, filename, "f4", 4, "f1", 1, username, password, req_method, subfolder);
                                                        break; 

                                                        case 1:

                                                        sendto_server(sockfd1, 1, filename, "f4", 4, "f1", 1, username, password, req_method, subfolder);
                                                        sendto_server(sockfd2, 2, filename, "f1", 1, "f2", 2, username, password, req_method, subfolder); 
                                                        sendto_server(sockfd3, 3, filename, "f2", 2, "f3", 3, username, password, req_method, subfolder); 
                                                        sendto_server(sockfd4, 4, filename, "f3", 3, "f4", 4, username, password, req_method, subfolder); 
 

                                                        break; 


                                                        case 2:

                                                        sendto_server(sockfd1, 1, filename, "f3", 3, "f4", 4, username, password, req_method, subfolder); 
                                                        sendto_server(sockfd2, 2, filename, "f4", 4, "f1", 1, username, password, req_method, subfolder);
                                                        sendto_server(sockfd3, 3, filename, "f1", 1, "f2", 2, username, password, req_method, subfolder);
                                                        sendto_server(sockfd4, 4, filename, "f2", 2, "f3", 3, username, password, req_method, subfolder); 
 
                                                        break; 


                                                        case 3:

                                                        sendto_server(sockfd1, 1, filename, "f2", 2, "f3", 3, username, password, req_method, subfolder); 
                                                        sendto_server(sockfd2, 2, filename, "f3", 3, "f4", 4, username, password, req_method, subfolder); 
                                                        sendto_server(sockfd3, 3, filename, "f4", 4, "f1", 1, username, password, req_method, subfolder);
                                                        sendto_server(sockfd4, 4, filename, "f1", 1, "f2", 2, username, password, req_method, subfolder); 

                                                        break; 

                                                }
                                                
                                                free(fbuff1);
                                                free(fbuff2);
                                                var=1;        
                                                while(var < 5)
                                                {
                                                        sprintf(cmd1 , "rm f%d", var);
                                                        system(cmd1);
                                                        var++;
                                                }
                                                var=1;


                                                close_sockets();
                
                                        } // if command put


                                        else if( (strncmp(req_method, "GET", 3) == 0 ) || (strncmp(req_method, "get", 3) == 0 ) )
                                        {

                                                connect_servers();
                                                printf("\n\n Connected ..\n\n");

                                                f_ver1 = (char*) calloc(buff_max_size , sizeof(char));
                                                if(f_ver1 == NULL) printf("No memory alloc 1\n");
                
                                                
                                                version_obtainedflag=0;

                                                req_ver4 =0;
        /*****  Check if server is down, if already found down, probing the same server again causes the program to exit    *****/


                                                int server_num = 1;      
                                                do
                                                {       
                                                if(server_num == 1)
                                                {
                                                        if(server1_down_flag == 0) get_version(sockfd1, filename, subfolder, username, password, 1, server_num, "fp1");
                                                }
                                                 else if(server_num == 2)
                                                {
                                                        if(server2_down_flag == 0) get_version(sockfd2, filename, subfolder, username, password, 1, server_num, "fp1");
                                                }
                                                else if(server_num == 3)
                                                {
                                                         if(server3_down_flag == 0) get_version(sockfd3, filename, subfolder,username, password, 1, server_num, "fp1");
                                                }
                                                else if(server_num == 4)
                                                {
                                                        if(server4_down_flag == 0) get_version(sockfd4, filename, subfolder, username, password, 1, server_num, "fp1");
                                                }
                                                
                                                        server_num++;
                                                }while( (version_obtainedflag != 2) && (server_num <5));              
                                                close_sockets();




                                                server_num = 1;      
                                                do
                                                {       
                                                if(server_num == 1)
                                                {
                                                        if(server1_down_flag == 0)
                                                        {
                                                                connect_server1();                
                                                                get_version(sockfd1, filename, subfolder, username, password, 2, server_num, "fp2");
                                                                close(sockfd1);
                                                        }
                                                }
                                                else if(server_num == 2)
                                                {
                                                        if(server2_down_flag == 0)
                                                        {
                                                                connect_server2();                
                                                                get_version(sockfd2, filename, subfolder, username, password, 2, server_num, "fp2");
                                                                close(sockfd2);
                                                        }
                                                }
                                                else if(server_num == 3)
                                                {
                                                        if(server3_down_flag == 0)
                                                        {
                                                                connect_server3();                
                                                                get_version(sockfd3, filename, subfolder, username, password, 2, server_num, "fp2");
                                                                close(sockfd3);
                                                        }
                                                }
                                                else if(server_num == 4)
                                                {
                                                       if(server4_down_flag == 0)
                                                        {
                                                                connect_server4();                
                                                                get_version(sockfd4, filename, subfolder, username, password, 2, server_num, "fp2");
                                                                close(sockfd4);
                                                        }
                                                }
                                                server_num++;


                                                }while( (version_obtainedflag <= 3) && (server_num <5));







                                                server_num = 1;      
                                                do
                                                {       
                                                if(server_num == 1)
                                                {
                                                        if(server1_down_flag == 0)
                                                        {
                                                                connect_server1();                
                                                                get_version(sockfd1, filename, subfolder, username, password, 3, server_num, "fp3");
                                                                close(sockfd1);
                                                        }
                                                }
                                                else if(server_num == 2)
                                                {
                                                        if(server2_down_flag == 0)
                                                        {
                                                                connect_server2();                
                                                                get_version(sockfd2, filename, subfolder, username, password, 3, server_num, "fp3");
                                                                close(sockfd2);
                                                        }
                                                }
                                                else if(server_num == 3)
                                                {
                                                         if(server3_down_flag == 0)
                                                        {
                                                                connect_server3();                
                                                                get_version(sockfd3, filename, subfolder, username, password, 3, server_num, "fp3");
                                                                close(sockfd3);
                                                        }
                                                }
                                                else if(server_num == 4)
                                                {
                                                        if(server4_down_flag == 0)
                                                        {
                                                                connect_server4();                
                                                                get_version(sockfd4, filename, subfolder, username, password, 3, server_num, "fp3");
                                                                close(sockfd4);
                                                        }
                                                }
                                                server_num++;

                                                }while( (version_obtainedflag <= 7) && (server_num <5));







                                                server_num = 1;      
                                                do
                                                {       
                                                if(server_num == 1)
                                                {
                                                        if(server1_down_flag == 0)
                                                        {
                                                                connect_server1();                
                                                                get_version(sockfd1, filename, subfolder, username, password, 4, server_num, "fp4");
                                                                close(sockfd1);
                                                        }
                                                }
                                                else if(server_num == 2)
                                                {
                                                        if(server2_down_flag == 0)
                                                        {
                                                                connect_server2();                
                                                                get_version(sockfd2, filename, subfolder, username, password, 4, server_num, "fp4");
                                                                close(sockfd2);
                                                        }
                                                }
                                                else if(server_num == 3)
                                                {
                                                         if(server3_down_flag == 0)
                                                        {
                                                                connect_server3();                
                                                                get_version(sockfd3, filename, subfolder, username, password, 4, server_num, "fp4");
                                                                close(sockfd3);
                                                        }
                                                }
                                                else if(server_num == 4)
                                                {
                                                        if(server4_down_flag == 0)
                                                        {
                                                                connect_server4();                
                                                                get_version(sockfd4, filename, subfolder, username, password, 4, server_num, "fp4");
                                                                close(sockfd4);
                                                        }
                                                }
                                                server_num++;
                                                req_ver4 = 1;
                                                }while( (version_obtainedflag <= 15) && (server_num <5));
                                                //close_sockets();
                                                clear_serverstatus_flags();
                
                                                if(req_ver4 == 1)
                                                {
                                                        if(version_obtainedflag == 30)
                                                        {
                                                                printf("Obtained requested file : %s\n\n", filename);

                                
                                                                fi = fopen(usr_dir, "wb");
                                                                
                                                                if(fi != NULL)
                                                                {
                                                                        fseek(fi, 0, SEEK_SET);
                                                                        fr = fopen("fp1", "rb");
                                                                        if(fr != NULL)
                                                                        {
                                                                                if(fsize_2 > buff_max_size)
                                                                                {
                                                                                 int an =1,div = fsize_2/buff_max_size;
                                                                                 int per = fsize_2%buff_max_size;
                                                                                 int n=0,n2=0;       
                        
                                                                                        while(n2<fsize_2)
                                                                                        {

                                                                                                if(an <= div)
                                                                                                {
                                                                                                fread(f_ver1, sizeof(char), buff_max_size, fr);
                                                                                                fwrite(f_ver1, buff_max_size, 1, fi);
                                                                                                memset(f_ver1, 0, sizeof(f_ver1));
                                                                                                n2 = n2 + buff_max_size;
                                                                                                an++;
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                fread(f_ver1, sizeof(char), per, fr);
                                                                                                fwrite(f_ver1, per, 1, fi);
                                                                                                memset(f_ver1, 0, sizeof(f_ver1));
                                                                                                n2 = n2 + per;
                                                                                                an++;
                                                                                                }
                                                                                        } 

                                                                                        

                                                                                }
                                                                                else
                                                                                {
                                                                                        fread(f_ver1, fsize_2, 1, fr);
                                                                                        fwrite(f_ver1, fsize_2, 1, fi);
                                                                                        memset(f_ver1, 0, sizeof(f_ver1));
                                                                                }
                                                                        }
                                                                        fclose(fr);

                                                                        fseek(fi, 0, SEEK_SET);
                                                                        fseek(fi, fsize_2, SEEK_SET);
                                                                        fr = fopen("fp2", "rb");
                                                                        if(fr != NULL)
                                                                        {
                                                                                if(fsize_2 > buff_max_size)
                                                                                {
                                                                                 int an =1,div = fsize_2/buff_max_size;
                                                                                 int per = fsize_2%buff_max_size;
                                                                                 int n=0,n2=0;       
                        
                                                                                        while(n2<fsize_2)
                                                                                        {

                                                                                                if(an <= div)
                                                                                                {
                                                                                                fread(f_ver1, sizeof(char), buff_max_size, fr);
                                                                                                fwrite(f_ver1, buff_max_size, 1, fi);
                                                                                                memset(f_ver1, 0, sizeof(f_ver1));
                                                                                                n2 = n2 + buff_max_size;
                                                                                                an++;
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                fread(f_ver1, sizeof(char), per, fr);
                                                                                                fwrite(f_ver1, per, 1, fi);
                                                                                                memset(f_ver1, 0, sizeof(f_ver1));
                                                                                                n2 = n2 + per;
                                                                                                an++;
                                                                                                }
                                                                                        } 

                                                                                        

                                                                                }
                                                                                else
                                                                                {
                                                                                        fread(f_ver1, fsize_2, 1, fr);
                                                                                        fwrite(f_ver1, fsize_2, 1, fi);
                                                                                        memset(f_ver1, 0, sizeof(f_ver1));
                                                                                }
                                                                        }
                                                                        fclose(fr);

                                                                        fseek(fi, 0, SEEK_SET);
                                                                        fseek(fi, (2*fsize_2), SEEK_SET);
                                                                        fr = fopen("fp3", "rb");
                                                                        if(fr != NULL)
                                                                        {
                                                                                if(fsize_2 > buff_max_size)
                                                                                {
                                                                                 int an =1,div = fsize_2/buff_max_size;
                                                                                 int per = fsize_2%buff_max_size;
                                                                                 int n=0,n2=0;       
                        
                                                                                        while(n2<fsize_2)
                                                                                        {

                                                                                                if(an <= div)
                                                                                                {
                                                                                                fread(f_ver1, sizeof(char), buff_max_size, fr);
                                                                                                fwrite(f_ver1, buff_max_size, 1, fi);
                                                                                                memset(f_ver1, 0, sizeof(f_ver1));
                                                                                                n2 = n2 + buff_max_size;
                                                                                                an++;
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                fread(f_ver1, sizeof(char), per, fr);
                                                                                                fwrite(f_ver1, per, 1, fi);
                                                                                                memset(f_ver1, 0, sizeof(f_ver1));
                                                                                                n2 = n2 + per;
                                                                                                an++;
                                                                                                }
                                                                                        } 

                                                                                        

                                                                                }
                                                                                else
                                                                                {
                                                                                        fread(f_ver1, fsize_2, 1, fr);
                                                                                        fwrite(f_ver1, fsize_2, 1, fi);
                                                                                        memset(f_ver1, 0, sizeof(f_ver1));
                                                                                }
                                                                        }
                                                                        fclose(fr);

                                                                        fseek(fi, 0, SEEK_SET);
                                                                        fseek(fi, (3*fsize_2), SEEK_SET);
                                                                        fr = fopen("fp4", "rb");
                                                                        if(fr != NULL)
                                                                        {
                                                                                if(fsize_4 > buff_max_size)
                                                                                {
                                                                                 int an =1,div = fsize_4/buff_max_size;
                                                                                 int per = fsize_4%buff_max_size;
                                                                                 int n=0,n2=0;       
                        
                                                                                        while(n2<fsize_4)
                                                                                        {

                                                                                                if(an <= div)
                                                                                                {
                                                                                                fread(f_ver1, sizeof(char), buff_max_size, fr);
                                                                                                fwrite(f_ver1, buff_max_size, 1, fi);
                                                                                                memset(f_ver1, 0, sizeof(f_ver1));
                                                                                                n2 = n2 + buff_max_size;
                                                                                                an++;
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                fread(f_ver1, sizeof(char), per, fr);
                                                                                                fwrite(f_ver1, per, 1, fi);
                                                                                                memset(f_ver1, 0, sizeof(f_ver1));
                                                                                                n2 = n2 + per;
                                                                                                an++;
                                                                                                }
                                                                                        } 

                                                                                        

                                                                                }
                                                                                else
                                                                                {
                                                                                        fread(f_ver1, fsize_4, 1, fr);
                                                                                        fwrite(f_ver1, fsize_4, 1, fi);
                                                                                        memset(f_ver1, 0, sizeof(f_ver1));
                                                                                }
                                                                        }
                                                                        fclose(fr);

                                                                        fclose(fi);
                                                                 }
                                                        }
                                                                                                                      
                                                        else printf("File is not constructable, %d\n\n", version_obtainedflag);

                                                        fsize_1 =0;
                                                        fsize_2 =0;
                                                        fsize_3 =0;
                                                        fsize_4 =0;
                                                        free(f_ver1);
                                                        var=1;        
                                                        while(var < 5)
                                                        {
                                                        sprintf(cmd1 , "rm fp%d", var);
                                                        system(cmd1);
                                                        var++;
                                                        }
                                                        var=1;
                                                        
                                                }
        

                                        }               // else if for req_method = get

  
                                }
                                else printf("\n USAGE : <(PUT/put) OR (GET/get)> <filename> <subfolder> \n\n");
                        }
                        else printf("\n USAGE : <(PUT/put) OR (GET/get)> <filename> <subfolder> \n\n");
                }

                else if( (strncmp(cmd, "LIST", 4) == 0) || (strncmp(cmd, "list", 4) == 0) || ( strncmp(cmd, "MKDIR", 5) == 0) || (strncmp(cmd, "mkdir", 5) == 0) )
                {
                        sscanf(cmd, "%s %s", req_method, subfolder);
                        if(strlen(subfolder) > 1)
                        {

                                if( (strncmp(req_method, "LIST", 4) == 0) || (strncmp(req_method, "list", 4) == 0) )
                                {
                        
                                        connect_servers();
                                        printf("\n\n Connected ..\n\n");

                                        bzero(DFS1_count, sizeof(DFS1_count));
                                        bzero(DFS2_count, sizeof(DFS2_count));
                                        bzero(DFS3_count, sizeof(DFS3_count));
                                        bzero(DFS4_count, sizeof(DFS4_count));
                                        memset(fv, 0, 100*sizeof(struct fileversions));
                                        memset(vf, 0, 100*sizeof(struct valid_file));

                                        svar = 0;
                       
                                        if(server1_down_flag == 0) get_dircontents(sockfd1, req_method, subfolder, username, password, 1);
                                        if(server2_down_flag == 0) get_dircontents(sockfd2, req_method, subfolder, username, password, 2);
                                        if(server3_down_flag == 0) get_dircontents(sockfd3, req_method, subfolder, username, password, 3);
                                        if(server4_down_flag == 0) get_dircontents(sockfd4, req_method, subfolder, username, password, 4);

                                        int yu =0;
                                        yu = DFS1_count[0] +  DFS2_count[0] +  DFS3_count[0] +  DFS4_count[0];
                                        yu = yu - (2*server_upflag);      
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

                                                /*for(int l=0; l<100; l++)
                                                {
                                                        if(vf[l].fl[0] != 0)
                                                        {
                                                                printf(" %d. %s %d\n", l, vf[l].fl, vf[l].vr);
                                                        }
                                                }*/        
                
                                        for(int d=0; d<100; d++)
                                        {
                                                if(vf[d].vr == 30) printf(" %d. %s Full File\n", d+1, vf[d].fl);
                                                else if( (vf[d].vr > 0) &&(vf[d].vr < 30)) printf(" %d. %s Incomplete File\n", d+1, vf[d].fl);
                                                else if( vf[0].fl[0] == 0)
                                                {
                                                        printf("\n THE DFS SERVERS DONT HAVE ANY FILES :( \n");
                                                        break;
                                                }
                                                else if(vf[d].fl[0] == 0) break;

                                        }

                                        svar=0;

                                        close_sockets();
                                        clear_serverstatus_flags();
                                }       // if req_method = LIST


                                else if( (strncmp(req_method, "MKDIR", 5) == 0) || (strncmp(req_method, "mkdir", 5) == 0) )
                                {

                                        connect_servers();
                                        printf("\n\n Connected ..\n\n");

                        
                                                /**************************/
                                        sprintf(send_buff, "%s %s %s %s %d", req_method, subfolder, username, password, 1);
                                        send(sockfd1, send_buff, strlen(send_buff), 0 );

                                        bzero(rec_buff1, sizeof(rec_buff1));
                                        int rec = recv(sockfd1, rec_buff1, 90, 0);
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


                                        close_sockets();
                                }       // else if req_method = MKDIR


                        }
                        else printf("\n USAGE : <LIST/list) OR (MKDIR/mkdir)> <filename> <subfolder> \n\n");
                }












        } // while



     } // else for the argument check
         
        
 //exit(0);
} // main
