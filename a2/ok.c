/*
*Ri Zhang
*cs360
*Programming assignment 2
*checking a word either in the dictionary or not using binary search
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>

#define location_name "dictionaries/webster"
#define word_length 16
#define TRUE 1
//#define DEBUG_MSG //display debug messages 

int ok(int fd, char *word);

int main(int argc, char *argv[]){
	
	if (argc != 2){  //checking for user input
		
		printf("Invalid input! Correct format: ./ok word (where word is a word that you want to check).\n");
		printf("Value of errno: %d\n", errno); 
		printf("The error message is : %s\n", strerror(errno)); 
		perror("Message from perror"); 

		exit(TRUE);
	}

	int fd;
	if((fd = open(location_name, O_RDONLY,0))<0){ //check file access
		
		printf("Value of errno: %d\n", errno); 
		printf("The error message is : %s\n", strerror(errno)); 
		perror("Message from perror"); 

		exit(TRUE);
	}

	if(ok(fd, argv[1])){ //passing the word from stdin to function ok()
		printf("yes\n"); //if the word exists in the dictionaries
	}else{
		printf("no\n");  //if not
	}

	if(close(fd) == 0) return 0; //check if we can close the file correctly
	else if (close(fd) != 0)
	{
		printf("Value of errno: %d\n", errno); 
		printf("The error message is : %s\n", strerror(errno)); 
		perror("Message from perror");

		exit(TRUE); 
	}
}

int ok(int fd, char *word){
	int bot=0;
	int top, mid;

	char *want = calloc(word_length-1, sizeof(char));

	if(lseek(fd, 0, SEEK_END) == -1){ //if lseek fails
		printf("fd is not a disk file\n");
		printf("Value of errno: %d\n", errno); 
		printf("The error message is : %s\n", strerror(errno)); 
		perror("Message from perror");

		exit(TRUE);
	}
	top = (lseek(fd, 0, SEEK_END) / word_length) + 1; //top= last line number + 1
	
	strcpy(want, word); //copy the word we want to check
	for(int i = strlen(want); i< word_length-1; i++){
		want[i] = ' ';
	}

	#ifdef DEBUG_MSG
		printf("word wanted = ''%s\t''\n", want);
	#endif

	while(TRUE){
		char *have = calloc(word_length-1,sizeof(char));
		if(bot >= top) return 0;
		mid = (bot + top) / 2; 

		int lseekerror = lseek(fd, mid*word_length, SEEK_SET);
		if(lseekerror == -1){ //lseek fails
			printf("lseek fails.\n");
			printf("Value of errno: %d\n", errno); 
			printf("The error message is : %s\n", strerror(errno));
			perror("Message from perror");
			exit(TRUE);
		} 

		//read(fd, have, word_length-1);//read line into have
		int readerror = read(fd, have, word_length-1);;
		if(readerror == -1){ //read fails set errno to [EAGAIN]
			printf("read fails.\n");
			printf("Value of errno: %d\n", errno); 
			printf("The error message is : %s\n", strerror(errno)); 
			perror("Message from perror");

			exit(TRUE);
		}

		#ifdef DEBUG_MSG
			printf("search range: bottom = %d,  top = %d\n", bot, top);
        	printf("middle = %d,  word have = ''%s''\n", mid, have);
        #endif

		int n = strcmp(want, have);//compare the word we want and the word we read
		if(n == 0){  //they are equal
			#ifdef DEBUG_MSG
				printf("test: want = have\n");
			#endif	
			return TRUE;
		}else if(n < 0){ 
			#ifdef DEBUG_MSG
				printf("test: want < have\n");
			#endif
			top = mid;
		}else{
			#ifdef DEBUG_MSG
				printf("test: want > have\n");
			#endif
			bot = mid + 1;
		}
		free(have);
	}
	free(want);

	return 0;
}
