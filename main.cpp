#include "Tomasulo.h"
#include <algorithm>
#include "assembly.h"
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
  if(t.checkfetch(PC)){
    string instruction;
    more_ins = true;
    vector<AddrInspair>::iterator aiter;
      for(aiter = t.addrinslist->begin(); aiter != t.addrinslist->end(); aiter++){
        if((*aiter).insaddr == PC){
          instruction = (*aiter).instruction;  
        }
      }
      nextPC = t.insfetch(cycle, PC, instruction);  //fetch one instruction
  }
  PC = nextPC;

  t.print_status(cycle);

  cycle++;
    
  //begin cycle by cycle implementation    
  while(not_finished){
    cout << "Cycle <" << cycle << "> starts here!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl; 
    
    
   // cout << "***********fetching PC: "<< PC << endl;
    
    not_finished = false;
    //check if there are more instruction to fetch
    /*-----------------------------
     *-----Fetch-------------------
     * ---------------------------*/
   if(t.checkfetch(PC)){
      string instruction;
      vector<AddrInspair>::iterator aiter;
      for(aiter = t.addrinslist->begin(); aiter != t.addrinslist->end(); aiter++){
        if((*aiter).insaddr == PC){
          instruction = (*aiter).instruction;  
        }
      }
    cout << "***********fetching PC in fetch: "<< PC << endl;
      nextPC = t.insfetch(cycle, PC, instruction);  //fetch one instruction, return nextPC to fetch
    cout << "***********nextPC: "<< t.IQ->front().op << nextPC << endl;
   }

    /*-----------------------------
     *-----Issue-------------------
     * ---------------------------*/
    if(t.rs_available() && t.rob_available() && !t.IQ->empty() && t.IQ->front().push_cycle < cycle){

       
    cout << "**************problem?? "<< endl;
      t.rs_rob_add(t.IQ->front(), cycle);
    cout << "**************problem??? "<< endl;
      t.IQ->pop_front();
    cout << "**************problem???? "<< endl;
    }
    
    deque<RScontent>::iterator rsiter;
    for(rsiter = t.RS->begin(); rsiter != t.RS->end(); rsiter++){
      cout << "*&*&*&*&- start EXE loop: " << (*rsiter).ins << endl;
      int rob_id = (*rsiter).Dest;
      deque<ROBcontent>::iterator robiter;
      bool exe;
    
             
      for(robiter = t.ROB->begin(); robiter != t.ROB->end(); robiter++){
       // cout << "Start Second Loop" << endl;
        if((*robiter).Entry == rob_id){
       // cout << "Start if" << endl;
          
          /*-----------------------------
          *-----Exe-------------------
          * ---------------------------*/  
          //check if this instruction can be executed
          if((*robiter).op.compare("SW") && (*robiter).op.compare("LW") && (*robiter).state == Issue && (*robiter).current_status_cycle < cycle){
            exe = true;
          }
          else if((!(*robiter).op.compare("LW")|| !(*robiter).op.compare("SW")) && (*robiter).state == Issue && (*robiter).current_status_cycle < cycle){
            exe = t.checkpreviousSL(rob_id);
          }
          else {
            exe = false;
          }

            //cout << "---------------------if EXE : "  << exe << endl;
         
          //sw
          if(!(*rsiter).op.compare("SW") && (*rsiter).Qj == 0 && exe){
            int swaddr;
            swaddr = t.caladdr((*rsiter).Vj, (*rsiter).A);
            

            (*robiter).addr = swaddr;
            if((*robiter).state == Issue){
              (*robiter).state = Exe;
            }
            (*rsiter).Busy = false;
            (*robiter).current_status_cycle = cycle;
            (*rsiter).current_status_cycle = cycle;
          }
          else if((*rsiter).op.compare("SW") && (*rsiter).Qj == 0 && (*rsiter).Qk == 0 && exe){
            //cout << "---------------------Enter or not ??? "  << endl;
            if(!(*rsiter).op.compare("BNE")|| !(*rsiter).op.compare("BEQ") || !(*rsiter).op.compare("BGEZ") || !(*rsiter).op.compare("BGTZ") || !(*rsiter).op.compare("BLTZ") ||!(*rsiter).op.compare("BLEZ") ||!(*rsiter).op.compare("J")){
              
              cout << "************test test test " << endl;
              bool btaken = t.checkbranch((*rsiter).op, (*rsiter).Vj, (*rsiter).Vk); 
              cout << "************btaken "<< btaken << endl;

              int bresult, insaddr_temp;
              vector<AddrInspair>::iterator aiter;
              for(aiter = t.addrinslist->begin(); aiter != t.addrinslist->end(); aiter++){
                if((*aiter).instruction == (*rsiter).ins){
                  insaddr_temp = (*aiter).insaddr;
                }
              }


              //cout << "***********branch addr  "<< insaddr_temp << endl;
            //  bresult = t.calbranch((*rsiter).op, btaken, insaddr_temp, (*rsiter).A, (*rsiter).Vj, (*rsiter).Vk);
              int predictor;
              predictor = t.btbuffer->checkpredictor(insaddr_temp);
             cout << "************branch predictor "<< predictor << endl;
            //new entry, update it only, nextPC doesn't change
              if(predictor == -1){
                t.btbuffer->ChangePredictor(insaddr_temp, btaken);
                if(btaken == true){

             // cout << "***********test test test test "<< endl;
                  t.flushout_rs_rob(rob_id);
             // cout << "***********test test test test "<< endl;
                  t.IQ->clear();
             // cout << "***********aaaaaaaaaaaaaast test test test "<< endl;
                  nextPC = t.btbuffer->checktargetaddr(insaddr_temp);
                }
              }
              else if(predictor == 1 && btaken == false){
                nextPC = insaddr_temp + 4;
                t.btbuffer->ChangePredictor(insaddr_temp, btaken);
                t.flushout_rs_rob(rob_id);
                t.IQ->clear();
              }
              else if(predictor == 0 && btaken == true){
               cout << "**********Enter P0BT condition" << endl;
                t.flushout_rs_rob(rob_id);
               cout << "**********Enter P0BT condition, flushed out" << endl;
                t.IQ->clear();
               cout << "**********Enter P0BT condition, IQ cleared" << endl;
                t.btbuffer->ChangePredictor(insaddr_temp, btaken);
                nextPC = t.btbuffer->checktargetaddr(insaddr_temp);
               cout << "**********Enter P0BT condition: nextPC"<< nextPC << endl;
              }
              if((*robiter).state == Issue){
                (*robiter).state = Exe;
              }
              (*robiter).current_status_cycle = cycle;
              (*rsiter).current_status_cycle = cycle;
              (*robiter).Ready = true;
              (*rsiter).Busy = false;
              t.btbuffer->ChangeLRU(insaddr_temp);
             // cout << "***********nextPC  "<< nextPC << endl;
             // cout << "@@@@@@@@@@@@@@@@@Reach here!!!!!! Branch EXE end"<< endl;
            }
          //LW
            else if(!(*rsiter).op.compare("LW")){
              int lwaddr;
              lwaddr = t.caladdr((*rsiter).Vj, (*rsiter).A);
              (*robiter).addr = lwaddr;

              if((*robiter).state == Issue){
                (*robiter).state = Exe;
              }
              (*robiter).current_status_cycle = cycle;
              (*rsiter).current_status_cycle = cycle;
            }
          //alu 
            else {
              int aresult;
              aresult = t.calalu((*rsiter).op, (*rsiter).Vj, (*rsiter).Vk);      
              cout << (*rsiter).op << " " << (*rsiter).Vj << " " << (*rsiter).Vk << " result:" << aresult << endl;
              (*rsiter).value = aresult;
              if((*robiter).state == Issue){
                (*robiter).state = Exe;
              }
              (*robiter).current_status_cycle = cycle;
              (*rsiter).current_status_cycle = cycle;
            }
          }
        }
      }
        // cout << "*&*&*&*&- end EXE loop: " << t.RS->size() << endl;
    }


         cout << "*&*&*&*&- Write result start: " << t.RS->size() << endl;
          /*-----------------------------
          *-----Write Result------------
          * ---------------------------*/
      for(rsiter = t.RS->begin(); rsiter != t.RS->end();){
        // cout << "*&*&*&*&- Start cycle : " << t.RS->size() << endl;
        int rob_id = (*rsiter).Dest;
        deque<ROBcontent>::iterator robiter;
        bool WR;
        bool eraseflag = false;
    
        for(robiter = t.ROB->begin(); robiter != t.ROB->end(); robiter++){
          if((*robiter).Entry == rob_id){
            //check if this instruction can be write back
            if((*robiter).op.compare("SW") && (*robiter).op.compare("LW") && (*robiter).state == Exe && (*robiter).current_status_cycle < cycle){
              WR = true;
            }
            else if(!(*robiter).op.compare("SW") && (*robiter).state == Exe && (*robiter).current_status_cycle < cycle){
              WR = true;
            }
            else if(!(*robiter).op.compare("LW") && ((*robiter).state == Exe || (*robiter).state == WR1) && (*robiter).current_status_cycle < cycle){
              WR = true;
            }
            else {
              WR = false;
            }
   
            //  cout << "---------------------if write result??"  << WR << endl;
            if(!(*robiter).op.compare("SW") && WR){
              if((*rsiter).Qk == 0){
                (*robiter).value = (*rsiter).Vk;
                (*robiter).source_ok = true;
              }
              else {
                (*robiter).source_ok = false;
                (*robiter).source = (*rsiter).Qk;
              }
              (*robiter).Ready = true;
              if((*robiter).state == Exe){
                (*robiter).state = WR1;
              } 
              (*rsiter).Busy = false;
              (*robiter).current_status_cycle = cycle;
              (*rsiter).current_status_cycle = cycle;
            }
            else if(!(*robiter).op.compare("LW") && (*robiter).state == Exe && WR){
              bool ifstall;
              ifstall = t.checkROB(rob_id, (*robiter).addr);
              //no address dependency
              if(ifstall == false){
                (*rsiter).value = t.Mem[((*robiter).addr-716)/4];//base 716
                (*robiter).state = WR1;
                (*robiter).current_status_cycle = cycle;
                (*rsiter).current_status_cycle = cycle;
              } 
            }
            else if(!(*robiter).op.compare("LW") && (*robiter).state == WR1 && WR){
              (*robiter).value = (*rsiter).value;
              t.updateRS(rob_id, (*rsiter).value);
              (*robiter).Ready = true;
              (*robiter).state = WR2;
              (*rsiter).Busy = false;
              (*robiter).current_status_cycle = cycle;
              (*rsiter).current_status_cycle = cycle;
            }
            else if((!(*rsiter).op.compare("BNE")|| !(*rsiter).op.compare("BEQ") ||
                 !(*rsiter).op.compare("BGEZ") || !(*rsiter).op.compare("BGTZ") ||
                 !(*rsiter).op.compare("BLTZ") || !(*rsiter).op.compare("BLEZ") ||
                 !(*rsiter).op.compare("J")) && WR){
              //simply erase this entry in RS
              cout << "simply erase this entry in RS" << t.RS->size()<< endl;
              rsiter = t.RS->erase(rsiter);
              eraseflag = true;
              cout << "simply erase this entry in RS" << t.RS->size()<< endl;
              break;
            }
            else if(WR) {
              (*robiter).value = (*rsiter).value;
              if((*robiter).state == Exe){
                (*robiter).state = WR1;
              }
              (*robiter).current_status_cycle = cycle;
              (*rsiter).current_status_cycle = cycle;
              (*robiter).Ready = true;
              (*rsiter).Busy = false;
              t.updateRS(rob_id, (*rsiter).value);
            }
          }
        } // inner for loop
        if (!eraseflag) {
          rsiter ++;
        }
       // cout << "*&*&*&*&- End cycle" << endl;
      }
    /*-----------------------------
     *-----Commit-------------------
     * ---------------------------*/
       cout << "***********enter Commit :)" << endl;
    deque<RScontent>::iterator deiter;
      for(deiter = t.RS->begin(); deiter != t.RS->end();){
       // cout << "------------------------" << (*deiter).Busy << "/" << (*deiter).current_status_cycle << "/" << (*deiter).ins << endl;
        if((*deiter).Busy == false && (*deiter).current_status_cycle < cycle){
        // cout << "***************" << endl;
         // //check if SW's source value available
          deiter = t.RS->erase(deiter);
        }
        else {
          deiter++;
        }
      }
    

    
    if(t.ROB->front().state == Commit && t.ROB->front().current_status_cycle < cycle){
      //TODO: do sth
      if(!t.ROB->front().op.compare("SW") && t.ROB->front().source_ok == true){

        t.Mem[(t.ROB->front().addr - 716)/4] = t.ROB->front().value;

        t.updatelaterLW(t.ROB->front().addr, cycle);
      }
      else if(t.ROB->front().op.compare("BNE") && t.ROB->front().op.compare("BEQ") &&
                 t.ROB->front().op.compare("BGEZ") && t.ROB->front().op.compare("BGTZ") &&
                 t.ROB->front().op.compare("BLTZ") && t.ROB->front().op.compare("BLEZ") &&
                 t.ROB->front().op.compare("J")){
        int reg_id;
        reg_id = StoI(t.ROB->front().Dest.substr(1));
        (*t.Register)[reg_id].value = t.ROB->front().value;
        (*t.Register)[reg_id].h = -1;
        (*t.Register)[reg_id].Busy = false;
      }
    // cout << "///////////////////////ROBlimit: " << t.ROB->size() << endl; 
     int prelimit = t.ROB->size(); 
     t.ROB->pop_front();
    // cout << "///////////////////////ROBlimit: " << t.ROB->size() << endl; 
    // t.ROBlimit++;
     int poslimit = t.ROB->size();

    // cout << "///////////////////////IQ size: " << t.IQ->size() << endl; 
     //if ins stays more than one cycle in IQ, push it into ROB & RS
     if(prelimit == 6 && poslimit == 5 && !t.IQ->empty() && t.IQ->front().push_cycle < cycle- 1){
       t.rs_rob_add(t.IQ->front(), cycle);
       t.IQ->pop_front();
      // t.ROBlimit--;
     }
    // cout << "///////////////////////ROBlimit: " << t.ROB->size() << endl; 
    }
    if(t.ROB->front().state != Commit && t.ROB->front().Ready == true && t.ROB->front().current_status_cycle < cycle){
      t.ROB->front().state = Commit;
      t.ROB->front().current_status_cycle = cycle;
    }
    
    if(!t.IQ->empty() || !t.RS->empty() || !t.ROB->empty()){
      not_finished = true;
    }
    PC = nextPC;
  
    t.print_status(cycle);

    cycle++;
   }
  return 0;
}

