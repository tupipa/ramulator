#include "Processor.h"
#include <cassert>
#include <sstream>
#include <string>

using namespace std;
using namespace ramulator;

/*
 * The Out-of-Order execution paradigm breaks up 
 * the processing of instructions into these steps:

	1, Instruction fetch.
	2, Instruction dispatch to an instruction queue (also called 
		instruction buffer or reservation stations).
	3, The instruction waits in the queue until its input operands
		are available. The instruction is then allowed to leave 
		the queue before earlier, older instructions.
	4, The instruction is issued to the appropriate functional unit
		and executed by that unit.
	5, The results are queued.
	6, Only after all older instructions have their results 
		written back to the register file, then this result is 
		written back to the register file. This is called the 
		graduation or retire stage.

 *	The key concept of OoOE processing is to allow the processor
 		to avoid a class of stalls that occur when the data needed
 		to perform an operation are unavailable. In the outline 
 		above, the OoOE processor avoids the stall that occurs in 
 		step (2) of the in-order processor when the instruction 
 		is not completely ready to be processed due to missing data.

 *	OoOE processors fill these "slots" in time with other instructions
 		that are ready, then re-order the results at the end to make 
 		it appear that the instructions were processed as normal.
 *
 * cite:https://en.wikipedia.org/wiki/Out-of-order_execution
 *      http://home.gwu.edu/~jiec/docs/sesc/sesc_intro.pdf


 * callback is bound to Processor::receive(). 
 *   This method is used for memory to send feedback to cpu when the data is ready for use.
 *		
 */


Processor::Processor(const Config& configs, const char* trace_fname, function<bool(Request)> send)
    : send(send), callback(bind(&Processor::receive, this, placeholders::_1)), trace(trace_fname)
{
    more_reqs = trace.get_request(bubble_cnt, req_addr, req_type);

    // regStats
    memory_access_cycles.name("memory_access_cycles")
                        .desc("cycle number in memory clock domain that there is at least one request in the queue of memory controller")
                        .precision(0)
                        ;
    memory_access_cycles = 0;
    cpu_inst.name("cpu_instructions")
            .desc("commited cpu instruction number")
            .precision(0)
            ;
    cpu_inst = 0;
    cpu_cycles.name("cpu_cycles")
              .desc("CPU cycle of the whole execution")
              .precision(0)
              ;
    cpu_cycles = 0;
	
	
}
Processor::Processor(const Config& configs, const char* trace_fname, function<bool(Request)> send, int id)
  : id(id),send(send), callback(bind(&Processor::receive, this, placeholders::_1)), trace(trace_fname)
{
    more_reqs = trace.get_request(bubble_cnt, req_addr, req_type);

    // regStats
    memory_access_cycles.name("memory_access_cycles")
                        .desc("cycle number in memory clock domain that there is at least one request in the queue of memory controller")
                        .precision(0)
                        ;
    memory_access_cycles = 0;
    cpu_inst.name("cpu_instructions")
            .desc("commited cpu instruction number")
            .precision(0)
            ;
    cpu_inst = 0;
    cpu_cycles.name("cpu_cycles")
              .desc("lele: CPU cycle of the whole execution")
              .precision(0)
              ;
    cpu_cycles = 0;
	
	//lele: try to count for cycles per core here:
    std::stringstream ss;
	//const char * stringSS=;
	ss<<"core_cycles on core["<<id<<"]";
	std::stringstream ss2;
	const char * stringSS2="lele: core cycles during execution of ";
	ss2<<stringSS2<<trace_fname;

	//core_cycles.name("core_cycles ["+id+"]")
        //      .desc("lele: core cycles during execution of:"+trace_fname)
	//.precision(0)
	//     ;
	core_cycles.name(ss.str())
	  		   .desc(ss2.str())
               .precision(0)
               ;
    core_cycles = 0;
	//core_cycles.init();
}




double Processor::calc_ipc()
{
    return (double) retired / clk;
}

/* Processor::tick()
 * 1, count for cpu cycle;
 * 2, simulate non-memory instructions: count along with window.ipc;
 * 3, simulate memory R/W:	read inserted in window with false (cpu need feedback from mem to proceed, call Window::set_ready(addr) when data is ready), 
 								but write not inserted in window (because cpu don't need the feedback to proceed);
 *							call send to count for memory request in Memory::send();
 *--Rq & ll.
 */

void Processor::tick() 
{
    clk++;
    cpu_cycles++;

	//lele: count for cycles per core here:
	core_cycles++;
	
    retired += window.retire();

    if (!more_reqs) return;
    // bubbles (non-memory operations)
    int inserted = 0;
    while (bubble_cnt > 0) {
        if (inserted == window.ipc) return; //ll: instructions exceed ipc of the window, a cpu cycle time out.
        if (window.is_full()) return; //ll: when a window is full, a cycle time out. 
        //Need to use another CPU cycle to run.
        //window.is_full() means: load == depth;

        window.insert(true, -1); //ll: insert instruction to window, emulating running.
        inserted++;
        bubble_cnt--;
        cpu_inst++;
    }

    if (req_type == Request::Type::READ) { // read request is inserted into ooo window, while write request is not.
        // read request
        if (inserted == window.ipc) return;
        if (window.is_full()) return;

        //Request req(req_addr, req_type, callback);
        Request req(req_addr, req_type, callback,id);
		if(clk%100==0) printf("lele: in %s: send read request <addr: 0x%lx, type %d> on core %d, cycle %ld\n"
				,__FUNCTION__,req_addr,req_type,id,clk);
        if (!send(req)) return;//ll: call 'send(req)'. count for request in channel ctrl.

        //cout << "Inserted: " << clk << "\n";

        window.insert(false, req_addr);
        cpu_inst++;
        more_reqs = trace.get_request(bubble_cnt, req_addr, req_type); //ll: get next request
        return;
    }
    else {
        // write request
        assert(req_type == Request::Type::WRITE);
        //Request req(req_addr, req_type, callback);
        Request req(req_addr, req_type, callback,id);
  		if(clk%100==0) printf("lele: in %s: send write request <addr: 0x%lx, type %d> on core %d, cycle %ld\n"
				,__FUNCTION__,req_addr,req_type,id,clk);
        if (!send(req)) return; //ll: call send(req), count for request in channel ctrl.
        cpu_inst++;
    }

    more_reqs = trace.get_request(bubble_cnt, req_addr, req_type);
}

bool Processor::finished()
{
    return !more_reqs && window.is_empty();
}
void Processor::receive(Request& req) 
{
    window.set_ready(req.addr);// when CPU get data from memory. Set ready for the address in the Window
    if (req.arrive != -1 && req.depart > last) { //what's this? After get one data in the Window?
      memory_access_cycles += (req.depart - max(last, req.arrive));
      last = req.depart;
    }
}

int Processor::getID(){
	return id;
}


bool Window::is_full()
{
    return load == depth;
}

bool Window::is_empty()
{
    return load == 0;
}


void Window::insert(bool ready, long addr)
{
    assert(load <= depth);

    ready_list.at(head) = ready;
    addr_list.at(head) = addr;

    head = (head + 1) % depth;
    load++;
}

/*Rq: retire: 
 * 		delete the ready Insts in the ooo window; return the number of Insts deleted.
 *
 *** 	*the window is presented by 'vector<bool> ready_list' and 'vector<long> addr_list',
 		 using 'tail' and 'head' to index the last and first elements; The head is the newest inserted.
 **/
long Window::retire()
{
    assert(load <= depth);

    if (load == 0) return 0;

    int retired = 0;
    while (load > 0 && retired < ipc) {
        if (!ready_list.at(tail)) 
            break;

        tail = (tail + 1) % depth;
        load--;
        retired++;
    }

    return retired;
}


void Window::set_ready(long addr)
{
    if (load == 0) return;

    for (int i = 0; i < load; i++) {
        int index = (tail + i) % depth;
        if (addr_list.at(index) != addr)
            continue;
        ready_list.at(index) = true;
    }
}



Trace::Trace(const char* trace_fname) : file(trace_fname)
{ 
	fname=trace_fname;
    if (!file.good()) {
        std::cerr << "Bad trace file: " << trace_fname << std::endl;
        exit(1);
    }else{
    	std::cout << "lelema: in Processor.cpp, Trace::Trace(): good trace file: " << trace_fname << std::endl;
    }
}

/*
 * read one request from Trace file.
 * 
 * 
 */

bool Trace::get_request(long& bubble_cnt, long& req_addr, Request::Type& req_type)
{
    static bool has_write = false;
    static long write_addr;
    static int line_num = 0;
	
	 //ll:'has_write' is a common var for all instances of Trace::get_request(), 
	 //ll: see static var: http://stackoverflow.com/questions/6223355/static-variables-in-class-methods
    if (has_write){
        bubble_cnt = 0;
        req_addr = write_addr;
        req_type = Request::Type::WRITE;
        has_write = false;
        return true;
    }
	//ll: read one line from trace file
    string line;
	getline(file, line);
    line_num ++;
	if (file.eof() || line.size() == 0) { //ll:reach the end of the line; or reach blank line; stop.
        file.clear();
        file.seekg(0);
        // getline(file, line);
        line_num = 0;
        return false;
    }

	
	//skip the line start with #. lele, 12-20-2015
	while (line.at(line.find_first_not_of(' '))=='#'){
		printf("lele: skip line %d in %s: '%s'\n",line_num,fname,line.c_str());
		getline(file, line);
    	line_num ++;
		if (file.eof() || line.size() == 0) { //ll:reach the end of the line; or reach blank line; stop.
	        file.clear();
	        file.seekg(0);
	        // getline(file, line);
	        line_num = 0;
	        return false;
    	}
	}
    //printf("lele: get line %d in %s: '%s'\n",line_num,fname,line.c_str());
	//exit(0);
	//ll: parse one request line. Format: <bubble, addr, type>
    size_t pos, end;
    bubble_cnt = std::stoul(line, &pos, 10);
    //std::cout << "lelema: in Processor.cpp, Trace::get_request(): Bubble_Count: " << bubble_cnt << std::endl;

    pos = line.find_first_not_of(' ', pos+1);
    req_addr = stoul(line.substr(pos), &end, 0);
    //std::cout << "lelema: in Processor.cpp, Trace::get_request(): Req_Addr: " << req_addr << std::endl;
    req_type = Request::Type::READ; // all read ????

    pos = line.find_first_not_of(' ', pos+end);
    if (pos != string::npos){
        has_write = true;
        write_addr = stoul(line.substr(pos), NULL, 0);
	//	std::cout << "lelema: in "<<__FUNCTION__<<": trace line "<<line_num<<"<" << bubble_cnt<<","<<req_addr <<","<<write_addr<<">"<< std::endl;
    }
	else{	
	//std::cout << "lelema: in "<<__FUNCTION__<<": trace line "<<line_num<<"<" << bubble_cnt<<","<<req_addr <<">"<< std::endl;
	}
    return true; //read a request successfully from the trace file.
}

bool Trace::get_request(long& req_addr, Request::Type& req_type)
{
    string line;
    getline(file, line);
    if (file.eof()) {
        return false;
    }
    size_t pos;
    req_addr = std::stoul(line, &pos, 16);

    pos = line.find_first_not_of(' ', pos+1);

    if (pos == string::npos || line.substr(pos)[0] == 'R')
        req_type = Request::Type::READ;
    else if (line.substr(pos)[0] == 'W')
        req_type = Request::Type::WRITE;
    else assert(false);
    return true;
}
