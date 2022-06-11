#!/bin/bash

make clean
make

for ((i=0; ;++i))
do
   echo "*****************"	
   echo	
   echo "index "$i   
   ./test_ueye
   echo "done"
done
