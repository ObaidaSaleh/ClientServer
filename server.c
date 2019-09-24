#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
  // init. main socket and retrieve commandline arguments
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in msoc;
	memset(&msoc, 0, sizeof(struct sockaddr_in));
	msoc.sin_family = AF_INET;
	msoc.sin_port = htons(atoi(argv[1]));
	msoc.sin_addr.s_addr = htonl(INADDR_ANY);
 
  // execute bind to port and being to listen for users
	if (-1 == bind(sfd, (struct sockaddr *)&msoc, sizeof(struct sockaddr_in))) {
		perror("bind");
		return 1;
	}
	if (-1 == listen(sfd, 5)) {
		perror("listen");
		return 1;
	}
 
  // being the epoll event list
  fcntl(sfd, F_SETFL, O_NONBLOCK);
	int ep = epoll_create1(0);
	struct epoll_event e;
	e.events = EPOLLIN;
	e.data.fd = sfd;
	epoll_ctl(ep, EPOLL_CTL_ADD, sfd, &e);
	for(;;){
		epoll_wait(ep, &e, 1, -1);
		if (e.data.fd == sfd) {
		  // able to accept new client
		  struct sockaddr_in a;
		  socklen_t sinlen = sizeof(struct sockaddr_in);
		  int cfd = accept(sfd, (struct sockaddr *)&a, &sinlen);
		  if (cfd != -1) {
			e.events = EPOLLIN;
			e.data.fd = cfd;
			epoll_ctl(ep, EPOLL_CTL_ADD, cfd, &e);
		  }
		}
     // perform client functions
		else{
			int cfd = e.data.fd;
      char buf[100];
			int name;
      int ch = 1;
      int con_ch = htonl(ch);
			memset(buf, 0, sizeof(buf));
			name = recv(cfd, buf, 100, 0);
			if (name <= 0) {
				epoll_ctl(ep, EPOLL_CTL_DEL, cfd, NULL);
				close(cfd);
			} 
			else if (name > 0) {
				if (access(buf, R_OK) == -1){
          ch = -1;
          con_ch = htonl(ch);
          write(cfd, &con_ch, sizeof(con_ch));
          
					perror("file DNE");
          
					close(cfd);
				}
				else{
          write(cfd, &con_ch, sizeof(con_ch));
          
          // find the size of the file and sent it to client so they can allocate the size of the destination on their end
          FILE *f = fopen(buf, "r");
        
          fseek(f, 0, SEEK_END); // seek to end of file
          int size = ftell(f); // get current file pointer
          int c_size = htonl(size); // converted
          fseek(f, 0, SEEK_SET); // seek back to beginning of file   
          
          fclose(f);
          
          write(cfd, &c_size, sizeof(c_size));
          
          int fd = open(buf, 'r');
           
					unsigned char fbuf[size];
                   
          // read the whole file and becareful in case of seperate packets being sent instead of a single large packet
					for(;;){
						int r = read(fd, fbuf, sizeof(fbuf));
						if (r == 0)
            {
							break;
						}
						if (r < 0) 
            {
							perror("read error");
							return 1;
						}
              
						void *left = fbuf;
                       
						while (r > 0) {
						int r_written = write(cfd, left, r);
			 			if (r_written <= 0) {
								perror("w error");
								return 1;
			 			}
							r -= r_written;
							left += r_written;
             
						}
					}
           // close all
					close(fd);
					close(cfd);
				}
			}
		}
	}
	close(sfd);
	return 0;
}
