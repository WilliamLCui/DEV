#/bin/bash
#rm ./*.yuv
for i in $(seq 0 3)                                                    
do                                                                       
    timeout -t 3 ./vpss_chn_dump 8 $i 1
done
timeout -t 3 ./vpss_chn_dump 9 0 1
timeout -t 3 ./vpss_chn_dump 9 1 1
tar -cvf ./yuv.tar.gz ./*.yuv
