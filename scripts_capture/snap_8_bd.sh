#/bin/bash

rm ./yuv_calibrate.tar.gz

if [ -d out_calibrate ]; then
    read -p "Delete old captures? [y/n]: " flag
    if [ $flag == "y" ]; then
        rm -rf out_calibrate
        mkdir -p ./out_calibrate
    fi
else
    mkdir -p ./out_calibrate
fi

x_s=13
n=0
while [ $n -lt 8 ]; do
    read -p "Input camera name like 'front' 'right_front' to continue or 'q' to save and quit: " camera
    case $camera in
        "front")
            x=$(((x_s + 0) % 20))
            ;;
        "right_front")
            x=$(((x_s + 1) % 20))
            ;;
        "right")
            x=$(((x_s + 2) % 20))
            ;;
        "right_back")
            x=$(((x_s + 3) % 20))
            ;;
        "back")
            x=$(((x_s + 4) % 20))
            ;;
        "left_back")
            x=$(((x_s + 5) % 20))
            ;;
        "left")
            x=$(((x_s + 6) % 20))
            ;;
        "left_front")
            x=$(((x_s + 7) % 20))
            ;;
        "q")
            break
            ;;
        *)
            echo -e "Please input proper camera name!\n"
            continue
            ;;
    esac
    n=$((n+1))
    if [ $x == 20 ]; then
        y=13
    else
        y=$((x+1))
    fi

    for z in $(seq 0 14)
        do
            read -p "Press enter to capture frame $z"
            i=$((x-x_s))
            j=$((y-x_s))

            timeout -t 3 ./vpss_chn_dump $x 1 1
            mv vpss_grp${x}_chn1_1920x1080_P420_1.yuv out_calibrate/camera${i}_${i}${j}_${z}.yuv
            timeout -t 3 ./vpss_chn_dump $y 1 1
            mv vpss_grp${y}_chn1_1920x1080_P420_1.yuv out_calibrate/camera${j}_${i}${j}_${z}.yuv

            echo -e "\n *** Frame $z success *** \n"
        done
        echo -e "\n ******** Capture success ********\n"

done

tar -cvf ./yuv_calibrate.tar.gz ./out_calibrate
echo -e "\n ******** Saving frames as ./yuv_calibrate.tar.gz ********\n"



