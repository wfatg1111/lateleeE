/**

H.264����ת�ɲ�����Ƶ

note ʹ��ffmpeg�汾Ϊ2.6.3�������Ŀ�Ϊ��̬��

����������H.264

*/

#ifndef H264TOVIDEO_H
#define H264TOVIDEO_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
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


#define _LL_DEBUG_

// low level debug
#ifdef _LL_DEBUG_
    #ifndef debug
    #define debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
    #endif
    #define LL_DEBUG(fmt, ...) printf("[DEBUG %s().%d @ %s]: " fmt, __func__, __LINE__, P_SRC, ##__VA_ARGS__)
#else
     #define debug(fmt, ...)
    #define LL_DEBUG(fmt, ...)
#endif


#ifndef min
#define min(a,b) ((a) > (b) ? (b) : (a))
#endif

/**
 @brief ��Ƶ�������ṹ��
*/
typedef struct AVIOBufferContext {
    unsigned char* ptr;
    int pos;
    int totalSize;
    int realSize;
}AVIOBufferContext;

class H264BS2Video
{
public:
    H264BS2Video();
    ~H264BS2Video();

public:
    /**
     * ��H.264��Ƶ�ļ�����ʼ��
     * 
     * @param rawfile ����Ƶ�ļ�·��ȫ��(����Ŀ¼����Ƶ�ļ�����)
     *
     * @return =< 0 �ɹ���= 0�� ʧ�� = -1
     */
    int openBSFile(const char* rawfile);

    /**
     * ��H.264��Ƶ�ļ�����ʼ��
     *
     * @param videofile ��Ƶ�ļ�·��ȫ��(����Ŀ¼����Ƶ�ļ�����)
     * @param fps ֡��
     * @param gop GOP��С�������Ƶû��B֡����GOP��СΪI֡���
     * @param width ��Ƶ��
     * @param height ��Ƶ�� 
     * @param bitrate ��Ƶ���ʣ�Ĭ��Ϊ2048kbps
     *
     * @return =< 0 �ɹ���= 0�� ʧ�� = -1
     *
     * @note ��ǰֻ���Է�װ��avi��ʽ����Ƶ�ļ�
     */
    int openVideoFile(const char* videofile,
                    int fps = 25,
                    int gop = 10,
                    int width=1920,
                    int height=1080,
                    int bitrate = 2097152);
    /**
     * �����ڲ�������
     *
     * @note ���ڴ洢ת������Ƶ���ݣ���ʹ���ļ���������ñ�����
     */
    int allocBuffer(int size);

    /**
     * ��ʼ��H.264��Ƶ����
     *
     * @param fmt ��װ��ʽ��avi��mp4��mkv��Ĭ��avi
     * @param fps ֡��
     * @param gop GOP��С�������Ƶû��B֡����GOP��СΪI֡���
     * @param width ��Ƶ��
     * @param height ��Ƶ�� 
     * @param bitrate ��Ƶ���ʣ�Ĭ��Ϊ2048kbps
     *
     * @return =< 0 �ɹ���= 0�� ʧ�� = -1
     *
     * @note ��ǰֻ���Է�װ��avi��ʽ����Ƶ�ļ�
     */
    int openVideoMem(const char* fmt = "avi", int fps = 25, int gop = 10, int width=1920, int height=1080, int bitrate = 2097152);

    /**
     * ����һ֡H.264��Ƶ
     * @param bitstream H.264����������
     * @param size  H.264������������С
     * @param keyframe �ؼ�֡��־(I֡)���ؼ�֡Ϊ1������Ϊ0
     *
     */
    int writeFrame(char* bitstream, int size, int keyframe);
   
    /**
     * ����H.264��Ƶ
     *
     * @return =< 0 �ɹ���= 0�� ʧ�� = -1
     */
    int writeFrame(void);

    /**
     * �ͷ���Դ��д��Ƶ�ļ�β������
     *
     * @note ��������ô˺�������AVI�������Ƶ���еĲ������޷����ţ�
     *       ��Ϊβ������Ϊ��֡�������Ǳ����
     */
    int close(void);

    /**
     * ��ȡת������Ƶ���ݣ�������Ϊ���ڲ�ʹ��
     * @param buffer ������
     * @param size  ��������С
     *
     */
    void getBuffer(unsigned char** buffer, int* size);
    
    /**
     * �ͷ���Դ�ڲ�������
     *
     * @note �粻���ã��ڱ�������������Ҳ�������ͷ�
     */
    void freeBuffer(void);
    
    /**
     * ��λ�ڲ�������
     *
     * @note �˺����Ḵԭ��������������������Ƶ�����롢�ͷ���Դ
     */
    void resetBuffer(void);


private:
    AVFormatContext *m_infctx;
    AVFormatContext *m_outfctx; 
    AVStream *m_stream;
    AVIOContext *m_avio;
    AVIOBufferContext m_avbuffer;
    int m_videoidx;
    int m_isfile;
};

#endif // H264TOVIDEO_H
