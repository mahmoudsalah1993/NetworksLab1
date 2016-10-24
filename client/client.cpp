#include <bits/stdc++.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
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
#define MAXDATASIZE 100 // max number of bytes we can get at once
using namespace std;

int PORT = 3490;// the port users will be connecting to
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
	{
		cout << c << endl;
		res.push_back(c);
	}
	return res;
}

void receive_file(string file_name, int client_socket){
    ssize_t len;
    char buffer[BUFSIZ];
    int file_size;
    FILE *received_file;
    int remain_data = 0;

	/* Receiving file size */
    recv(client_socket, buffer, BUFSIZ, 0);
    //////////////////////check for error
    if (strcmp(buffer, "Error 404") == 0) {
		printf("Error 404: File not found\n");
		return;
	}
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

    fprintf(stdout, "Client sent %d bytes for the size\n", len);


    off_t offset = 0;
    int remain_data = file_stat.st_size;
    /* Sending file data */
    int sent_bytes = -1;
    sleep(1);
    while (((sent_bytes = sendfile(peer_socket, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
    {
            fprintf(stdout, "1. Client sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
            remain_data -= sent_bytes;
            fprintf(stdout, "2. Client sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
    }

    cout << "Client send file done\n";
}

int main(int argc, char *argv[]) {
	int sockfd, numbytes;
	//char buf[MAXDATASIZE];
	string buf;

	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	if (argc < 2) {
		fprintf(stderr, "usage: client hostname\n");
		exit(1);
	}
	if(argc = 3) {
   		PORT = atoi(argv[2]);
  	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(argv[1], t_string(PORT).c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
			servinfo->ai_protocol);
	unsigned char optval = 0x30;
	int iret1 = setsockopt(sockfd, IPPROTO_IP, IP_TOS, &optval, sizeof(optval));
	int iret = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	inet_ntop(servinfo->ai_family,
			get_in_addr((struct sockaddr *) servinfo->ai_addr), s, sizeof s);
	//printf("iret equal %d\n", iret);
	if (iret == 0)
	{
		printf("client: connecting to %s\n", s);
	}
	freeaddrinfo(servinfo); // all done with this structure
	int num_requests;
	cout << "Enter  number of requests:" << endl;
	cin >> num_requests;
	scanf("\n");
	while (num_requests--) {
		getline(std::cin, buf);

		int length = buf.length();

        char *ss="file.txt";

        int status = send(sockfd, buf.c_str(), buf.length(), 0);
		printf("Sent with status = %d\n", status);

		// int iret2 = setsockopt(sockfd, IPPROTO_IP, IP_TOS, &optval,
		// 		sizeof(optval));
		//status = send(sockfd, buf, length, 0);
		/*
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
			perror("recv");
			exit(1);
		} else{
		*///recived from server
		bool op_type = (split_str(buf)[0]=="GET");
		if(split_str(buf)[0]=="GET"){
			// GET
			receive_file(split_str(buf)[1], sockfd);
		}
		else{
			// POST
			char received_text[100];
			int numbytes = recv(sockfd, received_text, 100 - 1, 0);
			send_file(split_str(buf)[1], sockfd);
		}
		printf("DONE \n");
	}
	sleep(1);
	string bye = "bye";
	send(sockfd, bye.c_str(), 3, 0);
	close(sockfd);
	return 0;
}
