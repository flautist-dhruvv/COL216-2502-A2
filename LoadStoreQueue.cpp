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

            if (addr <0 || addr>=int(Memory.size())){
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
                rs[entry.rs_index].busy = false;
                rs[entry.rs_index].executing = false;
                
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

}

void LoadStoreQueue::addNew(){
    int absolute_oldest = -1;
    unsigned long long oldest_seq = ULLONG_MAX; // Use sequence instead of PC

    // 1. Find the ABSOLUTE oldest instruction based on sequence number
    for (size_t i = 0; i < rs.size(); i++) {
        if (rs[i].busy && !rs[i].executing) {
            if (rs[i].seq_num < oldest_seq) { // Compare tickets!
                oldest_seq = rs[i].seq_num;
                absolute_oldest = i;
            }
        }
    }

    // 2. Check if the absolute oldest instruction is actually ready.
    bool can_dispatch = false;
    if (absolute_oldest != -1) {
        // If it is NOT ready, we stall. We do not skip it.
        if (rs[absolute_oldest].qj == -1 && rs[absolute_oldest].qk == -1) {
            can_dispatch = true;
        }
    }

    if (can_dispatch && int(pipeline.size()) < latency){
        RSEntry& entry = rs[absolute_oldest];
        entry.executing = true;

        PipeLineEntry Pentry ;
        Pentry.op = entry.op;

        Pentry.val1 = entry.vj;
        Pentry.val2 = entry.vk;
        Pentry.imm = entry.imm;
        Pentry.rob_tag = entry.dest;
        Pentry.pc = entry.pc;
        Pentry.cycles_remaining = latency ;
        Pentry.rs_index = absolute_oldest;

        pipeline.push_back(Pentry);
    }
}

