#!/bin/bash

traceFolder=$1

lineStart=$2

for file in $(ls $traceFolder)
do
	lineData=$(grep $lineStart $traceFolder/$file)
	lineData="${file:10:6} ${file:16:15} $lineData"
	echo "$lineData"
done
