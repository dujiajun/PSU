#!/bin/bash

export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH

if [ $# -gt 0 ]
then
    dir=$1
else
    dir="data"
fi

MAX_THRDS=8


echo "===  Unbalanced set  ==="
echo "Unbalanced set. Cuckoo hash num: 3. Cuckoo scaler: 1.27."

SMALL_SIZE=(8 12)
BIG_SIZE=(16 20 24)

for small in ${SMALL_SIZE[@]}
do
    for big in ${BIG_SIZE[@]}
    do
        for ((thrds=1;thrds<=MAX_THRDS;thrds*=2))
        do
            echo "Unbalanced shuffle receiver $big vs $small"
            ./test_psu -p 2 -s $big -r $small -t $thrds --hashes 3 --scaler 1.27 | tee "$dir/unbalanced receiver $big $small $thrds.txt"
            echo "Unbalanced shuffle sender $small vs $big"
            ./test_psu -p 3 -s $small -r $big -t $thrds --hashes 3 --scaler 1.27 | tee "$dir/unbalanced sender $small $big $thrds.txt"
        done
    done
done


echo "===  Small set  ==="
echo "Balanced set. No bucket."

MIN_SIZE=8
MAX_SIZE=14

for ((size=MIN_SIZE;size<=MAX_SIZE;size+=2))
do
    for ((thrds=1;thrds<=MAX_THRDS;thrds*=2))
    do
        echo "small shuffle receiver $size"
        ./test_psu -p 0 -s $size -r $size -t $thrds | tee "$dir/small receiver $size $thrds.txt"
        echo "small shuffle sender $size"
        ./test_psu -p 1 -s $size -r $size -t $thrds | tee "$dir/small sender $size $thrds.txt"
    done
done


echo "===  Large set  ==="
echo "Balanced set. Cuckoo hash num: 4. Cuckoo scaler: 1.09."

MIN_SIZE=8
MAX_SIZE=22
MAX_THRDS=8

for ((size=MIN_SIZE;size<=MAX_SIZE;size+=2))
do
    for ((thrds=1;thrds<=MAX_THRDS;thrds*=2))
    do
        echo "large shuffle receiver cuckoo $size"
        ./test_psu -p 2 -s $size -r $size -t $thrds --hashes 4 --scaler 1.09 | tee "$dir/large receiver $size $thrds.txt"
        echo "large shuffle sender cuckoo $size"
        ./test_psu -p 3 -s $size -r $size -t $thrds --hashes 4 --scaler 1.09 | tee "$dir/large sender $size $thrds.txt"
    done
done

