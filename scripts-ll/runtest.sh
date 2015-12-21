#!/bin/bash

mode=multicores

traceFolder1=cputraces

traceFolder2=cputrace2


#########################
#
# runtraces(), must have four parameters
# <trace1,folder1,trace2,folder2>
#
#########################
function runtraces(){

    trace1=$1
    folder1=$2
    trace2=$3
    folder2=$4

	tracefiles="$folder1/$trace1 $folder2/$trace2"

	echo "trace file : $tracefiles"
    	#use flag to name the log file
	flag="${trace1:0:7}.${trace2:0:7}" #cut 'cputraces/xxx.xxx.'
	
	# compose the command 
	ramcmd="./ramulatorMulti configs/DDR3-config.cfg --mode=$mode $tracefiles 2>&1 > log.multi"
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
}


#######################
# main function here
#######################


##############################
# first run two same traces for each trace

for tr1 in $(ls $traceFolder1)
do
    #run 2 same trace file for $trace
    #runtraces $tr1 $traceFolder1 $tr1 $traceFolder1
    echo "ignore pair <$tr1 $tr1>"
done
for tr2 in $(ls $traceFolder2)
do
#	runtraces $tr2 $traceFolder2 $tr2 $traceFolder2
	echo "ignore pair <$tr2,$tr2>"
done

############################
# 2, pair each trace in Folder1 with Folder2
############################

for tr1 in $(ls $traceFolder1)
do
 for tr2 in $(ls $traceFolder2)
 do
     runtraces $tr1 $traceFolder1 $tr2 $traceFolder2
 done
done


