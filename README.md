#######################################################################
#
#	this is a quick start up for RamulatorMulti, a multicored version of Ramulator.


#	source files can be got by:
		$git clone https://github.com/tupipa/ramulator -b dualcore


## Usage

RamulatorMulti supports four different usage modes.

1. **Memory Trace Driven:** Ramulator directly reads memory traces from a
  file, and simulates only the DRAM subsystem. Each line in the trace file 
  represents a memory request, with the hexadecimal address followed by 'R' 
  or 'W' for read or write.

  - 0x12345680 R
  - 0x4cbd56c0 W
  - ...


2. **Single Core CPU Trace Driven:** Ramulator directly reads instruction traces from a 
  file, and simulates a simplified model of a "core" that generates memory 
  requests to the DRAM subsystem. Each line in the trace file represents a 
  memory request, and can have one of the following two formats.

  - `<num-cpuinst> <addr-read>`: For a line with two tokens, the first token 
        represents the number of CPU (i.e., non-memory) instructions before
        the memory request, and the second token is the decimal address of a
        *read*. 

  - `<num-cpuinst> <addr-read> <addr-writeback>`: For a line with three tokens,
        the third token is the decimal address of the *writeback* request, 
        which is the dirty cache-line eviction caused by the read request
        before it.
		
3. **Multi Core CPU Traces Driven:** Ramulator directly reads instruction traces from a 
  file, and simulates a simplified model of a "multi-cores" that generates memory 
  requests to the DRAM subsystem. Each core servicing one trace file represents a
  process running on that core.

4. **gem5 Driven:** Ramulator runs as part of a full-system simulator (gem5
  \[6\]), from which it receives memory request as they are generated.


## Getting Started

RamulatorMulti requires a C++11 compiler (e.g., `clang++`, `g++-5`).

1. **Memory Trace Driven**

        $ cd ramulator
        $ make -j
        $ ./ramulatorMulti configs/DDR3-config.cfg --mode=dram dram.trace
        Simulation done. Statistics written to DDR3.stats
        # NOTE: dram.trace is a very short trace file provided only as an example.
        $ ./ramulatorMulti configs/DDR3-config.cfg --mode=dram --stats my_output.txt dram.trace
        Simulation done. Statistics written to my_output.txt
        # NOTE: optional --stats flag changes the statistics output filename


2. **Single Core CPU Core Trace Driven**

        $ cd ramulator
        $ make -j
        $ ./ramulatorMulti configs/DDR3-config.cfg --mode=cpu cpu.trace
		#(many outputs here)
        Simulation done. Statistics written to DDR3.stats
        # NOTE: cpu.trace is a very short trace file provided only as an example.
        $ ./ramulatorMulti configs/DDR3-config.cfg --mode=cpu --stats my_output.txt cpu.trace
		#(many outputs here)
        Simulation done. Statistics written to my_output.txt
        # NOTE: optional --stats flag changes the statistics output filename

3. **Multicore CPU Core Trace Driven**
		# Note: Before you run with multicore feature, you should set the number of cores in the configure file for each DRAM standard.
		#
		#	An example setting can be seen in configs/DDR3-config.cfg. The line 
		#		cores_count = 3
		# 	is to set three cores for the CPU in RamulatorMulti.
		# 		
        $ cd ramulator
        $ make -j
        $ ./ramulatorMulti configs/DDR3-config.cfg --mode=multicores cpu.trace cpu.trace cpu.trace
        #(many outputs here)
        Simulation done. Statistics written to DDR3.stats
        # NOTE: cpu.trace is a very short trace file provided only as an example.
        # you can append as many trace file as you need to the end of the command
        # TODO: currently, each core compute one file only. Files more than number of cores will be ignored. 
        # So, if you have 4 cores, the first 4 trace files will be computed, other will be ignored if you have.
        
        $ ./ramulatorMulti configs/DDR3-config.cfg --mode=cpu --stats my_output.txt cpu.trace cpu.trace cpu.trace
        #(many outputs here)
        Simulation done. Statistics written to my_output.txt
        # NOTE: optional --stats flag changes the statistics output filename




#####################################################################################################################
#
#	below is original for original version of Ramulator: https://github.com/CMU-SAFARI/ramulator
#
#


# Ramulator: A DRAM Simulator

Ramulator is a fast and cycle-accurate DRAM simulator \[1\] that supports a
wide array of commercial, as well as academic, DRAM standards:

- DDR3 (2007), DDR4 (2012)
- LPDDR3 (2012), LPDDR4 (2014)
- GDDR5 (2009)
- WIO (2011), WIO2 (2014)
- HBM (2013)
- SALP \[2\]
- TL-DRAM \[3\]
- RowClone \[4\]
- DSARP \[5\]

[\[1\] Kim et al. *Ramulator: A Fast and Extensible DRAM Simulator.* IEEE CAL
2015.](http://dx.doi.org/10.1109/LCA.2015.2414456)  
[\[2\] Kim et al. *A Case for Exploiting Subarray-Level Parallelism (SALP) in
DRAM.* ISCA 2012.](http://dx.doi.org/10.1109/ISCA.2012.6237032)  
[\[3\] Lee et al. *Tiered-Latency DRAM: A Low Latency and Low Cost DRAM
Architecture.* HPCA 2013.](http://dx.doi.org/10.1109/HPCA.2013.6522354)  
[\[4\] Seshadri et al. *RowClone: Fast and Energy-Efficient In-DRAM Bulk Data
Copy and Initialization.* MICRO
2013.](http://dx.doi.org/10.1145/2540708.2540725)  
[\[5\] Chang et al. *Improving DRAM Performance by Parallelizing Refreshes with
Accesses.* HPCA 2014.](http://dx.doi.org/10.1109/HPCA.2014.6835946)


## Usage

Ramulator supports three different usage modes.

1. **Memory Trace Driven:** Ramulator directly reads memory traces from a
  file, and simulates only the DRAM subsystem. Each line in the trace file 
  represents a memory request, with the hexadecimal address followed by 'R' 
  or 'W' for read or write.

  - 0x12345680 R
  - 0x4cbd56c0 W
  - ...


2. **CPU Trace Driven:** Ramulator directly reads instruction traces from a 
  file, and simulates a simplified model of a "core" that generates memory 
  requests to the DRAM subsystem. Each line in the trace file represents a 
  memory request, and can have one of the following two formats.

  - `<num-cpuinst> <addr-read>`: For a line with two tokens, the first token 
        represents the number of CPU (i.e., non-memory) instructions before
        the memory request, and the second token is the decimal address of a
        *read*. 

  - `<num-cpuinst> <addr-read> <addr-writeback>`: For a line with three tokens,
        the third token is the decimal address of the *writeback* request, 
        which is the dirty cache-line eviction caused by the read request
        before it.

3. **gem5 Driven:** Ramulator runs as part of a full-system simulator (gem5
  \[6\]), from which it receives memory request as they are generated.

For some of the DRAM standards, Ramulator is also capable of reporting
power consumption by relying on DRAMPower \[7\] as the backend. 

[\[6\] The gem5 Simulator System.](http://www.gem5.org)  
[\[7\] Chandrasekar et al. *DRAMPower: Open-Source DRAM Power & Energy
Estimation Tool.* IEEE CAL 2015.](http://www.drampower.info)


## Getting Started

Ramulator requires a C++11 compiler (e.g., `clang++`, `g++-5`).

1. **Memory Trace Driven**

        $ cd ramulator
        $ make -j
        $ ./ramulator configs/DDR3-config.cfg --mode=dram dram.trace
        Simulation done. Statistics written to DDR3.stats
        # NOTE: dram.trace is a very short trace file provided only as an example.
        $ ./ramulator configs/DDR3-config.cfg --mode=dram --stats my_output.txt dram.trace
        Simulation done. Statistics written to my_output.txt
        # NOTE: optional --stats flag changes the statistics output filename

2. **CPU Trace Driven**

        $ cd ramulator
        $ make -j
        $ ./ramulator configs/DDR3-config.cfg --mode=cpu cpu.trace
        Simulation done. Statistics written to DDR3.stats
        # NOTE: cpu.trace is a very short trace file provided only as an example.
        $ ./ramulator configs/DDR3-config.cfg --mode=cpu --stats my_output.txt cpu.trace
        Simulation done. Statistics written to my_output.txt
        # NOTE: optional --stats flag changes the statistics output filename

3. **gem5 Driven**

   *Requires SWIG 2.0.12+, gperftools (`libgoogle-perftools-dev` package on Ubuntu)*

        $ hg clone http://repo.gem5.org/gem5-stable
        $ cd gem5-stable
        $ hg update -c 10231  # Revert to stable version from 5/31/2014 (10231:0e86fac7254c)
        $ patch -Np1 --ignore-whitespace < /path/to/ramulator/gem5-0e86fac7254c-ramulator.patch
        $ cd ext/ramulator
        $ mkdir Ramulator
        $ cp -r /path/to/ramulator/src Ramulator
        # Compile gem5
        # Run gem5 with `--mem-type=ramulator` and `--ramulator-config=configs/DDR3-config.cfg`

        
## Simulation Output

Ramulator will report a series of statistics for every run, which are written
to a file.  We have provided a series of gem5-compatible statistics classes in
`Statistics.h`.

**Memory Trace/CPU Trace Driven**: When run in memory trace driven or CPU trace
driven mode, Ramulator will write these statistics to a file.  By default, the
filename will be `<standard_name>.stats` (e.g., `DDR3.stats`).  You can write
the statistics file to a different filename by adding `--stats <filename>` to
the command line after the `--mode` switch (see examples above).

**gem5 Driven**: Ramulator automatically integrates its statistics into gem5.
Ramulator's statistics are written directly into the gem5 statistic file, with
the prefix `ramulator.` added to each stat's name.

*NOTE: When creating your own stats objects, don't place them inside STL
containers that are automatically resized (e.g, vector).  Since these
containers copy on resize, you will end up with duplicate statistics printed
in the output file.*


## Reproducing Results from Paper (Kim et al. \[1\])


### Debugging & Verification (Section 4.1)

For debugging and verification purposes, Ramulator can print the trace of every
DRAM command it issues along with their address and timing information. To do
so, please turn on the `print_cmd_trace` variable in the configuration file.


### Comparison Against Other Simulators (Section 4.2)

For comparing Ramulator against other DRAM simulators, we provide a script that
automates the process: `test_ddr3.py`. Before you run this script, however, you
must specify the location of their executables and configuration files at
designated lines in the script's source code: 

* Ramulator
* DRAMSim2 (https://wiki.umd.edu/DRAMSim2): `test_ddr3.py` lines 39-40
* USIMM, (http://www.cs.utah.edu/~rajeev/jwac12): `test_ddr3.py` lines 54-55
* DrSim (http://lph.ece.utexas.edu/public/Main/DrSim): `test_ddr3.py` lines 66-67
* NVMain (http://wiki.nvmain.org): `test_ddr3.py`  lines 78-79

Please refer to their respective websites to download, build, and set-up the
other simulators. The simulators must to be executed in saturation mode (always
filling up the request queues when possible).

All five simulators were configured using the same parameters:

* DDR3-1600K (11-11-11), 1 Channel, 1 Rank, 2Gb x8 chips
* FR-FCFS Scheduling
* Open-Row Policy
* 32/32 Entry Read/Write Queues
* High/Low Watermarks for Write Queue: 28/16

Finally, execute `test_ddr3.py <num-requests>` to start off the simulation.
Please make sure that there are no other active processes during simulation to
yield accurate measurements of memory usage and CPU time.


### Cross-Sectional Study of DRAM Standards (Section 4.3)

Please use the CPU traces (SPEC 2006) provided in the `cputraces` folder to run
CPU trace driven simulations.


## Other Tips

### Power Estimation

For estimating power consumption, Ramulator can record the trace of every DRAM
command it issues to a file in DRAMPower \[7\] format.  To do so, please turn
on the `record_cmd_trace` variable in the configuration file.  The resulting
DRAM command trace (e.g., `cmd-trace-chan-N-rank-M.cmdtrace`) should be fed
into DRAMPower with the correct configuration (standard/speed/organization)
to estimate energy/power usage for a single rank (a limitation of DRAMPower).


### Contributors

- Yoongu Kim (Carnegie Mellon University)
- Weikun Yang (Peking University)
- Kevin Chang (Carnegie Mellon University)
- Donghyuk Lee (Carnegie Mellon University)
- Vivek Seshadri (Carnegie Mellon University)
- Saugata Ghose (Carnegie Mellon University)
- Tianshi Li (Carnegie Mellon University)
- @henryzh
