export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/lib:/emmc/avs/lib
cd /emmc/avs/bin
sleep 5 
# 内同步，第一批10台车
#./sample_vdec_inner 1 1 &
# 外同步
#./sample_vdec_out 1 1 &
./img_cap 

