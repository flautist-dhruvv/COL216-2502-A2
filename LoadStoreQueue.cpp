#include "LoadStoreQueue.h"


void LoadStoreQueue::capture(int tag, int val){
    
    for (size_t i = 0; i <rs.size() ; i ++){
        RSEntry& entry = rs [i];
        if (entry.busy == true){

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

void LoadStoreQueue::executeCycle(std::vector<int>& Memory){

    has_result = false;
    has_exception = false;

    for (size_t i = 0; i < pipeline.size(); i++){
        PipeLineEntry& entry = pipeline[i];

        if (entry.valid == true && entry.cycles_remaining == 1){
            
            int addr = entry.val1 + entry.imm;

            if (addr <0 || addr>=Memory.size()){
                has_exception = true;
            }

            if (!has_exception){

                if (entry.op==OpCode::LW){
                    result_val = Memory[addr];
                }
                
                else if (entry.op == OpCode::SW){
                    store_addr = addr;
                    store_data = entry.val2;
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

    int idx=-1;
    for (size_t i = 0; i < rs.size(); i++) {
        if (rs[i].busy && rs[i].qj == -1 && rs[i].qk == -1) {
            idx = i;
            break;
        }
    }

    if (idx!=-1 && pipeline.size() < latency){
        RSEntry& entry = rs[idx];
     
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