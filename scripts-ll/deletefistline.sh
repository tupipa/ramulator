#!/bin/bash

sourcefolder=$1

destfolder=$2

for file in $(ls $sourcefolder)
do
 
	sed '1d' $sourcefolder/$file > $destfolder/tmpfile; mv $destfolder/tmpfile $destfolder/$file

done
