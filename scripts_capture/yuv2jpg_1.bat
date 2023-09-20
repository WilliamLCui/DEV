@setlocal enabledelayedexpansion
del out_calibrate\\*.jpg
set /a b=1
for /l %%i in (1,1,30) do (
ffmpeg.exe -s 1920x1080 -pix_fmt yuv420p -i out_calibrate\\out_!b!.yuv -c:v mjpeg -pred 1 out_calibrate\\output_!b!.jpg
set /a b +=1
echo %%i
)

pause
