#include <iostream>
#include <string>
#include "CFS.h"
using namespace std;

#define ROW 16
#define COL 3

class BTB {
  public:
    BTB(); //constructori
    void ChangeBit(int remainder, int LRU[3]);//change LRU bit
    void ChangeLRU(int insaddr); //change LRU line
    int checkBTB(int insaddr, int targetaddr); //check if branch exist, if exist, get targetaddr
    void ChangePredictor(int insaddr, int predictor); //change predictor
    int GetLRULineinSet(int LRU[3]); //get line in set
    int GetLRULine(int insaddr); //get line in BTB
    void UpdatePredictor(int insaddr, int targetaddr, int predictor); // add new predictor

    int BTBuffer[ROW][COL];  //BTB 
    int LRU0[3], LRU1[3], LRU2[3], LRU3[3];
};
