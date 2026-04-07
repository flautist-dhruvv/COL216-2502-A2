#include "ExecutionUnit.h"

void ExecutionUnit::capture(int tag, int val){
    for (size_t i =0 ; i < rs.size(); i++){
        RSEntry& entry =  rs[i];
        
        if (entry.busy == true) {

            if (entry.qj == tag){
                entry.vj = val;
                entry.qj = -1;
            }
            if (entry.qk == tag){
                entry.vk = val;
                entry.qk = -1;
            }
        }
    }
}

void ExecutionUnit::executeCycle(){

    has_result = false;
    has_exception = false;

    for (size_t i = 0; i < pipeline.size(); i++){
        PipeLineEntry& entry = pipeline[i];

        if (entry.valid == true && entry.cycles_remaining == 1){
            if (entry.op == OpCode::ADD){
                long long res = entry.val1 + entry.val2;
                if (res>2147483647 || res < -2147483648){
                    has_exception = true;
                }
                else{
                    result_val = res;
                    has_result = true;
                }

            }
            else if (entry.op == OpCode::SUB){
                long long res = entry.val1 - entry.val2;
                if (res>2147483647 || res < -2147483648){
                    has_exception = true;
                }
                else{
                    result_val = res;
                    has_result = true;
                }
            }
            else if (entry.op == OpCode::ADDI){
                long long res = entry.val1 + entry.imm;
                if (res>2147483647 || res < -2147483648){
                    has_exception = true;
                }
                else{
                    result_val = res;
                    has_result = true;
                }
            }
            else if (entry.op == OpCode::MUL){
                long long res = entry.val1 * entry.val2;
                if (res>2147483647 || res < -2147483648){
                    has_exception = true;
                }
                else{
                    result_val = res;
                    has_result = true;
                }
            }
            else if (entry.op == OpCode::DIV){
                if (entry.val2 == 0){
                    has_exception = true;
                }
                else{                
                    result_val = entry.val1/entry.val2;
                    has_result = true;
                }
            }
            else if (entry.op == OpCode::REM){
                if (entry.val2 == 0){
                    has_exception = true;
                }
                else{
                    result_val = entry.val1%entry.val2;
                    has_result = true;
                }
            }

            else if (entry.op == OpCode::SLT){
                result_val =  (entry.val1<entry.val2) ? 1 : 0;
                has_result = true;
            }
            else if (entry.op == OpCode::SLTI){
                result_val =  (entry.val1<entry.imm) ? 1 : 0;
                has_result = true;
            }

            else if (entry.op == OpCode::AND){
                result_val = entry.val1 & entry.val2;
                has_result = true;
            }
            else if (entry.op == OpCode::OR){
                result_val = entry.val1 | entry.val2;
                has_result = true;
            }
            else if (entry.op == OpCode::XOR){
                result_val = entry.val1 ^ entry.val2;
                has_result = true;
            }

            else if (entry.op == OpCode::ANDI){
                result_val = entry.val1 & entry.imm;
                has_result = true;
            }
            else if (entry.op == OpCode::ORI){
                result_val = entry.val1 | entry.imm;
                has_result = true;
            }
            else if (entry.op == OpCode::XORI){
                result_val = entry.val1 ^ entry.imm;
                has_result = true;
            }

            else if (entry.op == OpCode::BEQ){
                if (entry.val1 == entry.val2){
                    result_val = entry.imm;
                }
                else{
                    result_val = entry.pc + 1;
                }
                has_result = true;
            }
            else if (entry.op == OpCode:: BNE){
                if (entry.val1 != entry.val2){
                    result_val = entry.imm;
                }
                else{
                    result_val = entry.pc +1;
                }
                has_result = true;
            }
            else if (entry.op == OpCode:: BLT){
                if (entry.val1 < entry.val2){
                    result_val = entry.imm;
                }
                else{
                    result_val = entry.pc +1;
                }
                has_result = true;
            }
            else if (entry.op == OpCode:: BLE){
                if (entry.val1 <= entry.val2){
                    result_val = entry.imm;
                }
                else{
                    result_val = entry.pc +1;
                }
                has_result = true;
            }
            result_tag = entry.rob_tag;
        }

        if (entry.valid==true){
            entry.cycles_remaining-=1;
            if (entry.cycles_remaining==0){
                entry.valid = false;
            }
        }
    }
    pipeline.erase(
    std::remove_if(pipeline.begin(), pipeline.end(),
        [](const PipeLineEntry& n) {
            return !n.valid;   
        }),
    pipeline.end()
    );

    int oldest = -1;
    int oldest_pc = INT_MAX;

    for (size_t i = 0; i < rs.size(); i++) {
        if (rs[i].busy && rs[i].qj == -1 && rs[i].qk == -1) {
            if (rs[i].pc < oldest_pc) {
                oldest_pc = rs[i].pc;
                oldest = i;
            }
        }
    }

    if (oldest != -1 && pipeline.size()<latency){
        
        RSEntry& entry = rs[oldest];
     
        entry.busy=false;
        PipeLineEntry Pentry ;
        Pentry.op = entry.op;

        Pentry.val1 = entry.vj;
        Pentry.val2 = entry.vk;
        Pentry.imm = entry.imm;
        Pentry.rob_tag = entry.dest;
        Pentry.pc = entry.pc;
        Pentry.cycles_remaining = latency;

        pipeline.push_back(Pentry);
    }
}