On my honor, I have neither given nor received unauthorized aid on this assignment.
Name: Jing Qin
UFID: 2612-8368
Gatorlink: winday0215
Email: jq0215@gmail.com

Files in the package:
main.cpp
assembly.cpp -- implement disassembly functions
assembly.h
CFS.h
CFS.cpp
Tomasulo.h -- tomasulo algorithm 
Tomasulo.cpp
BTB.h -- Branch target buffer class
BTB.cpp
Makefile -- indicate "make all" and "make clean"
instruction.txt -- supported MIPS instructions list

How to use it:
1. Unzip the package with "unzip mips.zip".
2. Compile package with "make all". 
3. Run program with "./a.out".Some help text will be shown.
4. Input command "MIPSsim inputfilename outputfilename -Tm:n" to run the disassembler. 
   ** Note:Input file must be binary file and output must be txt file. If you type in the wrong file type, program will indicate error and force you to input command again.
5. Output is the file with the outputfilename you indicated in command.
    **Note: If a instruction isn't included in the instruction list,program will point out the instruction line No. and content on the screen. And in the ouput file, this instruction will be indicated as "UNKOWN".
6. Delete *.out and *.o file, use "make clean". If you use out.txt as output filename, it will be deleted as well.
