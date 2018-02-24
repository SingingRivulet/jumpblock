#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <math.h>
#include <string.h>
#include <string>
#include <sstream>
#ifdef _WIN32
	#include <winsock2.h>
	#include <time.h>
#else
	#include <sys/time.h>
#endif
using namespace std;
struct vec{
  int x;
  int y;
};
struct Color{
  int c;
};
unsigned long long gettm(){
  #ifdef _WIN32
  struct timeval tv;
  time_t clock;
		struct tm tm;
		SYSTEMTIME wtm;

		GetLocalTime(&wtm);
		tm.tm_year = wtm.wYear - 1900;
		tm.tm_mon = wtm.wMonth - 1;
		tm.tm_mday = wtm.wDay;
		tm.tm_hour = wtm.wHour;
		tm.tm_min = wtm.wMinute;
		tm.tm_sec = wtm.wSecond;
		tm.tm_isdst = -1;
		clock = mktime(&tm);
		tv.tv_sec = clock;
		tv.tv_usec = wtm.wMilliseconds * 1000;
		return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
#else
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
#endif
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
    vec position;          //position
    Color color;           //color
    int faceto;            //face to
    unsigned long long tm; //last update time
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
  
  block ** gmap=NULL;
  
  void destroy(){
    if(gmap){
      for(int ix=0;ix<map_size.x;ix++)
        delete [] gmap[ix];
      delete [] gmap;
    }
  }
  void init(int x,int y){
  	 if(gmap)return;
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
  
  inline void setme(int v1,int v2,int v3,int v4){
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
  inline void walk(int f){
    if(f==jubk_lastf)return;
    jubk_lastf=f;
    char buf[64];
    snprintf(buf,64,"walk %d \n",f);
    send(buf);
  }
  
  inline void jubk_put(int i){
    char buf[64];
    snprintf(buf,64,"put %d \n",i);
    send(buf);
  }
  
  void onmsg(const string & m){
    istringstream iss(m);
    string method,name;
    int i1,i2,i3,i4;
    iss>>method;
    
    if(method=="addplayer"){
    	iss>>name;
      addplayer(name);
    }
    if(method=="cremap"){
    	iss>>i1;
    	iss>>i2;
      createmap(
        i1,i2
      );
    }else
    if(method=="quit"){
    	iss>>name;
      quitplayer(name);
    }else
    if(method=="move"){
    	iss>>name;
    	iss>>i1;
    	iss>>i2;
      moveplayerto(
        name,
        i1,i2
      );
    }else
    if(method=="face"){
    	iss>>name;
    	iss>>i1;
      face(name,i1);
    }else
    if(method=="setme"){
    	iss>>i1;
    	iss>>i2;
    	iss>>i3;
    	iss>>i4;
      setme(
        i1,i2,i3,i4
      );
    }else
    if(method=="pick"){
    	iss>>i1;
    	iss>>i2;
      pick(
        i1,i2
      );
    }else
    if(method=="setobj"){
    	iss>>i1;
    	iss>>i2;
    	iss>>i3;
      setmapobj(
        i1,i2,i3
      );
    }else
    if(method=="setown"){
    	iss>>i1;
    	iss>>i2;
    	iss>>name;
      setmapown(
        i1,i2,name
      );
    }else
    if(method=="setname"){
    	iss>>name;
      setname(name);
    }else
    if(method=="exit"){
      exit(0);
    }
  }
}
namespace draw{
  vec camera;
  void init(){}
  inline void block_scr(int x,int y,Color c){}
  inline void block_abs(int x,int y,Color c){}
  inline void obj_abs(int x,int y,int i){}
  inline void player_scr(int x,int y,int f,Color c){}
  inline void player_abs(int x,int y,int f,Color c){}
  inline void carema_update(){}
  
  void all_block(){}
  void render(){}
}
