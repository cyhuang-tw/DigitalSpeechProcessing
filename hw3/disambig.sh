#!/bin/bash

for i in {1..10};
do
	/home/benny/srilm-1.5.10/bin/i686-m64/disambig -text ./testdata/$i.txt -map ZhuYin-Big5.map -lm bigram.lm -order 2 > ./result1/$i.txt
done