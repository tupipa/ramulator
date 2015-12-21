#!/bin/bash

mode=cpu

traceFolder=$1

traceFiles=$(ls $traceFolder)

echo "get trace files: $traceFiles"

for trace in $(ls $traceFolder)
do
	flag="${trace:0:7}" #cut 'cputraces/xxx.xxx.'
        
	ramcmd="./ramulatorMulti configs/DDR3-config.cfg --mode=$mode $traceFolder/$trace 2>&1 > log.multi"
	
	logFile="v0.2-08.0-multi-$flag-DDR3.stats"

	rm DDR3.stats
	rm log.multi

	echo "$ramcmd" > $logFile

	echo "now going to run $ramcmd"
	
	eval $ramcmd

	echo "done run $ramcmd"

	cat DDR3.stats >>  $logFile

	echo "done write DDR3.stats to log file :$logFile"

	cat log.multi >> $logFile.stdout

	echo "done write standard output to logfile: $logFile.stdout"

done


