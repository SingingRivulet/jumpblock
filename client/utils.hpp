#ifndef jubk_game_utils
#define jubk_game_utils
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
#include <mutex>
#include <atomic>
#include "config.hpp"
using namespace std;
int connfd;
atomic<bool> gameover;
struct vec{
  int x;
  int y;
};
struct Color{
  uint8_t r,g,b;
  Color()=default;
  Color(const Color&)=default;
  Color & operator=(const Color & in){
      r=in.r;
      g=in.g;
      b=in.b;
      return *this;
  }
  Color & set(int ir,int ig,int ib){
      r=ir;
      g=ig;
      b=ib;
      return *this;
  }
};
double gettm(){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return (((double)tv.tv_sec) + ((double)tv.tv_usec / 1000000.0));
}
#endif
