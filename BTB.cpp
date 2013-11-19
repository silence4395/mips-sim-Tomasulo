#include "BTB.h"
using namespace std;



BTB::BTB() {
  for(int i=0; i<ROW; i++){
    for(int j=0; j< COL; j++){
      BTBuffer[i][j] = -1; 
    }
  }

 for(int i=0; i<3; i++){
  LRU0[i] = 0;
  LRU1[i] = 0;
  LRU2[i] = 0;
  LRU3[i] = 0;
 } 
}

void BTB::ChangeBit(int remainder, int  LRU[3]){
  //use line 0
  if(remainder == 0){
    LRU[0] = 1;
    LRU[1] = 1;
  }
  //use line 1
  else if(remainder == 1){
    LRU[0] = 1;
    LRU[1] = 0;
  }
  //use line 2
  else if(remainder == 2){
    LRU[0] = 0;
    LRU[2] = 1;
  }
  //use line 3
  else if(remainder == 3){
    LRU[0] = 0;
    LRU[2] = 0;
  }
}
void BTB::ChangeLRU(int insaddr){
  for(int i=0; i<ROW; i++){
    if(BTBuffer[i][0] == insaddr){
      int set = i/4;
      int linenumber;
      linenumber = i;
      // line 0-3
      if(set == 0){
        ChangeBit(linenumber, LRU0);
      }
      // line 4-7
      else if(set == 1){
        ChangeBit(linenumber-4, LRU1);
      }
      //line 8-11
      else if(set == 2){
        ChangeBit(linenumber-8, LRU2);
      }
      // line 12-15
      else if(set == 3){
        ChangeBit(linenumber-12, LRU3);
      }
    } 
  }
}
int BTB::checkBTB(int insaddr, int targetaddr){
  int nextPC;
  for(int i=0; i<ROW; i++){
    if(BTBuffer[i][0] == insaddr){
      if(BTBuffer[i][2] == 1){
        //taken, return target
        return BTBuffer[i][1];
      }
      else if (BTBuffer[i][2] == 0){
        return insaddr+4;
      }
    
    }
  }
  int set;
  set = (insaddr/4) % 4;
  int line;
  if(set == 0){
    line = 0;
  }
  if(set == 1) {
    line = 4;
  }
  else if(set == 2) {
    line = 8;
  }
  else if(set == 3){ 
    line = 12;
  }

  bool flag = false;
  int insertline;

  for(int i=line; i<line+4; i++){
    if(BTBuffer[line][0] == -1 && flag == false){
      flag = true;
      insertline = i;
    }
    else if(BTBuffer[line][0] != -1 && flag == false){
      insertline = -1;
    }
  }
  //entry not occupied
  if(insertline != -1){
    BTBuffer[insertline][0] = insaddr;
    BTBuffer[insertline][1] = targetaddr;
  }
      //entry occupied
  else if(insertline == -1){
    UpdatePredictor(insaddr, targetaddr, -1);
  }
  return insaddr+4;
}


void BTB::ChangePredictor(int insaddr, bool taken){
  int predictor;
  if(taken == true) predictor = 1;
  else if(taken == false) predictor = 0;
  for(int i=0; i<ROW; i++){
    if(BTBuffer[i][0] == insaddr){
      BTBuffer[i][2] = predictor;
    } 
  }
}


int BTB::GetLRULineinSet(int LRU[3]){
  int lineinset;
  if(LRU[0] == 0 && LRU[1] == 0){
    lineinset = 0;
  }
  else if(LRU[0] == 0 && LRU[1] == 1){
    lineinset = 1;
  }
  else if(LRU[0] == 1 && LRU[2] == 0){
    lineinset = 2;
  }
  else if(LRU[0] == 1 && LRU[2] == 1){
    lineinset = 3;
  }
  return lineinset;
}

int BTB::GetLRULine(int insaddr){
  //set 0
  
  int set = (insaddr/4) % 4;
  int line;
  if(set == 0){
    line = GetLRULineinSet(LRU0);
  }
  else if(set == 1){
    line = GetLRULineinSet(LRU1) + 4;
  }
  else if(set == 2){
    line = GetLRULineinSet(LRU0) + 8;
  }
  else if(set == 3){
    line = GetLRULineinSet(LRU0) + 12;
  }
  return line;
}

void BTB::UpdatePredictor(int insaddr, int targetaddr, int predictor){
  int linenumber = GetLRULine(insaddr);
  BTBuffer[linenumber][0] = insaddr;
  BTBuffer[linenumber][1] = targetaddr;
  BTBuffer[linenumber][2] = predictor;
}

int BTB::checkpredictor(int insaddr){
  int result;
  for(int i=0; i< ROW; i++){
    if(BTBuffer[i][0] == insaddr){
      result = BTBuffer[i][2];
    }
  }
  return result;
}

int BTB::checktargetaddr(int insaddr){
  int result;
  for(int i=0; i< ROW; i++){
    if(BTBuffer[i][0] == insaddr){
      result = BTBuffer[i][1];
    }
  }
  return result;
}



