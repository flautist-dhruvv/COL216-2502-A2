#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <climits>
#include "Basics.h"

class ExecutionUnit {
public:
    // per-unit reservation station
    UnitType name;
    int latency;
    
    bool has_result = false; // result flag
    bool has_exception = false; // exception flag

    std::vector <RSEntry> rs;
    int rs_size; //Max RS entries it can hold
    std::vector<RSEntry> rs;
    std::vector <PipeLineEntry> pipeline;

    int result_val;
    int result_tag;

    ExecutionUnit(){
        latency = 0;
        rs_size = 0;
        has_result = false;
        has_exception = false;
        result_val = 0;
        result_tag = -1;
    }

    ExecutionUnit(UnitType type, int lat, int size){
        name = type;
        latency = lat;
        rs_size = size;
        has_result= false;
        has_exception = false;
        result_val = 0;
        result_tag = -1;
        rs.resize(rs_size);
    }

    
    void capture(int tag, int val);
    void executeCycle();
};