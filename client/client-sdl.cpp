#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <math.h>
#include <string.h>
#include <string>
#include <sstream>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/file.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <thread>
#include <mutex>
#include <atomic>
#include "config.h"
using namespace std;
int connfd;
atomic<bool> gameover;
struct vec{
  int x;
  int y;
};
struct Color{
  Uint8 r,g,b;
  Color()=default;
  Color(const Color&)=default;
  Color operator=(const Color & in){
	  r=in.r;
	  g=in.g;
	  b=in.b;
  }
};
double gettm(){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return (((double)tv.tv_sec) + ((double)tv.tv_usec / 1000000.0));
}
namespace draw{
  SDL_Window *Window = NULL;
  SDL_Surface *WindowScreen = NULL;
  SDL_Renderer *renderer = NULL;
  vector<SDL_Texture*> textures;
}

namespace game{
  mutex locker;
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
    double tm;             //last update time
    void init(int r,int g,int b){
      position.x=0;
      position.y=0;
      faceto=0;
      tm=gettm();
	  color.r=r;
	  color.g=g;
	  color.b=b;
    }
	void settm(){
      tm=gettm();
    }
  };
  unordered_map<string,Player> player;

  struct block{
    string player,owner;
    int obj;
	Color color_cache;
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
    gmap[x][y].owner=o;
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
    it->second.tm=gettm();
	
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

namespace draw{
  struct{
    double x,y;
  }camera;
  
  inline void abs2scr(double x,double y,vec & res){
    double tx=(camera.x-5);
    double ty=(camera.y-5);
    res.x=floor((x-tx)*5);
    res.y=floor((y-ty)*5);
  }
  
  inline void getposi_time(double bx,double by,int f,double t,double * ret){
    double x=bx;
    double y=by;
    if(t<0){
	  
	}else
    if(f==0){
      x+=(t-1);
    }else
    if(f==1){
      y+=(t-1);
    }else
    if(f==2){
      x-=(t-1);
    }else
    if(f==3){
      y-=(t-1);
    }
    if(x<=0)x=0;
    if(y<=0)y=0;
    if(x>=game::map_size.x)x=game::map_size.x;
    if(y>=game::map_size.y)y=game::map_size.y;
	ret[0]=x;
	ret[1]=y;
  }
  void loadTexture(const char * path){
    SDL_Surface *bitmapSurface = NULL;
    SDL_Texture *bitmapTex = NULL;
    bitmapSurface = SDL_LoadBMP(path);
	if(bitmapSurface==NULL)return;
	SDL_SetColorKey(bitmapSurface,true,SDL_MapRGB(bitmapSurface->format,255,255,255));
    bitmapTex = SDL_CreateTextureFromSurface(renderer, bitmapSurface);
    if(bitmapTex==NULL)return;
	SDL_FreeSurface(bitmapSurface);
    textures.push_back(bitmapTex);
  }

  void init(){
    SDL_Init(SDL_INIT_VIDEO);
    Window = SDL_CreateWindow(
      "jump block",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      500,                    500,
      SDL_WINDOW_SHOWN
    );
    if (Window == NULL){
      exit(1);
    }
	WindowScreen = SDL_GetWindowSurface(Window);
    renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
	loadTexture("img/right.bmp");
	loadTexture("img/down.bmp");
	loadTexture("img/left.bmp");
	loadTexture("img/up.bmp");
	loadTexture("img/bomb1.bmp");
	loadTexture("img/bomb2.bmp");
  }

  bool draw_texture(int id,int x,int y){
      if(id>=textures.size())return false;
	  SDL_Rect sr,dr;
      sr.x=(x-5)*5+150;
      sr.y=(y-5)*5+150;
	  sr.w=50;
      sr.h=50;
    SDL_RenderCopy(renderer,textures[id],&sr,&dr);
	return true;
  }
  void draw_val(){
	  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
	  SDL_Rect sr;
	  sr.x=10;
	  sr.y=10;
	  sr.w=game::me.hp;
	  sr.h=10;
	  SDL_RenderFillRect(renderer,&sr);
	  
	  if(game::me.hp<0)gameover=true;
	  
	  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
	  sr.x=10;
	  sr.y=20;
	  sr.w=game::me.pw;
	  sr.h=10;
	  SDL_RenderFillRect(renderer,&sr);
  }
  inline void block_scr(int x,int y,Color c){
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
	SDL_Rect sr;
      sr.x=(x-5)*5+150;
      sr.y=(y-5)*5+150;
	  sr.w=50;
      sr.h=50;
    SDL_RenderFillRect(renderer,&sr);
  }
  inline void block_abs(int x,int y,Color c){
    vec p;
    abs2scr(x,y,p);
    block_scr(p.x,p.y,c);
  }
  inline void obj_scr(int x,int y,int i){
    if(!draw_texture(i+3,x,y)){
		if(i==1){
			SDL_SetRenderDrawColor(renderer, 128,64,64,64);
		}else
		if(i==2){
			SDL_SetRenderDrawColor(renderer, 255,64,64,128);
		}
		SDL_Rect sr;
		sr.x=(x-5)*5+152;
		sr.y=(y-5)*5+152;
		sr.w=20;
		sr.h=20;
		SDL_RenderFillRect(renderer,&sr);

    }
  }
  inline void obj_abs(int x,int y,int i){
    vec p;
    abs2scr(x,y,p);
    obj_scr(p.x,p.y,i);
  }
  inline void player_scr(int x,int y,int f,Color c){
    if(f<4 && f>=0){
		if(!draw_texture(f,x,y)){
			SDL_SetRenderDrawColor(renderer, 128,128,128, 255);
			SDL_Rect sr;
			sr.x=(x-5)*5+152;
			sr.y=(y-5)*5+152;
			sr.w=20;
			sr.h=20;
			SDL_RenderFillRect(renderer,&sr);
		}
	}
  }
  inline void player_abs(int x,int y,int f,Color c,double t){
    double pt[2];
    getposi_time(x,y,f,t,pt);
	vec p;
    abs2scr(pt[0],pt[1],p);
    player_scr(p.x,p.y,f,c);
  }
  
  struct Camera{
	  struct posi{
		  double x,y;
		  posi(){
			  x=0;
			  y=0;
		  }
	  }from,to;
	  double tm,dt;
	  int x,y;
	  Camera(){
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
			  getCam(np);
			  from.x=np[0];
			  from.y=np[1];
		  }
		  tm=gettm();
		  to.x=nx;
		  to.y=ny;
		  x=nx;
		  y=ny;
	  }
	  void getCam(double * p){
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
  }cam_p;
  inline void camera_update(){
    auto pl=game::player[game::me.name];
	//auto systime=gettm();
	//double t=((double)(pl.tm));
    double pt[2];
	//getposi_time(game::me.position.x,game::me.position.y,pl.faceto,systime-t,pt);
    
	cam_p.update(game::me.position.x,game::me.position.y);
	cam_p.getCam(pt);
	//if(cam_p.dt>1 && cam_p.dt<2){
	//	pt[0]=cam_p.from.x+((cam_p.to.x-cam_p.from.x)*cam_p.dt*0.5);
	//	pt[1]=cam_p.from.y+((cam_p.to.y-cam_p.from.y)*cam_p.dt*0.5);
	//}
	
    camera.x=pt[0];
    camera.y=pt[1];
	//camera.x=game::me.position.x;
    //camera.y=game::me.position.y;
	//printf("player:(%d,%d) camera:(%f,%f)\n",game::me.position.x,game::me.position.y,camera.x,camera.y);
  }

  void all_block(){
    Color defcol;
	defcol.r=0;
	defcol.g=0;
	defcol.b=0;
	int cx=floor(camera.x);
    int cy=floor(camera.y);
	int bx=cx-10;
    int by=cy-10;
    int ex=cx+11;
    int ey=cy+11;
    for(int x=bx;x<ex;x++){
      for(int y=by;y<ey;y++){
      
        if(x<0)                {block_abs(x,y,defcol);continue;}
        if(y<0)                {block_abs(x,y,defcol);continue;}
        if(game::map_size.x<=x){block_abs(x,y,defcol);continue;}
        if(game::map_size.y<=y){block_abs(x,y,defcol);continue;}
      
        game::block & bk=game::gmap[x][y];
	    
        string & owner=bk.owner;
        
		block_abs(x,y,bk.color_cache);
		/*
		if(!owner.empty()){
          auto pl=game::player.find(owner);
          if(pl!=game::player.end()){
            block_abs(x,y,pl->second.color);
          }else{
            block_abs(x,y,defcol);
          }
        }else{
	      block_abs(x,y,defcol);
	    }
        */
	  }
    }
    for(int x=bx;x<ex;x++){
      for(int y=by;y<ey;y++){
        if(x<0)continue;
        if(y<0)continue;
        if(game::map_size.x<=x)continue;
        if(game::map_size.y<=y)continue;
      
        game::block & bk=game::gmap[x][y];
	    
        string & player=bk.player;
        if(!player.empty()){
          auto pt=game::player.find(player);
          if(pt!=game::player.end()){
            player_abs(
              x,y,
			  pt->second.faceto,pt->second.color,
              (double)(gettm()-pt->second.tm)
            );
          }
        }
        int obj=bk.obj;
        if(obj==2){
          obj_abs(x,y,2);
        }
		if(obj==1 && bk.owner==game::me.name){
          obj_abs(x,y,1);
        }
      }
    }
  }
  void render(){
    SDL_RenderClear(renderer);
    
    camera_update();
    all_block();
    draw_val();
	
    SDL_RenderPresent(renderer);
  }
}
char   Game_addr_default[]=ADDR;
const char * Game_addr    =Game_addr_default;
short  Game_port          =PORT;

void mainloop(){  
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(Game_addr);
  address.sin_port = htons(Game_port);
  
  game::locker.lock();
  connfd = socket(AF_INET, SOCK_STREAM, 0);
  if(connect(connfd, (struct sockaddr *)&address, sizeof(address))==-1)gameover=true;
  game::locker.unlock();
  
  char str[256];
  int ptr=0;
  char buf;
  string tmp;
  while(!gameover){
    if(read(connfd,&buf,1)>0){
      str[ptr]=buf;
	  ++ptr;
      if(buf=='\n' || buf=='\0' || ptr>=255){
		  str[ptr]='\0';
		  game::locker.lock();
		  tmp=str;
		  game::onmsg(tmp);
		  game::locker.unlock();
		  ptr=0;
	  }
    }
  }
  
  close(connfd);
}
int main(int argn,char ** args){
  if(argn==3){
	  Game_addr=args[1];
	  Game_port=atoi(args[2]);
  }
  SDL_Event e;
  srand(time(NULL));
  draw::init();
  gameover=false;
  thread ml(mainloop);
  ml.detach();
  double x,y,dx,dy,adx,ady;
  while(!gameover){
    game::locker.lock();
    draw::render();
	while( SDL_PollEvent( &e ) != 0 ){
      if( e.type == SDL_QUIT ){
        gameover=true;
      }
	  if(e.type==SDL_FINGERDOWN){
		x=e.tfinger.x;
		y=e.tfinger.y;
	  }
	  if(e.type==SDL_FINGERUP){
		  
		  dx = e.tfinger.x-x;
		  dy = e.tfinger.y-y;
		  adx=fabs(dx);
		  ady=fabs(dy);
		  if(adx<0.2d && ady<0.2d){
			  game::put(1);
			  continue;
		  }
		  if(adx>ady){
			  if(dx>0){
				  game::walk(0);
			  }else{
				  game::walk(2);
			  }
		  }else{
			  if(dy>0){
				  game::walk(1);
			  }else{
				  game::walk(3);
			  }
		  }
	  }
	  if(e.type == SDL_KEYDOWN ){
        switch(e.key.keysym.sym){
          case SDLK_UP:
		    game::walk(3);
		  break;
		  case SDLK_DOWN:
		    game::walk(1);
		  break;
		  case SDLK_LEFT:
		    game::walk(2);
		  break;
		  case SDLK_RIGHT:
		    game::walk(0);
		  break;
		  case SDLK_1:
		    game::put(1);
		  break;
		  case SDLK_2:
		    game::put(2);
		  break;
        }
      }
    }
	game::locker.unlock();
  }
  SDL_DestroyWindow(draw::Window);
  SDL_DestroyRenderer(draw::renderer);
  for(auto it:draw::textures){
    SDL_DestroyTexture(it);
  }
  SDL_Quit();
  game::destroy();
  return 0;
}
