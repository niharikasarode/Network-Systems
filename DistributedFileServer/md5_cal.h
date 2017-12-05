


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


void file_divide(char *filename, char *file_prt1, char *file_prt2, char *file_prt3, char *file_prt4, int *arr, char *key )
{

        FILE *fp, *fen;
        int len, chunk =0, rem = 0;
        char cmd[60], fbuff[99999], encrypted[99999], decrypted[99999];
        int len2 = strlen(key);

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

        fp = fopen(filename, "rb");
        if( fp != NULL)
        {
                fseek(fp, 0, SEEK_SET);
                fread(fbuff, sizeof(char), len, fp);
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
                bzero(file_prt3, sizeof(file_prt3));
                bzero(file_prt4, sizeof(file_prt4));

                arr[0] = fread(file_prt1, sizeof(char), chunk, fp);
                if(arr[0] < 0)
                {
                        printf("File not read");
                        exit(1);
                }

                for(int i=0; i <chunk; i++)
                {
                        file_prt1[i] = file_prt1[i]^key[i%(len2)];
                }

                /***** 2 ****/

                fseek(fp, 0, SEEK_SET);
                fseek(fp, chunk, SEEK_SET);

                arr[1] = fread(file_prt2, sizeof(char), chunk, fp);
                if(arr[1] < 0)
                {
                        printf("File not read");
                        exit(1);
                }
                for(int i=0; i <chunk; i++)
                {
                        file_prt2[i] = file_prt2[i]^key[i%(len2)];
                }

                /**** 3 *****/

                fseek(fp, 0, SEEK_SET);
                fseek(fp, (2*chunk), SEEK_SET);

                arr[2] = fread(file_prt3, sizeof(char), chunk, fp);
                if(arr[2] < 0)
                {
                        printf("File not read");
                        exit(1);
                }
                for(int i=0; i <chunk; i++)
                {
                        file_prt3[i] = file_prt3[i]^key[i%(len2)];
                }
               

                /***** 4 ****/

                fseek(fp, 0, SEEK_SET);
                fseek(fp, 3*chunk, SEEK_SET);
                int b;
                if(rem == 0)
                {
                        arr[3] = fread(file_prt4, sizeof(char), chunk, fp);
                        for(int i=0; i <chunk; i++)
                        {
                                file_prt4[i] = file_prt4[i]^key[i%(len2)];
                        }
                }                
                else
                {
                        arr[3] = fread(file_prt4, sizeof(char), rem, fp);
                        for(int i=0; i <chunk; i++)
                        {
                                file_prt4[i] = file_prt4[i]^key[i%(len2)];
                        }
                }

                if(arr[3] < 0)
                {
                        printf("File not read");
                        exit(1);
                }


                 fclose(fp);

                for(int p = 0; p<4; p++)
                {
                        printf("%d\n", arr[p]);
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


