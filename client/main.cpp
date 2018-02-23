#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <math.h>
#include <string.h>
#include <string>
using namespace std;
struct vec{int x;int y;};
int gettm(){
  
}
namespace game{
  vec map_size;
  class Me{
    public:
  	 string name;
  	 int pw,hp;
  	 vec position;
  }me;
  
  class Player{
    public:
    vec position; //position
    //int color;  //color
    int faceto;   //face to
    int tm;       //last update time
    void init(){
      position.x=0;
      position.y=0;
      faceto=0;
      tm=gettm();
    }
  };
  unordered_map<string,Player> player;
  
  struct block{
    string player,owner;
    int obj;
  };
  
  block ** gmap;
  
  void destroy(){
    if(gmap){
      for(int ix=0;ix<map_size.x;ix++)
        delete [] gmap[ix];
      delete [] gmap;
    }
  }
  void init(int x,int y){
    int ix,iy;
    gmap=new block*[x];
    for(ix=0;ix<x;ix++){
    gmap[ix]=new block[y];
      for(iy=0;iy<y;iy++){
        gmap[ix][iy].player.clear();
        gmap[ix][iy].owner.clear();
        gmap[ix][iy].obj=0;
      }
    }
  }
  
  inline void setname(const string & n){
    me.name=n;
  }
  
  inline int set_hp(int v){}
  
  inline int set_pw(int v){}
  
  inline void jubk_setme(int v1,int v2,int v3,int v4){
    me.position.x=v1;
    me.position.y=v2;
    set_hp(v3);
    set_pw(v4);
  }
  
  inline void send(const string & msg){
    
  }
  
  inline void addplayer(const string & unm){
    player[unm].init();
  }
  
  inline void face(const string & unm,int fc){
    player[unm].faceto=fc;
  }
  
  inline void quit(){
    static const string str="quit";
    send(str);
  }
  
  inline void quitplayer(const string & unm){
    vec & posi=player[unm].position;
    int x=posi.x;
    int y=posi.y;
    gmap[x][y].owner.clear();
    player.erase(unm);
  }
  
  inline void createmap(int x,int y){
    map_size.x=x;
    map_size.y=y;
    init(x,y);
  }
  
  inline void setmapown(int x,int y,const string & o){
    if(x<0)return;
    if(y<0)return;
    if(x>map_size.x)return;
    if(y>map_size.y)return;
    gmap[x][y].owner=o;
  }
  
  inline void pick(int x,int y){
    if(x<0)return;
    if(y<0)return;
    if(x>map_size.x)return;
    if(y>map_size.y)return;
    gmap[x][y].obj=0;
  }
  
  inline void setmapobj(int x,int y,int o){
    if(x<0)return;
    if(y<0)return;
    if(x>map_size.x)return;
    if(y>map_size.y)return;
    gmap[x][y].obj=o;
  }
  
  inline void moveplayerto(const string & name,int nx,int ny){
    if(nx<0)return;
    if(ny<0)return;
    if(nx>=map_size.x)return;
    if(ny>=map_size.y)return;
    
    auto it=player.find(name);
    if(it==player.end())return;
    int x=it->second.position.x;
    int y=it->second.position.y;
    
    if(x<map_size.x)
    if(y<map_size.y)
    if(x>=0 && y>=0){
      block & ob=gmap[x][y];
      ob.player.clear();
    }
    
    it->second.position.x=x;
    it->second.position.y=y;
    it->second.tm=gettm();
    
    block & obp=gmap[x][y];
    obp.player=name;
    obp.owner =name;
  }
  
  int jubk_lastf;
  void walk(int f){
    if(f==jubk_lastf)return;
    jubk_lastf=f;
    char buf[64];
    snprintf(buf,64,"walk %d \n",f);
    send(buf);
  }
  
  void jubk_put(int i){
    char buf[64];
    snprintf(buf,64,"put %d \n",i);
    send(buf);
  }
}
