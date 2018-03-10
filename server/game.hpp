#ifndef JUBK_SERVER_GAME
#define JUBK_SERVER_GAME
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <stdexcept>
#include <mutex>
#include <atomic>

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
  class player{
    public:
    bool start;
    int hp;
    int pow;
    int x;
    int y;
    int have;
    int face;
    int times;
    int fd;
    int r,g,b;
    int update_time;
    player(){
      start=false;
      x=0;
      y=0;
      have=0;
      face=0;
      times=0;
    }
  };
  std::map<int,player> players;
  void senduser(int name,const std::string & str){
    auto it=players.find(name);
    if(it==players.end())return;
    auto ps=str;
    send(it->second.fd,ps.c_str(),strlen(ps.c_str()),0);
  }
  void boardcast(const std::string & str){
    auto ps=str;
    auto len=strlen(ps.c_str());
    for(auto it:players){
      send(it.second.fd,ps.c_str(),len,0);
    }
  }
  struct block{
    int player,owner;
    int obj;
    block(){
        player=0;
        owner=0;
        obj=0;
    }
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
        gmap[ix][iy].player=0;
        gmap[ix][iy].owner=0;
        gmap[ix][iy].obj=0;
      }
    }
  }
  virtual void faceto(int name,int f){
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
        if(!it.second.start)continue;
        it.second.times++;
        onGetUser(
          it.first,
          it.second.x,it.second.y,
          it.second.hp,
          it.second.pow
        );
        int bttm=fabs(time(NULL)-it.second.update_time);
        if(it.second.hp<0 || bttm>5){
          //printf(KGRN "[Game]bttm:%d \n" RESET,bttm);
          quit(it.first);
          continue;
        }
        //printf(KGRN "[Game]player:%d (%d,%d) \n" RESET,
        //  it.first,
        //  it.second.x,it.second.y
        //);
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
  virtual bool put(int i,int nx,int ny){
    if(nx>=maxX)return false;
    if(ny>=maxY)return false;
    if(nx<0)return false;
    if(ny<0)return false;
    
    block & ob=gmap[nx][ny];

    if(ob.obj==i)return false;

    ob.obj=i;
    onPut(i,nx,ny);

    return true;
  }
  virtual void put(int i,int name){
    auto it=players.find(name);
    if(it==players.end())return;
    int x=it->second.x;
    int y=it->second.y;
    switch(i){
      case 1:
        if(it->second.pow<20)break;
        if(put(i,x,y))it->second.pow-=20;
      break;
      case 2:
        if(it->second.pow<20)break;
        if(put(i,x,y))it->second.pow-=20;
      break;
    }
  }
  virtual void moveplayerto(int name,int nx,int ny){
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
    
    if(x>=0 && y>=0){
      block & ob=gmap[x][y];
      ob.player=0;
    }
    
    block & b=gmap[nx][ny];
    if(b.obj!=0)
      collideobj(nx,ny,name,b.obj);
    
    if(b.owner!=0){
        if(b.owner!=name){
            auto ito=players.find(b.owner);
            if(ito!=players.end()){
                ito->second.hp-=5;
                it->second.pow-=10;
            }
        }else{
            if(it->second.hp<300) it->second.hp+=3;
            if(it->second.pow<300)it->second.pow+=5;
        }
    }
    
    if(b.player==0){
      it->second.x=nx;
      it->second.y=ny;
      
      if(it->second.hp<200) it->second.hp+=2;
      if(it->second.pow<200)it->second.pow+=5;
      it->second.have++;
      if(b.owner!=0){
          auto oit1=players.find(b.owner);
          if(oit1==players.end()){
              oit1->second.have--;
          }
          //players[b.owner].have--;
      }
      
      b.owner =name;
      b.player=name;
    }else{
      auto it=players.find(name);
      if(it==players.end()){
        it->second.x=nx;
        it->second.y=ny;
      
        it->second.have++;
        if(b.owner!=0){
          //players[b.owner].have--;
          auto oit2=players.find(b.owner);
          if(oit2==players.end()){
              oit2->second.have--;
          }
        }
      
        b.owner =name;
        b.player=name;
      }else
        collideplayer(nx,ny,name);
    }
    onMove(nx,ny,name);
  }
  virtual void quit(int name){
    auto it=players.find(name);
    if(it==players.end())return;
    try{
      auto it=players.find(name);
      if(it==players.end())return;
      int x=it->second.x;
      int y=it->second.y;
    
      block & ob=gmap[x][y];
      
      ob.player=0;
    }catch(std::out_of_range &){
        
    }
    close(it->second.fd);
    players.erase(it);
    onQuit(name);
  }
  virtual void login(int name,int fd){
    player & p=players[name];
    int x=rand()%(this->maxX);
    int y=rand()%(this->maxY);
    
    //printf("login\n");

    p.hp=50;
    p.pow=0;
    p.fd=fd;
    
    p.r=(rand()%236)+20;
    p.g=(rand()%236)+20;
    p.b=(rand()%236)+20;
    
    p.update_time=time(NULL);
    p.start=true;
    
    moveplayerto(name,x,y);
    onLogin(name,p.r,p.g,p.b);
  }
  
  virtual void onLogin(int name,int r,int g,int b){
    char buf[256];
    
    snprintf(buf,256,"setname %d %d %d %d \n",name,r,g,b);
    std::string bs=buf;
    senduser(name,bs);
    
    snprintf(buf,256,"addplayer %d %d %d %d \n",name,r,g,b);
    bs=buf;
    boardcast(bs);
  }
  virtual void onGetUser(int name,int a,int b,int c,int d){
    char buf[256];
    snprintf(buf,256,"setme %d %d %d %d \n",a,b,c,d);
    std::string bs=buf;
    senduser(name,bs);
  }
  virtual void onFaceto(int name,int t){
    char buf[256];
    snprintf(buf,256,"face %d %d \n",name,t);
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
  
  virtual void onMove(int nx,int ny,int name){
    char buf[256];
    snprintf(buf,256,"move %d %d %d \n",name,nx,ny);
    std::string bs=buf;
    boardcast(bs);
  }
  
  virtual void onQuit(int name){
    char buf[256];
    snprintf(buf,256,"quit %d \n",name);
    std::string bs=buf;
    boardcast(bs);
    
    bs="exit \n";
    senduser(name,bs);
  }
  
  virtual void collideplayer(int nx,int ny,int name){}
  virtual void collideobj(int nx,int ny,int name,int i){
    auto it=players.find(name);
    if(it==players.end())return;
    int ohp=it->second.hp;
    char buf[256];
    #define updateUser snprintf(buf,256,\
        "setme %d %d %d %d \n",\
        it->second.x,it->second.y,\
        it->second.hp,it->second.pow);\
        send(it->second.fd,buf,strlen(buf),0);
    try{
      //printf("%d %d ",it->second.hp,i);
      switch(i){
        case 1:
          it->second.hp=ohp-20;
          updateUser;
        break;
        case 2:
          it->second.hp=ohp-40;
          updateUser;
        break;
      }
      //printf("%d %d\n",name,it->second.hp);
      put(0,nx,ny);
    }catch(std::out_of_range &){
      
    }
    #undef updateUser
  }
  virtual void onMsg(int name,const std::string & str){
    std::istringstream iss(str);
    std::string mode;
    iss>>mode;
    int f;
    auto it=players.find(name);
    if(it!=players.end()){
        it->second.update_time=time(NULL);
    }
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
          if(b.owner!=0){
            bzero(buf,256);
            snprintf(buf,256,"setown %d %d %d \n",x,y,b.owner);
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
std::atomic<int> name_increment;
struct per_session_data {
  int name;
  void randname(){
    if(name_increment==0)name_increment++;
    name=name_increment++;
    //begin:
    //name=fabs(rand() | 1);
    
    //Game_locker.lock();
    //auto it=Game.players.find(name);
    //if(it!=Game.players.end()){
    //  Game_locker.unlock();
    //  goto begin;
    //}
    //Game_locker.unlock();
  }
  void init(int fd){
    randname();
    int n=name;
    char buf[256];
    
    Game_locker.lock();
    Game.login(name,fd);
    Game.getblock();
    snprintf(buf,256,"cremap %d %d \n",Game.maxX,Game.maxY);
    send(fd,buf,strlen(buf),0);
    for(auto uit:Game.players){
      //std::string bs="addplayer ";
      //bs+=uit.first;
      //bs+=" \n";
      snprintf(buf,256,"addplayer %d %d %d %d \n",uit.first,uit.second.r,uit.second.g,uit.second.b);
      send(fd,buf,strlen(buf),0);
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
    char buf[128];
    int l=len>127 ? 127 : len;
    auto str=(char*)data;
    
    for(int i=0;i<l;i++){
      buf[i]=str[i];
    }
    
    buf[127]='\0';
    
    std::string msg=buf;
    
    Game_locker.lock();
    Game.onMsg(name,msg);
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
