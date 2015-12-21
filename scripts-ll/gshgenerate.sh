#!/bin/bash
exeGene=generate.exe

gcc generatetrace.c -o $exeGene

declare -i ii

ii=8

while [ $ii -gt 0 ]
do 
	./$exeGene $ii > scan.$ii.step
	let ii-=1
done 
