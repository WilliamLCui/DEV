# 概述
本项目旨在借助pto参数生成红外图像的全景拼接查找表。

# 使用简述
1、将原图放置在images文件夹中，
2、需要先借助PTGui手动选点拼接，并将PTGui导出的pto参数保存在pto文件夹中，
3、运行stitch_infrared.py，即可在maps文件夹中得到所需要的dstmap_0-5.bin, maskmap_0-5.bin文件。
4、将本项目 maps\ 下的 dstmap_0-5.bin、maskmap_0-5.bin
    替换板上 \emmc\avs\data\stitch_6_640_512_1737_690\dstmap\ 目录下的同名文件
具体内容见本项目 docs 文件夹内文档

# 修改记录
2023.04.12
    增加了亮度平衡，使过亮画面亮度降低至平均亮度，
    修改了板端全景图生成的逻辑，权重文件设计与每个画面的map匹配，每张图的每个像素都有一个对应的权重值，拼接融合时相乘得到最终输出，以实现亮度平衡。
    为实现上述功能修改了板端 \emmc\avs\data\kernels\stitch_das.cl 的代码，需要更新此文件。新代码附在了本项目目录内。

    需要更新操作：
    将本项目下的 stitch_das.cl 替换板上 \emmc\avs\data\kernels\ 目录下的同名文件(仅需替换老版本文件一次)

