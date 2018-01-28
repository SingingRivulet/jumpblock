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
function jdw_player_abs(x,y,f,c){
  var p=jdw_abs2scr(x,y);
  jdw_player_scr(p.x,p.y,f,c);  
}

function jdw_all_block(){}
function jdw_all_player(){}
