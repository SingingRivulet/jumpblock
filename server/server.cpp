#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>
using namespace std;
class game{
  public:
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
  virtual void getmap(const string &name){
    char buf[256];
    snprintf(buf,256,"cremap %d %d",maxX,maxY);
    string bs=buf;
    senduser(name,bs);
    
    for(int x=0;x<gmap.size();x++){
      for(int y=0;y<gmap[x].size();y++){
        block & b=gmap[x][y];
        if(b.obj!=0){
          snprintf(buf,256,"setobj %d %d %d",x,y,b.obj);
          string bs=buf;
          senduser(name,bs);
        }
        if(!b.owner.empty()){
          snprintf(buf,256,"setown %d %d %s",x,y,b.owner.c_str());
          string bs=buf;
          senduser(name,bs);
        }
      }
    }
  }
  virtual void senduser(const string &name,const string & str){
    
  }
  virtual void boardcast(const string & str){
    
  }
};
/*
static int ws_service_callback(
                         struct lws *wsi,
                         enum lws_callback_reasons reason, void *user,
                         void *in, size_t len)
{

    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED:
            printf(KYEL"[Main Service] Connection established\n"RESET);
            break;

        // If receive a data from client
        case LWS_CALLBACK_RECEIVE:
            printf(KCYN_L"[Main Service] Server recvived:%s\n"RESET,(char *)in);

            // echo back to client
            websocket_write_back(wsi ,(char *)in, -1);

            break;
    case LWS_CALLBACK_CLOSED:
            printf(KYEL"[Main Service] Client close.\n"RESET);
        break;

    default:
            break;
    }

    return 0;
}
*/
int main(){}
