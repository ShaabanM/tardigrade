make clean
make

for i in {1..1}
do
   echo "*****************"	
   echo	
   echo "index "$i   
   ./capture
   echo "done"
done
