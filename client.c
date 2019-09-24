#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(int argc, char* argv[])
{
  // init. clientfd as a socket and mainsoc
	int cfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in msoc;
	
  // init. info for the main soc
	memset(&msoc, 0, sizeof(struct sockaddr_in));
	msoc.sin_family = AF_INET;
	msoc.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &msoc.sin_addr);
	
  // connect mainsoc to server
	if (-1 == connect(cfd, (struct sockaddr *)&msoc, sizeof(struct sockaddr_in))) 
  {
		perror("connect error");
		return 1;
	}
 
  // send the file name to search for 
	send(cfd, argv[3], strlen(argv[3]), 0);
 
  int ch = 0;
  int ret1 = read(cfd, &ch, sizeof(ch));
  int c_ch = ntohl(ch);

  if (ret1 < 0) {
  perror("ret < 0");
  return 1;
  }
  
  if (c_ch == -1)
  {
   perror("file DNE");
   return 1;
   }
 
  int size = 0;
  int ret = read(cfd, &size, sizeof(size));
  
  if (ret < 0){
  perror("ret < 0");
  return 1;
  }
	
  int c_size = ntohl(size);
	int i = 0;
	unsigned char buf[c_size]; // 1048576 b_written
 	memset(buf, '\0', sizeof(buf));
  
  // read the whole file
	for(;;)
  {
		int r = read(cfd, buf, sizeof(buf));
    if (r < 0) 
    {
			perror("read error");
			return 1;
		}
		if (r == 0)
    {
			break;
		}
		i++;
	}
  // check if file was 0 
	if (i == 0)
  {
		perror("file not found on server or file size 0");
		close(cfd);
		return 1;
	}
 
 // open the file and begin to write to it
	FILE* file = fopen(argv[4], "w+");
	if (file == NULL)
  {
		perror("error opening the file");
		close(cfd);
		return 1;
	}

	fwrite(buf, 1, sizeof(buf), file);
 
	close(cfd);
	return 0;
}
