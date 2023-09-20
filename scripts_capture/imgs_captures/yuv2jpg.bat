@setlocal enabledelayedexpansion
del yuv\\*.jpg
set /a b=13
set /a e=0
set /a d=0
for /l %%i in (1,1,12) do (
ffmpeg.exe -s 1920x1080 -pix_fmt yuv420p -i yuv\\vpss_grp!b!_chn1_1920x1080_P420_1.yuv -c:v mjpeg -pred 1 yuv\\output!d!_!e!.jpg
set /a b +=1
set /a e +=1
if !e! equ 8 (
    set /a e=0 
    set /a d=1)
echo %%i
)

set /a b=8
set /a e=0
set /a d=0
for /l %%i in (1,1,6) do (
ffmpeg.exe -s 640x512 -pix_fmt yuv422p -i yuv\\vpss_grp!b!_chn!e!_640x512_P422_1.yuv -c:v mjpeg -pred 1 yuv\\output2_!d!.jpg
@REM set /a b +=1
set /a e +=1
set /a d +=1
if !e! equ 4 (
    set /a e=0 
    set /a b=9)
)
pause
