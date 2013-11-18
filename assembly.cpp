#include<iostream>
#include<string>
#include<map>
#include<vector>
#include<fstream>
#include<sstream>
#include<cstring>

#define INSADDR 600

using namespace std;

//convert int to string
string intTostring(int i){
    stringstream ss;
    ss << i;
    string result(ss.str());
    return result;
}
//convert big endian to little endian
int big_endian2little_endian(int i){
    return((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff);
}
//convert binary to string
string binTostring(char a){    
    string eachchar;
    int j=0;
    while(j<8){
        if(a & 0x1){
            eachchar = '1'+ eachchar;
        } 
        else {
            eachchar = '0'+ eachchar;
        }
        a >>= 1;
        j++;
    }
    return eachchar;
}

//read instruction set
void inputins(map<string,string> *RImap, map<string,string> *IJmap){
    
    string line;

    ifstream insfp("instruction.txt",ios::in);
    while(insfp.good()){
        getline(insfp,line);
        stringstream splitline(line);

        vector<string> linepara;
        string eachpara;
        while(getline(splitline,eachpara,' ')){
            linepara.push_back(eachpara);
        }

        //insert R-type and BGEZ, BLTZ
        if(linepara[2] == "000000" || linepara[2] == "000001"){
            RImap->insert(pair<string,string>(linepara[3],linepara[0]));
        }
        else {
            IJmap->insert(pair<string,string>(linepara[2],linepara[0]));
        }
        
    }

    insfp.close();
}
//assembler function
void assembly(string inputfilename){
    string outputfilename = "mid.txt";
    //open input file for read
    char * input = new char[inputfilename.length()+1];
    strcpy(input,inputfilename.c_str());

    FILE *inputfp = fopen(input, "rb");
    if(inputfp == NULL){
        fputs("File error",stderr);
        return;
    }
    //count input file size
    fseek(inputfp, 0, SEEK_END);
    size_t size = ftell(inputfp);
    rewind(inputfp);
    cout <<"Input file size is:"<< size <<"Bytes." << endl;
    
    char buffer[4];


    bool br = false;//break indicator
    int inslinecounter = 0;//indicate how many lines processed
    int datalinecounter = 0;
    int flinecounter = 0;

    //open output file for write
    char * output= new char[outputfilename.length()+1];
    strcpy(output,outputfilename.c_str());
    ofstream outfp(output,ios::out);
    
    //get instruction set
    map<string,string> RIset;
    map<string,string> IJset;
    inputins(&RIset, &IJset);
        
       /* 
        map<string,string>::iterator it;
        for(it = IJset.begin();it != IJset.end();it++){
            cout << it->first << " " << it->second << endl;
        }*/

    while(!feof(inputfp)){
    //read 32bits from inputfile
        if(!fread(buffer, 4, 1, inputfp)){
            break;
        }

        //convert binary to string
        string line = "";
        for(int i=0; i<4; i++){
            line = line + binTostring(buffer[i]);
        }
        //substr each part
        string opcode,function;
        opcode = line.substr(0,6); //first 6 bits=opcode
        function = line.substr(26,6);
        string rs,rt,rd,sa;
        rs = line.substr(6,5);
        rt = line.substr(11,5);
        rd = line.substr(16,5);
        sa = line.substr(21,5);
        string codeline = opcode + " " + rs + " " + rt + " " + rd + " " + sa + " " + function;
        
        // get the integer version of each line from .bin file
        int linevi;
        fseek(inputfp, -4, SEEK_CUR);
        fread(&linevi, 4, 1, inputfp);
        linevi = big_endian2little_endian(linevi);

        //if it's break
        if(opcode == "000000" && function == "001101"){
            string breakaddr = intTostring(INSADDR+inslinecounter*4);
            outfp << codeline << " " << breakaddr << " " << "BREAK"  << endl;
            inslinecounter++;
            br = true;
        }
        // not break
        else {
            if (br == false){//stil in instruction section, even if it's 32 0s
               //instruction translate here

                string insaddr = intTostring(INSADDR+inslinecounter*4);
                int rti,rsi;
                rsi = (linevi & 0x03E00000) >> 21;
                rti = (linevi & 0x001F0000) >> 16;
                
                //R-type parse
                if(opcode == "000000"){
                    int rdi,sai;

                    rdi = (linevi & 0x0000F800) >> 11;
                    sai = (linevi & 0x000007C0) >> 6;
                    
                    if(function == "000000"){//nop or sll
                        if(sa == "00000" && rs == "00000" && rt == "00000" && rd == "00000"){//NOP
                            outfp << codeline << " " << insaddr << " " << "NOP" << endl;
                        }
                        else {//sll
                            outfp << codeline << " " << insaddr << " " << "SLL R" << intTostring(rdi) << ", R" << intTostring(rti) << ", #" << intTostring(sai) << endl; 
                        }
                    }
                    else {//other R-type

                        string operation;
                        map<string,string>::iterator RIit;
                        RIit = RIset.find(function);
                        if(RIit == RIset.end()){
                            cout << "Instruction not recognized!" << endl;
                            cout << "Line " << inslinecounter+1 << ": " << line << endl;
                            operation = "UNKNOWN";
                        }
                        else{
                            operation = RIit->second;
                        }
                        
                        
                        if(operation == "SRA" || operation == "SRL"){
                            outfp << codeline << " " << insaddr << " " << operation <<  " R" << intTostring(rdi) << ", R" << intTostring(rti) << ", #" << intTostring(sai) << endl;
                        }
                        else{
                            outfp << codeline << " " << insaddr << " " << operation << " R" << intTostring(rdi) << ", R" << intTostring(rsi) << ", R" << intTostring(rti) << endl;
                        }
                    }
                }
                //J-type parse
                else if(opcode == "000010"){
                    linevi = linevi & 0x03FFFFFF;
                    linevi <<= 2;
                    string target = intTostring(linevi);
                    outfp << codeline << " " << insaddr << " " << "J #" << target << endl;
                }
                //I-type parse
                else {

                    if(opcode == "000001"){//BGEZ,BLTZ
                        linevi = (linevi & 0x0000FFFF) << 2;
                        int sign = (linevi & 0x00020000) >> 17;
                        int offset;
                        if(sign == 1){
                            offset = linevi | 0xFFFC0000;
                        }
                        else if(sign == 0){
                            offset = linevi;
                        }

                        string iopb;
                        map<string,string>::iterator RIgit;
                        RIgit = RIset.find(rt);
                        if(RIgit == RIset.end()){
                            cout << "Instruction not recognized" << endl;
                            cout << "Line " << inslinecounter+1 << ": " << line << endl;
                            iopb = "UNKNOWN"; 
                        }
                        else {
                            iopb = RIgit->second;
                        }
                        outfp << codeline << " " << insaddr << " " << iopb << " R" << intTostring(rsi) << ", #" << intTostring(offset) << endl; 
                    }
                    else{//other I-type
                        string ioperation;
                        map<string,string>::iterator IJit;
                        IJit = IJset.find(opcode);
                        if(IJit == IJset.end()){
                            cout << "Instruction not recognized" << endl;
                            cout << "Line " << inslinecounter+1 << ": " << line << endl;
                            ioperation = "UNKNOWN";
                        }
                        else{
                            ioperation = IJit->second;
                        }

                        if(ioperation == "BGTZ" || ioperation == "BLEZ" || ioperation == "BEQ" || ioperation == "BNE"){//BGTZ,BLEZ,BNE,BEQ
                            linevi = (linevi & 0x0000FFFF) << 2;
                            int sign_18 = (linevi & 0x00020000) >> 17;
                            int offset_18;
                            if(sign_18 == 1){
                                offset_18 = linevi | 0xFFFC0000;
                            }   
                            else if(sign_18 == 0){
                                offset_18 = linevi;
                            }
                            
                            if(ioperation == "BGTZ" || ioperation == "BLEZ" ){     
                                outfp << codeline << " " << insaddr << " " << ioperation << " R" << intTostring(rsi) << ", #" << intTostring(offset_18) << endl;
                            }
                            else if (ioperation == "BEQ" || ioperation == "BNE"){
                                outfp << codeline << " " << insaddr << " " << ioperation << " R"<< intTostring(rsi) << ", R" << intTostring(rti) << ", #" << intTostring(offset_18) << endl;
                            }
                        }
                        else {
                            linevi = linevi & 0x0000FFFF;
                            int sign_16 = (linevi & 0x00008000) >> 15;
                            int offset_16;
                            if(sign_16 == 1){
                                offset_16 = linevi | 0xFFFF0000;
                            }
                            else if(sign_16 == 0){
                                offset_16 = linevi;
                            }

                            if(ioperation == "LW" || ioperation == "SW"){//LW, SW
                                outfp << codeline << " " << insaddr << " " << ioperation << " R" << intTostring(rti) << ", " << intTostring(offset_16) << "(R" << intTostring(rsi) << ")" << endl;
                            }
                            else {//ADDI,ADDIU,SUB,SUBU
                                outfp << codeline << " " << insaddr << " " << ioperation << " R"<< intTostring(rti) << ", R" << intTostring(rsi) << ", #" << intTostring(offset_16) << endl;
                            }
                        }
                    }
                }
                inslinecounter++;
            }
            else if (br == true){
                if(inslinecounter < 29){
                    string filladdr = intTostring(inslinecounter*4+INSADDR);
                    outfp << line << " " << filladdr << " " << "0" << endl;
                    inslinecounter++;
                    flinecounter++;
                }
                else{
                    string dataaddr = intTostring(INSADDR+(datalinecounter+inslinecounter)*4);
                    string datavs = intTostring(linevi);
                    string dataline = line + " " + dataaddr  + " " + datavs;
                    outfp << dataline << endl;
                    datalinecounter++;
                }
            }
        }
    }
    cout << "Instructions start from address " << INSADDR << endl;
    cout << "Data start from address " << inslinecounter*4+INSADDR << endl;
    inslinecounter -= flinecounter;
    cout << inslinecounter << " lines in instruction section." << endl;
    cout << flinecounter << " lines 0s fill between instruction section and data section." << endl;
    cout << datalinecounter << " lines in data section." << endl;
    
      //output data section to output.txt
    delete input;
    delete output;
    fclose(inputfp);
    outfp.close();
}
/*
int main(int argc, char *argv){
    map<string,string> insmap;

    insmap = inputins();
    string inputfilename = "fibonacci_bin.bin";
    string outputfilename = "output.txt";
    assembly(inputfilename,outputfilename);
    return 0;
}*/
