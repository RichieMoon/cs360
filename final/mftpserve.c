#include "mftp.h"

int makeconnection(int portnumber);

int main(int argc, char *argv[]){
	int fd, fd2, fd3, fd4, ppid;
	int socketaddress = sizeof(struct sockaddr_in);
	char *hostName;

	struct hostent *hostEntry;
	struct sockaddr_in clientAddr;

	fd = makeconnection(MY_PORT_NUMBER);  
	listen(fd, 4);  // 4 clients only

	//keep waiting for connections
	while(1){ 
		//wait for process to change state to prevent child zombie state
		//return immediately if no child has exited
		waitpid(-1, NULL, WNOHANG); 
		//accept
		fd2 = accept(fd, (struct sockaddr*) &clientAddr, &socketaddress);
		if(fd2 < 0){ 
			fprintf(stderr, "Accept error. Error number: %d, error info: %s\n", errno, strerror(errno));
			exit(1);
		}
		if((ppid = fork())){ 
			close(fd2);
		}else{
			ppid = getppid();
			close(fd);
			break;
		}
	}
	//get a text host name
	if ((hostEntry = gethostbyaddr(&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET)) == NULL)
	{
		fprintf(stderr, "Error on gethostbyaddr(). Error number: %d, error info: %s\n", errno, strerror(errno));
	}else{ //otherwise print the host name and pid
		hostName = hostEntry->h_name;
		printf("Client id: %d, host name: %s\n", ppid, hostName);
	}

	//keep reading command from client
	while(1){ 
		char cmd[1];
		char path[1024] = {'0'};
		read(fd2, cmd, 1); //reading the command
		if (cmd[0] == 'D') //case D
		{ //make data connection(data fd)
			read(fd2, cmd, 1); 
			int port, pcid;
			int socketaddress1 = sizeof(struct sockaddr_in);
			char cmd2[10] = {'0'};

			struct sockaddr_in data;
			struct sockaddr_in client1Addr;
			memset(&data, 0, sizeof(struct sockaddr_in)); //set up the address
			fd3 = makeconnection(0); 
			listen(fd3, 1);
			getsockname(fd3,(struct sockaddr*) &data, &socketaddress1); //obtain an address structure
			port = ntohs(data.sin_port); //storing it into an int so that its byte order is correct
			sprintf(&cmd2[0], "A%d\n", port); //writing A
			write(fd2, cmd2, strlen(&cmd2[0]));

			fd4 = accept(fd3, (struct sockaddr *) &client1Addr, &socketaddress);//accpet fd
			if (fd4 < 0)
			{
				fprintf(stderr, "Error on accept. Error number: %d, error info: %s\n", errno, strerror(errno));
				exit(1);
			}
			if ((pcid = fork()))
			{ 
				wait(NULL);
				close(fd4);
			}else{
				break;
			}
		}else if (cmd[0] == 'C'){ //case C 
			int data;
			int i = 0;
			//get path and save it
			while ((data = read(fd2, cmd, 1)) > 0){
				if(cmd[0] == '\n'){
					path[i] = '\0';break;
				}else{path[i++] = cmd[0];}
			}
			char msg[512] = {'0'};
			char *e; //error msg
    		if (chdir(path) < 0){
        		e = strerror(errno);
        		msg[0] = 'E'; msg[1] = '\0';
        		strcat(&msg[0], e);
        		strcat(&msg[0], "\n");
        		write(fd2, msg, strlen(&msg[0]));
    		}else{
            	write(fd2, "A\n", 2); 
    		}
			printf("Client with id: %d has changed directory to: %s\n", ppid, path);
		}else if (cmd[0] == 'L' || cmd[0] == 'G' || cmd[0] == 'P'){
			write(fd2, "E\n", 2);
		}else if (cmd[0] == 'Q') //case Q
		{ //exit
			write(fd2, "A\n", 2);
			close(fd2);
			printf("Client with id: %d has discconected.\n", ppid);
			exit(1);
		}
	}

	while(1){ 
		char cmd[1];
		char path[1024] = {'0'};
		read(fd2, cmd, 1);
		if (cmd[0] == 'L') //case L
		{ 	
			int pcid2;
			char msg[512] = {'0'};
			write(fd2, "A\n", 2);
			read(fd2, cmd, 1);
			//fork
			if (pcid2 = fork())
			{
				wait(NULL);
				close(fd4);
				exit(1);
			}else{
				close(1);
				dup2(fd4, 1);
				close(fd4);
				close(fd2);
				execlp("ls", "ls", "-l",  NULL); //to run ls -l
			}
		}else if (cmd[0] == 'G') //case G
		{ //getting a file to show or transfer
			int i = 0;
			int fd_file;
			int data;
			char msg[512] = {'0'};
			//get path and save it
			while ((data = read(fd2, cmd, 1)) > 0){ //read from fd2 to cmd
				if (cmd[0] == '\n')
				{
					path[i] = '\0';
					break;
				}else{
					path[i++] = cmd[0];
				}
			}
			fd_file = open(path, O_RDONLY);
			if (fd_file < 0) //not readable
			{ 
				sprintf(&msg[0], "E%s\n", strerror(errno)); //write error message to msg[]
				write(fd2, msg, strlen(msg)); //write msg to fd
				exit(1);
			}
			char buffer[1024] = {'0'};
			if (isregular(path))  //regular file
			{
				write(fd2, "A\n", 2);
				printf("Client with id: %d is getting the file: %s\n", ppid, path);
				while ((data = read(fd_file, buffer, 1)) > 0){ //sending file
					write(fd4, buffer, data);  //write file
				}
				close(fd4);
				close(fd_file);
				exit(1);
			}else{ //not regular
				sprintf(&msg[0], "E%s\n", strerror(errno)); 
				write(fd2, msg, strlen(msg)); 
				close(fd4);
				close(fd_file);
				exit(1);
			}	
		}else if (cmd[0] == 'P')  //getting a file from client
		{
			int i = 0;
			int fd_file;
			int data;
			char msg[512] = {'0'};
			while ((data = read(fd2, cmd, 1)) > 0){ //reading file name save it in path
				if (cmd[0] == '\n')
				{
					path[i] = '\0';
					break;
				}else{
					path[i++] = cmd[0];
				}
			}
			fd_file = open(path, O_CREAT | O_APPEND | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
			if (fd_file < 0)
			{ 
				sprintf(&msg[0], "E%s\n", strerror(errno));
				write(fd2, msg, strlen(msg));
				close(fd4);
				exit(1);
			}
			char buffer[1024] = {'0'};
			write(fd2, "A\n", 2);
			while ((data = read(fd4, buffer, 1)) > 0){ //reading file data from client
				write(fd_file, buffer, data);
			}
			printf("Client with id: %d is sending file: %s\n", ppid, path);
			close(fd2);
			close(fd_file);
			exit(1);
		}else{ //unknow command
			printf("CLient with id: %d has put an unknown command.\n", ppid);
			close(fd4);
			exit(1);
		}
	}	
}

int makeconnection(int port){
    int fd;
    struct sockaddr_in servAddr;

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	
	//make a socket
	//AF_INET is the domain-> internet
	//SOCK_STREAM is the protocol family(TCP)
	fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0){
            fprintf(stderr, "Socket error. Error number: %d, error info: %s\n", errno, strerror(errno));
            exit(1);
    }
    if (bind(fd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
            perror("bind");
            exit(1);
    }
    return fd;
}

//check if the file is regular
int isregular(char *path){
    struct stat area, *s = &area;
    return (lstat(path, s) == 0) && S_ISREG(s->st_mode);
}
