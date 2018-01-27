#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <string>
using namespace std;
class game{
  public:
  struct player{
    int hp;
    int pow;
    int x;
    int y;
    int have;
  };
  unordered_map<string,player> players;
  struct block{
    string player,owner;
    int obj;
  };
  vector<vector<block> > gmap;
  
  void inti(int x,int y){
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
  void moveplayerto(const string & name,int nx,int ny){
    auto it=players.find(name);
    if(it==players.end())return;
    int x=it->second.x;
    int y=it->second.y;
    
    block & ob=gmap.at(x).at(y);
    if(ob.obj)
      collideobj(x,y,name);
    ob.player.clear();
    
    block & b=gmap.at(nx).at(ny);
    
    if(b.player.empty()){
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
  virtual void collideplayer(int nx,int ny,const string &name){}
  virtual void collideobj(int nx,int ny,const string &name){}
  virtual void put(int i){}
};
int main(){}
