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
    it->second.face=f;
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
  }
  virtual void onLogin(const string &name){}
  virtual void onFaceto(const string &name,int t){}
  virtual void onPut(int i,int x,int y){}
  virtual void onMove(int nx,int ny,const string &name){}
  virtual void onQuit(const string &name){}
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
};
int main(){}
