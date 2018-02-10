#include <memory.h>
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/file.h>
int main(){
  char addr[]="127.0.0.1";
  short port=5000;
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(addr);
  address.sin_port = htons(port);
  
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  connect(sockfd, (struct sockaddr *)&address, sizeof(address));
  
  char buf;
  while(1){
    read(sockfd,&buf,1);
    printf("%c",buf);
  }
  
  close(sockfd);
}
