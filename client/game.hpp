#ifndef jubk_game_game
#define jubk_game_game
#include "utils.hpp"
namespace game{
  mutex locker;
  vec map_size;
  struct Mover{
	  struct posi{
		  double x,y;
		  posi(){
			  x=0;
			  y=0;
		  }
	  }from,to;
	  double tm,dt;
	  int x,y;
	  Mover(){
		  x=-1;
		  y=-1;
	  }
	  void update(int nx,int ny){
		  if(nx==x && ny==y)return;
		  if(x==-1 && y==-1){
			  from.x=nx;
			  from.y=ny;
		  }else{
			  double np[2];
			  this->getPosi(np);
			  from.x=np[0];
			  from.y=np[1];
		  }
		  tm=gettm();
		  to.x=nx;
		  to.y=ny;
		  x=nx;
		  y=ny;
	  }
	  void getPosi(double * p){
		  auto t=gettm();
		  dt=t-tm;
		  if(dt>1){
			  p[0]=to.x;
			  p[1]=to.y;
		  }else{
			  p[0]=from.x+((to.x-from.x)*dt);
			  p[1]=from.y+((to.y-from.y)*dt);
		  }
	  }
  };
  class Me{
    public:
  	 string name;
  	 int pw,hp;
  	 vec position;
  }me;

  class Player{
    public:
    vec    position;        //position
    Color  color;           //color
    int    faceto;          //face to
	Mover  mover;
    void init(int r,int g,int b){
      position.x=0;
      position.y=0;
      faceto=0;
      color.r=r;
	  color.g=g;
	  color.b=b;
    }
	void mover_update(){
		this->mover.update(position.x,position.y);
    }
  };
  unordered_map<string,Player> player;

  struct block{
    string player,owner;
    int obj;
	Color color_cache;
	block(){
		color_cache.r=10;
		color_cache.g=10;
		color_cache.b=40;
	}
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
        gmap[ix][iy].player="";
        gmap[ix][iy].owner ="";
        gmap[ix][iy].obj=0;
      }
    }
  }

  inline void setname(const string & n){
    me.name=n;
  }

  inline int set_hp(int v){
    me.hp=v;
  }

  inline int set_pw(int v){
	me.pw=v;
  }

  inline void setme(int v1,int v2,int v3,int v4){
    me.position.x=v1;
    me.position.y=v2;
    set_hp(v3);
    set_pw(v4);
  }

  inline void send(const string & msg){
    const char * buf=msg.c_str();
    ::send(connfd,buf,strlen(buf),0);
    char end='\n';
    ::send(connfd,&end,sizeof(end),0);
  }

  inline void addplayer(const string & unm,int r,int g,int b){
    player[unm].init(r,g,b);
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
	if(unm==me.name)gameover=true;
  }

  inline void createmap(int x,int y){
    map_size.x=x;
    map_size.y=y;
    init(x,y);
  }

  inline void setmapown(int x,int y,const string & o){
    if(x<0)return;
    if(y<0)return;
    if(x>=map_size.x)return;
    if(y>=map_size.y)return;
	
	auto it=player.find(o);
    if(it==player.end())return;
    
    gmap[x][y].owner=o;
	gmap[x][y].color_cache=it->second.color;
  }

  inline void pick(int x,int y){
    if(x<0)return;
    if(y<0)return;
    if(x>=map_size.x)return;
    if(y>=map_size.y)return;
    gmap[x][y].obj=0;
  }

  inline void setmapobj(int x,int y,int o){
    if(x<0)return;
    if(y<0)return;
    if(x>=map_size.x)return;
    if(y>=map_size.y)return;
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

    if(x<map_size.x && y<map_size.y && x>=0 && y>=0){
      
      //gmap[x][y].owner=gmap[x][y].player;
	  gmap[x][y].player="";
    }

    it->second.position.x=nx;
    it->second.position.y=ny;
	it->second.mover.update(nx,ny);
	
    gmap[nx][ny].player=name;
    gmap[nx][ny].owner =name;
	gmap[nx][ny].color_cache=it->second.color;
  }

  inline void put(int i){
    char buf[64];
    snprintf(buf,64,"put %d",i);
    send(buf);
  }
  
  int jubk_lastf;
  inline void walk(int f){
    if(f==jubk_lastf){
		put(2);
		return;
    }
    jubk_lastf=f;
    char buf[64];
    snprintf(buf,64,"walk %d",f);
    send(buf);
  }


  void onmsg(const string & m){
    istringstream iss(m);
    string method,name;
    int i1,i2,i3,i4;
    iss>>method;

    if(method=="addplayer"){
    	iss>>name;
		iss>>i1;
		iss>>i2;
		iss>>i3;
      addplayer(name,i1,i2,i3);
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
      gameover=true;
    }
  }
}

#endif
