#!/bin/bash

make clean
make

for i in {1..10}
do
   echo "*****************"	
   echo	
   echo "index "$i   
   ./capture
   echo "done"
done
