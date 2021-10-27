#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "NaLParse.h"

#include "H264BSAnalyzerDlg.h"

static void h264_debug_nal(h264_stream_t* h, nal_t* nal);
static void h265_debug_nal(h265_stream_t* h, h265_nal_t* nal);

// todo����ʹ������д���ռ������
//��Ž����������ַ���
static char g_tmpStore[1024] = {0};
static char g_outputInfo[OUTPUT_SIZE] = {'\0'};

#define my_printf(...) do { \
    sprintf(g_tmpStore, __VA_ARGS__);\
    strcat(g_outputInfo, g_tmpStore);} while(0)


CNalParser::CNalParser()
{
    m_nType = FILE_H264; // default
    m_hH264 = NULL;
    m_hH265 = NULL;
    m_naluData = NULL;
}

CNalParser::~CNalParser()
{
    release();
}

int CNalParser::init(const char* filename)
{
    m_filename = filename;

    // judge file 
    m_nType = judeVideoFile(m_filename);

    // init handle
    if (m_nType == FILE_H265)
    {
        if (m_hH265 != NULL)
        {
            h265_free(m_hH265);
        }
        m_hH265 = h265_new();
    }
    else
    {
        if (m_hH264 != NULL)
        {
            h264_free(m_hH264);
        }
        m_hH264 = h264_new();
    }
    
    return 0;
}

int CNalParser::release(void)
{
    if (m_hH264 != NULL)
    {
        h264_free(m_hH264);
        m_hH264 = NULL;
    }
    if (m_hH265 != NULL)
    {
        h265_free(m_hH265);
        m_hH265 = NULL;
    }
    return 0;
}

int CNalParser::probeNALU(vector<NALU_t>& vNal, int num)
{
    NALU_t n;
    int nal_num=0;
    int offset=0;
    int nalLen;
    FILE* fp = NULL;

    fp=fopen(m_filename, "r+b");
    if (fp == NULL)
    {
        return -1;
    }

    memset(&n, '\0', sizeof(NALU_t));

    n.type = m_nType; // h.265

    offset = findFirstNALU(fp, &(n.startcodeLen));

    fseek(fp, offset, SEEK_SET);
    while (!feof(fp))
    {
        if (num > 0 && nal_num == num)
        {
            break;
        }
        nalLen = getAnnexbNALU(fp, &n);//ÿִ��һ�Σ��ļ���ָ��ָ�򱾴��ҵ���NALU��ĩβ����һ��λ�ü�Ϊ�¸�NALU����ʼ��0x000001
        n.offset = offset;
        n.num = nal_num;
        offset = offset + nalLen;

        vNal.push_back(n);

        nal_num++;
    }
    return 0;
}

int CNalParser::parseNALU(NALU_t& vNal, char** naluData, char** naluInfo)
{
    memset(g_outputInfo, '\0', OUTPUT_SIZE);

    if (m_naluData == NULL)
    {
        free(m_naluData);
        m_naluData = NULL;
    }
    m_naluData = (uint8_t *)malloc(vNal.len);

    FILE *fp = fopen(m_filename, "rb");
    if (fp == NULL)
    {
        return -1;
    }

    fseek(fp, vNal.offset, SEEK_SET);
    fread(m_naluData, vNal.len, 1, fp);

    // ����Ҫ�ٴβ�ѯnal
    //find_nal_unit(m_naluData, vNal.len, &nal_start, &nal_end);
    if (m_nType == 1)
    {
        // �˺�������ʱ���ǲ�����startcode�ģ�����Ҫ��ȥstartcodeLen
        h265_read_nal_unit(m_hH265, &m_naluData[vNal.startcodeLen], vNal.len - vNal.startcodeLen);
        h265_debug_nal(m_hH265,m_hH265->nal);    // ��ӡ��g_outputInfo��
    }
    else
    {
        read_nal_unit(m_hH264, &m_naluData[vNal.startcodeLen], vNal.len - vNal.startcodeLen);
        h264_debug_nal(m_hH264, m_hH264->nal);  // ��ӡ��g_outputInfo��
    }

    *naluData = (char*)m_naluData;
    *naluInfo = g_outputInfo;

    fclose(fp);
    return 0;
}

// ����SPS���õ���Ƶ��ߡ�yuv�ռ����Ϣ
int CNalParser::h264_sps_parse(char* filename,int offset,int data_lenth, SPSInfo_t& info)
{
    int nal_start,nal_end;

    //�ڴ����ڴ��NAL��������ʼ�룩
    uint8_t *nal_temp=(uint8_t *)malloc(data_lenth);

    //���ļ���ȡ
    FILE *fp=fopen(filename,"rb");
    if (fp == NULL)
    {
        return -1;
    }

    fseek(fp,offset,SEEK_SET);
    fread(nal_temp,data_lenth,1,fp);
    // read some H264 data into buf

    find_nal_unit(nal_temp, data_lenth, &nal_start, &nal_end);
    read_nal_unit(m_hH264, &nal_temp[nal_start], nal_end - nal_start);

    int pic_width_in_mbs_minus1;
    int pic_height_in_map_units_minus1;
    int frame_crop_left_offset, frame_crop_right_offset;
    int frame_mbs_only_flag, frame_crop_top_offset, frame_crop_bottom_offset;

    pic_width_in_mbs_minus1 =  m_hH264->sps->pic_width_in_mbs_minus1;
    pic_height_in_map_units_minus1 =  m_hH264->sps->pic_height_in_map_units_minus1;

    frame_mbs_only_flag =  m_hH264->sps->frame_mbs_only_flag;
    info.crop_left = frame_crop_left_offset = m_hH264->sps->frame_crop_left_offset;
    info.crop_right = frame_crop_right_offset = m_hH264->sps->frame_crop_right_offset;
    info.crop_top = frame_crop_top_offset = m_hH264->sps->frame_crop_top_offset;
    info.crop_bottom = frame_crop_bottom_offset = m_hH264->sps->frame_crop_bottom_offset;

    // ��߼��㹫ʽ
    info.width = ((pic_width_in_mbs_minus1 +1)*16) - frame_crop_left_offset*2 - frame_crop_right_offset*2;
    info.height= ((2 - frame_mbs_only_flag)* (pic_height_in_map_units_minus1 +1) * 16) - (frame_crop_top_offset * 2) - (frame_crop_bottom_offset * 2);

    info.profile_idc = m_hH264->sps->profile_idc;
    info.level_idc = m_hH264->sps->level_idc;

    // YUV�ռ�
    info.chroma_format_idc = m_hH264->sps->chroma_format_idc;

    // ע�������֡�ʼ��㻹������
    if (m_hH264->sps->vui_parameters_present_flag)
    {
        info.max_framerate = (float)(m_hH264->sps->vui.time_scale) / (float)(m_hH264->sps->vui.num_units_in_tick);
    }
    if (nal_temp != NULL)
    {
        free(nal_temp);
        nal_temp = NULL;
    }

    fclose(fp);
    return 0;
}

int CNalParser::h264_pps_parse(char* filename,int offset,int data_lenth, PPSInfo_t& info)
{
    int nal_start,nal_end;

    //�ڴ����ڴ��NAL��������ʼ�룩
    uint8_t *nal_temp=(uint8_t *)malloc(data_lenth);

    //���ļ���ȡ
    FILE *fp=fopen(filename,"rb");
    if (fp == NULL)
    {
        return -1;
    }

    fseek(fp,offset,SEEK_SET);
    fread(nal_temp,data_lenth,1,fp);
    // read some H264 data into buf
    find_nal_unit(nal_temp, data_lenth, &nal_start, &nal_end);
    read_nal_unit(m_hH264, &nal_temp[nal_start], nal_end - nal_start);

    info.encoding_type = m_hH264->pps->entropy_coding_mode_flag;

    if (nal_temp != NULL)
    {
        free(nal_temp);
        nal_temp = NULL;
    }

    fclose(fp);
    return 0;
}

static int ue(char *buff, int len, int &start_bit)
{
    int zero_num = 0;
    int ret = 0;

    while (start_bit < len * 8)
    {
        if (buff[start_bit / 8] & (0x80 >> (start_bit % 8)))
        {
            break;
        }
        zero_num++;
        start_bit++;
    }
    start_bit++;

    for (int i=0; i<zero_num; i++)
    {
        ret <<= 1;
        if (buff[start_bit / 8] & (0x80 >> (start_bit % 8)))
        {
            ret += 1;
        }
        start_bit++;
    }
    return (1 << zero_num) - 1 + ret;
}

/**
����NAL������������ʼ�ַ�֮�������ֽ�����������startcode��NALU�ĳ���

note��һ����Ƶ�ļ��в�ͬ��NAL��startcode���ܲ�һ��������SPSΪ4�ֽڣ���SEI����Ϊ3�ֽ�
todo:ÿ�ζ�һ���ֽڣ����������޺õķ�����
*/

int CNalParser::getAnnexbNALU(FILE* fp, NALU_t* nalu)
{
    int pos = 0;
    int found, rewind;
    unsigned char *buffer;
    int info2=0, info3=0;
    int eof = 0;

    if ((buffer = (unsigned char*)calloc (MAX_NAL_SIZE, sizeof(char))) == NULL)
        printf("Could not allocate buffer memory\n");

    if (3 != fread (buffer, 1, 3, fp))//�������ж�3���ֽ�
    {
        free(buffer);
        return 0;
    }
    info2 = findStartcode3(buffer);//�ж��Ƿ�Ϊ0x000001
    if(info2 != 1)
    {
        //������ǣ��ٶ�һ���ֽ�
        if(1 != fread(buffer+3, 1, 1, fp))//��һ���ֽ�
        {
            free(buffer);
            return 0;
        }
        info3 = findStartcode4(buffer);//�ж��Ƿ�Ϊ0x00000001
        if (info3 != 1)//������ǣ�����-1
        {
            free(buffer);
            return -1;
        }
        else
        {
            //�����0x00000001,�õ���ʼǰ׺Ϊ4���ֽ�
            nalu->startcodeLen = 4;
        }
    }
    else
    {
        //�����0x000001,�õ���ʼǰ׺Ϊ3���ֽ�
        nalu->startcodeLen = 3;
    }

    pos = nalu->startcodeLen;
    //������һ����ʼ�ַ��ı�־λ
    found = 0;
    info2 = 0;
    info3 = 0;

    while (!found)
    {
        if (feof(fp))//�ж��Ƿ����ļ�β
        {
            eof = 1;
            goto got_nal;
        }
        buffer[pos++] = fgetc(fp);//��һ���ֽڵ�BUF��

        info3 = findStartcode4(&buffer[pos-4]);//�ж��Ƿ�Ϊ0x00000001
        if(info3 != 1)
            info2 = findStartcode3(&buffer[pos-3]);//�ж��Ƿ�Ϊ0x000001

        found = (info2 == 1 || info3 == 1);
    }

    // startcode����Ϊ3��Ҳ����Ϊ4����Ҫ����ж�
    rewind = (info3 == 1)? -4 : -3;

    if (0 != fseek (fp, rewind, SEEK_CUR))//���ļ�ָ��ָ��ǰһ��NALU��ĩβ
    {
        free(buffer);
        printf("Cannot fseek in the bit stream file");
    }

got_nal:
    // ���ﵽ�ļ�ĩβʱ������1��λ��
    if (eof)
    {
        rewind = -1;
    }

    // ������ʼ�����ڵ�5���ֽ�
    if (nalu->startcodeLen == 3)
        sprintf(nalu->startcodeBuffer, "%02x%02x%02x%02x", buffer[0], buffer[1], buffer[2], buffer[3]);
    else
        sprintf(nalu->startcodeBuffer, "%02x%02x%02x%02x%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
    nalu->len = pos+rewind;

    uint8_t nal_header = 0;
    if (nalu->type)
    {
        m_hH265->sh->read_slice_type = 1;
        h265_read_nal_unit(m_hH265, &buffer[nalu->startcodeLen], nalu->len - nalu->startcodeLen);
        nalu->nalType = m_hH265->nal->nal_unit_type;
        nalu->sliceType = m_hH265->sh->slice_type;
        m_hH265->sh->read_slice_type = 0;
    }
    else
    {
    // simple version
#if 0
        nal_header = buffer[nalu->startcodeLen];
        nalu->nalType = nal_header & 0x1f;// 5 bit

        // ��ȡslice���ͣ�I֡��P֡��B֡
        // ע����nal����Ϊ1~5ʱ��ȡ
        if (nalu->nalType <= 5 && nalu->nalType >= 1)
        {
            int start_bit = 0;
            int first_mb_in_slice = ue((char*)buffer+nalu->startcodeLen+1, 8, start_bit);
            nalu->sliceType = ue((char*)buffer+nalu->startcodeLen+1, 8, start_bit);
        }
        if (nalu->nalType == 7 || nalu->nalType == 8) // sps pps
        {
            read_nal_unit(m_hH264, &buffer[nalu->startcodeLen], nalu->len - nalu->startcodeLen);
        }
#else
        m_hH264->sh->read_slice_type = 1;
        read_nal_unit(m_hH264, &buffer[nalu->startcodeLen], nalu->len - nalu->startcodeLen);
        nalu->nalType = m_hH264->nal->nal_unit_type;
        nalu->sliceType = m_hH264->sh->slice_type;
        m_hH264->sh->read_slice_type = 0;
#endif
    }

    free(buffer);

    return (pos+rewind);//����������ʼ�ַ�֮�������ֽ�������������ǰ׺��NALU�ĳ���
}

int CNalParser::findFirstNALU(FILE* fp, int* startcodeLenght)
{
    int found = 0;
    int info2 = 0;
    int info3 = 0;
    int eof = 0;
    int pos = 0;
    int startcode_len = 0;
    unsigned char *buffer = NULL;

    if ((buffer = (unsigned char*)calloc(MAX_NAL_SIZE, sizeof(char))) == NULL)
        printf ("Could not allocate buffer memory\n");

    while (!found && !feof(fp))
    {
        buffer[pos++] = fgetc(fp);//��һ���ֽڵ�BUF��

        info3 = findStartcode4(&buffer[pos-4]);//�ж��Ƿ�Ϊ0x00000001
        if(info3 != 1)
        {
            info2 = findStartcode3(&buffer[pos-3]);//�ж��Ƿ�Ϊ0x000001
            if (info2)
            {
                startcode_len = 3;
            }
        }
        else
        {
            startcode_len = 4;
        }

        found = (info2 == 1 || info3 == 1);
    }

    // �ļ�ָ��Ҫ�ָ�
    fseek(fp, -startcode_len, SEEK_CUR);

    free(buffer);
    if (startcodeLenght != NULL)
        *startcodeLenght = startcode_len;

    return pos - startcode_len;
}

FileType CNalParser::judeVideoFile(const char* filename)
{
    char szExt[16] = {0};
    FileType type = FILE_H264; // default

    _splitpath(filename, NULL, NULL, NULL, szExt);
    if (!strcmp(&szExt[1], "h265") || !strcmp(&szExt[1], "265") ||
        !strcmp(&szExt[1], "hevc"))
    {
        type = FILE_H265;
    }
    else if (!strcmp(&szExt[1], "h264") || !strcmp(&szExt[1], "264") ||
        !strcmp(&szExt[1], "avc"))
    {
        type = FILE_H264;
    }
    else
    {
        // read content 
        FILE* fp = NULL;
        int offset = 0;
        int startcode = 0;
        unsigned char nalHader = 0;
        unsigned char nalType = 0;

        fp = fopen(filename, "r+b");
        offset = findFirstNALU(fp, &startcode);
        fseek(fp, offset+startcode, SEEK_SET);
        fread((void*)&nalHader,1,1,fp);
        // check h264 first...
        nalType = nalHader & 0x1f; // 5 bit
        if (nalType > 0 && nalType < 22) // ok
        {
            type = FILE_H264;
        }
        else
        {
            // not h264, then check h265...
            nalType = (nalHader>>1) & 0x3f; // 6 bit
            if (nalType >= 0 && nalType <= 47) // ok
            {
                type = FILE_H265;
            }
        }
    }

    return type;
}

// ���´�������h264_stream.c����������
/***************************** debug ******************************/

static void h264_debug_sps(sps_t* sps)
{
    my_printf("======= SPS =======\r\n");
    my_printf(" profile_idc : %d\r\n", sps->profile_idc );
    my_printf(" constraint_set0_flag : %d\r\n", sps->constraint_set0_flag );
    my_printf(" constraint_set1_flag : %d\r\n", sps->constraint_set1_flag );
    my_printf(" constraint_set2_flag : %d\r\n", sps->constraint_set2_flag );
    my_printf(" constraint_set3_flag : %d\r\n", sps->constraint_set3_flag );
    my_printf(" constraint_set4_flag : %d\r\n", sps->constraint_set4_flag );
    my_printf(" constraint_set5_flag : %d\r\n", sps->constraint_set5_flag );
    my_printf(" reserved_zero_2bits : %d\r\n", sps->reserved_zero_2bits );
    my_printf(" level_idc : %d\r\n", sps->level_idc );
    my_printf(" seq_parameter_set_id : %d\r\n", sps->seq_parameter_set_id );
    my_printf(" chroma_format_idc : %d\r\n", sps->chroma_format_idc );
    if (sps->chroma_format_idc == 3)
        my_printf(" separate_colour_plane_flag : %d\r\n", sps->separate_colour_plane_flag );
    my_printf(" bit_depth_luma_minus8 : %d\r\n", sps->bit_depth_luma_minus8 );
    my_printf(" bit_depth_chroma_minus8 : %d\r\n", sps->bit_depth_chroma_minus8 );
    my_printf(" qpprime_y_zero_transform_bypass_flag : %d\r\n", sps->qpprime_y_zero_transform_bypass_flag );
    my_printf(" seq_scaling_matrix_present_flag : %d\r\n", sps->seq_scaling_matrix_present_flag );
    if (sps->seq_scaling_matrix_present_flag)
    {
        for (int i = 0; i < ((sps->chroma_format_idc!=3) ? 8 : 12); i++)
        {
            my_printf("   seq_scaling_list_present_flag[%d] : %d\r\n", i, sps->seq_scaling_list_present_flag[i]);
            if( sps->seq_scaling_list_present_flag[ i ] )
            {
                if( i < 6 )
                    my_printf("   ScalingList4x4[%d] : %d\r\n", i, sps->ScalingList4x4[i] );
                else
                    my_printf("   ScalingList4xScalingList8x84[%d] : %d\r\n", i, sps->ScalingList8x8[i] );
            }
        }
    }

    my_printf(" log2_max_frame_num_minus4 : %d\r\n", sps->log2_max_frame_num_minus4 );
    my_printf(" pic_order_cnt_type : %d\r\n", sps->pic_order_cnt_type );
    if( sps->pic_order_cnt_type == 0 )
        my_printf("   log2_max_pic_order_cnt_lsb_minus4 : %d\r\n", sps->log2_max_pic_order_cnt_lsb_minus4 );
    else if( sps->pic_order_cnt_type == 1 )
    {
        my_printf("   delta_pic_order_always_zero_flag : %d\r\n", sps->delta_pic_order_always_zero_flag );
        my_printf("   offset_for_non_ref_pic : %d\r\n", sps->offset_for_non_ref_pic );
        my_printf("   offset_for_top_to_bottom_field : %d\r\n", sps->offset_for_top_to_bottom_field );
        my_printf("   num_ref_frames_in_pic_order_cnt_cycle : %d\r\n", sps->num_ref_frames_in_pic_order_cnt_cycle );
        for( int i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++ )
        {
            my_printf("   offset_for_ref_frame[%d] : %d\r\n", i, sps->offset_for_ref_frame[i] );
        }
    }
    my_printf(" num_ref_frames : %d\r\n", sps->num_ref_frames );
    my_printf(" gaps_in_frame_num_value_allowed_flag : %d\r\n", sps->gaps_in_frame_num_value_allowed_flag );
    my_printf(" pic_width_in_mbs_minus1 : %d\r\n", sps->pic_width_in_mbs_minus1 );
    my_printf(" pic_height_in_map_units_minus1 : %d\r\n", sps->pic_height_in_map_units_minus1 );
    my_printf(" frame_mbs_only_flag : %d\r\n", sps->frame_mbs_only_flag );
    if( !sps->frame_mbs_only_flag )
        my_printf(" mb_adaptive_frame_field_flag : %d\r\n", sps->mb_adaptive_frame_field_flag );
    my_printf(" direct_8x8_inference_flag : %d\r\n", sps->direct_8x8_inference_flag );
    my_printf(" frame_cropping_flag : %d\r\n", sps->frame_cropping_flag );
    if (sps->frame_cropping_flag)
    {
        my_printf("   frame_crop_left_offset : %d\r\n", sps->frame_crop_left_offset );
        my_printf("   frame_crop_right_offset : %d\r\n", sps->frame_crop_right_offset );
        my_printf("   frame_crop_top_offset : %d\r\n", sps->frame_crop_top_offset );
        my_printf("   frame_crop_bottom_offset : %d\r\n", sps->frame_crop_bottom_offset );
    }
    my_printf(" vui_parameters_present_flag : %d\r\n", sps->vui_parameters_present_flag );
    if (sps->vui_parameters_present_flag)
    {
        my_printf("=== VUI ===\r\n");
        my_printf(" aspect_ratio_info_present_flag : %d\r\n", sps->vui.aspect_ratio_info_present_flag );
        if( sps->vui.aspect_ratio_info_present_flag )
        {
            my_printf("   aspect_ratio_idc : %d\r\n", sps->vui.aspect_ratio_idc );
            if( sps->vui.aspect_ratio_idc == SAR_Extended )
            {
                my_printf("     sar_width : %d\r\n", sps->vui.sar_width );
                my_printf("     sar_height : %d\r\n", sps->vui.sar_height );
            }
        }

        my_printf(" overscan_info_present_flag : %d\r\n", sps->vui.overscan_info_present_flag );
        if( sps->vui.overscan_info_present_flag )
            my_printf("   overscan_appropriate_flag : %d\r\n", sps->vui.overscan_appropriate_flag );
        my_printf(" video_signal_type_present_flag : %d\r\n", sps->vui.video_signal_type_present_flag );
        if( sps->vui.video_signal_type_present_flag )
        {
            my_printf("   video_format : %d\r\n", sps->vui.video_format );
            my_printf("   video_full_range_flag : %d\r\n", sps->vui.video_full_range_flag );
            my_printf("   colour_description_present_flag : %d\r\n", sps->vui.colour_description_present_flag );
            if( sps->vui.colour_description_present_flag )
            {
                my_printf("     colour_primaries : %d\r\n", sps->vui.colour_primaries );
                my_printf("   transfer_characteristics : %d\r\n", sps->vui.transfer_characteristics );
                my_printf("   matrix_coefficients : %d\r\n", sps->vui.matrix_coefficients );
            }
        }
        my_printf(" chroma_loc_info_present_flag : %d\r\n", sps->vui.chroma_loc_info_present_flag );
        if( sps->vui.chroma_loc_info_present_flag )
        {
            my_printf("   chroma_sample_loc_type_top_field : %d\r\n", sps->vui.chroma_sample_loc_type_top_field );
            my_printf("   chroma_sample_loc_type_bottom_field : %d\r\n", sps->vui.chroma_sample_loc_type_bottom_field );
        }
        if( sps->vui.timing_info_present_flag )
        {
            my_printf(" timing_info_present_flag : %d\r\n", sps->vui.timing_info_present_flag );
            my_printf("   num_units_in_tick : %d\r\n", sps->vui.num_units_in_tick );
            my_printf("   time_scale : %d\r\n", sps->vui.time_scale );
            my_printf("   fixed_frame_rate_flag : %d\r\n", sps->vui.fixed_frame_rate_flag );
        }
        my_printf(" nal_hrd_parameters_present_flag : %d\r\n", sps->vui.nal_hrd_parameters_present_flag );
        if( sps->vui.nal_hrd_parameters_present_flag )
        {
            my_printf("=== NAL HRD ===\r\n");
            my_printf(" cpb_cnt_minus1 : %d\r\n", sps->hrd.cpb_cnt_minus1 );
            my_printf(" bit_rate_scale : %d\r\n", sps->hrd.bit_rate_scale );
            my_printf(" cpb_size_scale : %d\r\n", sps->hrd.cpb_size_scale );
            int SchedSelIdx;
            for( SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++ )
            {
                my_printf("   bit_rate_value_minus1[%d] : %d\r\n", SchedSelIdx, sps->hrd.bit_rate_value_minus1[SchedSelIdx] ); // up to cpb_cnt_minus1, which is <= 31
                my_printf("   cpb_size_value_minus1[%d] : %d\r\n", SchedSelIdx, sps->hrd.cpb_size_value_minus1[SchedSelIdx] );
                my_printf("   cbr_flag[%d] : %d\r\n", SchedSelIdx, sps->hrd.cbr_flag[SchedSelIdx] );
            }
            my_printf(" initial_cpb_removal_delay_length_minus1 : %d\r\n", sps->hrd.initial_cpb_removal_delay_length_minus1 );
            my_printf(" cpb_removal_delay_length_minus1 : %d\r\n", sps->hrd.cpb_removal_delay_length_minus1 );
            my_printf(" dpb_output_delay_length_minus1 : %d\r\n", sps->hrd.dpb_output_delay_length_minus1 );
            my_printf(" time_offset_length : %d\r\n", sps->hrd.time_offset_length );
        }
        my_printf(" vcl_hrd_parameters_present_flag : %d\r\n", sps->vui.vcl_hrd_parameters_present_flag );
        if( sps->vui.vcl_hrd_parameters_present_flag )
        {
            my_printf("=== VCL HRD ===\r\n");
            my_printf(" cpb_cnt_minus1 : %d\r\n", sps->hrd.cpb_cnt_minus1 );
            my_printf(" bit_rate_scale : %d\r\n", sps->hrd.bit_rate_scale );
            my_printf(" cpb_size_scale : %d\r\n", sps->hrd.cpb_size_scale );
            int SchedSelIdx;
            for( SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++ )
            {
                my_printf("   bit_rate_value_minus1[%d] : %d\r\n", SchedSelIdx, sps->hrd.bit_rate_value_minus1[SchedSelIdx] ); // up to cpb_cnt_minus1, which is <= 31
                my_printf("   cpb_size_value_minus1[%d] : %d\r\n", SchedSelIdx, sps->hrd.cpb_size_value_minus1[SchedSelIdx] );
                my_printf("   cbr_flag[%d] : %d\r\n", SchedSelIdx, sps->hrd.cbr_flag[SchedSelIdx] );
            }
            my_printf(" initial_cpb_removal_delay_length_minus1 : %d\r\n", sps->hrd.initial_cpb_removal_delay_length_minus1 );
            my_printf(" cpb_removal_delay_length_minus1 : %d\r\n", sps->hrd.cpb_removal_delay_length_minus1 );
            my_printf(" dpb_output_delay_length_minus1 : %d\r\n", sps->hrd.dpb_output_delay_length_minus1 );
            my_printf(" time_offset_length : %d\r\n", sps->hrd.time_offset_length );
        }
        if( sps->vui.nal_hrd_parameters_present_flag || sps->vui.vcl_hrd_parameters_present_flag )
            my_printf("   low_delay_hrd_flag : %d\r\n", sps->vui.low_delay_hrd_flag );
        my_printf(" pic_struct_present_flag : %d\r\n", sps->vui.pic_struct_present_flag );
        my_printf(" bitstream_restriction_flag : %d\r\n", sps->vui.bitstream_restriction_flag );
        if( sps->vui.bitstream_restriction_flag )
        {
            my_printf("   motion_vectors_over_pic_boundaries_flag : %d\r\n", sps->vui.motion_vectors_over_pic_boundaries_flag );
            my_printf("   max_bytes_per_pic_denom : %d\r\n", sps->vui.max_bytes_per_pic_denom );
            my_printf("   max_bits_per_mb_denom : %d\r\n", sps->vui.max_bits_per_mb_denom );
            my_printf("   log2_max_mv_length_horizontal : %d\r\n", sps->vui.log2_max_mv_length_horizontal );
            my_printf("   log2_max_mv_length_vertical : %d\r\n", sps->vui.log2_max_mv_length_vertical );
            my_printf("   num_reorder_frames : %d\r\n", sps->vui.num_reorder_frames );
            my_printf("   max_dec_frame_buffering : %d\r\n", sps->vui.max_dec_frame_buffering );
        }
    }

}


static void h264_debug_pps(pps_t* pps)
{
    my_printf("======= PPS =======\r\n");
    my_printf(" pic_parameter_set_id : %d\r\n", pps->pic_parameter_set_id );
    my_printf(" seq_parameter_set_id : %d\r\n", pps->seq_parameter_set_id );
    my_printf(" entropy_coding_mode_flag : %d\r\n", pps->entropy_coding_mode_flag );
    my_printf(" pic_order_present_flag : %d\r\n", pps->pic_order_present_flag );
    my_printf(" num_slice_groups_minus1 : %d\r\n", pps->num_slice_groups_minus1 );
    if( pps->num_slice_groups_minus1 > 0 )
    {
        my_printf(" slice_group_map_type : %d\r\n", pps->slice_group_map_type );
        if( pps->slice_group_map_type == 0 )
        {
            for( int i_group = 0; i_group <= pps->num_slice_groups_minus1; i_group++ )
                my_printf(" run_length_minus1[%d] : %d\r\n", i_group, pps->run_length_minus1[i_group] );
        }
        else if( pps->slice_group_map_type == 2 )
        {
            for( int i_group = 0; i_group <= pps->num_slice_groups_minus1; i_group++ )
            {
                my_printf(" top_left[%d] : %d\r\n", i_group, pps->top_left[i_group] );
                my_printf(" bottom_right[%d] : %d\r\n", i_group, pps->bottom_right[i_group] );
            }
        }
        else if( pps->slice_group_map_type == 3 ||
            pps->slice_group_map_type == 4 ||
            pps->slice_group_map_type == 5 )
        {
            my_printf(" slice_group_change_direction_flag : %d\r\n", pps->slice_group_change_direction_flag );
            my_printf(" slice_group_change_rate_minus1 : %d\r\n", pps->slice_group_change_rate_minus1 );
        }
        else if( pps->slice_group_map_type == 6 )
        {            
            my_printf(" pic_size_in_map_units_minus1 : %d\r\n", pps->pic_size_in_map_units_minus1 );
            for( int i = 0; i <= pps->pic_size_in_map_units_minus1; i++ )
                my_printf(" slice_group_id[%d] : %d\r\n", i, pps->slice_group_id[i] );
        }
    }
    my_printf(" num_ref_idx_l0_active_minus1 : %d\r\n", pps->num_ref_idx_l0_active_minus1 );
    my_printf(" num_ref_idx_l1_active_minus1 : %d\r\n", pps->num_ref_idx_l1_active_minus1 );
    my_printf(" weighted_pred_flag : %d\r\n", pps->weighted_pred_flag );
    my_printf(" weighted_bipred_idc : %d\r\n", pps->weighted_bipred_idc );
    my_printf(" pic_init_qp_minus26 : %d\r\n", pps->pic_init_qp_minus26 );
    my_printf(" pic_init_qs_minus26 : %d\r\n", pps->pic_init_qs_minus26 );
    my_printf(" chroma_qp_index_offset : %d\r\n", pps->chroma_qp_index_offset );
    my_printf(" deblocking_filter_control_present_flag : %d\r\n", pps->deblocking_filter_control_present_flag );
    my_printf(" constrained_intra_pred_flag : %d\r\n", pps->constrained_intra_pred_flag );
    my_printf(" redundant_pic_cnt_present_flag : %d\r\n", pps->redundant_pic_cnt_present_flag );
    if( pps->_more_rbsp_data_present )
    {
        my_printf(" more_rbsp_data()\r\n" );
        my_printf(" transform_8x8_mode_flag : %d\r\n", pps->transform_8x8_mode_flag );
        my_printf(" pic_scaling_matrix_present_flag : %d\r\n", pps->pic_scaling_matrix_present_flag );
        if( pps->pic_scaling_matrix_present_flag )
        {
            for( int i = 0; i < 6 + 2* pps->transform_8x8_mode_flag; i++ )
            {
                my_printf(" pic_scaling_list_present_flag[%d] : %d\r\n", i, pps->pic_scaling_list_present_flag[i] );
                if( pps->pic_scaling_list_present_flag[ i ] )
                {
                    if( i < 6 )
                        my_printf(" ScalingList4x4[%d] : %d\r\n", i, pps->ScalingList4x4[i] );
                    else
                        my_printf(" ScalingList4xScalingList8x84[%d] : %d\r\n", i, pps->ScalingList8x8[i] );
                }
            }
        }
        my_printf(" second_chroma_qp_index_offset : %d\r\n", pps->second_chroma_qp_index_offset );
    }
}

static void h264_debug_slice_header(h264_stream_t* h)
{
    sps_t* sps = h->sps;
    pps_t* pps = h->pps;
    slice_header_t* sh = h->sh;
    nal_t* nal = h->nal;

    my_printf("======= Slice Header =======\r\n");
    my_printf(" first_mb_in_slice : %d\r\n", sh->first_mb_in_slice );
    const char* slice_type_name;
    switch(sh->slice_type)
    {
    case SH_SLICE_TYPE_P :       slice_type_name = "P slice"; break;
    case SH_SLICE_TYPE_B :       slice_type_name = "B slice"; break;
    case SH_SLICE_TYPE_I :       slice_type_name = "I slice"; break;
    case SH_SLICE_TYPE_SP :      slice_type_name = "SP slice"; break;
    case SH_SLICE_TYPE_SI :      slice_type_name = "SI slice"; break;
    case SH_SLICE_TYPE_P_ONLY :  slice_type_name = "P slice only"; break;
    case SH_SLICE_TYPE_B_ONLY :  slice_type_name = "B slice only"; break;
    case SH_SLICE_TYPE_I_ONLY :  slice_type_name = "I slice only"; break;
    case SH_SLICE_TYPE_SP_ONLY : slice_type_name = "SP slice only"; break;
    case SH_SLICE_TYPE_SI_ONLY : slice_type_name = "SI slice only"; break;
    default :                    slice_type_name = "Unknown"; break;
    }
    my_printf(" slice_type : %d ( %s )\r\n", sh->slice_type, slice_type_name );

    my_printf(" pic_parameter_set_id : %d\r\n", sh->pic_parameter_set_id );
    if (sps->separate_colour_plane_flag == 1)
    {
        my_printf(" colour_plane_id : %d\r\n", sh->colour_plane_id );
    }
    my_printf(" frame_num : %d\r\n", sh->frame_num );
    if( !sps->frame_mbs_only_flag )
    {
        my_printf(" field_pic_flag : %d\r\n", sh->field_pic_flag );
        if( sh->field_pic_flag )
            my_printf(" bottom_field_flag : %d\r\n", sh->bottom_field_flag );
    }
    if( nal->nal_unit_type == 5 )
        my_printf(" idr_pic_id : %d\r\n", sh->idr_pic_id );
    if( sps->pic_order_cnt_type == 0 )
    {
        my_printf(" pic_order_cnt_lsb : %d\r\n", sh->pic_order_cnt_lsb );
        if( pps->pic_order_present_flag && !sh->field_pic_flag )
            my_printf(" delta_pic_order_cnt_bottom : %d\r\n", sh->delta_pic_order_cnt_bottom );
    }

    if( sps->pic_order_cnt_type == 1 && !sps->delta_pic_order_always_zero_flag )
    {
        my_printf(" delta_pic_order_cnt[0] : %d\r\n", sh->delta_pic_order_cnt[0] );
        if( pps->pic_order_present_flag && !sh->field_pic_flag )
            my_printf(" delta_pic_order_cnt[1] : %d\r\n", sh->delta_pic_order_cnt[1] );
    }
    if( pps->redundant_pic_cnt_present_flag )
        my_printf(" redundant_pic_cnt : %d\r\n", sh->redundant_pic_cnt );
    if( is_slice_type( sh->slice_type, SH_SLICE_TYPE_B ) )
        my_printf(" direct_spatial_mv_pred_flag : %d\r\n", sh->direct_spatial_mv_pred_flag );
    if( is_slice_type( sh->slice_type, SH_SLICE_TYPE_P ) || is_slice_type( sh->slice_type, SH_SLICE_TYPE_SP ) || is_slice_type( sh->slice_type, SH_SLICE_TYPE_B ) )
    {
        my_printf(" num_ref_idx_active_override_flag : %d\r\n", sh->num_ref_idx_active_override_flag );
        if( sh->num_ref_idx_active_override_flag )
        {
            my_printf(" num_ref_idx_l0_active_minus1 : %d\r\n", sh->num_ref_idx_l0_active_minus1 );
            if( is_slice_type( sh->slice_type, SH_SLICE_TYPE_B ) )
                my_printf(" num_ref_idx_l1_active_minus1 : %d\r\n", sh->num_ref_idx_l1_active_minus1 );
        }
    }
    // ref_pic_list_modification
    if (nal->nal_unit_type == 20 || nal->nal_unit_type == 21)
    {
        // todo.....
    }
    else
    {
        my_printf("=== Ref Pic List Modification ===\r\n");
        if( ! is_slice_type( sh->slice_type, SH_SLICE_TYPE_I ) && ! is_slice_type( sh->slice_type, SH_SLICE_TYPE_SI ) )
        {
            my_printf("  ref_pic_list_modification_flag_l0 : %d\r\n", sh->rplm.ref_pic_list_modification_flag_l0 );
            if( sh->rplm.ref_pic_list_modification_flag_l0 )
            {
                for (unsigned int i = 0; i < sh->rplm.rplm.size(); i++)
                {
                    my_printf("  modification_of_pic_nums_idc : %d\r\n", sh->rplm.rplm[i].modification_of_pic_nums_idc);
                    if( sh->rplm.rplm[i].modification_of_pic_nums_idc == 0 ||
                        sh->rplm.rplm[i].modification_of_pic_nums_idc == 1 )
                        my_printf("  abs_diff_pic_num_minus1 : %d\r\n", sh->rplm.rplm[i].abs_diff_pic_num_minus1 );
                    else if( sh->rplm.rplm[i].modification_of_pic_nums_idc == 2 )
                        my_printf("  long_term_pic_num : %d\r\n", sh->rplm.rplm[i].long_term_pic_num );
                }

            }
        }
        if( is_slice_type( sh->slice_type, SH_SLICE_TYPE_B ) )
        {
            my_printf("  ref_pic_list_modification_flag_l1 : %d\r\n", sh->rplm.ref_pic_list_modification_flag_l1 );
            if( sh->rplm.ref_pic_list_modification_flag_l1 )
            {
                for (unsigned int i = 0; i < sh->rplm.rplm.size(); i++)
                {
                    my_printf("  modification_of_pic_nums_idc : %d\r\n", sh->rplm.rplm[i].modification_of_pic_nums_idc );
                    if( sh->rplm.rplm[i].modification_of_pic_nums_idc == 0 ||
                        sh->rplm.rplm[i].modification_of_pic_nums_idc == 1 )
                        my_printf("  abs_diff_pic_num_minus1 : %d\r\n", sh->rplm.rplm[i].abs_diff_pic_num_minus1 );
                    else if( sh->rplm.rplm[i].modification_of_pic_nums_idc == 2 )
                        my_printf("  long_term_pic_num : %d\r\n", sh->rplm.rplm[i].long_term_pic_num );
                }
            }
        }
        
    }

    // pred_weight_table()
    if( ( pps->weighted_pred_flag && ( is_slice_type( sh->slice_type, SH_SLICE_TYPE_P ) || is_slice_type( sh->slice_type, SH_SLICE_TYPE_SP ) ) ) ||
        ( pps->weighted_bipred_idc == 1 && is_slice_type( sh->slice_type, SH_SLICE_TYPE_B ) ) )
    {
        my_printf(" === Prediction Weight Table ===\r\n");
        my_printf("  luma_log2_weight_denom : %d\r\n", sh->pwt.luma_log2_weight_denom );
        if( sps->ChromaArrayType != 0 )
            my_printf("  chroma_log2_weight_denom : %d\r\n", sh->pwt.chroma_log2_weight_denom );
        for( int i = 0; i <= sh->num_ref_idx_l0_active_minus1; i++ )
        {
            my_printf("  luma_weight_l0_flag[%d] : %d\r\n", i, sh->pwt.luma_weight_l0_flag[i] );
            if( sh->pwt.luma_weight_l0_flag[i] )
            {
                my_printf("   luma_weight_l0[%d] : %d\r\n", i, sh->pwt.luma_weight_l0[i] );
                my_printf("  l uma_offset_l0[%d] : %d\r\n", i, sh->pwt.luma_offset_l0[i] );
            }
            if ( sps->ChromaArrayType != 0 )
            {
                my_printf("  chroma_weight_l0_flag[%d] : %d\r\n", i, sh->pwt.chroma_weight_l0_flag[i] );
                if( sh->pwt.chroma_weight_l0_flag[i] )
                {
                    for( int j =0; j < 2; j++ )
                    {
                        my_printf("   chroma_weight_l0[%d][%d] : %d\r\n", i, j, sh->pwt.chroma_weight_l0[i][j] );
                        my_printf("   chroma_weight_l0[%d][%d] : %d\r\n", i, j, sh->pwt.chroma_offset_l0[i][j] );
                    }
                }
            }
        }
        if( is_slice_type( sh->slice_type, SH_SLICE_TYPE_B ) )
        {
            for( int i = 0; i <= sh->num_ref_idx_l1_active_minus1; i++ )
            {
                my_printf("  luma_weight_l1_flag[%d] : %d\r\n", i, sh->pwt.luma_weight_l1_flag[i] );
                if( sh->pwt.luma_weight_l1_flag[i] )
                {
                    my_printf("   luma_weight_l1[%d] : %d\r\n", i, sh->pwt.luma_weight_l1[i] );
                    my_printf("   luma_offset_l1[%d] : %d\r\n", i, sh->pwt.luma_offset_l1[i] );
                }
                if ( sps->ChromaArrayType != 0 )
                {
                    my_printf("  chroma_weight_l1_flag[%d] : %d\r\n", i, sh->pwt.chroma_weight_l1_flag[i] );
                    if( sh->pwt.chroma_weight_l1_flag[i] )
                    {
                        for( int j =0; j < 2; j++ )
                        {
                            my_printf("   chroma_weight_l1[%d][%d] : %d\r\n", i, j, sh->pwt.chroma_weight_l1[i][j] );
                            my_printf("   chroma_offset_l1[%d][%d] : %d\r\n", i, j, sh->pwt.chroma_offset_l1[i][j] );
                        }
                    }
                }
            }
        }
    }
    // dec_ref_pic_marking()
    if( nal->nal_ref_idc != 0 )
    {
        my_printf(" === Decoded Ref Pic Marking ===\r\n");
        if( h->nal->nal_unit_type == 5 )
        {
            my_printf("  no_output_of_prior_pics_flag : %d\r\n", sh->drpm.no_output_of_prior_pics_flag );
            my_printf("  long_term_reference_flag : %d\r\n", sh->drpm.long_term_reference_flag );
        }
        else
        {
            my_printf("  adaptive_ref_pic_marking_mode_flag : %d\r\n", sh->drpm.adaptive_ref_pic_marking_mode_flag );
            if( sh->drpm.adaptive_ref_pic_marking_mode_flag )
            {
                for (unsigned int i = 0; i < sh->drpm.drpm.size(); i++)
                {
                    my_printf("  memory_management_control_operation : %d\r\n", sh->drpm.drpm[i].memory_management_control_operation );
                    if( sh->drpm.drpm[i].memory_management_control_operation == 1 ||
                        sh->drpm.drpm[i].memory_management_control_operation == 3 )
                        my_printf("  difference_of_pic_nums_minus1 : %d\r\n", sh->drpm.drpm[i].difference_of_pic_nums_minus1 );
                    if(sh->drpm.drpm[i].memory_management_control_operation == 2 )
                        my_printf("  long_term_pic_num : %d\r\n", sh->drpm.drpm[i].long_term_pic_num );
                    if( sh->drpm.drpm[i].memory_management_control_operation == 3 ||
                        sh->drpm.drpm[i].memory_management_control_operation == 6 )
                        my_printf("  long_term_frame_idx : %d\r\n", sh->drpm.drpm[i].long_term_frame_idx );
                    if( sh->drpm.drpm[i].memory_management_control_operation == 4 )
                        my_printf("  max_long_term_frame_idx_plus1 : %d\r\n", sh->drpm.drpm[i].max_long_term_frame_idx_plus1 );
                }
            }
        }
    }
    if( pps->entropy_coding_mode_flag && ! is_slice_type( sh->slice_type, SH_SLICE_TYPE_I ) && ! is_slice_type( sh->slice_type, SH_SLICE_TYPE_SI ) )
        my_printf(" cabac_init_idc : %d\r\n", sh->cabac_init_idc );
    my_printf(" slice_qp_delta : %d\r\n", sh->slice_qp_delta );
    if( is_slice_type( sh->slice_type, SH_SLICE_TYPE_SP ) || is_slice_type( sh->slice_type, SH_SLICE_TYPE_SI ) )
    {
        if( is_slice_type( sh->slice_type, SH_SLICE_TYPE_SP ) )
            my_printf(" sp_for_switch_flag : %d\r\n", sh->sp_for_switch_flag );
        my_printf(" slice_qs_delta : %d\r\n", sh->slice_qs_delta );
    }
    if( pps->deblocking_filter_control_present_flag )
    {
        my_printf(" disable_deblocking_filter_idc : %d\r\n", sh->disable_deblocking_filter_idc );
        if( sh->disable_deblocking_filter_idc != 1 )
        {
            my_printf(" slice_alpha_c0_offset_div2 : %d\r\n", sh->slice_alpha_c0_offset_div2 );
            my_printf(" slice_beta_offset_div2 : %d\r\n", sh->slice_beta_offset_div2 );
        }
    }

    if( pps->num_slice_groups_minus1 > 0 &&
        pps->slice_group_map_type >= 3 && pps->slice_group_map_type <= 5)
        my_printf(" slice_group_change_cycle : %d\r\n", sh->slice_group_change_cycle );
}

static void h264_debug_aud(aud_t* aud)
{
    my_printf("======= Access Unit Delimiter =======\r\n");
    const char* primary_pic_type_name;
    switch (aud->primary_pic_type)
    {
    case AUD_PRIMARY_PIC_TYPE_I :       primary_pic_type_name = "I"; break;
    case AUD_PRIMARY_PIC_TYPE_IP :      primary_pic_type_name = "I, P"; break;
    case AUD_PRIMARY_PIC_TYPE_IPB :     primary_pic_type_name = "I, P, B"; break;
    case AUD_PRIMARY_PIC_TYPE_SI :      primary_pic_type_name = "SI"; break;
    case AUD_PRIMARY_PIC_TYPE_SISP :    primary_pic_type_name = "SI, SP"; break;
    case AUD_PRIMARY_PIC_TYPE_ISI :     primary_pic_type_name = "I, SI"; break;
    case AUD_PRIMARY_PIC_TYPE_ISIPSP :  primary_pic_type_name = "I, SI, P, SP"; break;
    case AUD_PRIMARY_PIC_TYPE_ISIPSPB : primary_pic_type_name = "I, SI, P, SP, B"; break;
    default : primary_pic_type_name = "Unknown"; break;
    }
    my_printf(" primary_pic_type : %d ( %s )\r\n", aud->primary_pic_type, primary_pic_type_name );
}

static void h264_debug_seis( h264_stream_t* h)
{
    sei_t** seis = h->seis;
    int num_seis = h->num_seis;

    my_printf("======= SEI =======\r\n");
    const char* sei_type_name;
    int i;
    for (i = 0; i < num_seis; i++)
    {
        sei_t* s = seis[i];
        my_printf(" payloadType : %d\r\n", s->payloadType );
        my_printf(" payloadSize : %d\r\n", s->payloadSize );
        switch(s->payloadType)
        {
        case SEI_TYPE_BUFFERING_PERIOD :          sei_type_name = "Buffering period"; break;
        case SEI_TYPE_PIC_TIMING :                sei_type_name = "Pic timing"; break;
        case SEI_TYPE_PAN_SCAN_RECT :             sei_type_name = "Pan scan rect"; break;
        case SEI_TYPE_FILLER_PAYLOAD :            sei_type_name = "Filler payload"; break;
        case SEI_TYPE_USER_DATA_REGISTERED_ITU_T_T35 : sei_type_name = "User data registered ITU-T T35"; break;
        case SEI_TYPE_USER_DATA_UNREGISTERED :
            {
                
                my_printf(" sei_payload()\r\n");
                my_printf("  user_data_unregistered()\r\n");
                my_printf("   uuid_iso_iec_11578: ");
                for (int j = 0; j < 16; j++)
                    my_printf("%X", s->payload[j]);
                my_printf ("\r\n   ");
                for (int j = 16; j < s->payloadSize; j++)
                {
                    my_printf("%c", s->payload[j]);
                    if ((j+1) % 128 == 0) my_printf ("\r\n");
                }
                break;
            }
            
        case SEI_TYPE_RECOVERY_POINT :            sei_type_name = "Recovery point"; break;
        case SEI_TYPE_DEC_REF_PIC_MARKING_REPETITION : sei_type_name = "Dec ref pic marking repetition"; break;
        case SEI_TYPE_SPARE_PIC :                 sei_type_name = "Spare pic"; break;
        case SEI_TYPE_SCENE_INFO :                sei_type_name = "Scene info"; break;
        case SEI_TYPE_SUB_SEQ_INFO :              sei_type_name = "Sub seq info"; break;
        case SEI_TYPE_SUB_SEQ_LAYER_CHARACTERISTICS : sei_type_name = "Sub seq layer characteristics"; break;
        case SEI_TYPE_SUB_SEQ_CHARACTERISTICS :   sei_type_name = "Sub seq characteristics"; break;
        case SEI_TYPE_FULL_FRAME_FREEZE :         sei_type_name = "Full frame freeze"; break;
        case SEI_TYPE_FULL_FRAME_FREEZE_RELEASE : sei_type_name = "Full frame freeze release"; break;
        case SEI_TYPE_FULL_FRAME_SNAPSHOT :       sei_type_name = "Full frame snapshot"; break;
        case SEI_TYPE_PROGRESSIVE_REFINEMENT_SEGMENT_START : sei_type_name = "Progressive refinement segment start"; break;
        case SEI_TYPE_PROGRESSIVE_REFINEMENT_SEGMENT_END : sei_type_name = "Progressive refinement segment end"; break;
        case SEI_TYPE_MOTION_CONSTRAINED_SLICE_GROUP_SET : sei_type_name = "Motion constrained slice group set"; break;
        case SEI_TYPE_FILM_GRAIN_CHARACTERISTICS : sei_type_name = "Film grain characteristics"; break;
        case SEI_TYPE_DEBLOCKING_FILTER_DISPLAY_PREFERENCE : sei_type_name = "Deblocking filter display preference"; break;
        case SEI_TYPE_STEREO_VIDEO_INFO :         sei_type_name = "Stereo video info"; break;
        default: sei_type_name = "Unknown"; break;
        }
    }
}

/**
 Print the contents of a NAL unit to standard output.
 The NAL which is printed out has a type determined by nal and data which comes from other fields within h depending on its type.
 @param[in]      h          the stream object
 @param[in]      nal        the nal unit
 */
static void h264_debug_nal(h264_stream_t* h, nal_t* nal)
{
    my_printf("==================== NAL ====================\r\n");
    my_printf(" forbidden_zero_bit : %d\r\n", nal->forbidden_zero_bit );
    my_printf(" nal_ref_idc : %d\r\n", nal->nal_ref_idc );
    // TODO make into subroutine
    const char* nal_unit_type_name;
    switch (nal->nal_unit_type)
    {
    case  NAL_UNIT_TYPE_UNSPECIFIED :                   nal_unit_type_name = "Unspecified"; break;
    case  NAL_UNIT_TYPE_CODED_SLICE_NON_IDR :           nal_unit_type_name = "Coded slice of a non-IDR picture"; break;
    case  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A :  nal_unit_type_name = "Coded slice data partition A"; break;
    case  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B :  nal_unit_type_name = "Coded slice data partition B"; break;
    case  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C :  nal_unit_type_name = "Coded slice data partition C"; break;
    case  NAL_UNIT_TYPE_CODED_SLICE_IDR :               nal_unit_type_name = "Coded slice of an IDR picture"; break;
    case  NAL_UNIT_TYPE_SEI :                           nal_unit_type_name = "Supplemental enhancement information (SEI)"; break;
    case  NAL_UNIT_TYPE_SPS :                           nal_unit_type_name = "Sequence parameter set"; break;
    case  NAL_UNIT_TYPE_PPS :                           nal_unit_type_name = "Picture parameter set"; break;
    case  NAL_UNIT_TYPE_AUD :                           nal_unit_type_name = "Access unit delimiter"; break;
    case  NAL_UNIT_TYPE_END_OF_SEQUENCE :               nal_unit_type_name = "End of sequence"; break;
    case  NAL_UNIT_TYPE_END_OF_STREAM :                 nal_unit_type_name = "End of stream"; break;
    case  NAL_UNIT_TYPE_FILLER :                        nal_unit_type_name = "Filler data"; break;
    case  NAL_UNIT_TYPE_SPS_EXT :                       nal_unit_type_name = "Sequence parameter set extension"; break;
        // 14..18    // Reserved
    case  NAL_UNIT_TYPE_CODED_SLICE_AUX :               nal_unit_type_name = "Coded slice of an auxiliary coded picture without partitioning"; break;
        // 20..23    // Reserved
        // 24..31    // Unspecified
    default :                                           nal_unit_type_name = "Unknown"; break;
    }
    my_printf(" nal_unit_type : %d ( %s )\r\n", nal->nal_unit_type, nal_unit_type_name );

    if( nal->nal_unit_type == NAL_UNIT_TYPE_CODED_SLICE_NON_IDR) { h264_debug_slice_header(h); }
    else if( nal->nal_unit_type == NAL_UNIT_TYPE_CODED_SLICE_IDR) { h264_debug_slice_header(h); }
    else if( nal->nal_unit_type == NAL_UNIT_TYPE_SPS) { h264_debug_sps(h->sps); }
    else if( nal->nal_unit_type == NAL_UNIT_TYPE_PPS) { h264_debug_pps(h->pps); }
    else if( nal->nal_unit_type == NAL_UNIT_TYPE_AUD) { h264_debug_aud(h->aud); }
    else if( nal->nal_unit_type == NAL_UNIT_TYPE_SEI) { h264_debug_seis( h ); }
}

static void debug_bytes(uint8_t* buf, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        my_printf("%02X ", buf[i]);
        if ((i+1) % 16 == 0) { my_printf ("\r\n"); }
    }
    my_printf("\r\n");
}

////////////////////////////////////////////////////////
static void h265_debug_ptl(profile_tier_level_t* ptl, int profilePresentFlag, int max_sub_layers_minus1)
{
    if (profilePresentFlag)
    {
        my_printf(" general_profile_space: %d\r\n", ptl->general_profile_space);
        my_printf(" general_tier_flag: %d\r\n", ptl->general_tier_flag);
        my_printf(" general_profile_idc: %d\r\n", ptl->general_profile_idc);
        for (int i = 0; i < 32; i++)
        {
            my_printf(" general_profile_compatibility_flag[%d]: %d\r\n", i, ptl->general_profile_compatibility_flag[i]);
        }
        my_printf(" general_progressive_source_flag: %d\r\n", ptl->general_progressive_source_flag);
        my_printf(" general_interlaced_source_flag: %d\r\n", ptl->general_interlaced_source_flag);
        my_printf(" general_non_packed_constraint_flag: %d\r\n", ptl->general_non_packed_constraint_flag);
        my_printf(" general_frame_only_constraint_flag: %d\r\n", ptl->general_frame_only_constraint_flag);
        if (ptl->general_profile_idc==4 || ptl->general_profile_compatibility_flag[4] ||
            ptl->general_profile_idc==5 || ptl->general_profile_compatibility_flag[5] ||
            ptl->general_profile_idc==6 || ptl->general_profile_compatibility_flag[6] ||
            ptl->general_profile_idc==7 || ptl->general_profile_compatibility_flag[7])
        {
            my_printf(" general_max_12bit_constraint_flag: %d\r\n", ptl->general_max_12bit_constraint_flag);
            my_printf(" general_max_10bit_constraint_flag: %d\r\n", ptl->general_max_10bit_constraint_flag);
            my_printf(" general_max_8bit_constraint_flag: %d\r\n", ptl->general_max_8bit_constraint_flag);
            my_printf(" general_max_422chroma_constraint_flag: %d\r\n", ptl->general_max_422chroma_constraint_flag);
            my_printf(" general_max_420chroma_constraint_flag: %d\r\n", ptl->general_max_420chroma_constraint_flag);
            my_printf(" general_max_monochrome_constraint_flag: %d\r\n", ptl->general_max_monochrome_constraint_flag);
            my_printf(" general_intra_constraint_flag: %d\r\n", ptl->general_intra_constraint_flag);
            my_printf(" general_one_picture_only_constraint_flag: %d\r\n", ptl->general_one_picture_only_constraint_flag);
            my_printf(" general_lower_bit_rate_constraint_flag: %d\r\n", ptl->general_lower_bit_rate_constraint_flag);
            my_printf(" general_reserved_zero_34bits: %u\r\n", ptl->general_reserved_zero_34bits);// tocheck
        }
        else
        {
            my_printf(" general_reserved_zero_43bits: %u\r\n", ptl->general_reserved_zero_43bits);// tocheck
        }
        if ((ptl->general_profile_idc>=1 && ptl->general_profile_idc<=5) ||
            ptl->general_profile_compatibility_flag[1] || ptl->general_profile_compatibility_flag[2] ||
            ptl->general_profile_compatibility_flag[3] || ptl->general_profile_compatibility_flag[4] ||
            ptl->general_profile_compatibility_flag[5])
        {
            my_printf(" general_inbld_flag: %d\r\n", ptl->general_inbld_flag);
        }
        else
        {
            my_printf(" general_reserved_zero_bit: %d\r\n", ptl->general_reserved_zero_bit);
        }
    }
        
    my_printf(" general_level_idc: %d\r\n", ptl->general_level_idc);
    for (int i = 0; i < max_sub_layers_minus1; i++)
    {
        my_printf(" sub_layer_profile_present_flag[%d]: %d\r\n", i, ptl->sub_layer_profile_present_flag[i]);
        my_printf(" sub_layer_level_present_flag[%d]: %d\r\n", i, ptl->sub_layer_level_present_flag[i]);
    }
    if (max_sub_layers_minus1 > 0)
    {
        for (int i = max_sub_layers_minus1; i < 8; i++)
        {
            my_printf(" reserved_zero_2bits[%d]: %d\r\n", i, ptl->reserved_zero_2bits[i]);
        }
    }
    for (int i = 0; i < max_sub_layers_minus1; i++)
    {
        if (ptl->sub_layer_profile_present_flag[i])
        {
            my_printf("  sub_layer_profile_space[%d]: %d\r\n", i, ptl->sub_layer_profile_space[i]);
            my_printf("  sub_layer_tier_flag[%d]: %d\r\n", i, ptl->sub_layer_tier_flag[i]);
            my_printf("  sub_layer_profile_idc[%d]: %d\r\n", i, ptl->sub_layer_profile_idc[i]);
            for (int j = 0; j < 32; j++)
            {
                my_printf("  sub_layer_profile_compatibility_flag[%d][%d]: %d\r\n", i, j, ptl->sub_layer_profile_compatibility_flag[i][j]);
            }
            my_printf("  sub_layer_progressive_source_flag[%d]: %d\r\n", i, ptl->sub_layer_progressive_source_flag[i]);
            my_printf("  sub_layer_interlaced_source_flag[%d]: %d\r\n", i, ptl->sub_layer_interlaced_source_flag[i]);
            my_printf("  sub_layer_non_packed_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_non_packed_constraint_flag[i]);
            my_printf("  sub_layer_frame_only_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_frame_only_constraint_flag[i]);
            if (ptl->sub_layer_profile_idc[i]==4 || ptl->sub_layer_profile_compatibility_flag[i][4] ||
                ptl->sub_layer_profile_idc[i]==5 || ptl->sub_layer_profile_compatibility_flag[i][5] ||
                ptl->sub_layer_profile_idc[i]==6 || ptl->sub_layer_profile_compatibility_flag[i][6] ||
                ptl->sub_layer_profile_idc[i]==7 || ptl->sub_layer_profile_compatibility_flag[i][7])
            {
                my_printf("  sub_layer_max_12bit_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_max_12bit_constraint_flag[i]);
                my_printf("  sub_layer_max_10bit_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_max_10bit_constraint_flag[i]);
                my_printf("  sub_layer_max_8bit_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_max_8bit_constraint_flag[i]);
                my_printf("  sub_layer_max_422chroma_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_max_422chroma_constraint_flag[i]);
                my_printf("  sub_layer_max_420chroma_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_max_420chroma_constraint_flag[i]);
                my_printf("  sub_layer_max_monochrome_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_max_monochrome_constraint_flag[i]);
                my_printf("  sub_layer_intra_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_intra_constraint_flag[i]);
                my_printf("  sub_layer_one_picture_only_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_one_picture_only_constraint_flag[i]);
                my_printf("  sub_layer_lower_bit_rate_constraint_flag[%d]: %d\r\n", i, ptl->sub_layer_lower_bit_rate_constraint_flag[i]);
                my_printf("  sub_layer_reserved_zero_34bits[%d]: %ul\r\n", i, ptl->sub_layer_reserved_zero_34bits[i]);// todo
            }
            else
            {
                my_printf("  sub_layer_reserved_zero_43bits: %ul\r\n", ptl->sub_layer_reserved_zero_43bits);// todo
            }
            // to check
            if ((ptl->sub_layer_profile_idc[i]>=1 && ptl->sub_layer_profile_idc[i]<=5) ||
                ptl->sub_layer_profile_compatibility_flag[i][1] ||
                ptl->sub_layer_profile_compatibility_flag[i][2] ||
                ptl->sub_layer_profile_compatibility_flag[i][3] ||
                ptl->sub_layer_profile_compatibility_flag[i][4] ||
                ptl->sub_layer_profile_compatibility_flag[i][5])
            {
                my_printf("  sub_layer_inbld_flag[%d]: %d\r\n", i, ptl->sub_layer_inbld_flag[i]);
            }
            else
            {
                my_printf("  sub_layer_reserved_zero_bit[%d]: %d\r\n", i, ptl->sub_layer_reserved_zero_bit[i]);
            }
        }
        if (ptl->sub_layer_level_present_flag[i])
        {
            my_printf("  sub_layer_level_idc[%d]: %d\r\n", i, ptl->sub_layer_level_idc[i]);
        }
    }
    
}

void h265_debug_sub_layer_hrd_parameters(sub_layer_hrd_parameters_t* subhrd, int sub_pic_hrd_params_present_flag, int CpbCnt, const char* p)
{
    my_printf("  [%s] sub_layer_hrd_parameters(%d)\r\n", p);
    for (int i = 0; i <= CpbCnt; i++)
    {
        my_printf("   bit_rate_value_minus1[%d]: %d\r\n", i, subhrd->bit_rate_value_minus1[i]);
        my_printf("   cpb_size_value_minus1[%d]: %d\r\n", i, subhrd->cpb_size_value_minus1[i]);
        if (sub_pic_hrd_params_present_flag)
        {
            my_printf("    cpb_size_du_value_minus1[%d]: %d\r\n", i, subhrd->cpb_size_du_value_minus1[i]);
            my_printf("    bit_rate_du_value_minus1[%d]: %d\r\n", i, subhrd->bit_rate_du_value_minus1[i]);
        }
        my_printf("   cbr_flag[%d]: %d\r\n", i, subhrd->cbr_flag[i]);
    }
}
static void h265_debug_hrd_parameters(hrd_parameters_t* hrd, int commonInfPresentFlag, int maxNumSubLayersMinus1)
{
    if(commonInfPresentFlag)
    {
        my_printf("  nal_hrd_parameters_present_flag: %d\r\n", hrd->nal_hrd_parameters_present_flag);
        my_printf("  vcl_hrd_parameters_present_flag: %d\r\n", hrd->vcl_hrd_parameters_present_flag);
        if (hrd->nal_hrd_parameters_present_flag ||
            hrd->vcl_hrd_parameters_present_flag)
        {
            
            my_printf("   sub_pic_hrd_params_present_flag: %d\r\n", hrd->sub_pic_hrd_params_present_flag);
            if (hrd->sub_pic_hrd_params_present_flag)
            {
                my_printf("    tick_divisor_minus2: %d\r\n", hrd->tick_divisor_minus2);
                my_printf("    du_cpb_removal_delay_increment_length_minus1: %d\r\n", hrd->du_cpb_removal_delay_increment_length_minus1);
                my_printf("    sub_pic_cpb_params_in_pic_timing_sei_flag: %d\r\n", hrd->sub_pic_cpb_params_in_pic_timing_sei_flag);
                my_printf("    dpb_output_delay_du_length_minus1: %d\r\n", hrd->dpb_output_delay_du_length_minus1);
            }
            my_printf("   bit_rate_scale: %d\r\n", hrd->bit_rate_scale);
            my_printf("   cpb_size_scale: %d\r\n", hrd->cpb_size_scale);
            if (hrd->sub_pic_hrd_params_present_flag)
                my_printf("    cpb_size_du_scale: %d\r\n", hrd->cpb_size_du_scale);
            my_printf("   initial_cpb_removal_delay_length_minus1: %d\r\n", hrd->initial_cpb_removal_delay_length_minus1);
            my_printf("   au_cpb_removal_delay_length_minus1: %d\r\n", hrd->au_cpb_removal_delay_length_minus1);
            my_printf("   dpb_output_delay_length_minus1: %d\r\n", hrd->dpb_output_delay_length_minus1);
        }
    }
    for (int i = 0; i <= maxNumSubLayersMinus1; i++)
    {
        my_printf("  fixed_pic_rate_general_flag[%d]: %d\r\n", i, hrd->fixed_pic_rate_general_flag[i]);
        if (!hrd->fixed_pic_rate_general_flag[i])
            my_printf("  fixed_pic_rate_general_flag[%d]: %d\r\n", i, hrd->fixed_pic_rate_general_flag[i]);
        if (hrd->fixed_pic_rate_within_cvs_flag[i])
        {
            my_printf("   elemental_duration_in_tc_minus1[%d]: %d\r\n", i, hrd->elemental_duration_in_tc_minus1[i]);
        }
        else
        {
            my_printf("   low_delay_hrd_flag[%d]: %d\r\n", i, hrd->low_delay_hrd_flag[i]);
        }
        if (!hrd->low_delay_hrd_flag[i])
            my_printf("   cpb_cnt_minus1[%d]: %d\r\n", i, hrd->cpb_cnt_minus1[i]);
            
        if(hrd->nal_hrd_parameters_present_flag)
        {
            h265_debug_sub_layer_hrd_parameters(&(hrd->sub_layer_hrd_parameters), hrd->sub_pic_hrd_params_present_flag, hrd->cpb_cnt_minus1[i], "nal");
        }
        if(hrd->vcl_hrd_parameters_present_flag)
        {
            h265_debug_sub_layer_hrd_parameters(&(hrd->sub_layer_hrd_parameters), hrd->sub_pic_hrd_params_present_flag, hrd->cpb_cnt_minus1[i], "vcl");
        }
    }
}
// vps
static void h265_debug_vps(h265_vps_t* vps)
{
    int i, j;
    my_printf("======= HEVC VPS =======\r\n");
    my_printf("vps_video_parameter_set_id: %d\r\n", vps->vps_video_parameter_set_id);
    my_printf("vps_base_layer_internal_flag: %d\r\n", vps->vps_base_layer_internal_flag);
    my_printf("vps_base_layer_available_flag: %d\r\n", vps->vps_base_layer_available_flag);
    my_printf("vps_max_layers_minus1: %d\r\n", vps->vps_max_layers_minus1);
    my_printf("vps_max_sub_layers_minus1: %d\r\n", vps->vps_max_sub_layers_minus1);
    my_printf("vps_temporal_id_nesting_flag: %d\r\n", vps->vps_temporal_id_nesting_flag);
    my_printf("vps_reserved_0xffff_16bits: %d\r\n", vps->vps_reserved_0xffff_16bits);
    // ptl
    my_printf("profile_tier_level()");
    h265_debug_ptl(&vps->ptl, 1, vps->vps_max_layers_minus1);
    
    my_printf("vps_sub_layer_ordering_info_present_flag: %d\r\n", vps->vps_sub_layer_ordering_info_present_flag);
    my_printf("SubLayers");
    for (i = (vps->vps_sub_layer_ordering_info_present_flag ? 0 : vps->vps_max_sub_layers_minus1);
        i <= vps->vps_max_sub_layers_minus1; i++ )
    {
        my_printf(" vps_max_dec_pic_buffering_minus1[%d]: %d\r\n", i, vps->vps_max_dec_pic_buffering_minus1[i]);
        my_printf(" vps_max_num_reorder_pics[%d]: %d\r\n", i, vps->vps_max_num_reorder_pics[i]);
        my_printf(" vps_max_latency_increase_plus1[%d]: %d\r\n", i, vps->vps_max_latency_increase_plus1[i]);
    }
    my_printf("vps_max_layer_id: %d\r\n", vps->vps_max_layer_id);
    my_printf("vps_num_layer_sets_minus1: %d\r\n", vps->vps_num_layer_sets_minus1);
    for (i = 1; i <= vps->vps_num_layer_sets_minus1; i++)
    {
        for (j = 0; j <= vps->vps_max_layer_id; j++)
        {
            my_printf(" layer_id_included_flag[%d][%d]: %d\r\n", i, j, vps->layer_id_included_flag[i][j]);
        }
    }
    my_printf("vps_timing_info_present_flag: %d\r\n", vps->vps_timing_info_present_flag);
    if (vps->vps_timing_info_present_flag)
    {
        my_printf(" vps_num_units_in_tick: %d\r\n", vps->vps_num_units_in_tick);
        my_printf(" vps_time_scale: %d\r\n", vps->vps_time_scale);
        my_printf(" vps_poc_proportional_to_timing_flag: %d\r\n", vps->vps_poc_proportional_to_timing_flag);
        if (vps->vps_poc_proportional_to_timing_flag)
        {
            my_printf("  vps_num_ticks_poc_diff_one_minus1: %d\r\n", vps->vps_num_ticks_poc_diff_one_minus1);
        }
        for (i = 0; i < vps->vps_num_hrd_parameters; i++)
        {
            my_printf("  hrd_layer_set_idx[%d]: %d\r\n", i, vps->hrd_layer_set_idx[i]);
            if (i > 0)
            {
                my_printf("   cprms_present_flag[%d]: %d\r\n", i, vps->cprms_present_flag[i]);
            }
            //  hrd_parameters()
            h265_debug_hrd_parameters(&(vps->hrd_parameters), vps->cprms_present_flag[i], vps->vps_max_sub_layers_minus1);
        }
    }
    my_printf("vps_extension_flag: %d\r\n", vps->vps_extension_flag);
    if (vps->vps_extension_flag)
    {
        // do nothing...
    }
}

static void h265_debug_scaling_list(scaling_list_data_t* sld)
{
    for(int sizeId = 0; sizeId < 4; sizeId++)
    {
        for(int matrixId = 0; matrixId < 6; matrixId += ( sizeId == 3 ) ? 3 : 1)
        {
            my_printf("  scaling_list_pred_mode_flag[%d][%d]: %d\r\n", sizeId, matrixId, sld->scaling_list_pred_mode_flag[sizeId][matrixId]);
            if (!sld->scaling_list_pred_mode_flag[sizeId][matrixId])
            {
                my_printf("   scaling_list_pred_mode_flag[%d][%d]: %d\r\n", sizeId, matrixId, sld->scaling_list_pred_matrix_id_delta[sizeId][matrixId]);
            }
            else
            {
                if (sizeId > 1)
                {
                    my_printf("   scaling_list_dc_coef_minus8[%d][%d]: %d\r\n", sizeId, matrixId, sld->scaling_list_dc_coef_minus8[sizeId - 2][matrixId]);
                }
                for (int i = 0; i < sld->coefNum; i++)
                {

                    my_printf("   ScalingList[%d][%d][%d]: %d\r\n", sizeId, matrixId, i, sld->ScalingList[sizeId][matrixId][i]);
                }
            }
        }
    }
}
void h265_debug_short_term_ref_pic_set(h265_sps_t* sps, st_ref_pic_set_t*st, referencePictureSets_t* rps, int stRpsIdx)
{
    my_printf(" st_ref_pic_set[%d]\r\n", stRpsIdx);

    my_printf(" inter_ref_pic_set_prediction_flag: %d\r\n", st->inter_ref_pic_set_prediction_flag);
    if (st->inter_ref_pic_set_prediction_flag)
    {
        my_printf("  delta_idx_minus1: %d\r\n", st->delta_idx_minus1);
        my_printf("  delta_rps_sign: %d\r\n", st->delta_rps_sign);
        my_printf("  abs_delta_rps_minus1: %d\r\n", st->abs_delta_rps_minus1);
        int rIdx = stRpsIdx - 1 - st->delta_idx_minus1;
        referencePictureSets_t* rpsRef = &sps->m_RPSList[rIdx];
        for (int j = 0; j <= rpsRef->m_numberOfPictures; j++)
        {
            my_printf("  used_by_curr_pic_flag[%d]: %d\r\n", j, st->used_by_curr_pic_flag[j]);
            if (!st->used_by_curr_pic_flag[j])
            {
                my_printf("  use_delta_flag[%d]: %d\r\n", j, st->use_delta_flag[j]);
            }
        }
    }
    else
    {
        my_printf("  num_negative_pics: %d\r\n", st->num_negative_pics);
        my_printf("  num_positive_pics: %d\r\n", st->num_positive_pics);
        for (int i = 0; i < st->num_negative_pics; i++)
        {
            my_printf("  delta_poc_s0_minus1[%d]: %d\r\n", i, st->delta_poc_s0_minus1[i]);
            my_printf("  used_by_curr_pic_s0_flag[%d]: %d\r\n", i, st->used_by_curr_pic_s0_flag[i]);
        }
        for (int i = 0; i < st->num_positive_pics; i++)
        {
            my_printf("  delta_poc_s1_minus1[%d]: %d\r\n", i, st->delta_poc_s1_minus1[i]);
            my_printf("  used_by_curr_pic_s1_flag[%d]: %d\r\n", i, st->used_by_curr_pic_s1_flag[i]);
        }
    }
}
static void h265_debug_vui_parameters(vui_parameters_t* vui, int maxNumSubLayersMinus1)
{
    my_printf(" aspect_ratio_info_present_flag: %d\r\n", vui->aspect_ratio_info_present_flag);
    if (vui->aspect_ratio_info_present_flag)
    {
        my_printf(" aspect_ratio_idc: %d\r\n", vui->aspect_ratio_idc);
        if (vui->aspect_ratio_idc == H265_SAR_Extended)
        {
            my_printf("  sar_width: %d\r\n", vui->sar_width);
            my_printf("  sar_height: %d\r\n", vui->sar_height);
        }
    }
    my_printf(" overscan_info_present_flag: %d\r\n", vui->overscan_info_present_flag);
    if (vui->overscan_info_present_flag)
    {
        my_printf(" overscan_appropriate_flag: %d\r\n", vui->overscan_appropriate_flag);
    }
    my_printf(" video_signal_type_present_flag: %d\r\n", vui->video_signal_type_present_flag);
    if (vui->video_signal_type_present_flag)
    {
        my_printf("  video_format: %d\r\n", vui->video_format);
        my_printf("  video_full_range_flag: %d\r\n", vui->video_full_range_flag);
        my_printf("  colour_description_present_flag: %d\r\n", vui->colour_description_present_flag);
        if (vui->colour_description_present_flag)
        {
            my_printf("   colour_primaries: %d\r\n", vui->colour_primaries);
            my_printf("   transfer_characteristics: %d\r\n", vui->transfer_characteristics);
            my_printf("   matrix_coeffs: %d\r\n", vui->matrix_coeffs);
        }
    }
    my_printf(" chroma_loc_info_present_flag: %d\r\n", vui->chroma_loc_info_present_flag);

    if (vui->chroma_loc_info_present_flag)
    {
        my_printf("  chroma_sample_loc_type_top_field: %d\r\n", vui->chroma_sample_loc_type_top_field);
        my_printf("  chroma_sample_loc_type_bottom_field: %d\r\n", vui->chroma_sample_loc_type_bottom_field);
    }
    my_printf(" neutral_chroma_indication_flag: %d\r\n", vui->neutral_chroma_indication_flag);
    my_printf(" field_seq_flag: %d\r\n", vui->field_seq_flag);
    my_printf(" frame_field_info_present_flag: %d\r\n", vui->frame_field_info_present_flag);
    my_printf(" default_display_window_flag: %d\r\n", vui->default_display_window_flag);
    if (vui->default_display_window_flag)
    {
        my_printf("  def_disp_win_left_offset: %d\r\n", vui->def_disp_win_left_offset);
        my_printf("  def_disp_win_left_offset: %d\r\n", vui->def_disp_win_left_offset);
        my_printf("  def_disp_win_right_offset: %d\r\n", vui->def_disp_win_right_offset);
        my_printf("  def_disp_win_bottom_offset: %d\r\n", vui->def_disp_win_bottom_offset);
    }
    my_printf(" vui_timing_info_present_flag: %d\r\n", vui->vui_timing_info_present_flag);
    if (vui->vui_timing_info_present_flag)
    {
        my_printf("  vui_num_units_in_tick: %d\r\n", vui->vui_num_units_in_tick);
        my_printf("  vui_time_scale: %d\r\n", vui->vui_time_scale);
        my_printf("  vui_poc_proportional_to_timing_flag: %d\r\n", vui->vui_poc_proportional_to_timing_flag);
        if (vui->vui_poc_proportional_to_timing_flag)
        {
            my_printf("   vui_num_ticks_poc_diff_one_minus1: %d\r\n", vui->vui_num_ticks_poc_diff_one_minus1);
        }
        my_printf("  vui_hrd_parameters_present_flag: %d\r\n", vui->vui_hrd_parameters_present_flag);
        if (vui->vui_hrd_parameters_present_flag)
        {
            h265_debug_hrd_parameters(&vui->hrd_parameters, 1, maxNumSubLayersMinus1);
        }
    }
    my_printf(" bitstream_restriction_flag: %d\r\n", vui->bitstream_restriction_flag);
    if (vui->bitstream_restriction_flag)
    {
        my_printf("  tiles_fixed_structure_flag: %d\r\n", vui->tiles_fixed_structure_flag);
        my_printf("  motion_vectors_over_pic_boundaries_flag: %d\r\n", vui->motion_vectors_over_pic_boundaries_flag);
        my_printf("  restricted_ref_pic_lists_flag: %d\r\n", vui->restricted_ref_pic_lists_flag);
        my_printf("  min_spatial_segmentation_idc: %d\r\n", vui->min_spatial_segmentation_idc);
        my_printf("  max_bytes_per_pic_denom: %d\r\n", vui->max_bytes_per_pic_denom);
        my_printf("  max_bits_per_min_cu_denom: %d\r\n", vui->max_bits_per_min_cu_denom);
        my_printf("  log2_max_mv_length_horizontal: %d\r\n", vui->log2_max_mv_length_horizontal);
        my_printf("  log2_max_mv_length_vertical: %d\r\n", vui->bitstream_restriction_flag);
    }
}
// sps
static void h265_debug_sps(h265_sps_t* sps)
{
    my_printf("======= HEVC SPS =======\r\n");
    my_printf("sps_video_parameter_set_id: %d\r\n", sps->sps_video_parameter_set_id);
    my_printf("sps_max_sub_layers_minus1: %d\r\n", sps->sps_max_sub_layers_minus1);
    my_printf("sps_temporal_id_nesting_flag: %d\r\n", sps->sps_temporal_id_nesting_flag);
    // ptl
    my_printf("profile_tier_level()");
    h265_debug_ptl(&sps->ptl, 1, sps->sps_max_sub_layers_minus1);

    my_printf("sps_seq_parameter_set_id: %d\r\n", sps->sps_seq_parameter_set_id);
    my_printf("chroma_format_idc: %d\r\n", sps->chroma_format_idc);
    if (sps->chroma_format_idc == 3)
    {
        my_printf(" separate_colour_plane_flag: %d\r\n", sps->separate_colour_plane_flag);
    }
    my_printf("pic_width_in_luma_samples: %d\r\n", sps->pic_width_in_luma_samples);
    my_printf("pic_height_in_luma_samples: %d\r\n", sps->pic_height_in_luma_samples);
    my_printf("conformance_window_flag: %d\r\n", sps->conformance_window_flag);
    if (sps->conformance_window_flag)
    {
        my_printf(" conf_win_left_offset: %d\r\n", sps->conf_win_left_offset);
        my_printf(" conf_win_right_offset: %d\r\n", sps->conf_win_right_offset);
        my_printf(" conf_win_top_offset: %d\r\n", sps->conf_win_top_offset);
        my_printf(" conf_win_bottom_offset: %d\r\n", sps->conf_win_bottom_offset);
    }
    my_printf("bit_depth_luma_minus8: %d\r\n", sps->bit_depth_luma_minus8);
    my_printf("bit_depth_chroma_minus8: %d\r\n", sps->bit_depth_chroma_minus8);
    my_printf("log2_max_pic_order_cnt_lsb_minus4: %d\r\n", sps->log2_max_pic_order_cnt_lsb_minus4);
    my_printf("sps_sub_layer_ordering_info_present_flag: %d\r\n", sps->sps_sub_layer_ordering_info_present_flag);
    for (int i = (sps->sps_sub_layer_ordering_info_present_flag ? 0 : sps->sps_max_sub_layers_minus1);
        i <= sps->sps_max_sub_layers_minus1; i++ )
    {
        my_printf(" sps_max_dec_pic_buffering_minus1[%d]: %d\r\n", i, sps->sps_max_dec_pic_buffering_minus1[i]);
        my_printf(" sps_max_num_reorder_pics[%d]: %d\r\n", i, sps->sps_max_num_reorder_pics[i]);
        my_printf(" sps_max_latency_increase_plus1[%d]: %d\r\n", i, sps->sps_max_latency_increase_plus1[i]);
    }
    my_printf("log2_min_luma_coding_block_size_minus3: %d\r\n", sps->log2_min_luma_coding_block_size_minus3);
    my_printf("log2_diff_max_min_luma_coding_block_size: %d\r\n", sps->log2_diff_max_min_luma_coding_block_size);
    my_printf("log2_min_luma_transform_block_size_minus2: %d\r\n", sps->log2_min_luma_transform_block_size_minus2);
    my_printf("log2_diff_max_min_luma_transform_block_size: %d\r\n", sps->log2_diff_max_min_luma_transform_block_size);
    my_printf("max_transform_hierarchy_depth_inter: %d\r\n", sps->max_transform_hierarchy_depth_inter);
    my_printf("max_transform_hierarchy_depth_intra: %d\r\n", sps->max_transform_hierarchy_depth_intra);
    my_printf("scaling_list_enabled_flag: %d\r\n", sps->scaling_list_enabled_flag);
    if (sps->scaling_list_enabled_flag)
    {
        my_printf(" sps_scaling_list_data_present_flag: %d\r\n", sps->sps_scaling_list_data_present_flag);
        {
            if (sps->sps_scaling_list_data_present_flag)
            {
                h265_debug_scaling_list(&sps->scaling_list_data);
            }
        }
    }

    my_printf("amp_enabled_flag: %d\r\n", sps->amp_enabled_flag);
    my_printf("sample_adaptive_offset_enabled_flag: %d\r\n", sps->sample_adaptive_offset_enabled_flag);
    my_printf("pcm_enabled_flag: %d\r\n", sps->pcm_enabled_flag);
    if (sps->pcm_enabled_flag)
    {
        my_printf(" pcm_sample_bit_depth_luma_minus1: %d\r\n", sps->pcm_sample_bit_depth_luma_minus1);
        my_printf(" pcm_sample_bit_depth_chroma_minus1: %d\r\n", sps->pcm_sample_bit_depth_chroma_minus1);
        my_printf(" log2_min_pcm_luma_coding_block_size_minus3: %d\r\n", sps->log2_min_pcm_luma_coding_block_size_minus3);
        my_printf(" log2_diff_max_min_pcm_luma_coding_block_size: %d\r\n", sps->log2_diff_max_min_pcm_luma_coding_block_size);
        my_printf(" pcm_loop_filter_disabled_flag: %d\r\n", sps->pcm_loop_filter_disabled_flag);
    }
    my_printf("num_short_term_ref_pic_sets: %d\r\n", sps->num_short_term_ref_pic_sets);
    referencePictureSets_t* rps = NULL;
    st_ref_pic_set_t* st = NULL;
    for (int i = 0; i < sps->num_short_term_ref_pic_sets; i++)
    {
        st = &sps->st_ref_pic_set[i];
        rps = &sps->m_RPSList[i];
        h265_debug_short_term_ref_pic_set(sps, st, rps, i);
    }
    my_printf("long_term_ref_pics_present_flag: %d\r\n", sps->long_term_ref_pics_present_flag);
    if (sps->long_term_ref_pics_present_flag)
    {
        my_printf(" num_long_term_ref_pics_sps: %d\r\n", sps->num_long_term_ref_pics_sps);
        for (int i = 0; i < sps->num_long_term_ref_pics_sps; i++)
        {
            my_printf(" lt_ref_pic_poc_lsb_sps[%d]: %d\r\n", i, sps->lt_ref_pic_poc_lsb_sps[i]);
            my_printf(" used_by_curr_pic_lt_sps_flag[%d]: %d\r\n", i, sps->used_by_curr_pic_lt_sps_flag[i]);
        }
    }
    my_printf("sps_temporal_mvp_enabled_flag: %d\r\n", sps->sps_temporal_mvp_enabled_flag);
    my_printf("strong_intra_smoothing_enabled_flag: %d\r\n", sps->strong_intra_smoothing_enabled_flag);
    my_printf("vui_parameters_present_flag: %d\r\n", sps->vui_parameters_present_flag);
    if (sps->vui_parameters_present_flag)
    {
        // vui
        h265_debug_vui_parameters(&sps->vui, sps->sps_max_sub_layers_minus1);
    }
    my_printf("sps_extension_present_flag: %d\r\n", sps->sps_extension_present_flag);
    if (sps->sps_extension_present_flag)
    {
        my_printf(" sps_range_extension_flag: %d\r\n", sps->sps_range_extension_flag);
        my_printf(" sps_multilayer_extension_flag: %d\r\n", sps->sps_multilayer_extension_flag);
        my_printf(" sps_3d_extension_flag: %d\r\n", sps->sps_3d_extension_flag);
        my_printf(" sps_extension_5bits: %d\r\n", sps->sps_extension_5bits);
    }
    if (sps->sps_range_extension_flag)
    {
        my_printf(" transform_skip_rotation_enabled_flag: %d\r\n", sps->sps_range_extension.transform_skip_rotation_enabled_flag);
        my_printf(" transform_skip_context_enabled_flag: %d\r\n", sps->sps_range_extension.transform_skip_context_enabled_flag);
        my_printf(" implicit_rdpcm_enabled_flag: %d\r\n", sps->sps_range_extension.implicit_rdpcm_enabled_flag);
        my_printf(" explicit_rdpcm_enabled_flag: %d\r\n", sps->sps_range_extension.explicit_rdpcm_enabled_flag);
        my_printf(" extended_precision_processing_flag: %d\r\n", sps->sps_range_extension.extended_precision_processing_flag);
        my_printf(" intra_smoothing_disabled_flag: %d\r\n", sps->sps_range_extension.intra_smoothing_disabled_flag);
        my_printf(" high_precision_offsets_enabled_flag: %d\r\n", sps->sps_range_extension.high_precision_offsets_enabled_flag);
        my_printf(" persistent_rice_adaptation_enabled_flag: %d\r\n", sps->sps_range_extension.persistent_rice_adaptation_enabled_flag);
        my_printf(" cabac_bypass_alignment_enabled_flag: %d\r\n", sps->sps_range_extension.cabac_bypass_alignment_enabled_flag);
    }
    if (sps->sps_multilayer_extension_flag)
    {
        my_printf(" inter_view_mv_vert_constraint_flag: %d\r\n", sps->inter_view_mv_vert_constraint_flag);
    }
    // todo sps_3d_extension_flag

}

// pps
static void h265_debug_pps(h265_pps_t* pps)
{
    my_printf("======= HEVC PPS =======\r\n");
    my_printf("pps_pic_parameter_set_id: %d\r\n", pps->pps_pic_parameter_set_id);
    my_printf("pps_seq_parameter_set_id: %d\r\n", pps->pps_seq_parameter_set_id);
    my_printf("dependent_slice_segments_enabled_flag: %d\r\n", pps->dependent_slice_segments_enabled_flag);
    my_printf("output_flag_present_flag: %d\r\n", pps->output_flag_present_flag);
    my_printf("num_extra_slice_header_bits: %d\r\n", pps->num_extra_slice_header_bits);
    my_printf("sign_data_hiding_enabled_flag: %d\r\n", pps->sign_data_hiding_enabled_flag);
    my_printf("cabac_init_present_flag: %d\r\n", pps->cabac_init_present_flag);
    my_printf("num_ref_idx_l0_default_active_minus1: %d\r\n", pps->num_ref_idx_l0_default_active_minus1);
    my_printf("num_ref_idx_l1_default_active_minus1: %d\r\n", pps->num_ref_idx_l1_default_active_minus1);
    my_printf("init_qp_minus26: %d\r\n", pps->init_qp_minus26);
    my_printf("constrained_intra_pred_flag: %d\r\n", pps->constrained_intra_pred_flag);
    my_printf("transform_skip_enabled_flag: %d\r\n", pps->transform_skip_enabled_flag);
    my_printf("cu_qp_delta_enabled_flag: %d\r\n", pps->cu_qp_delta_enabled_flag);
    if (pps->cu_qp_delta_enabled_flag)
        my_printf("diff_cu_qp_delta_depth: %d\r\n", pps->diff_cu_qp_delta_depth);
    my_printf("pps_cb_qp_offset: %d\r\n", pps->pps_cb_qp_offset);
    my_printf("pps_cr_qp_offset: %d\r\n", pps->pps_cr_qp_offset);
    my_printf("pps_slice_chroma_qp_offsets_present_flag: %d\r\n", pps->pps_slice_chroma_qp_offsets_present_flag);
    my_printf("weighted_pred_flag: %d\r\n", pps->weighted_pred_flag);
    my_printf("weighted_bipred_flag: %d\r\n", pps->weighted_bipred_flag);
    my_printf("transquant_bypass_enabled_flag: %d\r\n", pps->transquant_bypass_enabled_flag);
    my_printf("tiles_enabled_flag: %d\r\n", pps->tiles_enabled_flag);
    my_printf("entropy_coding_sync_enabled_flag: %d\r\n", pps->entropy_coding_sync_enabled_flag);
    if (pps->tiles_enabled_flag)
    {
        my_printf("num_tile_columns_minus1: %d\r\n", pps->num_tile_columns_minus1);
        my_printf("num_tile_rows_minus1: %d\r\n", pps->num_tile_rows_minus1);
        my_printf("uniform_spacing_flag: %d\r\n", pps->uniform_spacing_flag);
        if (!pps->uniform_spacing_flag)
        {
            for (int i = 0; i < pps->num_tile_columns_minus1; i++)
                my_printf(" column_width_minus1[%d]: %d\r\n", i, pps->column_width_minus1[i]);
            for (int i = 0; i < pps->num_tile_rows_minus1; i++)
                my_printf(" row_height_minus1[%d]: %d\r\n", i, pps->row_height_minus1[i]);
        }
        my_printf(" loop_filter_across_tiles_enabled_flag: %d\r\n", pps->loop_filter_across_tiles_enabled_flag); // to check
    }
    my_printf("pps_loop_filter_across_slices_enabled_flag: %d\r\n", pps->pps_loop_filter_across_slices_enabled_flag); // to check
    my_printf("deblocking_filter_control_present_flag: %d\r\n", pps->deblocking_filter_control_present_flag);
    if (pps->deblocking_filter_control_present_flag)
    {
        my_printf(" deblocking_filter_override_enabled_flag: %d\r\n", pps->deblocking_filter_override_enabled_flag);
        my_printf(" pps_deblocking_filter_disabled_flag: %d\r\n", pps->pps_deblocking_filter_disabled_flag);
        if (pps->pps_deblocking_filter_disabled_flag)
        {
            my_printf("  pps_beta_offset_div2: %d\r\n", pps->pps_beta_offset_div2);
            my_printf("  pps_tc_offset_div2: %d\r\n", pps->pps_tc_offset_div2);
        }
    }
    my_printf("pps_scaling_list_data_present_flag: %d\r\n", pps->pps_scaling_list_data_present_flag);
    if (pps->pps_scaling_list_data_present_flag)
    {
        // scaling_list_data()
        h265_debug_scaling_list(&pps->scaling_list_data);
    }
    my_printf("lists_modification_present_flag: %d\r\n", pps->lists_modification_present_flag);
    my_printf("log2_parallel_merge_level_minus2: %d\r\n", pps->log2_parallel_merge_level_minus2);
    my_printf("slice_segment_header_extension_present_flag: %d\r\n", pps->slice_segment_header_extension_present_flag);
    my_printf("pps_extension_present_flag: %d\r\n", pps->pps_extension_present_flag);
    if (pps->pps_extension_present_flag)
    {
        my_printf(" pps_range_extension_flag: %d\r\n", pps->pps_range_extension_flag);
        my_printf(" pps_multilayer_extension_flag: %d\r\n", pps->pps_multilayer_extension_flag);
        my_printf(" pps_3d_extension_flag: %d\r\n", pps->pps_3d_extension_flag);
        my_printf(" pps_extension_5bits: %d\r\n", pps->pps_extension_5bits);
    }
    if (pps->pps_range_extension_flag)
    {
        if (pps->transform_skip_enabled_flag)
            my_printf(" pps_extension_5bits: %d\r\n", pps->pps_range_extension.log2_max_transform_skip_block_size_minus2);
        my_printf(" cross_component_prediction_enabled_flag: %d\r\n", pps->pps_range_extension.cross_component_prediction_enabled_flag);
        my_printf(" chroma_qp_offset_list_enabled_flag: %d\r\n", pps->pps_range_extension.chroma_qp_offset_list_enabled_flag);
        if (pps->pps_range_extension.chroma_qp_offset_list_enabled_flag)
        {
            my_printf(" diff_cu_chroma_qp_offset_depth: %d\r\n", pps->pps_range_extension.diff_cu_chroma_qp_offset_depth);
            my_printf(" chroma_qp_offset_list_len_minus1: %d\r\n", pps->pps_range_extension.chroma_qp_offset_list_len_minus1);
            for (int i = 0; i < pps->pps_range_extension.chroma_qp_offset_list_len_minus1; i++)
            {
                my_printf(" cb_qp_offset_list[%d]: %d\r\n", i, pps->pps_range_extension.cb_qp_offset_list[i]);
                my_printf(" cr_qp_offset_list[%d]: %d\r\n", i, pps->pps_range_extension.cb_qp_offset_list[i]);
            }
        }
        my_printf(" log2_sao_offset_scale_luma: %d\r\n", pps->pps_range_extension.log2_sao_offset_scale_luma);
        my_printf(" log2_sao_offset_scale_chroma: %d\r\n", pps->pps_range_extension.log2_sao_offset_scale_chroma);
    }
    if (pps->pps_multilayer_extension_flag)
    {
        // todo...
    }
    if (pps->pps_3d_extension_flag)
    {
        // todo...
    }
}

// aud
static void h265_debug_aud(h265_aud_t* aud)
{
    my_printf("======= HEVC AUD =======\r\n");
    const char* pic_type;
    switch (aud->pic_type)
    {
    case H265_AUD_PRIMARY_PIC_TYPE_I :    pic_type = "I"; break;
    case H265_AUD_PRIMARY_PIC_TYPE_IP :   pic_type = "P, I"; break;
    case H265_AUD_PRIMARY_PIC_TYPE_IPB :  pic_type = "B, P, I"; break;
    default : pic_type = "Unknown"; break;
    }
    my_printf("pic_type: %d ( %s ) \r\n", aud->pic_type, pic_type );
}

// sei
static void h265_debug_seis(h265_stream_t* h)
{
    h265_sei_t** seis = h->seis;
    int num_seis = h->num_seis;

    my_printf("======= HEVC SEI =======\r\n");
    const char* sei_type_name;
    int i;
    for (i = 0; i < num_seis; i++)
    {
        h265_sei_t* s = seis[i];
        my_printf(" payloadType : %d\r\n", s->payloadType);
        my_printf(" payloadSize : %d\r\n", s->payloadSize);
        my_printf(" sei_payload()\r\n");
        if (h->nal->nal_unit_type == NAL_UNIT_PREFIX_SEI)
        {
            switch(s->payloadType)
            {
            case 0:
                my_printf("  buffering_period()\r\n");
                break;
            case 1:
                my_printf("  pic_timing()\r\n");
                break;
            case 2:
                my_printf("  pan_scan_rect()\r\n");
                break;
            case 3:
                my_printf("  pan_scan_rect()\r\n");
                break;
            case 4:
                my_printf("  pan_scan_rect()\r\n");
                break;
            case 5:
                my_printf("  user_data_unregistered()\r\n");
                my_printf("   uuid_iso_iec_11578: ");
                for (int j = 0; j < 16; j++)
                    my_printf("%X", s->payload[j]);
                my_printf ("\r\n   ");
                for (int j = 16; j < s->payloadSize; j++)
                {
                    my_printf("%c", s->payload[j]);
                    if ((j+1) % 128 == 0) my_printf ("\r\n");
                }
                break;
            case 6:
                my_printf("  recovery_point()\r\n");
                break;
            case 9:
                my_printf("  scene_info()\r\n");
                break;
            case 15:
                my_printf("  picture_snapshot()\r\n");
                break;
            case 16:
                my_printf("  progressive_refinement_segment_start()\r\n");
                break;
            case 17:
                my_printf("  progressive_refinement_segment_end()\r\n");
                break;
            default:
                my_printf("  reserved_sei_message()\r\n");
                break;
            }
        }
        else if (h->nal->nal_unit_type == NAL_UNIT_SUFFIX_SEI)
        {
            switch(s->payloadType)
            {
            case 3:
                my_printf("  filler_payload()\r\n");
                break;
            case 4:
                my_printf("  user_data_registered_itu_t_t35()\r\n");
                break;
            case 5:
                my_printf("  user_data_unregistered()\r\n");
                break;
            case 17:
                my_printf("  progressive_refinement_segment_end()\r\n");
                break;
            case 22:
                my_printf("  post_filter_hint()\r\n");
                break;
            case 132:
                my_printf("  decoded_picture_hash()\r\n");
                break;
            case 16:
                my_printf("  progressive_refinement_segment_start()\r\n");
                break;
            default:
                my_printf("  reserved_sei_message()\r\n");
                break;
            }
        }
    }
}

static void h265_debug_slice_header(h265_stream_t* h)
{
    h265_slice_header_t* hrd = h->sh;
    h265_sps_t* sps = NULL;
    h265_pps_t* pps = NULL;
    int nal_unit_type = h->nal->nal_unit_type;
    pps = h->pps = h->pps_table[hrd->slice_pic_parameter_set_id];
    sps = h->sps = h->sps_table[pps->pps_seq_parameter_set_id];

    my_printf("======= HEVC Slice Header =======\r\n");
    my_printf("first_slice_segment_in_pic_flag: %d\r\n", hrd->first_slice_segment_in_pic_flag);
    my_printf("no_output_of_prior_pics_flag: %d\r\n", hrd->no_output_of_prior_pics_flag);
    my_printf("slice_pic_parameter_set_id: %d\r\n", hrd->slice_pic_parameter_set_id);
    my_printf("dependent_slice_segment_flag: %d\r\n", hrd->dependent_slice_segment_flag);
    my_printf("slice_segment_address: %d\r\n", hrd->slice_segment_address);
    if (!hrd->dependent_slice_segment_flag)
    {
        my_printf("dependent_slice_segment_flag\r\n");
        for (int i = 0; i < pps->num_extra_slice_header_bits; i++)
            my_printf(" slice_reserved_flag[%d]: %d\r\n", i, hrd->slice_reserved_flag[i]);
        const char* slice_type_name;
        switch(hrd->slice_type)
        {
            case H265_SH_SLICE_TYPE_P:  slice_type_name = "P slice"; break;
            case H265_SH_SLICE_TYPE_B:  slice_type_name = "B slice"; break;
            case H265_SH_SLICE_TYPE_I:  slice_type_name = "I slice"; break;
            default :                   slice_type_name = "Unknown"; break;
        }
        my_printf(" slice_type: %d (%s)\r\n", hrd->slice_type, slice_type_name);
        if (pps->output_flag_present_flag)
            my_printf("  pic_output_flag: %d\r\n", hrd->pic_output_flag);
        if (sps->separate_colour_plane_flag == 1)
            my_printf("  colour_plane_id: %d\r\n", hrd->colour_plane_id);
        my_printf(" slice_pic_order_cnt_lsb: %d\r\n", hrd->slice_pic_order_cnt_lsb);
        my_printf(" short_term_ref_pic_set_sps_flag: %d\r\n", hrd->short_term_ref_pic_set_sps_flag);
        if (!hrd->short_term_ref_pic_set_sps_flag)
        {
            referencePictureSets_t* rps = &hrd->m_localRPS;
            h265_debug_short_term_ref_pic_set(sps, &hrd->st_ref_pic_set, rps, sps->num_short_term_ref_pic_sets);
        }
        else if (sps->num_short_term_ref_pic_sets > 1)
        {
            my_printf("  short_term_ref_pic_set_idx: %d\r\n", hrd->short_term_ref_pic_set_idx);
        }
        if (sps->long_term_ref_pics_present_flag)
        {
            my_printf("  num_long_term_sps: %d\r\n", hrd->num_long_term_sps);
            my_printf("  num_long_term_pics: %d\r\n", hrd->num_long_term_pics);
            for (int i = 0; i < (int)hrd->lt_idx_sps.size(); i++)
            {
                if (i < hrd->num_long_term_sps)
                    my_printf("   hrd->lt_idx_sps[%d]: %d\r\n", i, hrd->lt_idx_sps[i]);
                else
                {
                    my_printf("   hrd->poc_lsb_lt[%d]: %d\r\n", i, hrd->poc_lsb_lt[i]);
                    my_printf("   hrd->used_by_curr_pic_lt_flag[%d]: %d\r\n", i, hrd->used_by_curr_pic_lt_flag[i]);
                }
                my_printf("  hrd->delta_poc_msb_present_flag[%d]: %d\r\n", i, hrd->delta_poc_msb_present_flag[i]);
                if (hrd->delta_poc_msb_present_flag[i])
                    my_printf("  hrd->delta_poc_msb_cycle_lt[%d]: %d\r\n", i, hrd->delta_poc_msb_cycle_lt[i]);
            }
        }
        if(sps->sps_temporal_mvp_enabled_flag)
        {
            my_printf(" slice_temporal_mvp_enabled_flag: %d\r\n", hrd->slice_temporal_mvp_enabled_flag);
        }
        if(sps->sample_adaptive_offset_enabled_flag)
        {
            my_printf(" slice_sao_luma_flag: %d\r\n", hrd->slice_sao_luma_flag);
            my_printf(" slice_sao_chroma_flag: %d\r\n", hrd->slice_sao_chroma_flag);
        }
        if (hrd->slice_type == H265_SH_SLICE_TYPE_P || hrd->slice_type == H265_SH_SLICE_TYPE_B)
        {
            my_printf("  num_ref_idx_active_override_flag: %d\r\n", hrd->num_ref_idx_active_override_flag);
            if (hrd->num_ref_idx_active_override_flag)
            {
                my_printf("  num_ref_idx_l0_active_minus1: %d\r\n", hrd->num_ref_idx_l0_active_minus1);
                my_printf("  num_ref_idx_l1_active_minus1: %d\r\n", hrd->num_ref_idx_l1_active_minus1);
            }
            if(pps->lists_modification_present_flag)
            {
                // h265_read_ref_pic_lists_modification
            }
            my_printf("  mvd_l1_zero_flag: %d\r\n", hrd->mvd_l1_zero_flag);
            my_printf("  cabac_init_flag: %d\r\n", hrd->cabac_init_flag);
            my_printf("  collocated_from_l0_flag: %d\r\n", hrd->collocated_from_l0_flag);
            my_printf("  collocated_ref_idx: %d\r\n", hrd->collocated_ref_idx);
            // h265_read_pred_weight_table
            my_printf("  five_minus_max_num_merge_cand: %d\r\n", hrd->five_minus_max_num_merge_cand);
        }
        my_printf(" slice_qp_delta: %d\r\n", hrd->slice_qp_delta);
        if (pps->pps_slice_chroma_qp_offsets_present_flag)
        {
            my_printf("  slice_cb_qp_offset: %d\r\n", hrd->slice_cb_qp_offset);
            my_printf("  slice_cr_qp_offset: %d\r\n", hrd->slice_cr_qp_offset);
        }
        if (pps->pps_range_extension.chroma_qp_offset_list_enabled_flag)
        {
            my_printf("  cu_chroma_qp_offset_enabled_flag: %d\r\n", hrd->cu_chroma_qp_offset_enabled_flag);
        }
        if (pps->deblocking_filter_override_enabled_flag)
        {
            my_printf("  deblocking_filter_override_flag: %d\r\n", hrd->deblocking_filter_override_flag);
        }
        if (hrd->deblocking_filter_override_flag)
        {
            my_printf("  slice_deblocking_filter_disabled_flag: %d\r\n", hrd->slice_deblocking_filter_disabled_flag);
            if (!hrd->slice_deblocking_filter_disabled_flag)
            {
                my_printf("   slice_beta_offset_div2: %d\r\n", hrd->slice_beta_offset_div2);
                my_printf("   slice_tc_offset_div2: %d\r\n", hrd->slice_tc_offset_div2);
            }
        }
        my_printf(" slice_loop_filter_across_slices_enabled_flag: %d\r\n", hrd->slice_loop_filter_across_slices_enabled_flag);
        
    }
    if (pps->tiles_enabled_flag || pps->entropy_coding_sync_enabled_flag)
    {
        my_printf(" num_entry_point_offsets: %d\r\n", hrd->num_entry_point_offsets);
        if (hrd->num_entry_point_offsets > 0)
        {
            my_printf("  offset_len_minus1: %d\r\n", hrd->offset_len_minus1);
            my_printf(" NumEntryPointOffsets\r\n");
            for (int i = 0; i < hrd->num_entry_point_offsets; i++)
                my_printf("  entry_point_offset_minus1[%d]: %d\r\n", i, hrd->entry_point_offset_minus1[i]);
        }
    }
    if (pps->slice_segment_header_extension_present_flag)
    {
        my_printf("slice_segment_header_extension_length: %d\r\n", hrd->slice_segment_header_extension_length);
        for (int i = 0; i < hrd->slice_segment_header_extension_length; i++)
            my_printf("slice_segment_header_extension_data_byte[%d]: %d\r\n", hrd->slice_segment_header_extension_data_byte[i]);
    }
    // no need to debug...
    my_printf("slice_segment_data()\r\n");
    my_printf("rbsp_slice_segment_trailing_bits()\r\n");
}

static void h265_debug_nal(h265_stream_t* h, h265_nal_t* nal)
{
    int my_nal_type = -1;

    const char* nal_unit_type_name;
    switch (nal->nal_unit_type)
    {
    case NAL_UNIT_VPS:
        nal_unit_type_name = "Video parameter set";
        my_nal_type = 0;
        break;
    case NAL_UNIT_SPS:
        nal_unit_type_name = "Sequence parameter set";
        my_nal_type = 1;
        break;
    case NAL_UNIT_PPS:
        nal_unit_type_name = "Picture parameter set";
        my_nal_type = 2;
        break;
    case NAL_UNIT_AUD:
        nal_unit_type_name = "Access unit delimiter";
        my_nal_type = 3;
        break;
    case NAL_UNIT_EOS:
        nal_unit_type_name = "End of sequence";
        break;
    case NAL_UNIT_EOB:
        nal_unit_type_name = "End of bitstream";
        break;
    case NAL_UNIT_FILLER_DATA:
        nal_unit_type_name = "Filler data";
        break;
    case NAL_UNIT_PREFIX_SEI:
    case NAL_UNIT_SUFFIX_SEI:
        nal_unit_type_name = "Supplemental enhancement information";
        my_nal_type = 4;
        break;
    case NAL_UNIT_CODED_SLICE_TRAIL_N:
    case NAL_UNIT_CODED_SLICE_TRAIL_R:
        nal_unit_type_name = "Coded slice segment of a non-TSA, non-STSA trailing picture";
        my_nal_type = 5;
        break;
    case NAL_UNIT_CODED_SLICE_TSA_N:
    case NAL_UNIT_CODED_SLICE_TSA_R:
        nal_unit_type_name = "Coded slice segment of a TSA picture";
        my_nal_type = 5;
        break;
    case NAL_UNIT_CODED_SLICE_STSA_N:
    case NAL_UNIT_CODED_SLICE_STSA_R:
        nal_unit_type_name = "Coded slice segment of an STSA picture";
        my_nal_type = 5;
        break;
    case NAL_UNIT_CODED_SLICE_RADL_N:
    case NAL_UNIT_CODED_SLICE_RADL_R:
        nal_unit_type_name = "Coded slice segment of a RADL picture";
        my_nal_type = 5;
        break;
    case NAL_UNIT_CODED_SLICE_RASL_N:
    case NAL_UNIT_CODED_SLICE_RASL_R:
        nal_unit_type_name = "Coded slice segment of a RASL picture";
        my_nal_type = 5;
        break;
    case NAL_UNIT_RESERVED_VCL_N10:
    case NAL_UNIT_RESERVED_VCL_N12:
    case NAL_UNIT_RESERVED_VCL_N14:
        nal_unit_type_name = "Reserved non-IRAP SLNR VCL NAL unit types";
        my_nal_type = 5;
        break;
    case NAL_UNIT_RESERVED_VCL_R11:
    case NAL_UNIT_RESERVED_VCL_R13:
    case NAL_UNIT_RESERVED_VCL_R15:
        nal_unit_type_name = "Reserved non-IRAP sub-layer reference VCL NAL unit types";
        break;
    case NAL_UNIT_CODED_SLICE_BLA_W_LP:
    case NAL_UNIT_CODED_SLICE_BLA_W_RADL:
    case NAL_UNIT_CODED_SLICE_BLA_N_LP:
        nal_unit_type_name = "Coded slice segment of a BLA picture";
        my_nal_type = 5;
        break;
    case NAL_UNIT_CODED_SLICE_IDR_W_RADL:
    case NAL_UNIT_CODED_SLICE_IDR_N_LP:
        nal_unit_type_name = "Coded slice segment of an IDR picture";
        my_nal_type = 5;
        break;
    case NAL_UNIT_CODED_SLICE_CRA:
        nal_unit_type_name = "Coded slice segment of a CRA picture";
        break;

    case NAL_UNIT_RESERVED_IRAP_VCL22:
    case NAL_UNIT_RESERVED_IRAP_VCL23:
        nal_unit_type_name = "Reserved IRAP VCL NAL unit types";
        break;
    case NAL_UNIT_RESERVED_VCL24:
    case NAL_UNIT_RESERVED_VCL25:
    case NAL_UNIT_RESERVED_VCL26:
    case NAL_UNIT_RESERVED_VCL27:
    case NAL_UNIT_RESERVED_VCL28:
    case NAL_UNIT_RESERVED_VCL29:
    case NAL_UNIT_RESERVED_VCL30:
    case NAL_UNIT_RESERVED_VCL31:
        nal_unit_type_name = "Reserved non-IRAP VCL NAL unit types";
        break;
    case NAL_UNIT_RESERVED_NVCL41:
    case NAL_UNIT_RESERVED_NVCL42:
    case NAL_UNIT_RESERVED_NVCL43:
    case NAL_UNIT_RESERVED_NVCL44:
    case NAL_UNIT_RESERVED_NVCL45:
    case NAL_UNIT_RESERVED_NVCL46:
    case NAL_UNIT_RESERVED_NVCL47:
        nal_unit_type_name = "Reserved";
        break;
    case NAL_UNIT_UNSPECIFIED_48:
    case NAL_UNIT_UNSPECIFIED_49:
    case NAL_UNIT_UNSPECIFIED_50:
    case NAL_UNIT_UNSPECIFIED_51:
    case NAL_UNIT_UNSPECIFIED_52:
    case NAL_UNIT_UNSPECIFIED_53:
    case NAL_UNIT_UNSPECIFIED_54:
    case NAL_UNIT_UNSPECIFIED_55:
    case NAL_UNIT_UNSPECIFIED_56:
    case NAL_UNIT_UNSPECIFIED_57:
    case NAL_UNIT_UNSPECIFIED_58:
    case NAL_UNIT_UNSPECIFIED_59:
    case NAL_UNIT_UNSPECIFIED_60:
    case NAL_UNIT_UNSPECIFIED_61:
    case NAL_UNIT_UNSPECIFIED_62:
    case NAL_UNIT_UNSPECIFIED_63:
        nal_unit_type_name = "Unspecified";
        break;
    default :
        nal_unit_type_name = "Unknown";
        break;
    }
    // nal header
    my_printf("==================== HEVC NAL ====================\r\n");
    my_printf(" forbidden_zero_bit : %d\r\n", nal->forbidden_zero_bit);
    my_printf(" nal_unit_type : %d ( %s )\r\n", nal->nal_unit_type, nal_unit_type_name);
    my_printf(" nal_ref_idc : %d\r\n", nal->nuh_layer_id);
    my_printf(" nal_ref_idc : %d\r\n", nal->nuh_temporal_id_plus1);
    
    // nal unit
    if(my_nal_type == 0)
        h265_debug_vps(h->vps);
    else if(my_nal_type == 1)
        h265_debug_sps(h->sps);
    else if(my_nal_type == 2)
        h265_debug_pps(h->pps);
    else if(my_nal_type == 3)
        h265_debug_aud(h->aud);
    else if(my_nal_type == 4)
        h265_debug_seis(h);
    else if(my_nal_type == 5)
        h265_debug_slice_header(h);
}