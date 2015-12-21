#!/bin/bash

mode=$1

trace=$2

traceRun=$3

flag="${trace:10:7}.${traceRun:10:7}" #cut 'cputraces/xxx.xxx.'
        
traceRun="$traceRun $trace"

ramcmd="./ramulator configs/DDR3-config.cfg --mode=$mode $traceRun > log.multi"
	
logFile="v0.2-08-multi-$flag-DDR3.stats"


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

