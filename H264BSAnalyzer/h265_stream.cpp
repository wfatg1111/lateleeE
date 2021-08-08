#include "stdafx.h" // for mfc

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bs.h"
#include "h264_stream.h" // for nal_to_rbsp...
#include "h265_stream.h"
#include "h265_sei.h"



FILE* h265_dbgfile = NULL;

#define printf(...) fprintf((h265_dbgfile == NULL ? stdout : h265_dbgfile), __VA_ARGS__)

/**
 Create a new H265 stream object.  Allocates all structures contained within it.
 @return    the stream object
 */
h265_stream_t* h265_new()
{
    h265_stream_t* h = (h265_stream_t*)calloc(1, sizeof(h265_stream_t));

    h->nal = (h265_nal_t*)calloc(1, sizeof(h265_nal_t));

    // initialize tables
    for ( int i = 0; i < 32; i++ ) { h->vps_table[i] = (h265_vps_t*)calloc(1, sizeof(h265_vps_t)); }
    for ( int i = 0; i < 32; i++ ) { h->sps_table[i] = (h265_sps_t*)calloc(1, sizeof(h265_sps_t)); }
    for ( int i = 0; i < 256; i++ ) { h->pps_table[i] = (h265_pps_t*)calloc(1, sizeof(h265_pps_t)); }

    h->vps = h->vps_table[0];
    h->sps = h->sps_table[0];
    h->pps = h->pps_table[0];
    h->aud = (h265_aud_t*)calloc(1, sizeof(h265_aud_t));
    h->num_seis = 0;
    h->seis = NULL;
    h->sei = NULL;  //This is a TEMP pointer at whats in h->seis...
    h->sh = (h265_slice_header_t*)calloc(1, sizeof(h265_slice_header_t));

    return h;   
}

/**
 Free an existing H265 stream object.  Frees all contained structures.
 @param[in,out] h   the stream object
 */
void h265_free(h265_stream_t* h)
{
    free(h->nal);
    for ( int i = 0; i < 32; i++ ) { free( h->vps_table[i] ); }
    for ( int i = 0; i < 32; i++ ) { free( h->sps_table[i] ); }
    for ( int i = 0; i < 256; i++ ) { free( h->pps_table[i] ); }

    free(h->aud);
    if(h->seis != NULL)
    {
        for( int i = 0; i < h->num_seis; i++ )
        {
            h265_sei_t* sei = h->seis[i];
            h265_sei_free(sei);
        }
        free(h->seis);
    }
    free(h->sh);
    free(h);
}

/**
 Read a NAL unit from a byte buffer.
 The buffer must start exactly at the beginning of the nal (after the start prefix).
 The NAL is read into h->nal and into other fields within h depending on its type (check h->nal->nal_unit_type after reading).
 @param[in,out] h          the stream object
 @param[in]     buf        the buffer
 @param[in]     size       the size of the buffer
 @return                   the length of data actually read, or -1 on error
 */
//7.3.1 NAL unit syntax
int h265_read_nal_unit(h265_stream_t* h, uint8_t* buf, int size)
{
    h265_nal_t* nal = h->nal;

    bs_t* b = bs_new(buf, size);

    // nal header
    nal->forbidden_zero_bit = bs_read_f(b,1);
    nal->nal_unit_type = bs_read_u(b,6);
    nal->nuh_layer_id = bs_read_u(b,6);
    nal->nuh_temporal_id_plus1 = bs_read_u(b,3);
    nal->parsed = NULL;
    nal->sizeof_parsed = 0;

    bs_free(b);

    int nal_size = size;
    int rbsp_size = size;
    uint8_t* rbsp_buf = (uint8_t*)malloc(rbsp_size);
 
    int rc = nal_to_rbsp(buf, &nal_size, rbsp_buf, &rbsp_size);

    if (rc < 0) { free(rbsp_buf); return -1; } // handle conversion error

    b = bs_new(rbsp_buf, rbsp_size);

    // nal data
    switch ( nal->nal_unit_type )
    {
        case NAL_UNIT_VPS:
            h265_read_vps_rbsp(h,b);
            break;
        case NAL_UNIT_SPS: 
            h265_read_sps_rbsp(h, b); 
            nal->parsed = h->sps;
            nal->sizeof_parsed = sizeof(h265_sps_t);
            break;
        case NAL_UNIT_PPS:   
            h265_read_pps_rbsp(h, b);
            nal->parsed = h->pps;
            nal->sizeof_parsed = sizeof(h265_pps_t);
            break;
        case NAL_UNIT_PREFIX_SEI:
            h265_read_sei_rbsp(h, b);
            nal->parsed = h->sei;
            nal->sizeof_parsed = sizeof(h265_slice_header_t);
            break;
        case NAL_UNIT_SUFFIX_SEI: // todo
            h265_read_sei_rbsp(h, b);
            nal->parsed = h->sei;
            nal->sizeof_parsed = sizeof(h265_slice_header_t);
            break;
        case NAL_UNIT_AUD:     
            h265_read_aud_rbsp(h, b); 
            nal->parsed = h->aud;
            nal->sizeof_parsed = sizeof(h265_aud_t);
            break;
        case NAL_UNIT_EOS: 
            h265_read_end_of_seq_rbsp(h, b);
            break;
        case NAL_UNIT_EOB:
            h265_read_end_of_stream_rbsp(h, b);
            break;
        case NAL_UNIT_CODED_SLICE_TRAIL_R:
        case NAL_UNIT_CODED_SLICE_TRAIL_N:
        case NAL_UNIT_CODED_SLICE_TSA_R:
        case NAL_UNIT_CODED_SLICE_TSA_N:
        case NAL_UNIT_CODED_SLICE_STSA_R:
        case NAL_UNIT_CODED_SLICE_STSA_N:
        case NAL_UNIT_CODED_SLICE_BLA_W_LP:
        case NAL_UNIT_CODED_SLICE_BLA_W_RADL:
        case NAL_UNIT_CODED_SLICE_BLA_N_LP:
        case NAL_UNIT_CODED_SLICE_IDR_W_RADL:
        case NAL_UNIT_CODED_SLICE_IDR_N_LP:
        case NAL_UNIT_CODED_SLICE_CRA:
        case NAL_UNIT_CODED_SLICE_RADL_N:
        case NAL_UNIT_CODED_SLICE_RADL_R:
        case NAL_UNIT_CODED_SLICE_RASL_N:
        case NAL_UNIT_CODED_SLICE_RASL_R:
            //h265_read_slice_layer_rbsp(h, b);
            nal->parsed = h->sh;
            nal->sizeof_parsed = sizeof(h265_slice_header_t);
            break;
        case NAL_UNIT_RESERVED_VCL_N10:
        case NAL_UNIT_RESERVED_VCL_R11:
        case NAL_UNIT_RESERVED_VCL_N12:
        case NAL_UNIT_RESERVED_VCL_R13:
        case NAL_UNIT_RESERVED_VCL_N14:
        case NAL_UNIT_RESERVED_VCL_R15:

        case NAL_UNIT_RESERVED_IRAP_VCL22:
        case NAL_UNIT_RESERVED_IRAP_VCL23:

        case NAL_UNIT_RESERVED_VCL24:
        case NAL_UNIT_RESERVED_VCL25:
        case NAL_UNIT_RESERVED_VCL26:
        case NAL_UNIT_RESERVED_VCL27:
        case NAL_UNIT_RESERVED_VCL28:
        case NAL_UNIT_RESERVED_VCL29:
        case NAL_UNIT_RESERVED_VCL30:
        case NAL_UNIT_RESERVED_VCL31:
            printf ("Note: found reserved VCL NAL unit.\n");
            nal->parsed = NULL;
            nal->sizeof_parsed = 0;
            break;
        case NAL_UNIT_RESERVED_NVCL41:
        case NAL_UNIT_RESERVED_NVCL42:
        case NAL_UNIT_RESERVED_NVCL43:
        case NAL_UNIT_RESERVED_NVCL44:
        case NAL_UNIT_RESERVED_NVCL45:
        case NAL_UNIT_RESERVED_NVCL46:
        case NAL_UNIT_RESERVED_NVCL47:
            printf ("Note: found reserved NAL unit.\n");
            nal->parsed = NULL;
            nal->sizeof_parsed = 0;
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
            printf ("Note: found unspecified NAL unit.\n");
            nal->parsed = NULL;
            nal->sizeof_parsed = 0;
            break;
        default:
            // here comes the reserved/unspecified/ignored stuff
            nal->parsed = NULL;
            nal->sizeof_parsed = 0;
            return 0;
    }

    if (bs_overrun(b)) { bs_free(b); free(rbsp_buf); return -1; }

    bs_free(b); 
    free(rbsp_buf);

    return nal_size;
}

// tbd:VPS是否只有一个？
void  h265_read_vps_rbsp(h265_stream_t* h, bs_t* b)
{
    int i = 0;
    int j = 0;
    // NOTE 不能直接赋值给vps，因为还未知是哪一个vps。

    int vps_video_parameter_set_id  = bs_read_u(b, 4);
    // 选择正确的sps表
    h->vps = h->vps_table[vps_video_parameter_set_id];
    h265_vps_t* vps = h->vps;
    memset(vps, 0, sizeof(h265_vps_t));

    vps->vps_video_parameter_set_id    = vps_video_parameter_set_id;
    vps->vps_base_layer_internal_flag  = bs_read_u1(b);
    vps->vps_base_layer_available_flag = bs_read_u1(b);
    vps->vps_max_layers_minus1         = bs_read_u(b, 6);
    vps->vps_max_sub_layers_minus1     = bs_read_u(b, 3);
    vps->vps_temporal_id_nesting_flag  = bs_read_u1(b);
    vps->vps_reserved_0xffff_16bits    = bs_read_u(b, 16);

    // profile tier level...
    // parse_ptl();

    vps->vps_sub_layer_ordering_info_present_flag = bs_read_u1(b);
    for (i = (vps->vps_sub_layer_ordering_info_present_flag ? 0 : vps->vps_max_sub_layers_minus1); 
         i <= vps->vps_max_sub_layers_minus1; i++ )
    {
        vps->vps_max_dec_pic_buffering_minus1[i] = bs_read_ue(b);
        vps->vps_max_num_reorder_pics[i]         = bs_read_ue(b);
        vps->vps_max_latency_increase_plus1[i]   = bs_read_ue(b);
    }
    vps->vps_max_layer_id           = bs_read_u(b, 6);
    vps->vps_num_layer_sets_minus1  = bs_read_ue(b);
    for (i = 1; i <= vps->vps_num_layer_sets_minus1; i++)
    {
        for (j = 0; j <= vps->vps_max_layer_id; j++)
        {
            vps->layer_id_included_flag[i][j] = bs_read_u1(b);
        }
    }
    vps->vps_timing_info_present_flag = bs_read_u1(b);
    if (vps->vps_timing_info_present_flag)
    {
        vps->vps_num_units_in_tick = bs_read_u(b, 32);
        vps->vps_time_scale = bs_read_u(b, 32);
        vps->vps_poc_proportional_to_timing_flag = bs_read_u1(b);
        if (vps->vps_poc_proportional_to_timing_flag)
        {
            vps->vps_num_ticks_poc_diff_one_minus1 = bs_read_ue(b);
        }
        vps->vps_num_hrd_parameters  = bs_read_u1(b);
        for (i = 0; i < vps->vps_num_hrd_parameters; i++)
        {
            vps->hrd_layer_set_idx[i] = bs_read_ue(b);
            if (i > 0)
            {
                vps->cprms_present_flag[i] = bs_read_u1(b);
            }
            // todo hrd_parameters(()
        }
    }
    vps->vps_extension_flag  = bs_read_u1(b);
    if (vps->vps_extension_flag)
    {
        while (h265_more_rbsp_trailing_data(b))
        {
            int sps_extension_data_flag = bs_read_u1(b);
        }
    }
    h265_read_rbsp_trailing_bits(b);
}
//7.3.2.1 Sequence parameter set RBSP syntax
void  h265_read_sps_rbsp(h265_stream_t* h, bs_t* b)
{
    int i;

    // NOTE 不能直接赋值给sps，因为还未知是哪一个sps。

    int sps_video_parameter_set_id = 0;
    int sps_max_sub_layers_minus1 = 0;
    int sps_temporal_id_nesting_flag = 0;
    int sps_seq_parameter_set_id = 0;

    sps_video_parameter_set_id = bs_read_u(b, 4);
    sps_max_sub_layers_minus1 = bs_read_u(b, 3);
    sps_temporal_id_nesting_flag = bs_read_u1(b);

    // profile tier level...
    // parse_ptl();

    sps_seq_parameter_set_id = bs_read_ue(b);
    // 选择正确的sps表
    h->sps = h->sps_table[sps_seq_parameter_set_id];
    h265_sps_t* sps = h->sps;
    memset(sps, 0, sizeof(h265_sps_t));

    sps->chroma_format_idc  = bs_read_ue(b);
    if (sps->chroma_format_idc == 3)
    {
        sps->separate_colour_plane_flag = bs_read_u1(b);
    }
    sps->sps_video_parameter_set_id   = sps_video_parameter_set_id;
    sps->sps_max_sub_layers_minus1    = sps_max_sub_layers_minus1;
    sps->sps_temporal_id_nesting_flag = sps_temporal_id_nesting_flag;
    sps->sps_seq_parameter_set_id     = sps_seq_parameter_set_id;

    sps->pic_width_in_luma_samples  = bs_read_ue(b);
    sps->pic_height_in_luma_samples = bs_read_ue(b);
    sps->conformance_window_flag    = bs_read_u1(b);
    if (sps->conformance_window_flag)
    {
        sps->conf_win_left_offset   = bs_read_ue(b);
        sps->conf_win_right_offset  = bs_read_ue(b);
        sps->conf_win_top_offset    = bs_read_ue(b);
        sps->conf_win_bottom_offset = bs_read_ue(b);
    }
    sps->bit_depth_luma_minus8   = bs_read_ue(b);
    sps->bit_depth_chroma_minus8 = bs_read_ue(b);
    sps->log2_max_pic_order_cnt_lsb_minus4 = bs_read_ue(b);

    sps->sps_sub_layer_ordering_info_present_flag = bs_read_u1(b);
    for (i = (sps->sps_sub_layer_ordering_info_present_flag ? 0 : sps->sps_max_sub_layers_minus1); 
        i <= sps->sps_max_sub_layers_minus1; i++ )
    {
        sps->sps_max_dec_pic_buffering_minus1[i] = bs_read_ue(b);
        sps->sps_max_num_reorder_pics[i]         = bs_read_ue(b);
        sps->sps_max_latency_increase_plus1[i]   = bs_read_ue(b);
    }

    sps->log2_min_luma_coding_block_size_minus3      = bs_read_ue(b);
    sps->log2_diff_max_min_luma_coding_block_size    = bs_read_ue(b);
    sps->log2_min_luma_transform_block_size_minus2   = bs_read_ue(b);
    sps->log2_diff_max_min_luma_transform_block_size = bs_read_ue(b);
    sps->max_transform_hierarchy_depth_inter         = bs_read_ue(b);
    sps->max_transform_hierarchy_depth_intra         = bs_read_ue(b);

    sps->scaling_list_enabled_flag = bs_read_u1(b);
    if (sps->scaling_list_enabled_flag)
    {
        sps->sps_scaling_list_data_present_flag = bs_read_u1(b);
        if (sps->sps_scaling_list_data_present_flag)
        {
            //parse_scaling_list();
        }
    }

    sps->amp_enabled_flag = bs_read_u1(b);
    sps->sample_adaptive_offset_enabled_flag = bs_read_u1(b);
    sps->pcm_enabled_flag = bs_read_u1(b);
    if (sps->pcm_enabled_flag)
    {
        sps->pcm_sample_bit_depth_luma_minus1   = bs_read_u(b, 4);
        sps->pcm_sample_bit_depth_chroma_minus1 = bs_read_u(b, 4);
        sps->log2_min_pcm_luma_coding_block_size_minus3   = bs_read_ue(b);
        sps->log2_diff_max_min_pcm_luma_coding_block_size = bs_read_ue(b);
        sps->pcm_loop_filter_disabled_flag      = bs_read_u1(b);
    }

    sps->num_short_term_ref_pic_sets = bs_read_ue(b);
    for (i = 0; i < sps->num_short_term_ref_pic_sets; i++)
    {
        // todo
    }

    sps->long_term_ref_pics_present_flag = bs_read_u1(b);
    if (sps->long_term_ref_pics_present_flag)
    {
        sps->num_long_term_ref_pics_sps = bs_read_ue(b);
        for (i = 0; i < sps->num_long_term_ref_pics_sps; i++)
        {
            sps->lt_ref_pic_poc_lsb_sps[i] = bs_read_u1(b); // todo u(v)
            sps->used_by_curr_pic_lt_sps_flag[i] = bs_read_u1(b);
        }
    }

    sps->sps_temporal_mvp_enabled_flag = bs_read_u1(b);
    sps->strong_intra_smoothing_enabled_flag = bs_read_u1(b);
    sps->vui_parameters_present_flag = bs_read_u1(b);
    if (sps->vui_parameters_present_flag)
    {
        // todo parse_vui_parameters();
    }

    sps->sps_extension_present_flag = bs_read_u1(b);
    if (sps->sps_extension_present_flag)
    {
        sps->sps_range_extension_flag      = bs_read_u1(b);
        sps->sps_multilayer_extension_flag = bs_read_u1(b);
        sps->sps_3d_extension_flag         = bs_read_u1(b);
        sps->sps_extension_5bits           = bs_read_u(b, 5);
    }

    if (sps->sps_range_extension_flag)
    {
        sps->sps_range_extension.transform_skip_rotation_enabled_flag    = bs_read_u1(b);
        sps->sps_range_extension.transform_skip_context_enabled_flag     = bs_read_u1(b);
        sps->sps_range_extension.implicit_rdpcm_enabled_flag             = bs_read_u1(b);
        sps->sps_range_extension.explicit_rdpcm_enabled_flag             = bs_read_u1(b);
        sps->sps_range_extension.extended_precision_processing_flag      = bs_read_u1(b);
        sps->sps_range_extension.intra_smoothing_disabled_flag           = bs_read_u1(b);
        sps->sps_range_extension.high_precision_offsets_enabled_flag     = bs_read_u1(b);
        sps->sps_range_extension.persistent_rice_adaptation_enabled_flag = bs_read_u1(b);
        sps->sps_range_extension.cabac_bypass_alignment_enabled_flag     = bs_read_u1(b);
    }
    if (sps->sps_multilayer_extension_flag)
    {
        // todo sps_multilayer_extension( ) 
    }
    if (sps->sps_3d_extension_flag)
    {
        // todo sps_3d_extension( ) 
    }
    if (sps->sps_extension_5bits)
    {
        while (h265_more_rbsp_trailing_data(b))
        {
            int sps_extension_data_flag = bs_read_u1(b);
        }
    }
    h265_read_rbsp_trailing_bits(b);
}


//7.3.2.2 Picture parameter set RBSP syntax
void h265_read_pps_rbsp(h265_stream_t* h, bs_t* b)
{
    int pps_pic_parameter_set_id = bs_read_ue(b); // get id

    h265_pps_t* pps = h->pps = h->pps_table[pps_pic_parameter_set_id] ;

    memset(pps, 0, sizeof(h265_pps_t));

    int i;
    int i_group;

    pps->pps_pic_parameter_set_id      = pps_pic_parameter_set_id;
    pps->pps_seq_parameter_set_id      = bs_read_ue(b);
    pps->dependent_slice_segments_enabled_flag  = bs_read_u1(b);
    pps->output_flag_present_flag      = bs_read_u1(b);
    pps->num_extra_slice_header_bits   = bs_read_u(b, 3);
    pps->sign_data_hiding_enabled_flag = bs_read_ue(b);
    pps->cabac_init_present_flag       = bs_read_ue(b);
    pps->num_ref_idx_l0_default_active_minus1   = bs_read_ue(b);
    pps->num_ref_idx_l1_default_active_minus1   = bs_read_ue(b);
    pps->init_qp_minus26               = bs_read_se(b);
    pps->constrained_intra_pred_flag   = bs_read_u1(b);
    pps->transform_skip_enabled_flag   = bs_read_u1(b);
    if (pps->transform_skip_enabled_flag)
    {
        pps->cu_qp_delta_enabled_flag   = bs_read_ue(b);
    }
    
    pps->pps_cb_qp_offset  = bs_read_se(b);
    pps->pps_cr_qp_offset  = bs_read_se(b);
    pps->pps_slice_chroma_qp_offsets_present_flag = bs_read_u1(b);
    pps->transquant_bypass_enabled_flag           = bs_read_u1(b);
    pps->tiles_enabled_flag = bs_read_u1(b);
    pps->entropy_coding_sync_enabled_flag         = bs_read_u1(b);
    if (pps->tiles_enabled_flag)
    {
        pps->num_tile_columns_minus1 = bs_read_ue(b);
        pps->num_tile_rows_minus1    = bs_read_ue(b);
        pps->uniform_spacing_flag    = bs_read_u1(b);
        if (!pps->uniform_spacing_flag)
        {
            for (i = 0; i < pps->num_tile_columns_minus1; i++)
            {
                pps->column_width_minus1[i] = bs_read_ue(b);
            }
            for (i = 0; i < pps->num_tile_rows_minus1; i++)
            {
                pps->row_height_minus1[i]   = bs_read_ue(b);
            }
        }
        pps->loop_filter_across_tiles_enabled_flag  = bs_read_u1(b);
    }

    pps->pps_loop_filter_across_slices_enabled_flag = bs_read_u1(b);
    pps->deblocking_filter_control_present_flag     = bs_read_u1(b);
    if (pps->deblocking_filter_control_present_flag)
    {
        pps->deblocking_filter_override_enabled_flag = bs_read_u1(b);
        pps->pps_deblocking_filter_disabled_flag     = bs_read_u1(b);
        if (pps->pps_deblocking_filter_disabled_flag)
        {
            pps->pps_beta_offset_div2 = bs_read_se(b);
            pps->pps_tc_offset_div2   = bs_read_se(b);
        }
    }

    pps->pps_scaling_list_data_present_flag = bs_read_u1(b);
    if (pps->pps_scaling_list_data_present_flag)
    {
        // todo...scaling_list_data()
    }

    pps->lists_modification_present_flag  = bs_read_u1(b);
    pps->log2_parallel_merge_level_minus2 = bs_read_ue(b);
    pps->slice_segment_header_extension_present_flag = bs_read_u1(b);
    pps->pps_extension_present_flag       = bs_read_u1(b);
    if (pps->pps_extension_present_flag)
    {
        pps->pps_range_extension_flag      = bs_read_u1(b);
        pps->pps_multilayer_extension_flag = bs_read_u1(b);
        pps->pps_3d_extension_flag         = bs_read_u1(b);
        pps->pps_extension_5bits           = bs_read_u(b, 5);
    }
    if (pps->pps_range_extension_flag)
    {
        if (pps->transform_skip_enabled_flag)
        {
            pps->pps_range_extension.log2_max_transform_skip_block_size_minus2 = bs_read_ue(b);
        }
        pps->pps_range_extension.cross_component_prediction_enabled_flag = bs_read_u1(b);
        pps->pps_range_extension.chroma_qp_offset_list_enabled_flag      = bs_read_u1(b);
        if (pps->pps_range_extension.chroma_qp_offset_list_enabled_flag)
        {
            pps->pps_range_extension.diff_cu_chroma_qp_offset_depth   = bs_read_ue(b);
            pps->pps_range_extension.chroma_qp_offset_list_len_minus1 = bs_read_ue(b);
            for (i = 0; i < pps->pps_range_extension.chroma_qp_offset_list_len_minus1; i++)
            {
                 pps->pps_range_extension.cb_qp_offset_list[i] = bs_read_se(b);
                 pps->pps_range_extension.cr_qp_offset_list[i] = bs_read_se(b);
            }
        }
        pps->pps_range_extension.log2_sao_offset_scale_luma    = bs_read_ue(b);
        pps->pps_range_extension.log2_sao_offset_scale_chroma  = bs_read_ue(b);
    }
    if (pps->pps_multilayer_extension_flag)
    {
        // todo sps_multilayer_extension( ) 
    }
    if (pps->pps_3d_extension_flag)
    {
        // todo sps_3d_extension( ) 
    }
    if (pps->pps_extension_5bits)
    {
        while (h265_more_rbsp_trailing_data(b))
        {
            int pps_extension_data_flag = bs_read_u1(b);
        }
    }
    h265_read_rbsp_trailing_bits(b);
}

//7.3.2.5 End of sequence RBSP syntax
void h265_read_end_of_seq_rbsp(h265_stream_t* h, bs_t* b)
{
}

//7.3.2.6 End of stream RBSP syntax
void h265_read_end_of_stream_rbsp(h265_stream_t* h, bs_t* b)
{
}

//7.3.2.4 Access unit delimiter RBSP syntax
void h265_read_aud_rbsp(h265_stream_t* h, bs_t* b)
{
    h->aud->pic_type = bs_read_u(b,3);
     h265_read_rbsp_trailing_bits(b);
}

//7.3.2.3 Supplemental enhancement information RBSP syntax
void h265_read_sei_rbsp(h265_stream_t* h, bs_t* b)
{
    h265_read_rbsp_trailing_bits(b);
}

void h265_read_scaling_list( bs_t* b, int* scalingList )
{

}

void h265_read_vui_parameters( h265_stream_t* h, bs_t* b )
{

}

void h265_read_hrd_parameters( h265_stream_t* h, bs_t* b )
{

}

int h265_more_rbsp_trailing_data(bs_t* b) { return !bs_eof(b); }

//7.3.2.10 RBSP slice trailing bits syntax
// 与h.264略有不同
void h265_read_rbsp_slice_trailing_bits(bs_t* b)
{
    h265_read_rbsp_trailing_bits(b);
    int cabac_zero_word;
    while( h265_more_rbsp_trailing_data(b) )
    {
        cabac_zero_word = bs_read_f(b,16); // equal to 0x0000
    }
}

//7.3.2.11 RBSP trailing bits syntax
void h265_read_rbsp_trailing_bits(bs_t* b)
{
    int rbsp_stop_one_bit = bs_read_u1( b ); // equal to 1

    while( !bs_byte_aligned(b) )
    {
        int rbsp_alignment_zero_bit = bs_read_u1( b ); // equal to 0
    }
}
