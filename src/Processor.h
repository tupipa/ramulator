#ifndef __PROCESSOR_H
#define __PROCESSOR_H

#define __ENABLE_MULTICORES

#include "Config.h"
#include "Request.h"
#include "Statistics.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <ctype.h>
#include <functional>

namespace ramulator 
{
	
//ScalarStat cpu_cycles_main;
//VectorStat core_cycles_main;

class Trace {
public:

	const char* fname;

    Trace(const char* trace_fname);
    // trace file format 1:
    // [# of bubbles(non-mem instructions)] [read address(dec or hex)] <optional: write address(evicted cacheline)>
    bool get_request(long& bubble_cnt, long& req_addr, Request::Type& req_type);
    // trace file format 2:
    // [address(hex)] [R/W]
    bool get_request(long& req_addr, Request::Type& req_type);

private:
    std::ifstream file;
};


/*
* Window class: 
*	simulate the ooo buffer ? 
*
*the window is presented by 'vector<bool> ready_list' and 'vector<long> addr_list',
 		 using 'tail' and 'head' to index the last and first elements; The head is the newest inserted.
*/

class Window {
public:
    int ipc = 4;
    int depth = 128;

    Window() : ready_list(depth), addr_list(depth, -1) {}
    bool is_full();
    bool is_empty();
    void insert(bool ready, long addr);
    long retire();
    void set_ready(long addr);

private:
    int load = 0;
    int head = 0;
    int tail = 0;
    std::vector<bool> ready_list;
    std::vector<long> addr_list;
};


class Processor {
public:
	int id=0;
    long clk = 0;
    long retired = 0;
    function<bool(Request)> send;

    Processor(const Config& configs, const char* trace_fname, function<bool(Request)> send);
	Processor(const Config& configs, const char* trace_fname, function<bool(Request)> send, int id);
	int getID();
	void tick();
    void receive(Request& req);
    double calc_ipc();
    bool finished();
    function<void(Request&)> callback; 

private:
    Trace trace;
    Window window;

    long bubble_cnt;
    long req_addr;
    Request::Type req_type;
    bool more_reqs;

    ScalarStat memory_access_cycles;
    ScalarStat cpu_inst;
    ScalarStat cpu_cycles;
	ScalarStat core_cycles; //lele: cycles per core.
    long last = 0;
};

}
#endif /* __PROCESSOR_H */
