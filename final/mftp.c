
#include "mftp.h"

int makeconnection(char *hostname, int portnumber);

int main(int argc, char *argv[]){
    int fd1, fd2,fd3;
    char *token = (char *)calloc(1024, sizeof(char));
    char *cmd = (char *)calloc(1024, sizeof(char));
    char *sizeoftoken = (char *)calloc(1024, sizeof(char)); 
    char *delimiter = " \n\t";  //end line delimiter
    if (token == NULL || cmd == NULL || sizeoftoken == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory. error number: %d. error info: %s\n", errno, strerror(errno));
        exit(0);
    }

    if (argc != 2)
    {
        fprintf(stderr, "Wrong input. Error number: %d, error info: %s\n", errno, strerror(errno));
        exit(1);
    }
	fd1 = makeconnection(argv[1], MY_PORT_NUMBER);  //connecting to server port
	if (fd1 == -1)
    {
        exit(1);
    }

	while(1){
		printf("mftp$ "); 
		fgets(cmd, sizeof(char) * 1024, stdin);  //getting the command
		memcpy(sizeoftoken, cmd, sizeof(char) * 1024);
		token = strtok(sizeoftoken, delimiter); //parsing the command

        //the first token may be preceded by spaces
        //using isspace() to ignore it
        char *temp;
		temp = cmd;
		while(isspace(*temp) && *temp != '\n'){
            ++temp;
        } 
		if(*temp == '\n'){continue;}

        //cases
		if (!strcmp(token, "exit"))
        { //case exit
            char buffer[256];
            write(fd1, "Q\n", 2);  //send Q to server
            read(fd1, buffer, 256); //read by from server
            close(fd1);
            //free memory
            free(sizeoftoken);
            free(cmd);
            free(token);
            fprintf(stderr, "Client Exiting...\n");
            exit(1);
        }else if (!strcmp(token, "cd")) //case cd
        {
            token = strtok(NULL, delimiter);
          	if (chdir(token) < 0) //change dir
            {
    	   	   fprintf(stderr, "Error on change directory. Error number:%d, error info: %s\n", errno, strerror(errno));
		    }
		}else if (!strcmp(token, "rcd")) //case rcd
        {  //change server dir
			char msg[256] = {'0'};
			char *cmd2 = (char *) calloc(1024, sizeof(char));
            token = strtok(NULL, delimiter);
            if (token == NULL){
               fprintf(stderr, "Error on rcd, no path. Error number: %d, error info: %s\n", errno, strerror(errno));
               continue;
             }
    		cmd2[0] = 'C'; 
    		strcat(cmd2, token);
    		strcat(cmd2, "\n");
    		write(fd1, cmd2, strlen(cmd2));  //send C and token to server
    		read(fd1, msg, 256); //read back
    		if (msg[0] == 'E')
            {
            	printf("%s", &msg[1]);
    		}
    		free(cmd2);
		}else if (!strcmp(token, "ls")) //case ls
        {  
			system("ls -l | more -20"); //run ls -l | more -20
		}else if (!strcmp(token, "rls")) //case rls
        {  
            int pid;
            char msg[256] = {'0'};

			int id = portnumber(fd1); //get port number
			int socket = makeconnection(argv[1], id); //make a new socket
			if (id < 0 || socket < 0)
            {	
				fprintf(stderr, "Error on rls. Error number: %d, error info: %s\n", errno, strerror(errno));
				continue;
			}
			write(fd1, "L\n", 2); //send L to server
    		read(fd1, msg, 256);  //read back 
			if (msg[0] != 'E')
            {
				if ((pid = fork()))
                { //forking 
                	wait(NULL);
                }else{
                    close(0);
                    dup2(socket, 0); //for run ls -l | more -20 
                    close(socket);
                    execlp("more", "more", "-20", NULL); //copy of server file descriptor
                }
			}else{
                printf("%s", &msg[1]);
            }
			close(socket);
		}else if (!strcmp(token, "get"))  //case get
        { //getting a file from server
            int port_number, socket,data;
			char msg[256] = {'0'};
			char *buffer = (char *)calloc(10, sizeof(char));

            token = strtok(NULL, delimiter);
            if (token == NULL)
            {
                fprintf(stderr, "Error on get, no path. Error number: %d, error info: %s\n", errno, strerror(errno));
                continue;
            }

            char *path1 = strtok(token, "/"); //get the token
            char *path2 = path1; 
            while ((path1 = strtok(NULL, "/")) != NULL){  //parsing the path for "/" 
            	path2 = path1;
            }

			port_number = portnumber(fd1);  
			socket = makeconnection(argv[1], port_number);
            if (port_number < 0 || socket < 0)
            {
                    fprintf(stderr, "Error on get. Error number: %d, error info: %s\n", errno, strerror(errno));
                    continue;
            }
    		write(fd1, "G", 1);
    		write(fd1, token, strlen(token));
   	 		write(fd1, "\n", 1);
    		read(fd1, msg, 256);
			if (msg[0] != 'E')
            {
                //check the file if exits or not regular
				fd3 = open(path2, O_CREAT | O_APPEND | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            	if (fd3 < 0)
                {  //open file to write on client directory
                    	fprintf(stderr, "Error on get, cannot open the file. Error number: %d, error info: %s\n", errno, strerror(errno));
                    	continue;
            	}
				while((data = read(socket, buffer, 1)) > 0){ //read the file
                	write(fd3, buffer, data); //write whatever read to fd3
                }
			}else{
                printf("%s", &msg[1]);
            }	
            close(socket);
			close(fd3);
			free(buffer);
		}else if (!strcmp(token, "show")) //case show
        { //get path and show the file
			int port_number, socket, pid;
			char *cmd2 = (char*)calloc(1024, sizeof(char));
            char msg[256] = {'0'};

			token = strtok(NULL, delimiter);
			if (token == NULL)
            {
				fprintf(stderr, "Error on show, no path. Error number: %d, error info: %s\n", errno, strerror(errno));
				continue;
			}

			port_number = portnumber(fd1);
			socket = makeconnection(argv[1], port_number);
			if (port_number < 0 || socket < 0)
            {
				fprintf(stderr, "Error on show. Error number: %d, error info: %s\n", errno, strerror(errno));
				continue;
			}

    		cmd2[0] = 'G';
    		strcat(cmd2, token); 
    		strcat(cmd2, "\n");  
    		write(fd1, cmd2, strlen(cmd2));  //sending path to server
    		read(fd1, msg, 256);
			if (msg[0] != 'E')
            {
                if ((pid = fork()))
                { 
                        wait(NULL);
                }else{
                        close(0);
                        dup2(socket, 0);
                        close(socket);
                        execlp("more", "more", "-20", NULL); //pipe more -20
                }
            }else{
                printf("%s", &msg[1]);
            }
			close(socket);
			free(cmd2);
		}else if (!strcmp(token, "put")) //case put
        {  //sending file to server
			int port_number, socket, fdfile, data;
			
			token = strtok(NULL, delimiter); //split by delimiter
			if (token == NULL)
            {
				fprintf(stderr, "Error on put, no path. Error number: %d, error info: %s\n", errno, strerror(errno));
				continue;
			}
            
			char *path1 = strtok(token, "/"); //split the / 
			char *path2 = path1;
			while((path1 = strtok(NULL, "/")) != NULL){  //parsing the path
				path2 = path1; //save it to path2
			}
			fdfile = open(token, O_RDONLY);	 //open file 
			if (fdfile < 0)
            { //open fails
				fprintf(stderr, "Error on put, on opening file. Error number: %d, error info: %s\n", errno, strerror(errno));
				continue;
			}
			if (isregular(token))
            {  //if file is regular
				port_number = portnumber(fd1);
				socket = makeconnection(argv[1], port_number);
				if (port_number < 0 || socket < 0)
                {
					fprintf(stderr, "Error on put. Error number: %d, error info: %s\n", errno, strerror(errno));					continue;
				}
                char msg[256] = {'0'};
                char *buffer = (char *)calloc(10, sizeof(char));
    			write(fd1, "P", 1);
    			write(fd1, path2, strlen(path2));
    			write(fd1, "\n", 1);
    			read(fd1, msg, 256);
				if (msg[0] != 'E'){
                    while((data = read(fdfile, buffer, 1)) > 0){ //read the file
                        write(socket, buffer, data); //send to server
                    }
				}else{
                    printf("%s", &msg[1]);
                }
    			free(buffer);		
			    close(fdfile);
			    close(socket);
			}else{  //file is not regular
				close(fdfile);
				fprintf(stderr, "Error: File not regular.\n");
			}
		}else{  //unknow command
			fprintf(stderr, "Unknown command.\n");	
		}
	}
}

int makeconnection(char *hostname, int port){
    
    struct sockaddr_in servAddr;
    struct hostent *hostEntry;
    struct in_addr **pptr; 

    int fd = socket(AF_INET, SOCK_STREAM, 0);  //socket connection
    if (fd < 0){
        fprintf(stderr, "Error on making socket. Error number: %d, error info: %s\n", errno, strerror(errno));
        return -1;
    }
    //copying the server address
    memset(&servAddr, 0, sizeof(servAddr));  
    servAddr.sin_port = htons(port);
    servAddr.sin_family = AF_INET;

    hostEntry = gethostbyname(hostname);   //getting the hostname
    if (hostEntry == NULL){
        fprintf(stderr, "Error on gethostbyname(). Error number: %d error info: %s\n", errno, strerror(errno));
        return -1;
    }
    //copy and over write that host name 
    pptr = (struct in_addr **) hostEntry->h_addr_list;
    memcpy(&servAddr.sin_addr, *pptr, sizeof(struct in_addr));
    //connect
    if ((connect(fd, (struct sockaddr *) &servAddr, sizeof(servAddr))) < 0){ 
        fprintf(stderr, "Error on connecting. Error number: %d error info:  %s\n", errno, strerror(errno));
        return -1;
    }  
    return fd;
}
//get port number
int portnumber(int fd){
    int port_number;
    char *cmd = "D\n";
    char msg[256] = {'0'};
    write(fd, cmd, 2);  
    read(fd, msg, 256); //reading the port number from server
    if (msg[0] != 'E'){
        sscanf(msg, "A%d\n", &port_number); //save port number from msg to port_number
    }else{
        printf("%s\n", &msg[1]); //otherwise its a error msg
        return -1;
    }
    return port_number;
}
//check if the file is regular
int isregular(char *path){
    struct stat area, *s = &area;
    return (lstat(path, s) == 0) && S_ISREG(s->st_mode);
}
