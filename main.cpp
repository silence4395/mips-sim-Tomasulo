#include "Tomasulo.h"
using namespace std;

int main(int argc, char ** argv){
  string filename = "out.txt";
  Tomasulo t;
  t.parseinputfile(filename);

  bool more_ins = true; //more ins to fetch
  bool not_finished = true; //still execution ins
  int PC = 600;
  int nextPC;
  int cycle = 1;
  
  
  if(DEBUG){
    cout << "Cycle <" << cycle << ">" << endl;
  }
 
  //fetch the first cycle
  if(t.checkfetch()){
    string instruction;
    more_ins = true;
    vector<AddrInspair>::iterator aiter;
      for(aiter = t.addrinslist->begin(); aiter != t.addrinslist->end(); aiter++){
        if((*aiter).insaddr == PC){
          instruction = (*aiter).instruction;  
        }
      }
      nextPC = t.insfetch(PC, instruction);  //fetch one instruction
  }
  PC = nextPC;

  t.print_status(cycle);

  cycle++;
    
  //begin cycle by cycle implementation    
  while(more_ins || not_finished){
   // cout << "Cycle <" << cycle << ">" << endl; 
    more_ins = false;
    not_finished = false;
    //check if there are more instruction to fetch
    /*-----------------------------
     *-----Fetch-------------------
     * ---------------------------*/
    if(t.checkfetch()){
      string instruction;
      more_ins = true;
      vector<AddrInspair>::iterator aiter;
      for(aiter = t.addrinslist->begin(); aiter != t.addrinslist->end(); aiter++){
        if((*aiter).insaddr == PC){
          instruction = (*aiter).instruction;  
        }
      }
      nextPC = t.insfetch(PC, instruction);  //fetch one instruction, return nextPC to fetch
    }
    /*-----------------------------
     *-----Issue-------------------
     * ---------------------------*/
    if(t.rs_available() && t.rob_available()){
      t.rs_rob_add(t.IQ->front());
      t.IQ->pop_front();
    }



    /*-----------------------------
     *-----Exe-------------------
     * ---------------------------*/
    



    /*-----------------------------
     *-----Write Result------------
     * ---------------------------*/




    /*-----------------------------
     *-----Commit-------------------
     * ---------------------------*/
    PC = nextPC;
  
    t.print_status(cycle);

    cycle++;
   }
  return 0;
}

