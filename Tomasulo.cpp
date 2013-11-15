#include <iostream>
#include <fstream>
#include "Tomasulo.h"
#include <cstring>
using namespace std;

void Tomasulo::parseinputfile(string filename){
    ifstream infile;
    infile.open(filename.c_str(), ifstream::in);
    string line;
    while(getline(infile, line)){
    //parse instruction line
      if(line.size() > 38){
        int memaddr = StoI(line.substr(38, 3));
        string instruction = line.substr(42);
        AddrInspair aip;
        aip.insaddr = memaddr;
        aip.instruction = instruction;
        addrinslist->push_back(aip);
        inscounter++;
      }
      //parse dataline, store data into main memory
      else if(line.size() == 38){
        int dataaddr = StoI(line.substr(33,3));
        if(dataaddr >= 716){
          int data = StoI(line.substr(37));
          Mem[(dataaddr-716)/4] = data;
          datacounter++;
        }
      }
    }

}

Instruction Tomasulo::parseinstruction(string instruction){
  Instruction ins;
  ins.op = "NONE";
  ins.rd = "NONE";
  ins.rs = "NONE";
  ins.rt = "NONE";
  vector<string> wordVector;
  size_t prev = 0, pos;
  while ((pos = instruction.find_first_of(" ,()", prev)) != std::string::npos) {
    if (pos > prev){
      wordVector.push_back(instruction.substr(prev, pos-prev));
    }
    prev = pos+1;
  }
  if(prev < instruction.length() - 1){
    wordVector.push_back(instruction.substr(prev, std::string::npos));
  }
  string op = wordVector[0];
  ins.op = wordVector[0];
  if(!op.compare("SW") || !op.compare("LW")){
    ins.rd = wordVector[1]; //destination
    ins.rt = wordVector[2]; //imm
    ins.rs = wordVector[3]; //source
  }
  else if(!op.compare("ADDI") || !op.compare("ADDIU") || !op.compare("SLTI")){
    ins.rd = wordVector[1]; //dest
    ins.rs = wordVector[2]; //source
    ins.rt = wordVector[3].substr(1); //imm
  }
  else if(!op.compare("BEQ") || !op.compare("BNE")){
    ins.rs = wordVector[1]; //source 1
    ins.rt = wordVector[2]; //source 2
    ins.rd = wordVector[3].substr(1); //offset
  }
  else if(!op.compare("BGEZ") || !op.compare("BGTZ") || !op.compare("BLEZ") || !op.compare("BLTZ")){
    ins.rs = wordVector[1]; //source, compare to 0
    ins.rt = wordVector[2].substr(1); //offset
  }
  else if(!op.compare("J")){
    ins.rs = wordVector[1].substr(1);//target
  }
  else if(!op.compare("SLL") || !op.compare("SRA") || !op.compare("SRL")){
    ins.rd = wordVector[1]; //dest
    ins.rt = wordVector[2]; //source
    ins.rs = wordVector[3].substr(1); //sa, shift
  }
  else if(!op.compare("NOP") || !op.compare("BREAK")){}
  else {
    ins.rd = wordVector[1]; //dest
    ins.rs = wordVector[2]; //source 1
    ins.rt = wordVector[3]; //source 2
  }
  return ins;
}

int Tomasulo::caltarget(int PC, string instruction, string op){
  int targetaddr;  
  int pos, prev = 0;
    //get number from instruction
  if ((pos = instruction.find_first_of("#", prev)) != std::string::npos) {
    if (pos > prev){
      targetaddr = StoI(instruction.substr(pos+1));
    }
  }
  if(op.compare("J")){
    targetaddr = targetaddr +PC +4;
  }
  return targetaddr;
}

//PC = insaddr
int Tomasulo::insfetch(int PC, string instruction){
    Instruction ins;
    int nextPC;
    int targetaddr = 0;
 
 

    if(DEBUG){
      cout << "PC: " << PC << endl;
    }
  //TODO: fetch only 1 ins
    ins = parseinstruction(instruction);
    
    IQ->push_back(ins);
    
    inscounter--; //count down to fetched instruction
    cout << "Inscounter " <<inscounter << endl;
    
    //if branches or jump, fetch nextPC from BTB
    if(!ins.op.compare("BNE")|| !ins.op.compare("BEQ") || !ins.op.compare("BGEZ") || !ins.op.compare("BGTZ") || !ins.op.compare("BLTZ") ||!ins.op.compare("BLEZ") ||!ins.op.compare("J")){
      targetaddr = caltarget(PC, instruction, ins.op); 
      nextPC = btbuffer->checkBTB(PC, targetaddr);
    }
    //other ops
    else {
      nextPC = PC + 4;
    }
    

    fetchcounter++;  //indicate next fetch instruction

    if(DEBUG){
      cout << "nextPC: " << nextPC << endl;
      cout << "fetchcounter " <<fetchcounter << endl;
    }
    return nextPC;
}



//check if inscounter = 0, if yes, no more instruction to fetch
bool Tomasulo::checkfetch(){
  if(inscounter == 0){
    return false;
  }
  return true;
}

//check if RScounter = 0, if yes, stall.
bool Tomasulo::rs_available(){
  if(RSlimit == 0){
    return false;
  }
  return true;
}


//check if ROBcounter = 0, if yes, stall.
bool Tomasulo::rob_available(){
  if(ROBlimit == 0){
    return false;
  }
  return true;
}
void Tomasulo::issuevj(RScontent * rs_entry, int rs, int rs_robid, bool value_ready, int rs_robvalue){

    if((*Register)[rs].Busy){


      rs_robid = (*Register)[rs].h; //give reorder buffer number
      deque<ROBcontent>::iterator robit;
      //check if value is ready in ROB: ROB_status queue
      for(robit = ROB->begin(); robit != ROB->end(); robit++){
        if((*robit).Entry == rs_robid){
          value_ready = (*robit).Ready;
          rs_robvalue = (*robit).value;
        }
      }
      if(value_ready){
        //give value to Vj in Rs, set Qj = 0
        rs_entry->Vj = rs_robvalue;
        rs_entry->Vj_ok = true;
        rs_entry->Qj = 0;
      }
      else {
        //if not ready, give the reorder buffer id, which hold the value to Qj
        rs_entry->Qj = rs_robid;
        rs_entry->Vj_ok = false;
        rs_entry->Vj = 1111;  
      } 
    }
    //if reg[rs] is not busy, value available
    else {
      rs_entry->Vj = (*Register)[rs].value;
      rs_entry->Vj_ok = true;
      rs_entry->Qj = 0;
    }
}

void Tomasulo::issuevk(RScontent * rs_entry, int rt, int rt_robid, bool value_ready, int rt_robvalue){
  if((*Register)[rt].Busy){
    rt_robid = (*Register)[rt].h;
    deque<ROBcontent>::iterator robit;
      //check if value is ready in ROB: ROB_status queue
      for(robit = ROB->begin(); robit != ROB->end(); robit++){
        if((*robit).Entry == rt_robid){
          value_ready = (*robit).Ready;
          rt_robvalue = (*robit).value;
        }
      }
      if(value_ready){
        //give value to Vj in Rs, set Qj = 0
        rs_entry->Vk = rt_robvalue;
        rs_entry->Vk_ok = true;
        rs_entry->Qk = 0;
      }
      else {
        //if not ready, give the reorder buffer id, which hold the value to Qj
        rs_entry->Qk = rt_robid;
        rs_entry->Vk_ok = false;
        rs_entry->Vk = 1111;  
      } 
    }
    //if reg[rs] is not busy, value available
    else {
      rs_entry->Vk = (*Register)[rt].value;
      rs_entry->Vk_ok = true;
      rs_entry->Qk = 0;
    }
}
//add instruction into RS
void Tomasulo::rs_rob_add(Instruction instruction){
  string op = instruction.op;
  RScontent rs_entry;
  ROBcontent rob_entry;
  int rd, rs, rt, imm, A, offset;
  int rob_id, rs_robid, rt_robid, rs_robvalue, rt_robvalue;
  bool value_ready;

  //rs, rob content, unified
  rob_entry.Entry = ROBcounter;
  rob_entry.state = Issue;
  rob_entry.value = 1111; //initialize rob value
  rob_entry.Ready = false;
  rs_entry.op = op; //opcode
  rob_entry.op = instruction.op;
  rob_id = ROBcounter;
  
  if(!op.compare("SW")){
    rt = StoI(instruction.rt);
    rs = StoI(instruction.rs.substr(1));
    rd = StoI(instruction.rd.substr(1));

    rob_entry.ins = instruction.op + " " + instruction.rd + ", " + instruction.rt + "(" + instruction.rs + ")";
    rs_entry.ins = instruction.op + " " + instruction.rd + ", " + instruction.rt + "(" + instruction.rs + ")";
    //insert vj: source value for addr cal
    issuevj(&rs_entry, rs, rs_robid, value_ready, rs_robvalue);    
    //insert vk: source value to be stored
    issuevk(&rs_entry, rd, rt_robid, value_ready, rt_robvalue);
 
    rs_entry.Busy = true;
    //set rs entry's destination to be rob entry id
    rs_entry.Dest = rob_id;

    //destination is vj+a
    rob_entry.Dest = instruction.rt + "+" + instruction.rs;

    rs_entry.A = rt; 
  }
  
  else if (!op.compare("LW")){
    rt = StoI(instruction.rt);
    rs = StoI(instruction.rs.substr(1));
    rd = StoI(instruction.rd.substr(1));

    rob_entry.ins = instruction.op + " " + instruction.rd + ", " + instruction.rt + "(" + instruction.rs + ")";
    rs_entry.ins = instruction.op + " " + instruction.rd + ", " + instruction.rt + "(" + instruction.rs + ")";
    //insert vj, source value for addr cal
    issuevj(&rs_entry, rs, rs_robid, value_ready, rs_robvalue);    
    //no vk
    rs_entry.Vk = 1111;
    rs_entry.Vk_ok = true;
    rs_entry.Qk = 0;
    //other inserts
    rs_entry.Busy = true;
    rs_entry.Dest = rob_id;

    //destination register
    rob_entry.Dest = instruction.rd;
    (*Register)[rd].h = rob_id;
    (*Register)[rd].Busy = true;
    //addr offset, load from vj+a mem address
    rs_entry.A = rt;
  }
  else if(!op.compare("ADDI") || !op.compare("ADDIU") || !op.compare("SLTI")){
    rt = StoI(instruction.rt);  //imm
    rs = StoI(instruction.rs.substr(1));//source register
    rd = StoI(instruction.rd.substr(1));//destination register

    rob_entry.ins = instruction.op + " " + instruction.rd + ", " + instruction.rs + ", #" + instruction.rt;
    rs_entry.ins = instruction.op + " " + instruction.rd + ", " + instruction.rs + ", #" + instruction.rt;
    //insert vj: srouce value 1 
    issuevj(&rs_entry, rs, rs_robid, value_ready, rs_robvalue);    
    rs_entry.Vk = rt;
    rs_entry.Vk_ok = true;
    rs_entry.Qk = 0;

    //other inserts
    rs_entry.Busy = true;
    rs_entry.Dest = rob_id;

    //destination register
    rob_entry.Dest = instruction.rd;
    (*Register)[rd].h = rob_id;
    (*Register)[rd].Busy = true;
    
    rs_entry.A = 1111;
  }
  else if(!op.compare("BEQ") || !op.compare("BNE")){
    rd = StoI(instruction.rd);  //offset
    rs = StoI(instruction.rs.substr(1));//source register 1
    rt = StoI(instruction.rd.substr(1));//source  register 2

    rob_entry.ins = instruction.op + " " + instruction.rs + ", " + instruction.rt + "#" + instruction.rd;
    rs_entry.ins = instruction.op + " " + instruction.rs + ", " + instruction.rt + "#" + instruction.rd;
    
    //insert vj: srouce value 1 
    issuevj(&rs_entry, rs, rs_robid, value_ready, rs_robvalue);    
    issuevk(&rs_entry, rt, rt_robid, value_ready, rt_robvalue);    

    //other inserts
    rs_entry.Busy = true;
    rs_entry.Dest = rob_id;

    //destination register
    rob_entry.Dest = "#" + instruction.rd;
    
    //branch offset
    rs_entry.A = rd;
  }
  
  //compare to 0
  else if(!op.compare("BGEZ") || !op.compare("BGTZ") || !op.compare("BLEZ") || !op.compare("BLTZ")){
    rt = StoI(instruction.rd);  //offset
    rs = StoI(instruction.rs.substr(1));//source register 1

    rob_entry.ins = instruction.op + " " + instruction.rs + ", " + "#" + instruction.rt;
    rs_entry.ins = instruction.op + " " + instruction.rs + ", " + "#" + instruction.rt;
    
    //insert vj: srouce value 1 
    issuevj(&rs_entry, rs, rs_robid, value_ready, rs_robvalue);    

    //other inserts
    rs_entry.Busy = true;
    rs_entry.Dest = rob_id;

    //branch offset value
    rs_entry.Vk = rt;
    rs_entry.Vk_ok = true;
    rs_entry.Qk = 0;

    //destination register
    rob_entry.Dest = "#" + instruction.rt;
    rs_entry.A = 1111; 

  }
  else if(!op.compare("J")){
    rs = StoI(instruction.rs);
    rob_entry.ins = instruction.op + " #" + instruction.rs;
    rs_entry.ins = instruction.op + " #" + instruction.rs;
    

    //other inserts
    rs_entry.Busy = true;
    rs_entry.Dest = rob_id;

    //jump target
    rs_entry.Vj = rs;
    rs_entry.Vj_ok = true;
    rs_entry.Qj = 0;
    
    rs_entry.Vk = 1111;
    rs_entry.Vk_ok = true;
    rs_entry.Qk = 0;

    //destination register
    rob_entry.Dest = "#" + instruction.rs;
    rs_entry.A =1111;
  }
  else if(!op.compare("SLL") || !op.compare("SRA") || !op.compare("SRL")){
    rs = StoI(instruction.rs);//shift sa
    rt = StoI(instruction.rt.substr(1));//source register
    rd = StoI(instruction.rd.substr(1));//dest register


    rob_entry.ins = instruction.op + " " + instruction.rd +", " + instruction.rt +", " + instruction.rs;
    rs_entry.ins = instruction.op + " " + instruction.rd +", " + instruction.rt +", " + instruction.rs;
    
    //insert vj: srouce value 1 
    issuevj(&rs_entry, rt, rt_robid, value_ready, rt_robvalue);    

    //other inserts
    rs_entry.Busy = true;
    rs_entry.Dest = rob_id;
    rs_entry.Vk = rs;
    rs_entry.Vk_ok = true;
    rs_entry.Qk = 0;

    //destination register
    rob_entry.Dest = instruction.rd;
    rs_entry.A = 1111; 
    (*Register)[rd].h = rob_id;
    (*Register)[rd].Busy = true;


  }
  else if(!op.compare("NOP") || !op.compare("BREAK")){
  
    rob_entry.ins = instruction.op;
    rs_entry.ins = instruction.op;
  
  //other inserts
    rs_entry.Busy = true;
    rs_entry.Dest = rob_id;
    rs_entry.Vk = 1111;
    rs_entry.Vk_ok = true;
    rs_entry.Qk = 0;
    rs_entry.Vj = 1111;
    rs_entry.Vj_ok = true;
    rs_entry.Qj = 0;

    //destination register
    rs_entry.A = 1111;
    rob_entry.Dest = op;
  }
  else {
    rs = StoI(instruction.rs.substr(1));//source 1
    rt = StoI(instruction.rt.substr(1));//source register 2
    rd = StoI(instruction.rd.substr(1));//dest register


    rob_entry.ins = instruction.op + " " + instruction.rd +", " + instruction.rs +", " + instruction.rt;
    rs_entry.ins = instruction.op + " " + instruction.rd +", " + instruction.rs +", " + instruction.rt;
    
    //insert vj: srouce value 1 
    issuevj(&rs_entry, rs, rs_robid, value_ready, rs_robvalue);    
    issuevk(&rs_entry, rt, rt_robid, value_ready, rt_robvalue);    

    //other inserts
    rs_entry.Busy = true;
    rs_entry.Dest = rob_id;

    //destination register
    rob_entry.Dest = instruction.rd;
    rs_entry.A = 1111; 
    (*Register)[rd].h = rob_id;
    (*Register)[rd].Busy = true;


  }
  RS->push_back(rs_entry);
  ROB->push_back(rob_entry);
  ROBcounter++;
  ROBlimit--;
  RScounter++;
  RSlimit--;
}

int Tomasulo::calalu(string op, int Vj, int Vk){
  int result;
  if(!op.compare("SLTI")){
    //If rs < imm, rt = 1; else rt =0
    if(Vj < Vk){
      result = 1;
    }
    result = 0;
  }
  else if(!op.compare("SLL")){
    result = (int)((unsigned int)Vj << Vk);
  }
  else if(!op.compare("SRA")){
    result = Vj >> Vk;
  }
  else if(!op.compare("SRL")){
    result = (int)(((unsigned int)Vj) /pow((double)2, (double)Vk));
  }
  else {
    result = Vj + Vk;
  }
  return result;
}

int Tomasulo::caladdr(int Vj, int A){
  int result;
  //Vj+A
  result = Vj + A;
  return result;
}

bool Tomasulo::calbranch(string op, int Vj, int Vk){
  bool result;
  if(!op.compare("BEQ")){
    if(Vj == Vk){
      result = true;
    }
    result = false;
  }
  else if(!op.compare("BNE")){
    if(Vj != Vk){
      result = true;
    }
    result = false;
  }
  else if(!op.compare("BGEZ")){
    if(Vj >= 0){
      result = true;
    }
    result = false;
  }
  else if(!op.compare("BGTZ")){
    if(Vj > 0) result = true;
    result = false;
  }
  else if(!op.compare("BLEZ")){
    if(Vj <= 0) result = true;
    result = false;
  }
  else if(!op.compare("BLTZ")){
    if(Vj < 0) result = true;
    result = false;
  }
  else if(!op.compare("J")){
    result = true;
  }
  return result;
}

void Tomasulo::print_status(int cycle){
      cout << "Cycle <" << cycle << ">" << endl;
      cout << "IQ:" << endl;
      deque<Instruction>::iterator iqiter;
      for(iqiter = IQ->begin(); iqiter != IQ->end(); iqiter++){
          cout << (*iqiter).op << " "<<(*iqiter).rd<<" " << (*iqiter).rs<<" " << (*iqiter).rt << endl;
      }
      cout << endl;
      cout << "RS:" << endl;
      deque<RScontent>::iterator rsiter;
      for(rsiter = RS->begin(); rsiter != RS->end(); rsiter++){
          cout << (*rsiter).ins<<" " << (*rsiter).Vk << endl;
      }
      cout << endl;
      cout << "ROB:" << endl;
      deque<ROBcontent>::iterator robiter;
      for(robiter = ROB->begin(); robiter != ROB->end(); robiter++){
          cout << (*robiter).ins<<" " << (*robiter).Dest << " " << (*robiter).state  << endl;
      }
      cout << endl;
      cout << "BTB: " << endl;
      for(int i=0; i<ROW; i++){
        if(btbuffer->BTBuffer[i][0] != -1){
          cout << "[Entry " << i << "]:" << btbuffer->BTBuffer[i][0] << " " << btbuffer->BTBuffer[i][1] << " " << btbuffer->BTBuffer[i][2] << endl;
        }  
      }

      cout << endl;
      //TODO
      cout << ""<< endl;
      cout << "---------------------------------------------" << endl;
    

}