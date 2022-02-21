#ifndef _H264DECODE_H
#define _H264DECODE_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

#ifdef WIN32
// ��̬��
#pragma comment(lib, "libgcc.a") // divdi3(), etc.
#pragma comment(lib, "libmingwex.a") // snprintf()....
#pragma comment(lib, "libiconv.a") // libiconv_open(), etc.

#pragma comment(lib, "libavcodec.a")
#pragma comment(lib, "libavformat.a")
#pragma comment(lib, "libavutil.a")
#pragma comment(lib, "libswscale.a")
#pragma comment(lib, "libswresample.a")

#endif

#define _DEBUG_

typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef long            LONG;

// 2�ֽڶ���
#pragma pack(2)
typedef struct tagMYBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} MYBITMAPFILEHEADER;

// 40
// �������Ϊ2�ֽڶ��룬����Ҫ�ָ�Ϊԭ��Ĭ��8�ֽڶ���
#pragma pack(8)
typedef struct tagMYBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} MYBITMAPINFOHEADER;

class CH264Decoder {
public:
    CH264Decoder();
    ~CH264Decoder();

    /**
    * openVideoFile - ��H264��Ƶ
    * 
    * @param  avifile H264��ʽ��AVI��Ƶ�ļ�����
    *
    * @return 0 - �ɹ�   -1 - ʧ��(�����Ҳ����������������ڴ治�ɹ�)
    */  
    int openVideoFile(const char* avifile);

    /**
    * closeVideoFile - �ر���Ƶ���ͷ���Դ
    * 
    */
    void closeVideoFile(void);

    /**
    * jumpToTime - ����ָ��ʱ��
    *
    * @param     time��ʱ�䣬����Ϊ��λ
    *
    * @return    0 - �ɹ�   -1 ����ת���ɹ���ָ��ʱ�䳬����Ƶ��ʱ���С��0
    */
    int jumpToTime(int64_t time);

    /**
    * getFrame - ����Ƶ�л�ȡһ֡����
    * 
    * @param[OUT] yuvBuffer ��������������ԭʼ����ָ��
    * @param[OUT] rgbBuffer ����������RGB����ָ��
    * @param[OUT] size      ����������ԭʼ���ݴ�С
    * @param[OUT] width     ������������
    * @param[OUT] height    ������������
    *
    * @return  1 - �ɹ�   0 - û�ж���֡(��Ƶ������)  -1 - ����ʧ��
    *
    * @note  ���������еĿ�͸�ָ��Ƶ���ݵĿ�͸�
    *        ԭʼ����ΪYUV420P��ʽ
    *        ��Щ��Ƶ����I֡��ʼ������jumpToTime(0)��������ʼ��I֡
    *        �ú�������av_read_frame���᷵��һ��֡����>0������Ƶ�л������ݣ����Լ�����ȡ
    */
    int getFrame(unsigned char** yuvBuffer = NULL, unsigned char** rgbBuffer = NULL, int* size = NULL, int* width = NULL, int* height = NULL);

    /**
    * getSkippedFrame - ��ȡffmpeg�����֡
    * 
    * @param[OUT] yuvBuffer ��������������ԭʼ����ָ��
    * @param[OUT] rgbBuffer ����������RGB����ָ��
    * @param[OUT] size      ����������ԭʼ���ݴ�С
    * @param[OUT] width     ������������
    * @param[OUT] height    ������������
    *
    * @return  1 - �ɹ�   0 - û�л���֡(��Ƶ������)  -1 - ����ʧ��
    *
    * @note  �е�֡A��Ҫ�ο������֡B������˳��ffmpeg��û�н⵽B֡��û��������A���Ỻ��������
    *        ���������ȡffmpeg�������л��е�֡��
    *        ���ʣ�����ǿ�ͷ��֡�������⣬�����ŵ�������������õ���֡�ǿ�ʼ�Ļ������ģ��������Խ��������������
    */
    int getSkippedFrame(unsigned char** yuvBuffer = NULL, unsigned char** rgbBuffer = NULL, int* size = NULL, int* width = NULL, int* height = NULL);

    /**
    * writeYUVFile - дYUV�ļ���ָ���ļ�
    *
    * @param   filename  ͼƬ�ļ�����
    *
    * @return  0 - �ɹ�  -1 - д�ļ�ʧ��
    *
    */
    int writeYUVFile(const char* filename);

    /**
    * writeBmpFile - дBMPͼƬ��ָ���ļ�
    *
    * @param   filename  ͼƬ�ļ�����
    *
    * @return  0 - �ɹ�  -1 - д�ļ�ʧ��
    *
    */
    int writeBMPFile(const char* filename);

    int writeBMPFile2(const char* filename);

    /**
    * writeBmpFile - дJPEGͼƬ��ָ���ļ�
    *
    * @param   filename  ͼƬ�ļ�����
    *
    * @return  0 - �ɹ�  -1 - д�ļ�ʧ��
    *
    */
    int writeJPGFile(const char* filename);

    int writeJPGFile2(const char* filename);

private:
    /**
    *  convertToRgb - �������������ת��ΪRGB��ʽ(ʵ��ΪBGR24)
    *
    * @return ����RGB����ָ��
    */
    unsigned char* convertToRgb();

private:
    int m_skippedFrame;
    int m_picWidth;
    int m_picHeight;

    int m_videoStream;    // ��Ƶ������
    unsigned char* m_picBuffer;
    AVFormatContext* m_fmtctx;
    AVCodecContext* m_avctx;
    AVFrame* m_picture;
    AVFrame* m_frameRGB;
    unsigned char* m_bufferYUV;
    AVFrame* m_frameYUV;
    struct SwsContext* m_imgctx;
    struct SwsContext* m_imgctxyuv;
};
#endif