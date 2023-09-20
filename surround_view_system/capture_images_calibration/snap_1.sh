#/bin/bash
#rm ./*.yuv
if [ -d out ]; then
    rm -rf out
fi
mkdir -p ./out_calibrate
i=1
for n in $(seq 1 30)
do
    read -p "Press enter to continue"
    timeout -t 3 ./vpss_chn_dump 21 1 1
    mv vpss_grp21_chn1_1920x1080_P420_1.yuv out_calibrate/out_$i.yuv
    i=$((i+1))
done

tar -cvf ./yuv_calibrate.tar.gz ./out_calibrate
