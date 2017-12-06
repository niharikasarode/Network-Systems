


int mod_from_md5(char *filename)
{
    unsigned char c[MD5_DIGEST_LENGTH];
    char Full_md5[50], LastHalf_md5[30];
    //char *filename="foo2";
    int i,mod;
    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];
    char *ptr;
    

    if (inFile == NULL) {
        printf ("%s can't be opened.\n", filename);
        return 0;
    }

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);

    for(i = 0; i < MD5_DIGEST_LENGTH; i++) 
	{

                sprintf(&Full_md5[2*i], "%02x", c[i]);
   	}
	 
        /*printf("Copied : ");
        puts(Full_md5);*/
        //printf("\n\n");
        ptr = &Full_md5[16];
        strcpy(LastHalf_md5, ptr);
        printf("\n Last half %s\n", LastHalf_md5);
        char *ptr1;
        unsigned long ret;
        ret = strtoul(LastHalf_md5, &ptr1, 16);
        printf("ret : %lu\n", ret);
        mod = (ret%4);
        printf("mod value : %d\n", mod);

    	fclose (inFile);
    	return mod;
}


void file_divide(char *filename, char *file_prt1, char *file_prt2, int *arr )
{

        FILE *fp, *fen, *fs;
        int len, chunk =0, rem = 0, a, b, c, d, rem_new=0;
        char cmd[60], fbuff[99999], encrypted[99999], decrypted[99999];
        //int len2 = strlen(key);

        fp = fopen(filename, "rb");
        if( fp != NULL)
        {

                fseek(fp, 0 , SEEK_END);
                len = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                printf("File size : %d\n", len);

                if((len % 4) != 0)
                {
                        chunk =( len/4 ) + 1;
                        rem = (len - (3*chunk));

                        printf("chunk size : %d and rem size : %d\n", chunk, rem);
                }

                else
                {
                        chunk = len/4;
                        rem = 0;
                }
                fclose(fp);
        }

        else
        {
                printf("Error opening file");
                exit(1);
        }

        
        /************************************ File read, encrypted n written in another **************************************************/

        /*for(int i=0; i <len; i++)
        {
                encrypted[i] = fbuff[i]^key[i%len2];
        }
        
        
        fen = fopen("filename","wb");
        if( fen !=NULL)
        {
                fwrite(encrypted, len, 1, fen);

        }
        else
        {
                printf("Could not encrypt");
        }
        fclose(fen);*/

        fp = fopen(filename, "rb");
        if( fp != NULL)
        {

                /**************************** Divide file into 4 parts ****************************/


                /***** 1 ****/

                fseek(fp, 0, SEEK_SET);

                bzero(file_prt1, sizeof(file_prt1));
                bzero(file_prt2, sizeof(file_prt2));
                

                if(chunk > 90000)
                {
                        int f=1, div = chunk/90000;
                        int mod_ch = chunk%90000;

                        if((a < 0) || (b<0))
                        {
                                printf("File not read");
                                exit(1);
                        }
                        fs = fopen("f1","wb");
                        if(fs != NULL)
                        {
                        fseek(fs, 0, SEEK_SET);
                        
                        while(f<=div)
                        {
                        a = fread(file_prt1, sizeof(char), 90000, fp);
                        fwrite(file_prt1, 90000, 1, fs);
                        memset(file_prt1, 0, sizeof(file_prt1));
                        f++;
                        }
                        
                        b = fread(file_prt2, sizeof(char), mod_ch, fp);
                        fseek(fs, 0, SEEK_SET);
                        fseek(fs, (div*90000) , SEEK_SET);
                        fwrite(file_prt2,mod_ch, 1, fs );
                        memset(file_prt2, 0, sizeof(file_prt2));
                        arr[0] = (div*90000)+b;
                        }

                }
                else
                {
                        arr[0] = fread(file_prt1, sizeof(char), chunk, fp);
                        if(arr[0] < 0)
                        {
                                printf("File not read");
                                exit(1);
                        }
                        fs = fopen("f1","wb");
                        if(fs != NULL)
                        {
                        fseek(fs, 0, SEEK_SET);
                        fwrite(file_prt1, chunk, 1, fs);
                        memset(file_prt1, 0, sizeof(file_prt1));
                        }
                
                }
                fclose(fs);

                /***** 2 ****/
                bzero(file_prt1, sizeof(file_prt1));
                bzero(file_prt2, sizeof(file_prt2));
                fseek(fp, 0, SEEK_SET);
                fseek(fp, chunk, SEEK_SET);

                if(chunk > 90000)
                {
                        int f=1, div = chunk/90000;
                        int mod_ch = chunk%90000;

                        if((a < 0) || (b<0))
                        {
                                printf("File not read");
                                exit(1);
                        }
                        fs = fopen("f2","wb");
                        if(fs != NULL)
                        {
                        fseek(fs, 0, SEEK_SET);
                        
                        while(f<=div)
                        {
                        a = fread(file_prt1, sizeof(char), 90000, fp);
                        fwrite(file_prt1, 90000, 1, fs);
                        memset(file_prt1, 0, sizeof(file_prt1));
                        f++;
                        }
                        
                        b = fread(file_prt2, sizeof(char), mod_ch, fp);
                        fseek(fs, 0, SEEK_SET);
                        fseek(fs, (div*90000) , SEEK_SET);
                        fwrite(file_prt2,mod_ch, 1, fs );
                        memset(file_prt2, 0, sizeof(file_prt2));
                        arr[1] = (div*90000)+b;
                        }
                }
                else
                {
                        arr[1] = fread(file_prt1, sizeof(char), chunk, fp);
                        if(arr[1] < 0)
                        {
                                printf("File not read");
                                exit(1);
                        }
                        fs = fopen("f2","wb");
                        if(fs != NULL)
                        {
                        fseek(fs, 0, SEEK_SET);
                        fwrite(file_prt1, chunk, 1, fs);
                        memset(file_prt1, 0, sizeof(file_prt1));
                        }
                
                }

                fclose(fs);
                /**** 3 *****/
                bzero(file_prt1, sizeof(file_prt1));
                bzero(file_prt2, sizeof(file_prt2));
                fseek(fp, 0, SEEK_SET);
                fseek(fp, (2*chunk), SEEK_SET);

                 if(chunk > 90000)
                {
                        int f=1, div = chunk/90000;
                        int mod_ch = chunk%90000;

                        if((a < 0) || (b<0))
                        {
                                printf("File not read");
                                exit(1);
                        }
                        fs = fopen("f3","wb");
                        if(fs != NULL)
                        {
                        fseek(fs, 0, SEEK_SET);
                        
                        while(f<=div)
                        {
                        a = fread(file_prt1, sizeof(char), 90000, fp);
                        fwrite(file_prt1, 90000, 1, fs);
                        memset(file_prt1, 0, sizeof(file_prt1));
                        f++;
                        }
                        
                        b = fread(file_prt2, sizeof(char), mod_ch, fp);
                        fseek(fs, 0, SEEK_SET);
                        fseek(fs, (div*90000) , SEEK_SET);
                        fwrite(file_prt2,mod_ch, 1, fs );
                        memset(file_prt2, 0, sizeof(file_prt2));
                        arr[2] = (div*90000)+b;
                        }
                }
                else
                {
                        arr[2] = fread(file_prt1, sizeof(char), chunk, fp);
                        if(arr[2] < 0)
                        {
                                printf("File not read");
                                exit(1);
                        }
                        fs = fopen("f3","wb");
                        if(fs != NULL)
                        {
                        fseek(fs, 0, SEEK_SET);
                        fwrite(file_prt1, chunk, 1, fs);
                        memset(file_prt1, 0, sizeof(file_prt1));
                        }
                
                }
               
                fclose(fs);


                /***** 4 ****/

                bzero(file_prt1, sizeof(file_prt1));
                bzero(file_prt2, sizeof(file_prt2));
                fseek(fp, 0, SEEK_SET);
                fseek(fp, 3*chunk, SEEK_SET);

                 if((rem == 0) && (chunk > 90000))
                {
                        int f=1, div = chunk/90000;
                        int mod_ch = chunk%90000;

                        if((a < 0) || (b<0))
                        {
                                printf("File not read");
                                exit(1);
                        }
                        fs = fopen("f4","wb");
                        if(fs != NULL)
                        {
                        fseek(fs, 0, SEEK_SET);
                        
                        while(f<=div)
                        {
                        a = fread(file_prt1, sizeof(char), 90000, fp);
                        fwrite(file_prt1, 90000, 1, fs);
                        memset(file_prt1, 0, sizeof(file_prt1));
                        f++;
                        }
                        
                        b = fread(file_prt2, sizeof(char), mod_ch, fp);
                        fseek(fs, 0, SEEK_SET);
                        fseek(fs, (div*90000) , SEEK_SET);
                        fwrite(file_prt2,mod_ch, 1, fs );
                        memset(file_prt2, 0, sizeof(file_prt2));
                        arr[3] = (div*90000)+b;
                        }
                }

                else if((rem != 0) && (rem > 90000))
                {
                        int f=1, div = rem/90000;
                        int mod_ch = rem%90000;

                        if((a < 0) || (b<0))
                        {
                                printf("File not read");
                                exit(1);
                        }
                        fs = fopen("f4","wb");
                        if(fs != NULL)
                        {
                        fseek(fs, 0, SEEK_SET);
                        
                        while(f<=div)
                        {
                        a = fread(file_prt1, sizeof(char), 90000, fp);
                        fwrite(file_prt1, 90000, 1, fs);
                        memset(file_prt1, 0, sizeof(file_prt1));
                        f++;
                        }
                        
                        b = fread(file_prt2, sizeof(char), mod_ch, fp);
                        fseek(fs, 0, SEEK_SET);
                        fseek(fs, (div*90000) , SEEK_SET);
                        fwrite(file_prt2,mod_ch, 1, fs );
                        memset(file_prt2, 0, sizeof(file_prt2));
                        arr[3] = (div*90000)+b;
                        }
                }

                else if( (rem ==0 ) && (chunk <= 90000) )
                {
                        arr[3] = fread(file_prt1, sizeof(char), chunk, fp);
                        if(arr[3] < 0)
                        {
                                printf("File not read");
                                exit(1);
                        }
                        fs = fopen("f4","wb");
                        if(fs != NULL)
                        {
                        fseek(fs, 0, SEEK_SET);
                        fwrite(file_prt1, chunk, 1, fs);
                        memset(file_prt1, 0, sizeof(file_prt1));
                        }
                
                }


                else if( (rem !=0 ) && (rem <= 90000) )
                {
                        arr[3] = fread(file_prt1, sizeof(char), rem, fp);
                        if(arr[3] < 0)
                        {
                                printf("File not read");
                                exit(1);
                        }
                        fs = fopen("f4","wb");
                        if(fs != NULL)
                        {
                        fseek(fs, 0, SEEK_SET);
                        fwrite(file_prt1, rem, 1, fs);
                        memset(file_prt1, 0, sizeof(file_prt1));
                        }
                
                }
                fclose(fs);

                printf("\n\nSizes recorded:\n");
                for(int nk =0; nk < 4; nk++)
                {
                        printf("%d\n", arr[nk]);
                }

                /*sprintf(cmd, "split -b %d -d -a 1 %s", chunk_size, filename );
                system(cmd);

                int var =0;
                while (var<4)
                {
                        sprintf(cmd, "cp x%d .%s.%d", var, filename, var+1);
                        system(cmd);
                        sprintf(cmd , "rm x%d", var);
                        system(cmd);
                        var++;

                }

                sprintf(fname1, ".%s.1", filename);
                sprintf(fname2, ".%s.2", filename);
                sprintf(fname3, ".%s.3", filename);
                sprintf(fname4, ".%s.4", filename);*/

                
                

        }

        else
        {
                printf("Error opening file");
                exit(1);
        }


}


