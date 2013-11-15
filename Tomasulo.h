#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include "CFS.h"
#include "BTB.h"
#include <cmath>
using namespace std;

#define NO_USE 1111
#define DEBUG 1
#define NUM_REG 32
#define NUM_RS 8
#define NUM_ROB 6
#define NUM_BTB 16
#define MEM_SIZE 10
struct Instruction {
  string op; //opcode
  string rd; //dest register
  string rs; //source register
  string rt; //source register
};

struct RScontent {
  bool Busy;
  string op;
  string ins;
  int Vj;
  int Vk;
  int Qj;
  int Qk;
  int Dest;
  int A;
  bool Vj_ok;
  bool Vk_ok;
};

enum Rob_state {Issue, Exe, WriteBack, Commit};
struct ROBcontent {
  int Entry;
  string op;
  string ins;
  Rob_state state;
  string Dest;
  int value;
  bool Ready; 
};
struct AddrInspair {
  int insaddr;
  string instruction;
};
struct Reg {
  int value;
  int h; //indicate reorder buffer number
  bool Busy;
};
class Tomasulo {
  public:

    deque<Instruction> * IQ; //instruction queue
    vector<Reg> * Register; // register array, 32
    int Mem[1024]; //main memory
    deque<RScontent> * RS; //reservation station
    deque<ROBcontent> * ROB; // redorder buffer
    BTB * btbuffer; //branch target buffer
    
    vector<AddrInspair> * addrinslist;
    int inscounter, datacounter, fetchcounter, RSlimit, RScounter, ROBlimit, ROBcounter;
    
    Tomasulo(){
      IQ = new deque<Instruction>();
      Register = new vector<Reg>();
      //initialize registers
      for(int i=0; i< NUM_REG; i++){
        Reg reginit;
        reginit.value = 0;
        reginit.h = -1;
        reginit.Busy = false;
        Register->push_back(reginit);
      }
      RS = new deque<RScontent>();
      ROB = new deque<ROBcontent>();
      btbuffer = new BTB();
      addrinslist = new vector<AddrInspair>();
      inscounter = 0;
      datacounter = 0;
      fetchcounter = 0;
      RScounter = 1;
      RSlimit = 8;
      ROBcounter = 1;
      ROBlimit = 6;
    };
    ~Tomasulo() {
      delete IQ;
      delete Register;
      delete RS;
      delete ROB;
      delete btbuffer;
      delete addrinslist;
    }
    void parseinputfile(string filename); //read file and parse it, store data into mem array and store instructions into addrinslit 

    Instruction parseinstruction(string instruction); // divide an instruction into op, rd, rs, rt
    int caltarget(int PC, string instruction, string op); //cal target addrress for branch
    int insfetch(int PC, string instruction); //push  one instruction into IQ:w
    bool checkfetch(); //check if there are more instructions to fetch
    bool rs_available(); //check if there's any empty slot in RS
    bool rob_available(); //check if there's any empty slot in ROB
    void issuevj(RScontent * rs_entry, int rs, int rs_robid, bool value_ready, int rs_robvalue); 
    void issuevk(RScontent * rs_entry, int rt, int rt_robid, bool value_ready, int rt_robvalue); 
    void rs_rob_add(Instruction ins); //add instruction into RS
    int calalu(string op, int operandA, int operandB); //calculation
    int caladdr(int Vj, int A); //cal mem address
    bool calbranch(string op, int Vj, int Vk);//check if branch taken or not taken
    void print_status(int cycle);//print status after each cycle

};
