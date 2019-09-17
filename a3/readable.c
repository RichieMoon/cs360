/*Ri Zhang
*cs360
*2/14/2019
*Assignment 3
*readable.c: takes one or zero command line arguments.
*Argument, if present, is a pathname (relative or absolute). 
*If no argument is present, the pathname of the present working
directory is assumed (getcwd).
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<dirent.h>
#include<ctype.h>
#include<errno.h>

#define MAX_SIZE 1024
#define TRUE 1


 
void Traverse(char *pathname){
    //check if the pathname access
    if(access(pathname, R_OK)){
        fprintf(stderr, "Error Pathname. error number: %d, error info: %s\n", errno, strerror(errno));
        exit(-1);
    }
    //if access, open the directory
    DIR *dp = opendir(pathname);
    
    if(dp == NULL){ //directory can't be open or null
        fprintf(stderr, "Open Fails. error number: %d, error info: %s\n", errno, strerror(errno));
        exit(-1);
    }
    
    while(dp){
        
        struct dirent* d;
        d = readdir(dp); //read the directory 
        if(d == NULL){
            break;
        }

        struct stat file_type;
        
        char *fullpath = calloc(MAX_SIZE, sizeof(char));
        
        if(fullpath == NULL){
            fprintf(stderr, "Error. error number: %d, error info: %s\n", errno, strerror(errno));
            exit(-1);
        }
        //to create path
        strcpy(fullpath, pathname);
        strcat(fullpath, "/");
        strcat(fullpath, d->d_name);
        
        //check full path
        if(lstat(fullpath, &file_type) == -1){
            fprintf(stderr, "Not good lstat. error number: %d, error info: %s\n", errno, strerror(errno));
            exit(-1);
        }
        
        //each regular file will be print out as a full pathname
        if (S_ISREG(file_type.st_mode)) {  
            printf("%s/%s\n", pathname, d->d_name);   
        }
        
        if (S_ISDIR(file_type.st_mode) && strcmp(d->d_name, ".") != 0 &&
            strcmp(d->d_name, "..") != 0){  
            Traverse(fullpath);
        }
        
        free(fullpath);
    }
    
    //check if close ok
    if (closedir(dp)){
        fprintf (stderr, "Cannot Be Closed. error number: %d, error info: %s\n", errno, strerror(errno));
        exit (-1);
    }
}

int main(int argc, char *argv[]){
    
    char *cwd = calloc(MAX_SIZE, sizeof(char));
    char *path = calloc(MAX_SIZE, sizeof(char));
    
    if(argc == 1){
        if(cwd == NULL){  //when cwd fails
            fprintf(stderr, "Error. error number: %d, error info: %s\n", errno, strerror(errno));
            exit(-1);
        }
        //If no argument is present, the pathname of the present working directory is assumed by using getcwd
        getcwd(cwd, MAX_SIZE);
        Traverse(cwd);
    }else if(argc == 2){ //passed wrong or null input
        if(path == NULL){
            fprintf(stderr, "Error input. error number: %d, error info: %s\n", errno, strerror(errno));
            exit(-1);
        }
        strcpy(path, argv[1]); //save file name to path
        
        while(TRUE){
            int i = 1;
            
            if (isalpha(path[strlen(path) - i])) {
                break;
            }
            if( path[strlen(path) - i] == '/') {   
                path[strlen(path) - i] = '\0';
                i++;
            }
        }
        Traverse(path);
    }else{
        fprintf(stderr, "Error. error number: %d, error info: %s\n", errno, strerror(errno)); //more than one path exit
        exit(-1);
    }
    free(cwd);
    free(path);
    
    return 0;
    
}
