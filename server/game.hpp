#ifndef JUBK_SERVER_GAME
#define JUBK_SERVER_GAME
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <mutex>

#define KGRN "\033[0;32;32m"
#define KCYN "\033[0;36m"
#define KRED "\033[0;32;31m"
#define KYEL "\033[1;33m"
#define KMAG "\033[0;35m"
#define KBLU "\033[0;32;34m"
#define KCYN_L "\033[1;36m"
#define RESET "\033[0m"


int destroy_flag = 0;

class game{
  public:
  int times;
  game(){
    times=0;
    block_buffer_times=-1;
    gmap=NULL;
  }
  ~game(){
    destroy();
  }
  struct player{
    int hp;
    int pow;
    int x;
    int y;
    int have;
    int face;
    int times;
    int fd;
    player(){
      hp=100;
      pow=10;
      x=0;
      y=0;
      have=0;
      face=0;
      times=0;
    }
  };
  std::unordered_map<std::string,player> players;
  void senduser(const std::string &name,const std::string & str){
    auto it=players.find(name);
    if(it==players.end())return;
    auto ps=str.c_str();
    send(it->second.fd,ps,strlen(ps),0);
  }
  void boardcast(const std::string & str){
    auto ps=str.c_str();
    auto len=strlen(ps);
    for(auto it:players){
      send(it.second.fd,ps,len,0);
    }
  }
  struct block{
    std::string player,owner;
    int obj;
  };
  block ** gmap;
  int maxX;
  int maxY;
  
  void destroy(){
    if(gmap){
      for(int ix=0;ix<maxX;ix++)
        delete [] gmap[ix];
      delete [] gmap;
    }
  }
  void init(int x,int y){
    maxX=x;
    maxY=y;
    int ix,iy;
    printf(KGRN "[Game] create map height=%d width=%d \n" RESET,x,y);
    gmap=new block*[maxX];
    for(ix=0;ix<maxX;ix++){
    gmap[ix]=new block[maxY];
      for(iy=0;iy<maxY;iy++){
        gmap[ix][iy].player.clear();
        gmap[ix][iy].owner.clear();
        gmap[ix][iy].obj=0;
      }
    }
  }
  virtual void faceto(const std::string & name,int f){
    if(f<0 || f>=4)return;
    auto it=players.find(name);
    if(it==players.end())return;
    if(it->second.face==f)return;
    it->second.face=f;
    onFaceto(name,f);
  }
  virtual void updateplayer(){
    times++;
    for(auto it:players){
      try{
        it.second.times++;
        if(it.second.times>10){
          if(it.second.hp<0){
            quit(it.first);
            continue;
          }
        }
        if(it.second.hp<=100)
          it.second.hp++;
        it.second.pow+=5;
        onGetUser(
          it.first,
          it.second.x,it.second.y,
          it.second.hp,
          it.second.pow
        );
        printf(KGRN "[Game]player:%s (%d,%d) \n" RESET,
          it.first.c_str(),
          it.second.x,it.second.y
        );
        switch(it.second.face){
          case 0:
          moveplayerto(
            it.first,
            it.second.x+1,it.second.y
          );
          break;
          case 1:
          moveplayerto(
            it.first,
            it.second.x,it.second.y+1
          );
          break;
          case 2:
          moveplayerto(
            it.first,
            it.second.x-1,it.second.y
          );
          break;
          case 3:
          moveplayerto(
            it.first,
            it.second.x,it.second.y-1
          );
          break;
        }
      }catch(std::out_of_range &){
        
      }
    }
  }
  virtual void put(int i,int nx,int ny){
    if(nx>=maxX)return;
    if(ny>=maxY)return;
    if(nx<0)return;
    if(ny<0)return;
    
    block & ob=gmap[nx][ny];
    
    ob.obj=i;
    onPut(i,nx,ny);
  }
  virtual void put(int i,const std::string & name){
    auto it=players.find(name);
    if(it==players.end())return;
    int x=it->second.x;
    int y=it->second.y;
    switch(i){
      case 1:
        if(it->second.pow<20)break;
        it->second.pow-=20;
        put(i,x,y);
      break;
    }
  }
  virtual void moveplayerto(const std::string & name,int nx,int ny){
    if(nx>=maxX)return;
    if(ny>=maxY)return;
    if(nx<0)return;
    if(ny<0)return;
    
    auto it=players.find(name);
    if(it==players.end())return;
    int x=it->second.x;
    int y=it->second.y;
    
    if(x>=maxX)return;
    if(y>=maxY)return;
    
    if(x>0 && y>0){
      block & ob=gmap[x][y];
      ob.player.clear();
    }
    
    block & b=gmap[nx][ny];
    if(b.obj!=0)
      collideobj(nx,ny,name,b.obj);
    if(b.player.empty()){
      it->second.x=nx;
      it->second.y=ny;
      
      it->second.have++;
      if(!b.owner.empty())
        players[b.owner].have--;
      
      b.owner =name;
      b.player=name;
    }else{
      auto it=players.find(name);
      if(it==players.end()){
        it->second.x=nx;
        it->second.y=ny;
      
        it->second.have++;
        if(!b.owner.empty())
          players[b.owner].have--;
      
        b.owner =name;
        b.player=name;
      }else
        collideplayer(nx,ny,name);
    }
    onMove(nx,ny,name);
  }
  virtual void quit(const std::string &name){
    auto it=players.find(name);
    if(it==players.end())return;
    try{
      auto it=players.find(name);
      if(it==players.end())return;
      int x=it->second.x;
      int y=it->second.y;
    
      block & ob=gmap[x][y];
      
      ob.player.clear();
    }catch(std::out_of_range &){
        
    }
    players.erase(it);
    onQuit(name);
  }
  virtual void login(const std::string &name,int fd){
    player & p=players[name];
    int x=rand()%(this->maxX);
    int y=rand()%(this->maxY);
    p.hp=100;
    p.pow=0;
    p.fd=fd;
    moveplayerto(name,x,y);
    onLogin(name);
  }
  
  virtual void onLogin(const std::string &name){
    char buf[256];
    
    snprintf(buf,256,"setname %s \n",name.c_str());
    std::string bs=buf;
    senduser(name,bs);
    
    snprintf(buf,256,"addplayer %s \n",name.c_str());
    bs=buf;
    boardcast(bs);
  }
  virtual void onGetUser(const std::string &name,int a,int b,int c,int d){
    char buf[256];
    snprintf(buf,256,"setme %d %d %d %d \n",a,b,c,d);
    std::string bs=buf;
    senduser(name,bs);
  }
  virtual void onFaceto(const std::string &name,int t){
    char buf[256];
    snprintf(buf,256,"face %s %d \n",name.c_str(),t);
    std::string bs=buf;
    boardcast(bs);
  }
  
  virtual void onPut(int i,int x,int y){
    char buf[256];
    if(i==0)
      snprintf(buf,256,"pick %d %d \n",x,y);
    else
      snprintf(buf,256,"setobj %d %d %d \n",x,y,i);
    std::string bs=buf;
    boardcast(bs);
  }
  
  virtual void onMove(int nx,int ny,const std::string &name){
    char buf[256];
    snprintf(buf,256,"move %s %d %d \n",name.c_str(),nx,ny);
    std::string bs=buf;
    boardcast(bs);
  }
  
  virtual void onQuit(const std::string &name){
    char buf[256];
    snprintf(buf,256,"quit %s \n",name.c_str());
    std::string bs=buf;
    boardcast(bs);
    
    bs="exit \n";
    senduser(name,bs);
  }
  
  virtual void collideplayer(int nx,int ny,const std::string &name){}
  virtual void collideobj(int nx,int ny,const std::string &name,int i){
    auto it=players.find(name);
    if(it==players.end())return;
    try{
      switch(i){
        case 1:
        if(it->second.times>10)
          it->second.hp-=20;
        break;
      }
      put(0,nx,ny);
    }catch(std::out_of_range &){
      
    }
  }
  virtual void onMsg(const std::string &name,const std::string & str){
    std::istringstream iss(str);
    std::string mode;
    iss>>mode;
    int f;
    if(mode=="quit"){
      quit(name);
    }else
    if(mode=="walk"){
      iss>>f;
      faceto(name,f);
    }else
    if(mode=="put"){
      iss>>f;
      put(f,name);
    }
  }
  std::list<std::string> block_buffer;
  int          block_buffer_times;
  std::mutex        block_buffer_locker;
  virtual void getblock(){
    
    if(block_buffer_times==times)return;
    block_buffer_times=times;
    block_buffer.clear();
    
    char buf[256];
    
    block_buffer_locker.lock();
    for(int x=0;x<maxX;x++){
      for(int y=0;y<maxY;y++){
        try{
          block & b=gmap[x][y];
          if(b.obj!=0){
            bzero(buf,256);
            snprintf(buf,256,"setobj %d %d %d \n",x,y,b.obj);
            block_buffer.push_back(buf);
          }
          if(!b.owner.empty()){
            bzero(buf,256);
            snprintf(buf,256,"setown %d %d %s \n",x,y,b.owner.c_str());
            block_buffer.push_back(buf);
          }
        }catch(std::out_of_range &){
        }
      }
    }
    block_buffer_locker.unlock();
  }
}Game;

std::mutex Game_locker;

struct per_session_data {
  char name[16];
  void randname(){
    const static char chs[]=
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    begin:
    for(int i=0;i<15;i++){
      name[i]=chs[(rand()%(sizeof(chs)-1))];
    }
    name[15]='\0';
    
    Game_locker.lock();
    auto it=Game.players.find(name);
    if(it!=Game.players.end()){
      Game_locker.unlock();
      goto begin;
    }
    Game_locker.unlock();
  }
  void init(int fd){
    randname();
    std::string n=name;
    char buf[256];
    
    Game_locker.lock();
    Game.login(name,fd);
    Game.getblock();
    snprintf(buf,256,"cremap %d %d \n",Game.maxX,Game.maxY);
    send(fd,buf,strlen(buf),0);
    for(auto uit:Game.players){
      std::string bs="addplayer ";
      bs+=uit.first;
      bs+=" \n";
      send(fd,bs.c_str(),bs.size(),0);
    }
    Game_locker.unlock();
    
    Game.block_buffer_locker.lock();
    for(auto it:Game.block_buffer){
      snprintf(buf,256,"%s",it.c_str());
      send(fd,buf,strlen(buf),0);
    }
    Game.block_buffer_locker.unlock();
  }
  void onMessage(void * data,int len){
    name[15]='\0';
    char buf[128];
    int l=len>127 ? 127 : len;
    auto str=(char*)data;
    
    for(int i=0;i<l;i++){
      buf[i]=str[i];
    }
    
    buf[127]='\0';
    
    std::string msg=buf,
    n=name;
    
    Game_locker.lock();
    Game.onMsg(n,msg);
    Game_locker.unlock();
  }
  void quit(){
    Game_locker.lock();
    Game.quit(name);
    Game_locker.unlock();
  }
};

class Init{
  public:
  pthread_t mlthread;
  Init(){
    Game.init(128,128);
    srand(time(0));
    if(pthread_create(&mlthread,NULL,mainloop,NULL)!=0)
      perror(KRED "[Main Loop] pthread_create" RESET);
  }
  static void * mainloop(void*){
    printf(KGRN "[Main Loop] Game start success \n" RESET);
    while( !destroy_flag){
    
      Game_locker.lock();
      Game.updateplayer();
      Game_locker.unlock();
      
      sleep(1);
    }
  }
}init;
#endif
