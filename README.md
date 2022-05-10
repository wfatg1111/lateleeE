# H264BSAnalyzer -- H.264/AVC H.265/HEVC bitstream analyze tool

[![GitHub stars](https://img.shields.io/github/stars/latelee/H264BSAnalyzer.svg)](https://github.com/latelee/H264BSAnalyzer)[![GitHub forks](https://img.shields.io/github/forks/latelee/H264BSAnalyzer.svg)](https://github.com/latelee/H264BSAnalyzer)

Stargazers over time  
[![Stargazers over time](https://starcharts.herokuapp.com/latelee/H264BSAnalyzer.svg)](https://starcharts.herokuapp.com/latelee/H264BSAnalyzer)

## Project
VS2010 MFC project, using h264bitstream to implement H.264 bitstream analyze.<br>
The code for H.265 bitstream analyzing is based on h264bitstream code and HM16.6.

## Project Feature
* support different NAL display, including VPS, SPS, PPS, SEI, AUD, Slice.
* support hex data display for NAL.
* support displaying detail information using cursor up and down.
* support different color for different slice, with frame number.
* auto parse file name.
* support file name suffix:
    * H.264 format file: .h264,.h264, .avc
    * H.265 format file: .h26, .h265, .hevc
    * auto decide format acording file content if no name suffix specify listing above.
* support playing H.264��H.265 bitstream video file.
* pause, stop, play frame by frame for video file.
* support saving for RGB(24bit) and YUV(yuv420p) file, BMP, JPEG (picture) file.
* support  saving for AVI, MP4, MOV format file.

## Usage
Click menu File->OPen option, or drag file to the main window, <br>
and the tool will auto parse file cotent. <br>
double click the item in the main windows will show the detail NAL information.<br>
to play the file, click "Play".

## Window view
V1.2 main window: <br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v1.2.png)

V2.0 main window for h.264: <br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v2.0_h264.png)

V2.0 main window for h.265: <br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v2.0_h265.png)

V2.1 main window for h.264: <br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v2.1_h264.png)

V2.1 main window for h.265: <br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v2.1_h265.png)

V3.0 main window for h.264: <br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v3.0_h264.png)

V3.0 main window for h.265: <br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v3.0_h265.png)
  
## Changelog
The binary file will locate in release directory. <br>
* v1.x <br>
> Complete H.264 bitstream analyze.
 
* v2.0 <br>
> Delete manual start function in v1.x.<br>
> Add for H.265/HEVC bitstream analyze.<br>
> Other update.<br>

* v2.1 <br>
> Dispaly bitstream information using CTreeCtrl, ref project: H264Visa��H264VideoESViewer.<br>
> Add windows resize.<br>
> Other update.<br>
> Some problems: <br>
> Not decode some SEI information, the H.264 frame rate calcuration may be inaccurate(2x for real frame rate).<br>

* v3.0 <br>
> fix code using cppcheck.<br>
> support play H.264��H.265 bitstream, minimize compile ffmpeg, static link.<br>
> pause, stop, play frame by frame for video file.<br>
> support saving for RGB(24bit) and YUV(yuv420p) file, BMP, JPEG (picture) file.<br>
> support  saving for AVI, MP4, MOV format file.<br>

## Testing
The tool is testing width H264Visa, CodecVisa and HM tool.<br>
The testing file is generating by x264/x265 tool, also use some H.265 test sequence file<br>
Only test under Windows 7 64bit OS.<br>

## Some bug
Parsing big file will be slow, and may be crash.<br>
Same slice information may be wrong.<br>
The avi file saving for h.265 can't be play. Note: it can't be play by ffplay.<br>
You ca fix yourself, and let me know.<br>

## Protocol
* Copyright [CST studio Late Lee](http://www.latelee.org)
* Fix some bug for h264bitstream, see the code.
* The code comes from h264bitstream, is LGPL.
* Total code is LGPL.
* You can use the code for study, and commercial purposes, but give no guarantee.

## Thanks
This project started at Feb, 2014 for work need, and see the article written by Dr leixiaohua, <br>
and then rewrite the code, refactor the code, and make improve.<br>
Tanks to [������](http://blog.csdn.net/leixiaohua1020) , He's gone, but will last spirit.

## Author
CST studio Late Lee<br>
[CST studio](http://www.latelee.org) <br>
Donate the author <br>
![Donate](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/latelee_pay_small.png)


# H264BSAnalyzer -- H.264/AVC H.265/HEVC������������

## ����˵��
VS2010 MFC���̣�ʹ��h264bitstream��Դ��Ŀʵ�ֶ�H.264����������<br>
H.265������h264bitstreamΪ�ο���׼���룬ͬʱ�ο�HM16.6���롣

## ����
* ֧�ֲ�ͬNAL����ʾ������VPS��SPS��PPS��SEI��AUD��Slice�Ľ�����
* ֧����ʾNALʮ���������ݡ�
* ֧�����¹���ƶ���ʾ��ϸ��Ϣ��
* ֧�ֲ�ͬslice����ɫ��ʾ����ʾ֡��š�
* �Զ������ļ�����
* ֧���ļ�����׺��
    * H.264�ļ���׺��Ϊ.h264��.h264��.avc��
    * H.265�ļ���׺��Ϊ.h265��.h265��.hevc��
    * ����������׺��������������Զ�ʶ��
* ֧�ֲ���H.264��H.265�������ļ���
* �߱���ͣ��ֹͣ����֡���Ź��ܡ�
* ֧�ֱ���ΪRGB(24bit)��YUV(yuv420p)ԭʼ�ļ���֧�ֱ���ΪBMP��JPEGͼƬ��֧���ļ�����%d�ַ���
* ֧�ֱ���ΪAVI��MP4��MOV��ʽ��Ƶ�ļ���

## �÷�
�˵�File->Openѡ�����ֱ����ҷ�ļ������߽��档<br>
���߻��Զ�������˫��ĳһ��ɲ鿴�����NAL��Ϣ��<br>
�����Play���˵����ֲ����Ӵ��ڡ�

## ����
V1.2�汾���棺<br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v1.2.png)

V2.0�汾H264�������棺<br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v2.0_h264.png)

V2.0�汾H265�������棺<br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v2.0_h265.png)

V2.1�汾H264�������棺<br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v2.1_h264.png)

V2.1�汾H265�������棺<br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v2.1_h265.png)

V3.0�汾H264�������棺<br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v3.0_h264.png)

V3.0�汾H265�������棺<br>
![GUI](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/v3.0_h265.png)
  
## �汾���
����õĹ���λ��releaseĿ¼�С�<br>
* v1.x <br>
> H264��������������ɡ�
 
* v2.0 <br>
> ȥ��v1.x�汾�ֶ������ʼ�������ܡ�<br>
> ���H.265/HEVC�����������ܡ�<br>
> �������漰��ʾ��Ϣ���ơ�<br>

* v2.1 <br>
> ʹ�����οؼ���ʾ�����ֶΡ���ʾ�ο���ԴΪH264Visa��H264VideoESViewer���ߡ�<br>
> �������Ź��ܡ�<br>
> �������漰��ʾ��Ϣ���ơ�<br>
> �������⣺<br>
> ����SEI��Ϣδ��������H264֡�ʼ�����ܲ�׼ȷ(����ʵ֡�ʵ�2��)��<br>

* v3.0 <br>
> ʹ��cppcheck���м�⣬�޸�����������֮����<br>
> ֧�ֲ���H.264��H.265�������ļ���ffmpeg��С���룬��̬���ӡ�<br>
> �߱���ͣ��ֹͣ����֡���Ź��ܡ�<br>
> ֧�ֱ���ΪRGB(24bit)��YUV(yuv420p)ԭʼ�ļ���֧�ֱ���ΪBMP��JPEGͼƬ��<br>
> ֧�ֱ���ΪAVI��MP4��MOV��ʽ��Ƶ�ļ���<br>

## ����
������ʹ��H264Visa��CodecVisa��HM���߶ԱȲ��ԡ�<br>
������Ƶ�ļ�Ϊx264/x265�������ɣ�����ʹ��H.265�������С�<br>
�����߽���Windows 7 64bit����ϵͳ�����в���ͨ����<br>

## ����Ǳ������
�������ļ����������ܻ������<br>
��������ʹ���ڶ��ļ������߶Աȷ��������޷��������������������﷨���ܷ�������<br>
H.265����ΪAVI��ʽ��Ƶ�޷����š�ע��ʹ��ffmpegת������ffplayҲ�޷��������š�<br>
������������Ҳ�ɷ��������ߡ�<br>

## Э��
* ��Ȩ���� [��˼�ù����� ���](http://www.latelee.org)
* ����h264bitstream����bug��������롣
* ����h264bitstream��Ӧ���޸ĵĴ��룬���LGPLЭ�顣
* ������Դ��ʹ��LGPLЭ�顣
* ������ѧϰ�о�֮Ŀ�ģ�Ҳ��������ҵĿ�ģ���������֤��������ȫ�ɿ���

## ��л
��������2014��2��������Ҫ�����⿴�������販ʿ֮���£�����������޸ġ��ع����������ơ�<br>
��л [������](http://blog.csdn.net/leixiaohua1020) ��ʿ��˹�����ţ����񳤴棡

## ����
˼�ù����� ���<br>
[��˼�ù�����](http://www.latelee.org) <br>
������ñ����������ӭ����֧������ <br>
![����](https://github.com/latelee/H264BSAnalyzer/blob/master/screenshots/latelee_pay_small.png)
