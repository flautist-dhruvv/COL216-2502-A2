#include "Processor.h"



void Processor::loadProgram(const std::string& filename) {
    std::ifstream file(filename);
    std:: string line;
    while (std::getline(file , line)){
        std:: istringstream iss(line);
        
        std::string opcode;
        iss>>opcode;
        Instruction inst;

        if (opcode ==".data"){
            iss >> opcode;

            int value ;
            int counter = 0;
            while (iss >> value){
                Memory[counter] = value;
                counter++;
            }
        }

        else if(opcode == "add"){
            inst.op = OpCode::ADD;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, rs2;
            args >> rd >> rs1 >> rs2;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = 0;
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "sub"){
            inst.op = OpCode::SUB;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, rs2;
            args >> rd >> rs1 >> rs2;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = 0;
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if (opcode == "addi"){
            inst.op = OpCode::ADDI;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, imm;
            args >> rd >> rs1 >> imm;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = -1;
            inst.imm = std::stoi(imm);
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "mul"){
            inst.op = OpCode::MUL;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, rs2;
            args >> rd >> rs1 >> rs2;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = 0;
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "div"){
            inst.op = OpCode::DIV;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, rs2;
            args >> rd >> rs1 >> rs2;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = 0;
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "rem"){
            inst.op = OpCode::REM;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, rs2;
            args >> rd >> rs1 >> rs2;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = 0;
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if (opcode == "lw") {
            inst.op = OpCode::LW;

            std::string rest;
            std::getline(iss, rest);
            for (char& c : rest) {
                if (c == ',') c = ' ';
            }

            std::istringstream args(rest);
            std::string rd, mem_part;
            args >> rd >> mem_part;

            // split 3(x1) into offset and base register
            int paren = mem_part.find('(');
            std::string offset = mem_part.substr(0, paren);
            std::string base = mem_part.substr(paren + 1);
            base = base.substr(0, base.find(')'));

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(base.substr(1));  // base register
            inst.src2 = -1;
            inst.imm = std::stoi(offset);           // memory offset
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if (opcode == "sw") {
            inst.op = OpCode::SW;

            std::string rest;
            std::getline(iss, rest);
            for (char& c : rest) {
                if (c == ',') c = ' ';
            }

            std::istringstream args(rest);
            std::string src2, mem_part;
            args >> src2 >> mem_part;

            // split 3(x1) into offset and base register
            int paren = mem_part.find('(');
            std::string offset = mem_part.substr(0, paren);
            std::string base = mem_part.substr(paren + 1);
            base = base.substr(0, base.find(')'));

            inst.dest = -1;
            inst.src1 = std::stoi(base.substr(1));  // base register
            inst.src2 = std::stoi(src2.substr(1));
            inst.imm = std::stoi(offset);           // memory offset
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "beq"){
            inst.op = OpCode::BEQ;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rs1, rs2, imm;
            args >> rs1 >> rs2 >> imm;

            inst.dest = -1;
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = std::stoi(imm);
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "bne"){
            inst.op = OpCode::BNE;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rs1, rs2, imm;
            args >> rs1 >> rs2 >> imm;

            inst.dest = -1;
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = std::stoi(imm);
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "blt"){
            inst.op = OpCode::BLT;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rs1, rs2, imm;
            args >> rs1 >> rs2 >> imm;

            inst.dest = -1;
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = std::stoi(imm);
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "ble"){
            inst.op = OpCode::BLE;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rs1, rs2, imm;
            args >> rs1 >> rs2 >> imm;

            inst.dest = -1;
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = std::stoi(imm);
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "j"){
            inst.op = OpCode::J;

            std::string imm;
            iss >> imm;

            inst.dest = -1;
            inst.src1 = -1;
            inst.src2 = -1;
            inst.imm = std::stoi(imm);
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "slt"){
            inst.op = OpCode::SLT;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, rs2;
            args >> rd >> rs1 >> rs2;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = 0;
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if (opcode == "slti"){
            inst.op = OpCode::SLTI;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, imm;
            args >> rd >> rs1 >> imm;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = -1;
            inst.imm = std::stoi(imm);
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "and"){
            inst.op = OpCode::AND;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, rs2;
            args >> rd >> rs1 >> rs2;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = 0;
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "or"){
            inst.op = OpCode::OR;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, rs2;
            args >> rd >> rs1 >> rs2;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = 0;
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if(opcode == "xor"){
            inst.op = OpCode::XOR;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, rs2;
            args >> rd >> rs1 >> rs2;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = std::stoi(rs2.substr(1));
            inst.imm = 0;
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if (opcode == "andi"){
            inst.op = OpCode::ANDI;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, imm;
            args >> rd >> rs1 >> imm;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = -1;
            inst.imm = std::stoi(imm);
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if (opcode == "ori"){
            inst.op = OpCode::ORI;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, imm;
            args >> rd >> rs1 >> imm;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = -1;
            inst.imm = std::stoi(imm);
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }

        else if (opcode == "xori"){
            inst.op = OpCode::XORI;
            
            std::string rest ;
            std::getline(iss,rest);

            for (char&c : rest){
                if (c == ','){c = ' ';}
            }

            std::istringstream args(rest);
            std::string rd, rs1, imm;
            args >> rd >> rs1 >> imm;

            inst.dest = std::stoi(rd.substr(1));
            inst.src1 = std::stoi(rs1.substr(1));
            inst.src2 = -1;
            inst.imm = std::stoi(imm);
            inst.pc = inst_memory.size();

            inst_memory.push_back(inst);
        }
        
    }
}