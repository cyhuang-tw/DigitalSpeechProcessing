#!/bin/bash

for i in {1..10};
do
	perl separator_big5.pl ./testdata/$i.txt > $i.txt;
	mv $i.txt ./testdata
done