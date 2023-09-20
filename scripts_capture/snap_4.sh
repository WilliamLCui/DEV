#/bin/bash
#rm ./*.yuv
for i in $(seq 21 24)
do 
    timeout -t 3 ./vpss_chn_dump $i 1 1 
done

