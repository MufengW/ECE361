#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FTP_STR "ftp"
int main(int argc, char *argv[] ){
    char *address;
    int port;
    if( argc == 3 ) {
        address = argv[1];
        port = atoi(argv[2]);
    }else {
        printf("Usage: deliver <server address> <server port number>. \n");
        return 0;
    }
    char ftp_cmd[3];
    char file_name[256];
    printf("Enter your message in the format of \'ftp <file name>\': ");
    scanf("%s %s", ftp_cmd, file_name);
    if (strcmp(ftp_cmd, FTP_STR) == 0){
        printf("ftp cmd \n");
    }else{
        printf("no ftp \n");
        return 0;
    }
    FILE *file;
    if((file = fopen(file_name,"r"))!=NULL){
        printf("file exists \n");
        fclose(file);
    }else{
        printf("file does not exist \n");
    }
    return 0;
}
