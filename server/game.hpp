#ifndef JUBK_SERVER_GAME
#define JUBK_SERVER_GAME
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <list>
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

extern "C"{
#include <libwebsockets.h>
}

using namespace std;

int destroy_flag = 0;

unordered_map<string,list<string> > msgs;
mutex msgs_locker;
void senduser(const string &name,const string & str){
  //auto it=msgs.find(name);
  //if(it==msgs.end())return;
  msgs_locker.lock();
  msgs[name].push_back(str);
  msgs_locker.unlock();
}
void boardcast(const string & str){
  msgs_locker.lock();
  for(auto it:msgs){
    it.second.push_back(str);
  }
  msgs_locker.unlock();
}

class game{
  public:
  int times;
  game(){
    times=0;
    block_buffer_times=-1;
  }
  struct player{
    int hp;
    int pow;
    int x;
    int y;
    int have;
    int face;
    int times;
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
  unordered_map<string,player> players;
  struct block{
    string player,owner;
    int obj;
  };
  vector<vector<block> > gmap;
  int maxX;
  int maxY;
  
  void init(int x,int y){
    maxX=x;
    maxY=y;
    gmap.resize(x);
    for(auto it:gmap){
      it.resize(y);
      for(auto it2:it){
        it2.player.clear();
        it2.owner.clear();
        it2.obj=0;
      }
    }
  }
  virtual void faceto(const string & name,int f){
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
    if(nx>maxX)return;
    if(ny>maxY)return;
    if(nx<0)return;
    if(ny<0)return;
    
    block & ob=gmap.at(nx).at(ny);
    
    ob.obj=i;
  }
  virtual void put(int i,const string & name){
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
  virtual void moveplayerto(const string & name,int nx,int ny){
    if(nx>maxX)return;
    if(ny>maxY)return;
    if(nx<0)return;
    if(ny<0)return;
    
    auto it=players.find(name);
    if(it==players.end())return;
    int x=it->second.x;
    int y=it->second.y;
    
    if(x>0 && y>0){
      block & ob=gmap.at(x).at(y);
      ob.player.clear();
    }
    
    block & b=gmap.at(nx).at(ny);
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
  virtual void quit(const string &name){
    auto it=players.find(name);
    if(it==players.end())return;
    try{
      auto it=players.find(name);
      if(it==players.end())return;
      int x=it->second.x;
      int y=it->second.y;
    
      block & ob=gmap.at(x).at(y);
      
      ob.player.clear();
    }catch(std::out_of_range &){
        
    }
    players.erase(it);
    onQuit(name);
  }
  virtual void login(const string &name){
    player & p=players[name];
    int x=rand()%maxX;
    int y=rand()%maxY;
    moveplayerto(name,x,y);
    onLogin(name);
  }
  
  virtual void onLogin(const string &name){
    getmap(name);
    
    char buf[256];
    
    snprintf(buf,256,"setname %s",name.c_str());
    string bs=buf;
    senduser(name,bs);
    
    snprintf(buf,256,"addplayer %s",name.c_str());
    bs=buf;
    boardcast(bs);
  }
  virtual void onGetUser(const string &name,int a,int b,int c,int d){
    char buf[256];
    snprintf(buf,256,"setme %d %d %d %d",a,b,c,d);
    string bs=buf;
    senduser(name,bs);
  }
  virtual void onFaceto(const string &name,int t){
    char buf[256];
    snprintf(buf,256,"face %s %d",name.c_str(),t);
    string bs=buf;
    boardcast(bs);
  }
  
  virtual void onPut(int i,int x,int y){
    char buf[256];
    snprintf(buf,256,"setobj %d %d %d",x,y,i);
    string bs=buf;
    boardcast(bs);
  }
  
  virtual void onMove(int nx,int ny,const string &name){
    char buf[256];
    snprintf(buf,256,"move %s %d %d",name.c_str(),nx,ny);
    string bs=buf;
    boardcast(bs);
  }
  
  virtual void onQuit(const string &name){
    char buf[256];
    snprintf(buf,256,"quit %s",name.c_str());
    string bs=buf;
    boardcast(bs);
    
    bs="exit";
    senduser(name,bs);
  }
  
  virtual void collideplayer(int nx,int ny,const string &name){}
  virtual void collideobj(int nx,int ny,const string &name,int i){
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
  virtual void onMsg(const string &name,const string & str){
    istringstream iss(str);
    string mode;
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
  list<string> block_buffer;
  int          block_buffer_times;
  mutex        block_buffer_locker;
  virtual void getblock(){
    
    if(block_buffer_times==times)return;
    block_buffer_times=times;
    block_buffer.clear();
    
    char buf[256];
    
    block_buffer_locker.lock();
    for(int x=0;x<maxX;x++){
      for(int y=0;y<maxY;y++){
        try{
          block & b=gmap.at(x).at(y);
          if(b.obj!=0){
            snprintf(buf,256,"setobj %d %d %d",x,y,b.obj);
            block_buffer.push_back(buf);
          }
          if(!b.owner.empty()){
            snprintf(buf,256,"setown %d %d %s",x,y,b.owner.c_str());
            block_buffer.push_back(buf);
          }
        }catch(std::out_of_range &){
        }
      }
    }
    block_buffer_locker.unlock();
  }
  virtual void getmap(const string &name){
    char buf[256];
    snprintf(buf,256,"cremap %d %d",maxX,maxY);
    string bs=buf;
    senduser(name,bs);
  }
}Game;

mutex Game_locker;

struct per_session_data {
  char name[16];
  void randname(){
    const static char chs[]=
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    begin:
    for(int i=0;i<15;i++){
      name[i]=chs[(rand()&(sizeof(chs)-1))];
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
  void init(struct lws *wsi_in){
    randname();
    string n=name;
    
    Game_locker.lock();
    Game.login(name);
    Game.getblock();
    Game_locker.unlock();
    
    char buf[256];
    Game.block_buffer_locker.lock();
    for(auto it:Game.block_buffer){
      snprintf(buf,256,"%s",it.c_str());
      lws_write(
        wsi_in,
        (unsigned char*)buf,sizeof(buf),
        LWS_WRITE_TEXT
      );
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
    
    string msg=buf,
           n  =name;
    
    Game_locker.lock();
    Game.onMsg(n,msg);
    Game_locker.unlock();
  }
  void onWriteable(struct lws *wsi_in){
    msgs_locker.lock();
    auto it=msgs.find(name);
    
    if(it==msgs.end()){
      msgs_locker.unlock();
      return;
    }
    
    for(auto itm : it->second){
      auto str=itm.c_str();
      char buf[256];
      snprintf(buf,256,"%s",str);
      lws_write(
        wsi_in,
        (unsigned char*)buf,sizeof(buf),
        LWS_WRITE_TEXT
      );
    }
    it->second.clear();
    
    msgs_locker.unlock();
  }
  void quit(){
    Game_locker.lock();
    Game.quit(name);
    Game_locker.unlock();
    
    msgs_locker.lock();
    msgs.erase(name);
    msgs_locker.unlock();
  }
};

int service_callback(
                         struct lws *wsi,
                         enum lws_callback_reasons reason, void *user,
                         void *in, size_t len){
    
    auto session=(per_session_data*)user;
    
    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED:
          printf(KYEL"[Main Service] Connection established\n"RESET);
          session->init(wsi);
          lws_callback_on_writable(wsi);
        break;
        // If receive a data from client
        case LWS_CALLBACK_RECEIVE:
          session->onMessage(in,len);
          lws_callback_on_writable(wsi);
        break;
        
        //writeable
        case LWS_CALLBACK_CLIENT_WRITEABLE:
          session->onWriteable(wsi);
          lws_callback_on_writable(wsi);
        break;
        
        //close
        case LWS_CALLBACK_CLOSED:
          session->quit();
        break;

        default:
        break;
    }
    return 0;
}
void * mainloop(void*){
  printf(KGRN"[Main Loop] Game start success \n"RESET);
  while( !destroy_flag){
    
    Game_locker.lock();
    Game.updateplayer();
    Game_locker.unlock();
    
    sleep(1);
  }
}
#endif
