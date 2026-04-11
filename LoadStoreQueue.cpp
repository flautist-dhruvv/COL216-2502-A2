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

void LoadStoreQueue::executeCycle(std::vector<int>& Memory , std::vector <ROBEntry>&ROB, int rob_head, int rob_count){

    has_result = false;
    has_exception = false;

    for (size_t i = 0; i < pipeline.size(); i++){
        PipeLineEntry& entry = pipeline[i];

        if (entry.valid == true && entry.cycles_remaining == 1){
            
            int addr = entry.val1 + entry.imm;
            bool current_inst_exception = false; // LOCAL flag for this specific instruction

            // 1. BOUNDS CHECK
            if (addr < 0 || addr >= int(Memory.size())){
                current_inst_exception = true;
            }

            if (current_inst_exception) {
                // If address is bad, update the ROB immediately so it can halt the CPU at commit
                ROB[entry.rob_tag].exception = true;
                ROB[entry.rob_tag].ready = true;
                
                // Alert the CDB
                has_exception = true;
                result_tag = entry.rob_tag;
            }
            else {
                // Address is safe! Proceed with Memory/ROB logic
                if (entry.op == OpCode::LW) {
                    bool forwarded = false;
                    int val = 0;

                    // CIRCULAR BUFFER SEARCH: Start from rob_head
                    for (int offset = 0; offset < rob_count; offset++) {
                        int idx = (rob_head + offset) % ROB.size();
                        
                        if (idx == entry.rob_tag) break; 

                        if (ROB[idx].op == OpCode::SW && ROB[idx].ready && ROB[idx].store_addr == addr) {
                            forwarded = true;
                            val = ROB[idx].store_value;
                        }
                    }

                    // We passed the bounds check above, so Memory[addr] is 100% safe to call here
                    ROB[entry.rob_tag].value = forwarded ? val : Memory[addr];
                    ROB[entry.rob_tag].ready = true;

                    // Expose to CDB so dependents can capture the loaded value
                    has_result = true;
                    result_val = ROB[entry.rob_tag].value; 
                } 
                else if (entry.op == OpCode::SW) {
                    ROB[entry.rob_tag].store_addr = addr;
                    ROB[entry.rob_tag].store_value = entry.val2;
                    ROB[entry.rob_tag].ready = true;
                    
                    has_result = true; // Tell CDB it's done (though no value needs to be captured)
                }
                
                result_tag = entry.rob_tag;
            }
        }

        // Cycle decrement and RS cleanup
        if (entry.valid == true){
            entry.cycles_remaining -= 1;
            if (entry.cycles_remaining == 0){
                rs[entry.rs_index].busy = false; 
                rs[entry.rs_index].executing = false;
                entry.valid = false;
            }
        }
    }

    // Remove finished instructions from the pipeline
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

