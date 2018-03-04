#include "game.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

namespace draw{
  SDL_Window *Window = NULL;
  SDL_Surface *WindowScreen = NULL;
  SDL_Renderer *renderer = NULL;
  
  SDL_Texture* player_textures[4];
  SDL_Texture* bomb_textures[2];
  
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
  SDL_Texture * loadTexture(const char * path){
    SDL_Surface *bitmapSurface = NULL;
    SDL_Texture *bitmapTex = NULL;
    bitmapSurface = SDL_LoadBMP(path);
	if(bitmapSurface==NULL)
		return NULL;
	SDL_SetColorKey(bitmapSurface,true,SDL_MapRGB(bitmapSurface->format,255,255,255));
    bitmapTex = SDL_CreateTextureFromSurface(renderer, bitmapSurface);
    if(bitmapTex==NULL)
		return NULL;
	SDL_FreeSurface(bitmapSurface);
    return (bitmapTex);
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
	player_textures[0]=loadTexture("img/right.bmp");
	player_textures[1]=loadTexture("img/down.bmp");
	player_textures[2]=loadTexture("img/left.bmp");
	player_textures[3]=loadTexture("img/up.bmp");
	
	bomb_textures[0]=loadTexture("img/bomb1.bmp");
	bomb_textures[1]=loadTexture("img/bomb2.bmp");
  }
  inline void freeTx(SDL_Texture * t){
	  if(t){
		  SDL_DestroyTexture(t);
	  }
  }
  void destroy(){
	int i;
	for(i=0;i<4;i++)freeTx(player_textures[i]);
	for(i=0;i<2;i++)freeTx(bomb_textures[i]);
	SDL_DestroyWindow(Window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
  }

  bool draw_texture(SDL_Texture * tx,int x,int y){
      if(tx==NULL)return false;
	  SDL_Rect sr,dr;
      sr.x=(x-5)*5+150;
      sr.y=(y-5)*5+150;
	  sr.w=50;
      sr.h=50;
    SDL_RenderCopy(renderer,tx,&sr,&dr);
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
    if(!draw_texture(player_textures[i],x,y)){
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
		if(!draw_texture(player_textures[f],x,y)){
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
  inline void player_abs(double x,double y,int f,Color c){
    vec p;
    abs2scr(x,y,p);
    player_scr(p.x,p.y,f,c);
  }
  
  game::Mover cam_p;
  inline void camera_update(){
    auto pl=game::player[game::me.name];
	//auto systime=gettm();
	//double t=((double)(pl.tm));
    double pt[2];
	//getposi_time(game::me.position.x,game::me.position.y,pl.faceto,systime-t,pt);
    
	cam_p.update(game::me.position.x,game::me.position.y);
	cam_p.getPosi(pt);
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
	defcol.r=20;
	defcol.g=20;
	defcol.b=20;
	double posi[2];
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
			pt->second.mover_update();
            pt->second.mover.getPosi(posi);
            player_abs(
              //x,y,
			  posi[0],posi[1],
			  pt->second.faceto,pt->second.color
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
  
  Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048);
  Mix_Music *sound=NULL;
  sound=Mix_LoadMUS("sound/SingingRivulet.ogg");
  if(sound)Mix_PlayMusic(sound,1);
  
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
  draw::destroy();
  game::destroy();
  return 0;
}
