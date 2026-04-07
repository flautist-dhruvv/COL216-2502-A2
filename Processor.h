#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include "Basics.h"
#include "BranchPredictor.h"
#include "ExecutionUnit.h"
#include "LoadStoreQueue.h"

class Processor {
public:
    int pc;
    int clock_cycle;

    // pipeline registers
    bool fetch_latch_valid = false ; // indicates whether the fetch latch is holding a valid instruction or not, this will be set to false when we flush the pipeline after a branch missprediction or an exception
    Instruction fetched_inst ; // this will hold the instruction fetched in current cycle, it will be moved to decode latch in next cycle
    int fetched_predicted_pc = -1; // this will hold the predicted PC for the fetched instruction
    std::vector<Instruction> inst_memory;

    // architectural state (do not change)
    std::vector<int> ARF; // regFile
    std::vector<int> Memory; // Memory
    bool exception = false; // exception bit

    // register alias table / reorder buffer
    std::vector<int> RAT; //defining the register alias table
    std::vector<ROBEntry> ROB; // defining the reorder buffer
    int rob_head = 0; // points to the oldest instruction in ROB
    int rob_tail = 0; // points to the next free entry in ROB
    int rob_count = 0; // number of instructions currently in ROB
    // ROB is defined as a circular queue, so we will be using modulo arithmetic to update head and tail pointers

    std::vector<ExecutionUnit> units;
    LoadStoreQueue* lsq;
    BranchPredictor bp;

    Processor(ProcessorConfig& config) {
        pc = 0;
        clock_cycle = 0;
        ARF.resize(config.num_regs, 0);
        Memory.resize(config.mem_size);

        // Instantiate Hardware Units
        RAT.resize(config.num_regs, -1); // initialize RAT entries to -1 which indicates architectural register is not renamed
        ROB.resize(config.rob_size); // initialize ROB entries with default constructor
        rob_head = 0;
        rob_tail = 0;
        rob_count = 0;
        
        // we will use this constuctor ,ExecutionUnit(UnitType, latency, rs_size)
        units.push_back(ExecutionUnit(UnitType::ADDER, config.add_lat, config.adder_rs_size)); // Adder
        units.push_back(ExecutionUnit(UnitType::MULTIPLIER,config.mul_lat,config.mult_rs_size)); // Multiplier
        units.push_back(ExecutionUnit(UnitType::DIVIDER,config.div_lat,config.div_rs_size)); // Divider
        units.push_back(ExecutionUnit(UnitType::BRANCH,1,config.br_rs_size)); // Branch Computation, here I have assumed that branch computation latency is 1 cycle, we can change it if needed
        units.push_back(ExecutionUnit(UnitType::LOGIC,config.logic_lat,config.logic_rs_size));// Bitwise Logic
        // Load-Store Unit
        lsq= new LoadStoreQueue(config.mem_lat, config.lsq_rs_size);
    }

    void loadProgram(const std::string& filename) ;
    
    void flush() {
        //Clear fetch latch
        fetch_latch_valid = false;

        //Clear all RS entries in every execution unit & Clear all pipeline entries in every execution unit
        for (auto& u : units) {
            for (int i = 0; i < u.rs_size; i++) {
                u.rs[i].busy = false;
            }
            u.pipeline.clear();
            u.pipeline.shrink_to_fit();
        }

        //Clear LSQ RS and pipeline
        for (int i = 0 ; i < lsq->rs_size ; i++){
            lsq->rs[i].busy = false;
        }
        lsq->pipeline.clear();
        lsq->pipeline.shrink_to_fit();

        //Clear the ROB
        rob_head = 0;
        rob_count = 0;
        rob_tail = 0;

        //Clear RAT
        for (int i = 1 ; i<RAT.size(); i++){
            RAT[i] = -1;
        }


    };

    void broadcastOnCDB() {
        ROBEntry* robEntry;
        for (auto& u : units) {
            if (u.result_tag != -1){
                robEntry = &ROB[u.result_tag];
                if (u.has_result || u.has_exception){
                    if (u.has_result){
                        robEntry->value = u.result_val;
                    }
                    if (u.has_exception){
                        robEntry->exception = true;
                    }
                    robEntry->ready = true;
                    for (auto& p : units){
                        p.capture(u.result_tag,u.result_val);
                    }
                    lsq->capture(u.result_tag,u.result_val);  
                    u.has_result = false; // two fixes ,that I missed initially
                    u.has_exception = false;
                }   
            }
        }

        if (lsq->result_tag != -1){
            robEntry = &ROB[lsq->result_tag];
            if (lsq->has_result || lsq->has_exception){
                if (lsq->has_result){
                    if (robEntry->op == OpCode::SW){
                        robEntry->store_addr = lsq->store_addr;
                        robEntry->store_value = lsq->store_data;
                    }
                    else{
                        robEntry->value = lsq->result_val;
                    }
                }
                if (lsq->has_exception){
                    robEntry->exception = true;
                }
                robEntry->ready = true;
                if (robEntry->op == OpCode::LW){
                    for (auto& p : units){
                        p.capture(lsq->result_tag,lsq->result_val);
                    }
                    lsq->capture(lsq->result_tag,lsq->result_val);
                }
                    lsq->has_result = false; //two fixes that I missed initially
                    lsq->has_exception = false;
                
            }
        }
    };

    void stageFetch() {
        if( pc >= inst_memory.size() || fetch_latch_valid) {
            return; // if there are no more instructions to fetch or the fetch latch is still holding the previous instruction, we cannot fetch a new instruction
        }
        fetched_inst = inst_memory[pc]; // fetching the instruction from instruction memory at current pc
        fetch_latch_valid = true; // setting the fetch latch valid bit to indicate that we have a valid instruction in fetch latch
       
        // checking the instruction type and updating the pc accordingly
        if(fetched_inst.op == OpCode::BEQ || fetched_inst.op == OpCode::BNE || fetched_inst.op == OpCode::BLT || fetched_inst.op == OpCode::BLE || fetched_inst.op == OpCode::J) {
            int predicted_pc = bp.predict(pc, fetched_inst.imm, fetched_inst.op); // getting the predicted target pc from our branch predictor
            fetched_predicted_pc = predicted_pc; 
            pc = predicted_pc; // update the PC to the predicted PC for next fetch
        }
        else {
            fetched_predicted_pc = pc + 1; // for non branch instructions, the predicted PC is simply the next sequential instruction
            pc = pc + 1; // for non branch instructions, we can simply move to the next instruction
        }
    }


    void stageDecode() {
        // checking the stall conditions for decode stage
        if (!fetch_latch_valid) return; 
        if (rob_count == ROB.size()) return; 

        // finding which unit this instr should go to based on its opcode
        UnitType target_unit;
        bool needs_rs = true; // as jump instructions don't need rs table
        OpCode op = fetched_inst.op;

        if (op == OpCode::ADD || op == OpCode::SUB || op == OpCode::ADDI || op == OpCode::SLT || op == OpCode::SLTI) {
            target_unit = UnitType::ADDER;
        } else if (op == OpCode::MUL) {
            target_unit = UnitType::MULTIPLIER;
        } else if (op == OpCode::DIV || op == OpCode::REM) {
            target_unit = UnitType::DIVIDER;
        } else if (op == OpCode::AND || op == OpCode::OR || op == OpCode::XOR || op == OpCode::ANDI || op == OpCode::ORI || op == OpCode::XORI) {
            target_unit = UnitType::LOGIC;
        } else if (op == OpCode::BEQ || op == OpCode::BNE || op == OpCode::BLT || op == OpCode::BLE) {
            target_unit = UnitType::BRANCH;
        } else if (op == OpCode::LW || op == OpCode::SW) {
            target_unit = UnitType::LOADSTORE;
        } else if (op == OpCode::J) {
            needs_rs = false; // as jump instructions don't use execution units
        }

        // finding a free RS ,if needed
        int rs_index = -1;
        ExecutionUnit* ex_unit = nullptr;

        if (needs_rs) {
            if (target_unit == UnitType::LOADSTORE) {
                // checking LSQ for a free spot (busy == false)
                for (int i = 0; i < lsq->rs_size; i++) {
                    if (!lsq->rs[i].busy) {
                        rs_index = i;
                        break;
                    }
                }
            } else {
                // checking standard units for a free spot
                for (auto& u : units) {
                    if (u.name == target_unit) {
                        ex_unit = &u; // getting the pointer to the target execution unit so that we can later allocate the RS entry in that unit
                        for (int i = 0; i < u.rs_size; i++) {
                            if (!u.rs[i].busy) {
                                rs_index = i;
                                break;
                            }
                        }
                        break;
                    }
                }
            }

            // if we needed an RS but couldn't find a free one, then we should STALL
            if (rs_index == -1) return; 
        }

        // we are here means we have passed all the checks and we can proceed with decode and allocation

        // allocating an ROb entry
        int rob_idx = rob_tail;
        ROB[rob_idx].valid = true;
        ROB[rob_idx].ready = !needs_rs; // because Jumps are ready instantly
        ROB[rob_idx].dest_reg = fetched_inst.dest; // for stores and branches, dest_reg will be -1, which is fine because they don't write to registers
        ROB[rob_idx].op = fetched_inst.op;
        ROB[rob_idx].pc = fetched_inst.pc;
        ROB[rob_idx].predicted_pc = fetched_predicted_pc;
        ROB[rob_idx].exception = false;
        
        // setting the target PC for branches,which is just the immediate (thanks to the python compiler)
        if (op == OpCode::BEQ || op == OpCode::BNE || op == OpCode::BLT || op == OpCode::BLE) {
            ROB[rob_idx].target_pc = fetched_inst.imm;
        }
        if (op == OpCode::J) {
            ROB[rob_idx].target_pc = fetched_inst.imm;
        }
        

        // now we will allocate the RS entry if needed and fill it with the appropriate values, we will also do operand renaming here by looking at the RAT and ROB as needed
        if (needs_rs) {
            RSEntry* rs_entry = nullptr;
            if (target_unit == UnitType::LOADSTORE) {
                rs_entry = &lsq->rs[rs_index];
            } else {
                rs_entry = &ex_unit->rs[rs_index];
            }

            rs_entry->busy = true;
            rs_entry->op = fetched_inst.op;
            rs_entry->dest = rob_idx; // this is the tag to broadcast when done
            rs_entry->pc = fetched_inst.pc;
            rs_entry->imm = fetched_inst.imm;

            //now we should resolve the source operands and do renaming if needed, we will look at the RAT to see if the source register is renamed, if it is renamed we will look at the ROB entry corresponding to that rename tag to see if the value is ready, if it is ready we will grab the value from ROB and put it in vj/vk, if it is not ready we will put the ROB tag in qj/qk to indicate that we are waiting for that value, if the source register is not renamed then we can directly grab the value from ARF and put it in vj/vk and set qj/qk to -1 to indicate that we are not waiting for any value for that operand
            // resolving src1
            if (fetched_inst.src1 == -1 || fetched_inst.src1 == 0) {  // if src1 is x0(which is immuatable 0) or no register(in case of immediates), we can directly set it to 0 and mark it as ready
                rs_entry->vj = 0; 
                rs_entry->qj = -1;
            } else if (RAT[fetched_inst.src1] != -1) { // if src1 is renamed( if it is in RAT ), we need to check the ROB entry it is renamed to see if the value is ready
                int dep_rob = RAT[fetched_inst.src1];
                if (ROB[dep_rob].ready) {
                    rs_entry->vj = ROB[dep_rob].value; // Grab value if already done!
                    rs_entry->qj = -1;
                } else {
                    rs_entry->qj = dep_rob; // Otherwise, wait for tag
                }
            } else { // if src1 is not renamed, we can directly grab the value from ARF
                rs_entry->vj = ARF[fetched_inst.src1];
                rs_entry->qj = -1;
            }

            // similarly for src2
            if (fetched_inst.src2 == -1 || fetched_inst.src2 == 0) { 
                rs_entry->vk = 0; 
                rs_entry->qk = -1;
            } else if (RAT[fetched_inst.src2] != -1) {
                int dep_rob = RAT[fetched_inst.src2];
                if (ROB[dep_rob].ready) {
                    rs_entry->vk = ROB[dep_rob].value; 
                    rs_entry->qk = -1;
                } else {
                    rs_entry->qk = dep_rob; 
                }
            } else {
                rs_entry->vk = ARF[fetched_inst.src2];
                rs_entry->qk = -1;
            }
        }

        // updating RAT,if needed
        if (fetched_inst.dest != -1 && fetched_inst.dest != 0) {
            RAT[fetched_inst.dest] = rob_idx;
        }

        // incrementing the ROB tail and count
        rob_tail = (rob_tail + 1) % ROB.size();
        rob_count++;
        fetch_latch_valid = false; // clear the fetch latch for next instruction
    }


    void stageExecuteAndBroadcast() {
        // we will command all hardware units to forward by one cycle, and then we will broadcast the results on CDB if any unit has finished execution in this cycle
        for (auto& u : units) {
            u.executeCycle();   }
        lsq->executeCycle(Memory); // we need to pass the memory to the LSQ because it needs to perform memory operations for loads and stores
        broadcastOnCDB(); // after we have forwarded all units by one cycle, we will broadcast the results on CDB if any unit has finished execution in this cycle
    }

    void stageCommit() {
        if (rob_count == 0 || !ROB[rob_head].ready){
            return;
        }

        ROBEntry* robEntry = &ROB[rob_head];
        
        if (robEntry ->exception){
            pc = robEntry->pc;
            exception = true;
            flush();
            return;
        }
        else if (robEntry->op==OpCode::BEQ || robEntry->op==OpCode::BNE || robEntry->op==OpCode::BLT || robEntry->op==OpCode::BLE){
            
            bool taken = (robEntry->value == robEntry->target_pc);
            bool was_correct = (robEntry->predicted_pc == robEntry->value);
            bp.update(robEntry->pc, robEntry->value, taken, was_correct);
            
            if (!was_correct){
                pc = robEntry->value;
                flush();
                 return; 
                  }
        }
        else if (robEntry->op==OpCode::SW){
            Memory[robEntry->store_addr] = robEntry->store_value;
        }
        else if (robEntry->dest_reg!=-1 && robEntry->dest_reg!=0){
            ARF[robEntry->dest_reg]= robEntry->value;
        }
        if (robEntry->dest_reg != -1 && robEntry->dest_reg != 0 && RAT[robEntry->dest_reg] == rob_head) {
            RAT[robEntry->dest_reg] = -1;
        }
        robEntry->valid = false;
        rob_head = (rob_head + 1)% ROB.size();
        rob_count--;
    };

    bool step() {
        if(pc >= inst_memory.size() && rob_count == 0 && !fetch_latch_valid) {
            return false; // no more instructions to fetch and ROB is empty, we can stop the simulation
        }
        clock_cycle++;
        // we call stages in reverse order because we want the effect of each stage to be visible in the next stage in the same cycle, for example when an instruction is committed in stageCommit, we want to see its effect in stageDecode of the same cycle, if we call stageCommit at the end of step function, then we will only see its effect in the next cycles stage, which is not what we want
        stageCommit();
        stageExecuteAndBroadcast();
        stageDecode();
        stageFetch();
        //for debugging
        std::cout << "Cycle " << std::setw(2) << clock_cycle 
                  << " | PC: " << std::setw(2) << pc 
                  << " | Fetch_Latch_Full: " << (fetch_latch_valid ? "YES" : "NO ")
                  << " | ROB Count: " << rob_count 
                  << " | RS Busy (Adder/Mul): ";
        
        // Count busy Reservation Stations to see if we are bottlenecking
        int add_busy = 0, mul_busy = 0;
        for (auto& u : units) {
            if (u.name == UnitType::ADDER) {
                for (int i = 0; i < u.rs_size; i++) if (u.rs[i].busy) add_busy++;
            }
            if (u.name == UnitType::MULTIPLIER) {
                for (int i = 0; i < u.rs_size; i++) if (u.rs[i].busy) mul_busy++;
            }
        }
        std::cout << add_busy << " / " << mul_busy << std::endl;
        return true; // return false if CPU has no more to do after this cycle
    }

    void dumpArchitecturalState() {
        std::cout << "\n=== ARCHITECTURAL STATE (CYCLE " << clock_cycle << ") ===\n";
        for (int i = 0; i < ARF.size(); i++) {
            std::cout << "x" << i << ": " << std::setw(4) << ARF[i] << " | ";
            if ((i+1) % 8 == 0) std::cout << std::endl;
        }
        if (exception) {
            std::cout << "EXCEPTION raised by instruction " << pc + 1 << std::endl;
        }
        std::cout << "Branch Predictor Stats: " << bp.correct_predictions << "/" << bp.total_branches << " correct.\n";
    }
};