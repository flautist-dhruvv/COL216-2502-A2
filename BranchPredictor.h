#pragma once
#include "Basics.h"
#include <iostream>
#include <vector>
#include <unordered_map> // this is for per-instruction 2 bit saturating counter

class BranchPredictor {
public:
    int total_branches = 0;
    int correct_predictions = 0;
    std::unordered_map<int ,int> branch_history_table ; // this is a map from PC to 2 bit saturating counter value (0,1,2,3) where 0 and 1 predict not taken and 2 and 3 predict taken


    int predict(int current_pc, int imm, OpCode op) {
        if(op == OpCode::J) {
            return imm ; // for jump instrn ,we can directly move to jump address, no need of predictor
        }
        else if(op != OpCode::BEQ && op != OpCode::BNE && op != OpCode::BLT && op != OpCode::BLE) {
            return current_pc + 1 ; // for non branch instrns, we can directly move to next instruction, we are doing +1 here because in our implementation we are treating PC as index of instruction in instruction memory, so next instruction will be at current_pc + 1, instead of PC+ 4
        }
        else {
            if(branch_history_table.find(current_pc) == branch_history_table.end()) {
                branch_history_table[current_pc] = 0 ; // initialize the counter value to 0 (not taken) if this is the first time we are seeing this branch instruction
            }
            int counter_value = branch_history_table[current_pc] ;
            if(counter_value < 2) { // predict taken
                return imm ; // predicted target address
            }
            else { // predict not taken
                return current_pc + 1 ; // predicted next sequential address
            }
        }
    }

    void update(int pc, int actual_target, bool taken, bool was_correct) {
        total_branches++;
        if (was_correct) {
            correct_predictions++;
        }
        int state = branch_history_table[pc] ;
        if(taken) {
            if(state>=1) {
                branch_history_table[pc] = state - 1 ; // move towards taken state
            }
        }
        else {
            if(state<=2) {
                branch_history_table[pc] = state + 1 ; // move towards not taken state
            }
        }

    }
};