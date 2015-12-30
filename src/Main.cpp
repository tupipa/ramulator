#include "Processor.h"
#include "Config.h"
#include "Controller.h"
#include "SpeedyController.h"
#include "Memory.h"
#include "DRAM.h"
#include "Statistics.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include <functional>
#include <map>

/* Standards */
#include "Gem5Wrapper.h"
#include "DDR3.h"
#include "DDR4.h"
#include "DSARP.h"
#include "GDDR5.h"
#include "LPDDR3.h"
#include "LPDDR4.h"
#include "WideIO.h"
#include "WideIO2.h"
#include "HBM.h"
#include "SALP.h"
#include "ALDRAM.h"
#include "TLDRAM.h"

using namespace std;
using namespace ramulator;

template<typename T>
void run_dramtrace(const Config& configs, Memory<T, Controller>& memory, const char* tracename) {

    /* initialize DRAM trace */
    Trace trace(tracename);

    /* run simulation */
    bool stall = false, end = false;
    int reads = 0, writes = 0, clks = 0;
    long addr = 0;
    Request::Type type = Request::Type::READ;
    map<int, int> latencies;
    auto read_complete = [&latencies](Request& r){latencies[r.depart - r.arrive]++;};

    Request req(addr, type, read_complete);

    while (!end || memory.pending_requests()){
        if (!end && !stall){
            end = !trace.get_request(addr, type);
        }

        if (!end){
            req.addr = addr;
            req.type = type;
            stall = !memory.send(req);
            if (!stall){
                if (type == Request::Type::READ) reads++;
                else if (type == Request::Type::WRITE) writes++;
            }
        }
        memory.tick();
        clks ++;
        Stats::curTick++; // memory clock, global, for Statistics
    }
    // This a workaround for statistics set only initially lost in the end
    memory.finish();
    Stats::statlist.printall();

}


/* @configs, Config instance from config file.
 *
 * @memory, the instance of Memory.
 *
 * @file, trace filename.
 * 
 * create Processor proc. run Processor::tick in loop, 
 *  and run one memory.tick after a certain number of cpu_tick.
 * 
 *
 */

template <typename T>
void run_cputrace(const Config& configs, Memory<T, Controller>& memory, const char * file)
{
    int cpu_tick = configs.get_cpu_tick();
    int mem_tick = configs.get_mem_tick();
    // create a send function by combining Memory::send with 
    //  first parameter fixed to &memory and second parameter left undetermined.
    //  send function can be called like 'send(parameter2);' where parameter2 will fill the placeholder _1.
    
    auto send = bind(&Memory<T, Controller>::send, &memory, placeholders::_1);
    // create Processor using <configs, trace filename, send function);>
    //  matching constructor Processor(const Config& configs, const char* trace_fname, function<bool(Request)> send)
    Processor proc(configs, file, send);
    //Processor proc0(configs, file, send);
    //Processor proc1(configs, file, send);
    for (long i = 0; ; i++) {
        proc.tick(); //
        //proc0.tick(); //
        //proc1.tick(); //
        Stats::curTick++; // processor clock, global, for Statistics
        if (i % cpu_tick == (cpu_tick - 1)) // do this branch every 4(=cpu_tick) cpu cycles, but why (cpu_tick-1) instead of cpu_tick
            for (int j = 0; j < mem_tick; j++)// ? what relation: cpu_tick v.s. mem_tick; why do 1 mem per 4 cpu ticks?(1=mem_tick, 4=cpu_tick)
                memory.tick();
      if (configs.is_early_exit()) {
        if (proc.finished())
            break;
      } else {
        if (proc.finished() && (memory.pending_requests() == 0))
            break;
      }
    }
    // This a workaround for statistics set only initially lost in the end
    memory.finish();
    Stats::statlist.printall();
}

#ifdef __ENABLE_MULTICORES

template <typename T>
void run_cputraces(const Config& configs, 
							Memory<T, Controller>& memory, 
							std::vector <const char *> files, 
							int filesCount)
{
	
	if(filesCount<2){ 
		printf("Need at least 2 trace files as input traces. Now do nothing and exit.\n");
		
		exit(-1);
	}
	// get number of cores from config file
	int coresCount = configs.get_cores_count();
	
	//printf("lele:in %s: coresCount parsed and read from config: %d. \n",__PRETTY_FUNCTION__,coresCount);
    printf("lele:in %s: coresCount parsed and read from config: %d. \n",__FUNCTION__,coresCount);
    int cpu_tick = configs.get_cpu_tick();
    int mem_tick = configs.get_mem_tick();
	
	
	//printf("lele:in %s: cpu_tick= %d.\t mem_tick=%d\n",__PRETTY_FUNCTION__,cpu_tick,mem_tick);
	printf("lele:in %s: cpu_tick= %d.\t mem_tick=%d\n",__FUNCTION__,cpu_tick,mem_tick);


	
	//exit(0); // for test
	
    // create a send function by combining Memory::send with 
    //  first parameter fixed to &memory and second parameter left undetermined.
    //  send function can be called like 'send(parameter2);' where parameter2 will fill the placeholder _1.
    
    auto send = bind(&Memory<T, Controller>::send, &memory, placeholders::_1);

	// for test:
		// add counters for cpu and per-core ticks here for comparing and verification
		
		ScalarStat cpu_cycles_main;
		VectorStat core_cycles_main;
		
	
		cpu_cycles_main.name("cpu_cycles_main")
				  .desc("lele: CPU cycle of the whole execution(test for Comparing)")
				  .precision(0)
				  ;
		cpu_cycles_main = 0;
		
		core_cycles_main
				.init(coresCount)
				.name("core_cycles_main")
				.desc("lele: core cycles during the whole execution(test for comparing)")
				;

	// create Processor using <configs, trace filename, send function);>
    //  matching constructor Processor(const Config& configs, const char* trace_fname, function<bool(Request)> send)
    //Processor proc(configs, file, send);
    //Processor proc0(configs, file, send);
    //Processor proc1(configs, file, send);
    std::vector<Processor*> cores;
     
    for (int id=0;id<coresCount && id<filesCount;id++){
		// TODO: now only simple schedule: one file for one core and no core can run two files.
		Processor* proc = new Processor(configs,files[id], send, id);
	    cores.push_back(proc);
		printf("lele: create core %d, running file %s\n",id,files[id]);
	}

	
    for (long i = 0; ; i++) {
        //proc.tick(); 
        for (auto core : cores) {
          core->tick();
		  ++core_cycles_main[core->getID()];
		  ++cpu_cycles_main;
		  //printf("lele: in %s: core %d tick\n",__FUNCTION__,core->getID());
          //Stats::curTick++; // processor clock, global, for Statistics
        }	
        Stats::curTick++; // processor clock, global, for Statistics
        if (i % cpu_tick == (cpu_tick - 1)) // do this branch every 4(=cpu_tick) cpu cycles, but why (cpu_tick-1) instead of cpu_tick
            for (int j = 0; j < mem_tick; j++)// ? what relation: cpu_tick v.s. mem_tick; why do 1 mem per 4 cpu ticks?(1=mem_tick, 4=cpu_tick)
                memory.tick();
      if (configs.is_early_exit()) {
        // TODO LELE: use all_finished to simulate this.

		//if (proc.finished())
		//	break; 
		
		bool all_finished=true;
		for (auto core : cores) {
          if(!core->finished()) {
		  	all_finished=false; 
		  	//printf("lele: in %s: core %d not finished\n",__FUNCTION__,core->getID());
		  	//break;
		  }
        }	
        if (all_finished) break; 
		
      } else {
        //TODO
        
	//if (proc.finished() && (memory.pending_requests() == 0))
    //        break;
    	bool all_finished=true;
		for (auto core : cores) {
          if(!core->finished()) {
		  	all_finished=false; 
		  	//printf("lele: in %s: core %d not finished\n",__FUNCTION__,core->getID());
		  	break;
		  }
        }	
        if (all_finished && (memory.pending_requests() == 0)) break; 
      }
    }
    // This a workaround for statistics set only initially lost in the end
    memory.finish();
    Stats::statlist.printall();
}



/* @configs, Config instance from config file.
 *
 * @spec, the instance of different standards.
 *
 * @file, trace filename.
 * 
 * initial channels and ranks, create DRAM channel, Controller, and Memory
 * then run the trace via <configs, memory, file>
 *
 */
template<typename T>
void start_run(const Config& configs, T* spec, std::vector <const char*> files, int filesCount) {
  // initiate controller and memory
  int C = configs.get_channels(), R = configs.get_ranks();
  // Check and Set channel, rank number
  spec->set_channel_number(C);
  spec->set_rank_number(R);
  std::vector<Controller<T>*> ctrls;
  for (int c = 0 ; c < C ; c++) {
    DRAM<T>* channel = new DRAM<T>(spec, T::Level::Channel);
    channel->id = c;
    channel->regStats("");
    Controller<T>* ctrl = new Controller<T>(configs, channel);
    ctrls.push_back(ctrl);
  }
  Memory<T, Controller> memory(configs, ctrls);

  if (configs["trace_type"] == "MULTICORES") {
  	printf("lele: in %s, multicores mode\n",__FUNCTION__);
	//printf("lele: in %s,\n",__PRETTY_FUNCTION__);
    run_cputraces(configs, memory, files,filesCount);
  } else if (configs["trace_type"] == "DRAM") {
    //run_dramtrace(configs, memory, file);
    printf("TODO: run dram trace not implemented yet.\n");
	exit(0);
  }
}


#endif /* __ENABLE_MULTICORES */


/* @configs, Config instance from config file.
 *
 * @spec, the instance of different standards.
 *
 * @file, trace filename.
 * 
 * initial channels and ranks, create DRAM channel, Controller, and Memory
 * then run the trace via <configs, memory, file>
 *
 */
template<typename T>
void start_run(const Config& configs, T* spec, const char* file) {
  // initiate controller and memory
  int C = configs.get_channels(), R = configs.get_ranks();
  // Check and Set channel, rank number
  spec->set_channel_number(C);
  spec->set_rank_number(R);
  std::vector<Controller<T>*> ctrls;
  for (int c = 0 ; c < C ; c++) {
    DRAM<T>* channel = new DRAM<T>(spec, T::Level::Channel);
    channel->id = c;
    channel->regStats("");
    Controller<T>* ctrl = new Controller<T>(configs, channel);
    ctrls.push_back(ctrl);
  }
  Memory<T, Controller> memory(configs, ctrls);

  if (configs["trace_type"] == "CPU") {
    run_cputrace(configs, memory, file);
  } else if (configs["trace_type"] == "DRAM") {
    run_dramtrace(configs, memory, file);
  }
}

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <configs-file> --mode=cpu,dram,multicore [--stats <filename>] <trace-filename>\n"
            "Example: %s ramulator-configs.cfg cpu.trace\n", argv[0], argv[0]);
        return 0;
    }
    printf("number of arg: argc:%d\n",argc);

	bool enable_multicores=false;
	
	/* create instance of Config using config file argv[1]*/
    Config configs(argv[1]);
	
	printf("lele: done reading and parsing config file: %s\n",argv[1]);
	
    const std::string& standard = configs["standard"];
    assert(standard != "" || "DRAM standard should be specified.");

    const char *trace_type = strstr(argv[2], "=");
    trace_type++;
	
	printf("lele: trace type: %s\n", trace_type);
	
    if (strcmp(trace_type, "cpu") == 0) {
      configs.add("trace_type", "CPU");
    } else if (strcmp(trace_type, "dram") == 0) {
      configs.add("trace_type", "DRAM");
    } else if (strcmp(trace_type, "multicores")==0){
      configs.add("trace_type", "MULTICORES");
	  enable_multicores=true;
    }else{
      printf("invalid trace type: %s\n", trace_type);
      assert(false);
    }

    int trace_start = 3;
    string stats_out;
	
	// parse the output file name
    if (strcmp(argv[3], "--stats") == 0) {
      Stats::statlist.output(argv[4]);
      stats_out = argv[4];
      trace_start = 5;
    } else {
      Stats::statlist.output(standard+".stats");
      stats_out = standard + string(".stats");
    }

	/* get the trace file name(s) */
    const char* file=0;
    std::vector <const char*> files;
	
	int filesCount=0;
	if(!enable_multicores){
	  file = argv[trace_start];
	}else{
	  printf("lele: multicore enabled\n");
	  filesCount=0;
	  
	  for(int i=trace_start;i<argc;i++){
		const char *onefile=argv[i];
		files.push_back(onefile);
		printf("trace file[%d]: %s\n",filesCount,files[filesCount]);
  		filesCount++;
	  }
	}
	//return 0;
	/* 1) parse the standard name, 
	 * 2) create the instance for that standard Class,
	 * 3) and start to run with 
	 		<configs, standard, trace> if in cpu mode.
	 * 		<configs, standard, traceVector, fileCount> if in multicores mode.
	 */
    if (standard == "DDR3") {
      DDR3* ddr3 = new DDR3(configs["org"], configs["speed"]);
	  
	  if(enable_multicores){
	  	printf("lele: in %s: start_run multicore simulation.\n",__PRETTY_FUNCTION__);
      	start_run(configs, ddr3, files,filesCount);
	  }else{
	  	printf("lele: in %s: start_run singlecore simulation.\n",__PRETTY_FUNCTION__);
	  	start_run(configs, ddr3, file);
	  }
    }
	//TODO: other types 
	else if (standard == "DDR4") {
      DDR4* ddr4 = new DDR4(configs["org"], configs["speed"]);
      start_run(configs, ddr4,  file);
    } else if (standard == "SALP-MASA") {
      SALP* salp8 = new SALP(configs["org"], configs["speed"], "SALP-MASA", configs.get_subarrays());
      start_run(configs, salp8, file);
    } else if (standard == "LPDDR3") {
      LPDDR3* lpddr3 = new LPDDR3(configs["org"], configs["speed"]);
      start_run(configs, lpddr3, file);
    } else if (standard == "LPDDR4") {
      // total cap: 2GB, 1/2 of others
      LPDDR4* lpddr4 = new LPDDR4(configs["org"], configs["speed"]);
      start_run(configs, lpddr4, file);
    } else if (standard == "GDDR5") {
      GDDR5* gddr5 = new GDDR5(configs["org"], configs["speed"]);
      start_run(configs, gddr5, file);
    } else if (standard == "HBM") {
      HBM* hbm = new HBM(configs["org"], configs["speed"]);
      start_run(configs, hbm, file);
    } else if (standard == "WideIO") {
      // total cap: 1GB, 1/4 of others
      WideIO* wio = new WideIO(configs["org"], configs["speed"]);
      start_run(configs, wio, file);
    } else if (standard == "WideIO2") {
      // total cap: 2GB, 1/2 of others
      WideIO2* wio2 = new WideIO2(configs["org"], configs["speed"], configs.get_channels());
      wio2->channel_width *= 2;
      start_run(configs, wio2, file);
    }
    // Various refresh mechanisms
      else if (standard == "DSARP") {
      DSARP* dsddr3_dsarp = new DSARP(configs["org"], configs["speed"], DSARP::Type::DSARP, configs.get_subarrays());
      start_run(configs, dsddr3_dsarp, file);
    } else if (standard == "ALDRAM") {
      ALDRAM* aldram = new ALDRAM(configs["org"], configs["speed"]);
      start_run(configs, aldram, file);
    } else if (standard == "TLDRAM") {
      TLDRAM* tldram = new TLDRAM(configs["org"], configs["speed"], configs.get_subarrays());
      start_run(configs, tldram, file);
    }

    printf("Simulation done. Statistics written to %s\n", stats_out.c_str());

    return 0;
}
