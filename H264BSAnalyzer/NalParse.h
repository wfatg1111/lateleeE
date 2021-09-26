#ifndef PALPARSE_H
#define PALPARSE_H

#include "stdafx.h"

#include "h264_stream.h"
#include "h265_stream.h"

#include <vector>
using std::vector;

typedef struct
{
    int type;                       // 0 -- h.264; 1 -- h.265
    unsigned int num;               // ���
    unsigned int len;               // ����ʼ����ܵĳ���
    unsigned int data_offset;       // nal�����ļ��е�ƫ��
    char slice_type;               // ֡����
    char nal_unit_type;            // NAL����
    char startcode_len;             // start code����
    char startcode_buf[16];         // ��ʼ�룬�ַ�����ʽ
} NALU_t;

typedef struct 
{
    int profile_idc;
    int level_idc;
    int width;
    int height;
    int crop_left;
    int crop_right;
    int crop_top;
    int crop_bottom;
    float max_framerate;  // ��SPS����õ���֡�ʣ�Ϊ0��ʾSPS��û����Ӧ���ֶμ���
    int chroma_format_idc;  // YUV��ɫ�ռ� 0: monochrome 1:420 2:422 3:444
}SPSInfo_t;

typedef struct 
{
    int encoding_type;  // Ϊ1��ʾCABAC 0��ʾCAVLC

}PPSInfo_t;

enum FileType
{
    FILE_H264 = 0,
    FILE_H265 = 1,
};

class CNalParser
{
public:
    CNalParser();
    ~CNalParser();
    
    int init(const char* filename);
    int release(void);

    int h264_nal_probe(char *fileurl, vector<NALU_t>& vNal, int num);

    int h264_nal_parse(char* filename,int data_offset,int data_lenth,LPVOID lparam);;

    int h264_sps_parse(char* filename,int data_offset,int data_lenth, SPSInfo_t& info);

    int h264_pps_parse(char* filename,int data_offset,int data_lenth, PPSInfo_t& info);

private:
    //�ж��Ƿ�Ϊ0x000001,����Ƿ���1
    inline int findStartcode3(unsigned char *Buf)
    {
        return (Buf[0]==0 && Buf[1]==0 && Buf[2]==1);
    }

    //�ж��Ƿ�Ϊ0x00000001,����Ƿ���1
    inline int findStartcode4(unsigned char *Buf)
    {
        return (Buf[0]==0 && Buf[1]==0 && Buf[2]==0 && Buf[3]==1);
    }
    int GetAnnexbNALU (FILE* fp, NALU_t *nalu);
    int find_first_nal(FILE* fp, int& startcodeLenght);

    FileType judeVideoFile(const char* filename);

private:
    h264_stream_t* m_hH264;
    h265_stream_t* m_hH265;
    FileType m_nType; // 0:264 1:265
    const char* m_filename;
};
#endif
