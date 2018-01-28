var jubk_map=[];
var jubk_map_size={"x":0,"y":0};
var jubk_player={};
var jubk_ws;
var jubk_me=[
  "null", //name
  0,0,    //position
  0,      //hp
  0       //pow
];

function jubk_setname(n){
  jubk_me[0]=n;
}
function jubk_setme(v1,v2,v3,v4){
  jubk_me[1]=v1;
  jubk_me[2]=v2;
  jubk_me[3]=v3;
  jubk_me[4]=v4;
}

function jubk_send(msg){
  jubk_ws.send(msg);
}

function jubk_time(){
  return new Date().time();
}

function jubk_addplayer(unm){
  jubk_player[unm]=[
    0,0 //position
    0,  //color
    0,  //face to
    jubk_time()
  ];
}

function jubk_face(unm,fc){
  jubk_player[unm][3]=fc;
}

function jubk_quit(){
  jubk_send("quit");
}

function jubk_quitplayer(unm){
  var x=jubk_player[unm][0];
  var y=jubk_player[unm][1];
  jubk_map[x][y][2]="";
  delete jubk_player[unm];
}

function jubk_createmap(x,y){
  jubk_map_size.x=x;
  jubk_map_size.y=y;
  for(var i=0;i<x;i++){
    jubk_map[i]=[];
    for(var j=0;j<y;j++){
      jubk_map[i][j]=[
        "",//owner
        0, //obj
        "" //player
      ];
    }
  }
}
function jubk_setmapown(x,y,o){
  if(x<0)return;
  if(y<0)return;
  if(x>jubk_map_size.x)return;
  if(y>jubk_map_size.y)return;
  jubk_map[x][y][0]=o;
}

function jubk_setmapobj(x,y,o){
  if(x<0)return;
  if(y<0)return;
  if(x>jubk_map_size.x)return;
  if(y>jubk_map_size.y)return;
  jubk_map[x][y][1]=o;
}

function jubk_moveplayerto(unm,x,y){
  if(x<0)return;
  if(y<0)return;
  if(x>jubk_map_size.x)return;
  if(y>jubk_map_size.y)return;
  
  try{
    var px=jubk_player[unm][0];
    var py=jubk_player[unm][1];
    
    if(px<0)return;
    if(py<0)return;
    if(px>jubk_map_size.x)return;
    if(py>jubk_map_size.y)return;
    
    jubk_map[px][py][2]="";//set player=0;
    
    jubk_player[unm][0]=x;
    jubk_player[unm][1]=y;
    jubk_player[unm][4]=jubk_time();
    
    jubk_map[x][y][2]=unm;
    jubk_map[x][y][1]=0;
    jubk_map[x][y][0]=unm;
    
  }catch(e){}
}

var jubk_lastf;
function jubk_walk(f){
  if(f==jubk_lastf)return;
  jubk_lastf=f;
  jubk_send("walk "+f);
}

function jubk_put(i){
  jubk_send("put "+i);
}

function jubk_onmsg(m){
  
  var s=m.split(" ");
  
  if(s[0]=="addplayer"){
    jubk_addplayer(s[1]);
  }
  if(s[0]=="cremap"){
    jubk_createmap(
      parseInt(s[1]),parseInt(s[2])
    );
  }else
  if(s[0]=="quit"){
    jubk_quitplayer(s[1]);
  }else
  if(s[0]=="move"){
    jubk_moveplayerto(
      s[1],
      parseInt(s[2]),parseInt(s[3])
    );
  }else
  if(s[0]=="face"){
    jubk_face(s[1],parseInt(s[2]));
  }else
  if(s[0]=="setme"){
    jubk_setme(
      parseInt(s[1]),
      parseInt(s[2]),
      parseInt(s[3]),
      parseInt(s[4])
    );
  }else
  if(s[0]=="setobj"){
    jubk_setmapobj(
      parseInt(s[1]),parseInt(s[2]),
      parseInt(s[3])
    );
  }else
  if(s[0]=="setown"){
    jubk_setmapown(
      parseInt(s[1]),parseInt(s[2]),
      s[3]
    );
  }else
  if(s[0]=="setname"){
    jubk_setname(s[1]);
  }else
  if(s[0]=="exit"){
    
  }
}

function jubk_conn(addr){
  jubk_ws=new WebSocket(addr); 
  jubk_ws.onmessage = function(evt){
    jubk_onmsg(evt.data);
  };
}
