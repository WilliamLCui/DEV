#/bin/bash
rm ./yuv.tar.gz
#rm ./*.yuv
for i in $(seq 13 20)
do 
    timeout -t 3 ./vpss_chn_dump $i 1 1 
done
