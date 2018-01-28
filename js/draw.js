//screen:11*11 blocks
var jdw_con;
var jdw_camera={"x":0,"y":0};

function jdw_init(){
  jdw_con=document.getElementById("mycanvas").getContext('2d');
}
function jdw_block_scr(x,y,c){
  jdw_con.fillStyle=c;
  jdw_con.strokeStyle = "black";
  jdw_con.lineWidth = 1;
  jdw_con.fillRect  (x-5,y-5,x+5,y+5);
  jdw_con.strokeRect(x-5,y-5,x+5,y+5);
}
function jdw_player_scr(x,y,f,c){
  jdw_con.fillStyle=c;
  jdw_con.strokeStyle = "black";
  jdw_con.lineWidth = 1;
  jdw_con.arc(x,y,9,0, Math.PI*2, true);
  jdw_con.lineWidth = 4;
  jdw_con.moveTo(x,y);
  if(f==0){
    jdw_con.lineTo(x+5,y);
  }else
  if(f==1){
    jdw_con.lineTo(x,y+5);  
  }else
  if(f==2){
    jdw_con.lineTo(x-5,y);
  }else
  if(f==3){
    jdw_con.lineTo(x,y-5);
  }
}

function jdw_abs2scr(x,y){
  var res={};
  var tx=(jdw_camera.x-6);
  var ty=(jdw_camera.y-6);
  res.x=(x-tx)*10;
  res.y=(y-ty)*10;
  return res;
}

function jdw_block_abs(x,y,c){
  var p=jdw_abs2scr(x,y);
  jdw_block_scr(p.x,p.y,c);
}
function jdw_player_abs(x,y,f,c,t){
  var p=jdw_abs2scr(x,y);
  jdw_player_scr(p.x,p.y,f,c);  
}

function jdw_all_block(){
  var cx=Math.floor(jdw_camera.x);
  var cy=Math.floor(jdw_camera.y);
  var bx=cx-6;
  var by=cy-6;
  var ex=cx+6;
  var ey=cy+6;
  for(var x=bx;x<ex;x++){
    for(var y=by;y<ey;y++){
      
      if(x<0)continue;
      if(y<0)continue;
      if(jubk_map_size.x<x)continue;
      if(jubk_map_size.y<y)continue;
      
      var bk=jubk_map[x][y];
      
      var owner=bk[0];
      if(owner){
        try{
          var pl=jubk_player[owner];
          if(pl){
            jdw_block_abs(x,y,pl[2]);
          }
        }catch(e){}
      }
      
      var player=bk[2];
      if(player){
        try{
          var pt=jubk_player[player];
          if(pl){
            jdw_player_abs(x,y,pt[3],pt[2],jubk_time()-pt[4]);
          }
        }catch(e){}
      }
    }
  }
}
