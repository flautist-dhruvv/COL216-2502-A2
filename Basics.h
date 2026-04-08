#pragma once
#include <string>

enum class OpCode { ADD, SUB, ADDI, MUL, DIV, REM, LW, SW, BEQ, BNE, BLT, BLE, J, SLT, SLTI, AND, OR, XOR, ANDI, ORI, XORI };
enum class UnitType { ADDER, MULTIPLIER, DIVIDER, LOADSTORE, BRANCH, LOGIC };

struct Instruction {
    OpCode op;
    int dest;
    int src1;
    int src2;
    int imm;
    int pc;
};

struct ProcessorConfig {
    int num_regs = 32;
    int rob_size = 64;
    int mem_size = 1024;

    int logic_lat = 1;
    int add_lat = 2;
    int mul_lat = 4;
    int div_lat = 5;
    int mem_lat = 4;

    int logic_rs_size = 4;
    int adder_rs_size = 4;
    int mult_rs_size = 2;
    int div_rs_size = 2;
    int br_rs_size = 2;
    int lsq_rs_size = 32;
};

struct ROBEntry {
    bool valid = false ;
    bool ready = false ;
    int dest_reg = -1; // -1 for branches and stores
    int value = 0; // 0 for branches and stores
     bool exception = false ;
     int pc = -1; // default value -1 indicates that this entry is not occupied 
     OpCode op;
     bool taken = false ; // for branches, default value is false, will be set to true if branch is taken
     int target_pc = -1; // for branches, the target PC if taken, for stores the value to be stored for other instructions it will be -1
     int predicted_pc = -1 ; // for branches, the predicted target PC, for other instructions it will be -1
     int store_addr = -1 ; // for stores, the memory address to store the value, for other instructions it will be -1
     int store_value = 0 ; // for stores, the value to be stored, for other instructions it will be 0   
     
};

struct RSEntry {
    bool busy = false ; // tells whether this entry is occupied or not
    OpCode op; // the operation to be performed
    int vj = 0, vk = 0; // operands values
    int qj = -1, qk = -1; // ROB tags (they will have value -1 if operands are ready)
    int dest = -1; // ROB tag for destination register
    int imm = 0; // immediate value for I type instrns
    int pc = -1; // program counter of the instruction
    bool executing = false;
    unsigned long long seq_num = 0;


};

struct PipeLineEntry {
    bool valid = true;
    int rob_tag = -1;
    OpCode op;
    int val1 = 0, val2= 0;
    int imm= 0;
    int pc= -1;
    int cycles_remaining= 0;
    int rs_index;
};