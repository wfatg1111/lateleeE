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
    char slice_type;               // ֡����
    char nal_unit_type;            // NAL����
    unsigned int data_offset;       // nal�����ļ��е�ƫ��
    char startcode_buf[14];         // ��ʼ�룬�ַ�����ʽ
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

typedef int handle_nalu_info(NALU_t* nalu);

int h264_nal_probe(char *fileurl, vector<NALU_t>& vNal, int num);

int h264_nal_parse(char* filename,int data_offset,int data_lenth,LPVOID lparam);;

int h264_sps_parse(char* filename,int data_offset,int data_lenth, SPSInfo_t& info);

int h264_pps_parse(char* filename,int data_offset,int data_lenth, PPSInfo_t& info);

#endif
