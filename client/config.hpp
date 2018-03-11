#ifndef jubk_game_config
#define jubk_game_config
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
char   Game_addr_default[128];
const char * Game_addr    =Game_addr_default;
short  Game_port;
struct{
    int bombframe;
    int playerframe;
    int volume;
}Config;
void config_readline(const std::string & l){
    std::istringstream iss(l);
    std::string k;
    int v;
    iss>>k;
    iss>>v;
    if(k=="bombframe"){
        Config.bombframe=v;
        printf("Config.bombframe=%d\n",v);
    }else
    if(k=="playerframe"){
        Config.playerframe=v;
        printf("Config.playerframe=%d\n",v);
    }else
    if(k=="volume"){
        Config.volume=v;
        printf("Config.volume=%d\n",v);
    }
}
void loadconfig(){
    char buf[128];
    auto fp=fopen("addr.conf","r");
    if(fp){
        bzero(Game_addr_default,128);
        fgets(Game_addr_default,128,fp);
    
        bzero(buf,128);
        fgets(buf,128,fp);
        Game_port=atoi(buf);
        printf("connect:%s:%d\n",Game_addr_default,Game_port);
        fclose(fp);
    }
    fp=fopen("game.conf","r");
    if(fp){
        std::string sbuf;
        while(!feof(fp)){
            bzero(buf,128);
            fgets(buf,128,fp);
            sbuf=buf;
            config_readline(sbuf);
        }
        fclose(fp);
    }
}
#endif
