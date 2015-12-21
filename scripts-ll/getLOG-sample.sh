#!/bin/bash

declare -i files

files=2

trace="cpu.trace"
traceRun="cpu.trace"

while [ $files -lt 16 ]
do
        traceRun="$traceRun $trace"

	ramcmd="./ramulatorMulti configs/DDR3-config.cfg --mode=multicores $traceRun > log.multi"
	
	logFile="v0.2-07-multi-$files-$trace-DDR3.stats"

#	echo "$ramcmd"

	echo "$ramcmd" > $logFile
	
	eval $ramcmd

#	echo "cat DDR3.stats >> $logFile"

#	sleep 4
	
	cat DDR3.stats >>  $logFile

#	echo "cat log.multi >> $logFile"

	cat log.multi >> $logFile

	rm log.multi
	rm DDR3.stats

	let files+=1

done

