#include <map>
#include <set>
#include <list>
#include <atomic>
#include <sys/epoll.h>
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
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <mutex>
#include "game.hpp"
  class msg_base{
    public:
    static void setnonblocking(int sock){
      int opts;
      opts = fcntl(sock, F_GETFL);
      if(opts < 0) {
        perror("fcntl(sock, GETFL)");
        return;
      }
      opts = opts | O_NONBLOCK;
      if(fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(sock, SETFL, opts)");
        return;
      }
    }
    static int startup(u_short *port){
      int rpcd = 0;
      struct sockaddr_in name;
      rpcd = socket(PF_INET, SOCK_STREAM, 0);
      if (rpcd == -1)
        return -1;
      int reuse = 1;
      if (setsockopt(rpcd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        return -1;
      }
      memset(&name, 0, sizeof(name));
      name.sin_family = AF_INET;
      name.sin_port = htons(*port);
      name.sin_addr.s_addr = htonl(INADDR_ANY);
      if (bind(rpcd, (struct sockaddr *)&name, sizeof(name)) < 0)
         return -1;
      if (*port == 0){
        int namelen = sizeof(name);
        if (getsockname(rpcd, (struct sockaddr *)&name, (socklen_t *)&namelen) == -1)
          return -1;
        *port = ntohs(name.sin_port);
      }
      if (listen(rpcd, 5) < 0)
        return -1;
      return (rpcd);
    }
    std::atomic<bool> running;
    int epfd;
    inline void stop(){
      running=false;
    }
    void run(u_short p,int maxnum=256){
      running=true;
      int listenfd = -1;
      u_short port = p;
      int connfd = -1;

      struct sockaddr_in client;
      int client_len = sizeof(client);
      
      signal(SIGPIPE,[](int){});
      //绑定监听端口
      listenfd = startup(&port);
      
      
      struct epoll_event ev, events[20];
      epfd = epoll_create(maxnum);
      setnonblocking(listenfd);
      ev.data.fd = listenfd;
      ev.events = EPOLLIN;
      epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
      
      
      while (running){
        
        int nfds = epoll_wait(epfd, events, 20, 4000);
        loop();
        for(int i = 0; i < nfds; ++i) {
          if(events[i].data.fd == listenfd) {
            
            connfd = accept(listenfd, (struct sockaddr *)&client, (socklen_t *)&client_len);
            if(connfd < 0) {
              continue;
            }
            
            //printf("conn\n");
            onConnect(connfd);
            
            setnonblocking(connfd);
            ev.data.fd = connfd;
            ev.events = EPOLLIN | EPOLLHUP;
            epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            
          }else if(events[i].events & EPOLLIN) {
            if((connfd = events[i].data.fd) < 0) continue;
            //处理请求
            char cbuf[256];
            int len;
            if((len=read(connfd, cbuf, 256)) <= 0) {
              
              onQuit(connfd);
              ev.data.fd = connfd;
              ev.events = 0;
              epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, &ev);
              
            }else{
              
              //printf("msg\n");
              onMessage(connfd,cbuf,len);
              
              ev.data.fd = connfd;
              ev.events = EPOLLIN | EPOLLHUP;
              epoll_ctl(epfd, EPOLL_CTL_MOD, connfd, &ev);
              
            }
          }else if(events[i].events & EPOLLHUP) {
            if((connfd = events[i].data.fd) < 0) continue;
            
            //printf("hup\n");
            onQuit(connfd);
            
          }else if(events[i].events & EPOLLOUT) {
            if((connfd = events[i].data.fd) < 0) continue;
            onWriAble(connfd);
          }
        }
      }
      close(listenfd);
      close(epfd);
      destruct();
      return;
    }
    virtual void onMessage(int,char*,int){}
    virtual void onConnect(int){}
    virtual void onQuit(int){}
    virtual void onWriAble(int){}
    virtual void destruct(){}
    virtual void loop(){}
  };
  class msg:public msg_base{
    public:
    struct conn{
      per_session_data session;
      char     buf[256];
      uint16_t ptr;
      int      fd;
      void reset(){
        bzero(buf,256);
        ptr=0;
      }
      void append(char c){
        char ic;
        
        if(ptr>=255){
          buf[255]='\0';
          session.onMessage(buf,255);
          reset();
        }
        
        if(c=='\0' || c=='\n')
          ic='\0';
        else
          ic=c;
        buf[ptr]=ic;
        ++ptr;
        if(ic=='\0'){
          session.onMessage(buf,ptr);
          reset();
        }
      }
      void quit(){
        session.quit();
      }
      void init(int ifd){
        reset();
        fd=ifd;
        session.init(fd);
      }
    };
    std::map<int,conn> conns;
    virtual void onConnect(int ifd){
      conns[ifd].init(ifd);
    }
    virtual void onQuit(int connfd){
      auto it=conns.find(connfd);
      if(it==conns.end())return;
      conn & cp=it->second;
      cp.quit();
    }
    virtual void onMessage(int connfd,char * fst,int len){
      auto it=conns.find(connfd);
      if(it==conns.end())return;
      conn & cp=it->second;
      
      for(int j=0;j<len;j++)
        cp.append(fst[j]);
      
      while(1){
        char buf[256];
        int slen=read(connfd,buf,256);
        if(slen<=0)return;
        
        for(int j=0;j<slen;j++)
          cp.append(buf[j]);
        
        if(slen<256)return;
      }
      
    }
  }M;
int main(){
  signal(SIGPIPE,[](int){});
  signal(2,[](int){
    printf("\n" KGRN "[Main] Quit \n" RESET);
    destroy_flag=1;
    sleep(1);
    M.stop();
  });
  M.run(5000);
}