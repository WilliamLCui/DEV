
@setlocal enabledelayedexpansion
del out_calibrate\\*.jpg
set /a x=0
set /a y=1
for /l %%i in (0,1,7) do (
    set /a z=0
    for /l %%j in (0,1,14) do (
        ffmpeg.exe -s 1920x1080 -pix_fmt yuv420p -i out_calibrate\\camera!x!_!x!!y!_!z!.yuv -c:v mjpeg -pred 1 out_calibrate\\camera!x!_!x!!y!_!z!.jpg
        ffmpeg.exe -s 1920x1080 -pix_fmt yuv420p -i out_calibrate\\camera!y!_!x!!y!_!z!.yuv -c:v mjpeg -pred 1 out_calibrate\\camera!y!_!x!!y!_!z!.jpg
        set /a z +=1
    )
    set /a x +=1
    if x equ 7 (
        set /a y=0
    ) else (
        set /a y=x+1
    )
    echo %%i
)

pause