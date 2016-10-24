#include <bits/stdc++.h>
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#define BACKLOG 10
using namespace std;

int PORT = 3490;// the port users will be connecting to
// how many pending connections queue will hold
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}


string t_string(int a){
    std::stringstream ss;
    ss << a;
    return ss.str();
}

vector<string> split_str(string s){
    stringstream ss(s);
	std::vector<string> res;
	string c;
	while(ss >> c)
		res.push_back(c);
	return res;
}

void receive_file(string file_name, int client_socket){
    ssize_t len;
    char buffer[BUFSIZ];
    int file_size;
    FILE *received_file;
    int remain_data = 0;

	/* Receiving file size */
    recv(client_socket, buffer, BUFSIZ, 0);//////////////////////check for error
    file_size = atoi(buffer);
    //fprintf(stdout, "\nFile size : %d\n", file_size);

    received_file = fopen(file_name.c_str(), "w");
    if (received_file == NULL)
    {
            fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));

            exit(EXIT_FAILURE);
    }

    remain_data = file_size;
    cout<<"Start recieving file"<<endl;
    while (((len = recv(client_socket, buffer, BUFSIZ, 0)) > 0) && (remain_data > 0))
    {
            fwrite(buffer, sizeof(char), len, received_file);
            remain_data -= len;
            fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", len, remain_data);

            if(remain_data == 0)
            	break;
    }
    cout<<"File Recived Done"<<endl;
    fclose(received_file);
}

void send_file(string file_name, int peer_socket){

	struct stat file_stat;
    cout<<file_name<<endl;
	int fd = open(file_name.c_str(), O_RDONLY);

    if (fd == -1)
    {	
    	//send 404
        cout<<"ERROR 404"<<endl;
        string e = "Error 404";
        send(peer_socket,e.c_str(), 9,0);
        return;
    }
    /* Get file stats */
    if (fstat(fd, &file_stat) < 0)
    {
    	// send 404
    	return;
    }
    fprintf(stdout, "File Size: \n%d bytes\n", file_stat.st_size);
    char file_size[256];

    sprintf(file_size, "%d", file_stat.st_size);
    cout<<"file size:"<<file_size<<endl;
    /* Sending file size */
    int len = send(peer_socket, file_size, sizeof(file_size), 0);
    if (len < 0)
    {
          fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));

          exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server sent %d bytes for the size\n", len);


    off_t offset = 0;
    int remain_data = file_stat.st_size;
    /* Sending file data */
    int sent_bytes = -1;
    sleep(1);
    while (((sent_bytes = sendfile(peer_socket, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
    {
            fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
            remain_data -= sent_bytes;
            fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
    }

    cout << "Server send file done\n";
}

void *handle_client(void *param) {
	int *socket = (int *) param;
	printf("%d\n", *socket);
	int numbytes = -1;
	char received_text[100];
	while (1) {
        if ((numbytes = recv(*socket, received_text, 100 - 1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		received_text[numbytes] = '\0';
		if (strcmp(received_text, "bye") == 0) {
			printf("socket %d closed\n", *socket);
			break;
		}
        bool op_type = split_str(string(received_text))[0] == "GET";
        string file_name = split_str(string(received_text))[1];
        if(op_type){
            // GET
            send_file(file_name, *socket);
        }
        else{
            //POST
            string response = "ok";
            send(*socket, response.c_str(), 2 , 0);
            receive_file(file_name, *socket);
        }
    }
    close(*socket);
    pthread_exit(NULL);
}
//	close(*socket);
//	pthread_exit(NULL);
//}



int main(int argc, char *argv[]) {
  	if(argc > 1) {
   		PORT = atoi(argv[1]);
  	}

	int sockets[BACKLOG + 1];
	pthread_t threads[BACKLOG];
	int next_thread = 0;

	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];
	int rv;

	// Initialize server configuration.
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(NULL, t_string(PORT).c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	rv = 1;
	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
			servinfo->ai_protocol);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &rv, sizeof(rv));
	if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		printf("%s\n", strerror(errno));
		exit(1);
	}
	freeaddrinfo(servinfo); // all done with this structure
	listen(sockfd, BACKLOG);
	cout << "Start Listnening on port " << PORT << "..." << endl;
	printf("Waiting for connections...\n");

	// main accept() loop
	while (1) {
		sin_size = sizeof their_addr;
		sockets[next_thread] = accept(sockfd, (struct sockaddr *) &their_addr,
				&sin_size);
		printf("%d\n", new_fd);
		struct sockaddr_in *sin = (struct sockaddr_in*) &their_addr;
		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
		char port_string[7];
		sprintf(port_string, "%i", sin->sin_port);
		printf("server: got connection from %s:%s\n", s, port_string);
		//create thread here
		if (next_thread < BACKLOG) {
			int status = pthread_create(&threads[next_thread], NULL,
					&handle_client, &sockets[next_thread]);
			if (status != 0) {
				fprintf(stderr, "Error creating thread\n");
				return -1;
			}
			next_thread++;
		// close(new_fd);
		} else {
			next_thread = 0;
			if (pthread_join(threads[next_thread], NULL)) {
				fprintf(stderr, "Error joining thread\n");
				return -2;
			}
			sockets[next_thread] = sockets[BACKLOG];
			int status = pthread_create(&threads[next_thread], NULL,
					&handle_client, &sockets[next_thread]);
			if (status != 0) {
				fprintf(stderr, "Error creating thread\n");
				return -1;
			}
		}
	}
	return 0;
}
