#ifndef jubk_game_config
#define jubk_game_config
#include <stdio.h>
#include <stdlib.h>
char   Game_addr_default[128];
const char * Game_addr    =Game_addr_default;
short  Game_port;
void loadconfig(){
    char buf[128];
    auto fp=fopen("addr.conf","r");
    if(!fp)return;
    bzero(Game_addr_default,128);
    fgets(Game_addr_default,128,fp);
    
    bzero(buf,128);
    fgets(buf,128,fp);
    Game_port=atoi(buf);
    
    fclose(fp);
}
#endif
