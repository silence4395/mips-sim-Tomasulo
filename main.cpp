#include "Tomasulo.h"
#include <algorithm>
#include <sstream>
#include "assembly.h"
using namespace std;

int main(int argc, char ** argv){
    /*--------------------------ASSEMBLER-------------------------------
     -------------------------------------------------------------------
     --Read input .bin file and translate it into mid.txt---------------
     -------------------------------------------------------------------*/
    string helptxt="Usage: MIPSsim inputfilename outputfilename -Tm:n\n*Inputfilename - The file name of the binary input file.\n*Outputfilename - The file name to which to print the output.\n*-Tm:n -Argument to specify the start (m) and end (n) cycles of simulation tracing output. Tracing should be done in a single-step fashion with the contents of registers and memory shown after every processor cycle. -T0:0 indicates that no tracing is to be performed; eliminating the argument specifies that every cycle is to be traced.\n";

    string inputfilename, outputfilename;
    int startcycle, endcycle;
    
    
    while(true){
        string command,eachparameter,eachcycle;
        
        
        cout << "COMMAND USAGE" << helptxt << endl;
        cout << "-----------------------------------------------------------------" << endl;
        cout << "Please input your command:" << endl;
        getline(cin,command);
        stringstream split(command);
        
        vector<string> token;
        //split command input to parameters
        while(getline(split,eachparameter,' ')){
            token.push_back(eachparameter);
        }
        
        //check if each parameter is correct
        if(token.size() == 4  && token[0] == "MIPSsim"){
            inputfilename = token[1];
            outputfilename = token[2];
            
            string inputtype = inputfilename.substr(inputfilename.length()-3);
            string outputtype = outputfilename.substr(outputfilename.length()-3);
            
            if(inputtype != "bin"){
                cerr << "/////ERROR:Input file must be .bin file/////" << endl;
                continue;
            }
            if(outputtype != "txt"){
                cerr << "/////ERROR:Output file must be .txt file/////" << endl;
                continue;
            }
            
            //if exist, get start and end cycle
            string temp = token[3];
            if(temp[0] != '-' || temp[1]  != 'T'){
                cerr << "/////ERROR:Fifth parameter should start with '-T'/////" << endl;
                continue;
            }
            temp = temp.substr(2,temp.length()-1);
            //check if there's only one colon
            int coloncount = count(temp.begin(),temp.end(),':');
            if(coloncount > 1){
                cerr << "/////ERROR:Fifth parameter only has one ':'/////" << endl;
                continue;
            }
            stringstream splitcycle(temp);
            vector<string> cycles;
            while(getline(splitcycle,eachcycle,':')){
                cycles.push_back(eachcycle);
            }
            
            startcycle = atoi(cycles[0].c_str());
            endcycle = atoi(cycles[1].c_str());
            break;
        }
        else{
            cerr << "/////ERROR: Command should contain at least inputfile, outputfile and operation./////" << endl;
        }
    }
    
    
    /*--------------------------------------------------------
     * Disassembler Starts
     * ------------------------------------------------------*/
    
        assembly(inputfilename);
 


    /*--------------------------SIMULATOR-------------------------------
     -------------------------------------------------------------------
     --Read mid.txt and simulation cycle by cycle Tomasulo Algorithm ---
     -------------------------------------------------------------------*/
    string filename = "mid.txt";
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

 // t.print_status(cycle);

  //check if print this cycle
  if(startcycle != 0 && endcycle != 0 && cycle >= startcycle && cycle <= endcycle){
    t.print(outputfilename, cycle);
  }
  cycle++;
    
  //begin cycle by cycle implementation    
  while(not_finished){
    
    
    
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
      nextPC = t.insfetch(cycle, PC, instruction);  //fetch one instruction, return nextPC to fetch
   }

    /*-----------------------------
     *-----Issue-------------------
     * ---------------------------*/
    if(t.rs_available() && t.rob_available() && !t.IQ->empty() && t.IQ->front().push_cycle < cycle){

       
      t.rs_rob_add(t.IQ->front(), cycle);
      t.IQ->pop_front();
    }
    
    deque<RScontent>::iterator rsiter;
    for(rsiter = t.RS->begin(); rsiter != t.RS->end(); rsiter++){
      int rob_id = (*rsiter).Dest;
      deque<ROBcontent>::iterator robiter;
      bool exe;
    
             
      for(robiter = t.ROB->begin(); robiter != t.ROB->end(); robiter++){
        if((*robiter).Entry == rob_id){
          
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
            if(!(*rsiter).op.compare("BNE")|| !(*rsiter).op.compare("BEQ") || !(*rsiter).op.compare("BGEZ") || !(*rsiter).op.compare("BGTZ") || !(*rsiter).op.compare("BLTZ") ||!(*rsiter).op.compare("BLEZ") ||!(*rsiter).op.compare("J")){
              
              bool btaken = t.checkbranch((*rsiter).op, (*rsiter).Vj, (*rsiter).Vk); 

              int bresult, insaddr_temp;
              vector<AddrInspair>::iterator aiter;
              for(aiter = t.addrinslist->begin(); aiter != t.addrinslist->end(); aiter++){
                if((*aiter).instruction == (*rsiter).ins){
                  insaddr_temp = (*aiter).insaddr;
                }
              }


              int predictor;
              predictor = t.btbuffer->checkpredictor(insaddr_temp);
            //new entry, update it only, nextPC doesn't change
              if(predictor == -1){
                t.btbuffer->ChangePredictor(insaddr_temp, btaken);
                if(btaken == true){

                  t.flushout_rs_rob(rob_id);
                  t.IQ->clear();
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
                t.flushout_rs_rob(rob_id);
                t.IQ->clear();
                t.btbuffer->ChangePredictor(insaddr_temp, btaken);
                nextPC = t.btbuffer->checktargetaddr(insaddr_temp);
              }
              if((*robiter).state == Issue){
                (*robiter).state = Exe;
              }
              (*robiter).current_status_cycle = cycle;
              (*rsiter).current_status_cycle = cycle;
              (*robiter).Ready = true;
              (*rsiter).Busy = false;
              t.btbuffer->ChangeLRU(insaddr_temp);
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
    }


          /*-----------------------------
          *-----Write Result------------
          * ---------------------------*/
      for(rsiter = t.RS->begin(); rsiter != t.RS->end();){
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
              rsiter = t.RS->erase(rsiter);
              eraseflag = true;
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
      }
    /*-----------------------------
     *-----Commit-------------------
     * ---------------------------*/
    deque<RScontent>::iterator deiter;
      for(deiter = t.RS->begin(); deiter != t.RS->end();){
        if((*deiter).Busy == false && (*deiter).current_status_cycle < cycle){
          deiter = t.RS->erase(deiter);
        }
        else {
          deiter++;
        }
      }
    

    
    if(t.ROB->front().state == Commit && t.ROB->front().current_status_cycle < cycle){
      //store
      if(!t.ROB->front().op.compare("SW") && t.ROB->front().source_ok == true){

        t.Mem[(t.ROB->front().addr - 716)/4] = t.ROB->front().value;

        t.updatelaterLW(t.ROB->front().addr, cycle);
      }
      //regular ALU and LW, not branch , nop, break 
      else if(t.ROB->front().op.compare("BNE") && t.ROB->front().op.compare("BEQ") &&
                 t.ROB->front().op.compare("BGEZ") && t.ROB->front().op.compare("BGTZ") &&
                 t.ROB->front().op.compare("BLTZ") && t.ROB->front().op.compare("BLEZ") &&
                 t.ROB->front().op.compare("J") && t.ROB->front().op.compare("NOP") && t.ROB->front().op.compare("BREAK")){
        int reg_id;
        reg_id = atoi(t.ROB->front().Dest.substr(1).c_str());
        (*t.Register)[reg_id].value = t.ROB->front().value;
        (*t.Register)[reg_id].h = -1;
        (*t.Register)[reg_id].Busy = false;
      }
     int prelimit = t.ROB->size(); 
     t.ROB->pop_front();
     int poslimit = t.ROB->size();

     if(prelimit == 6 && poslimit == 5 && !t.IQ->empty() && t.IQ->front().push_cycle < cycle- 1){
       t.rs_rob_add(t.IQ->front(), cycle);
       t.IQ->pop_front();
     }
    }
    if(t.ROB->front().state != Commit && t.ROB->front().Ready == true && t.ROB->front().current_status_cycle < cycle){
      t.ROB->front().state = Commit;
      t.ROB->front().current_status_cycle = cycle;
    }
    
    if(!t.IQ->empty() || !t.RS->empty() || !t.ROB->empty()){
      not_finished = true;
    }
    PC = nextPC;
  
//    t.print_status(cycle);
    if(startcycle != 0 && endcycle != 0 && cycle >= startcycle && cycle <= endcycle){
      t.print(outputfilename, cycle);
    }
    cycle++;
   }

  //print final result only
  if(startcycle == 0 && endcycle == 0){
    t.print(outputfilename, cycle-1);
  }

  cout << "***********Simulation is Done. Please review results in file " << outputfilename << "*************"<< endl;
  return 0;
}

