#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "Basics.h"

class LoadStoreQueue {
public:
    // LSQ reservation station
    int latency;
    
    bool has_result = false; // result flag
    bool has_exception = false; // exception flag

    int store_addr ;
    int store_data ;

    int result_val ;
    int result_tag ;

    std::vector <RSEntry> rs ;
    int rs_size;
    int front ;

    std::vector <PipeLineEntry> pipeline;


    LoadStoreQueue(){
        latency = 0;
        rs_size =0 ;
        front = 0;
        has_result = false;
        has_exception = false;

        result_val = 0;
        result_tag = -1;
        
        store_data = 0;
        store_addr = -1;
    }

    LoadStoreQueue(int lat, int size){
        latency = lat;
        rs_size = size ;
        front = 0;
        has_result = false;
        has_exception = false;

        result_val = 0;
        result_tag = -1;
        
        store_data = 0;
        store_addr = -1;

        rs.resize(rs_size);
    }
    
    void capture(int tag, int val);
    void executeCycle(std::vector<int>& Memory);
};