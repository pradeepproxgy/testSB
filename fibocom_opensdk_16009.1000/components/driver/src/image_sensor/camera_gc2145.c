/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */
#include "drv_config.h"
//#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_INFO
#ifdef CONFIG_CAMERA_GC2145_SUPPORT
#include "osi_log.h"
#include "osi_api.h"
#include "hwregs.h"
#include <stdlib.h>
#include "hal_chip.h"
#include "drv_i2c.h"
#include "image_sensor.h"
#include "drv_camera.h"
#include "drv_names.h"
#include "osi_clock.h"

static osiClockConstrainRegistry_t gcCamCLK = {.tag = HAL_NAME_CAM};

static const cameraReg_t RG_InitVgaMIPI[] =
    {

        /////////////p sub
        //////// gain 12x
        /////////a gain 8x
        /////////d gain 1.5x
        {0xfe, 0xf0},
        {0xfe, 0xf0},
        {0xfe, 0xf0},
        {0xfc, 0x06},
        {0xf6, 0x00},
        {0xf7, 0x1f}, //37 //17 //37 //1d//05
        {0xf8, 0x83}, //87 //83 //82
        {0xfa, 0x11},
        {0xf9, 0x8e}, //ff
        {0xf2, 0x00},
        //////////////////////////////////////////////////////
        ////////////////////  Analog & Cisctl ////////////////
        //////////////////////////////////////////////////////
        {0xfe, 0x00},
        {0x03, 0x04}, //exp time
        {0x04, 0x00}, //exp time
        {0x05, 0x01}, //00 //hb[11:8]
        {0x06, 0x68}, //0b //hb
        {0x09, 0x00}, //row start
        {0x0a, 0x00}, //
        {0x0b, 0x00}, //col start
        {0x0c, 0x00},
        {0x0d, 0x04}, //height
        {0x0e, 0xc0},
        {0x0f, 0x06}, //width
        {0x10, 0x52},
        {0x12, 0x2e}, //sh_delay ��??�� YUV3?��?����3��
        {0x17, 0x14}, //CISCTL Mode1 [1:0]mirror flip
        {0x18, 0x22}, //sdark mode
        {0x19, 0x0f}, // AD pipe number
        {0x1a, 0x01}, //AD manual switch mode
        {0x1b, 0x4b}, //48 restg Width,SH width
        {0x1c, 0x07}, //06	???��?��o����?o����??? //12 //TX Width,Space Width
        {0x1d, 0x10}, //double reset
        {0x1e, 0x88}, //90//98 //fix  ����??//Analog Mode1,TX high,Coln_r
        {0x1f, 0x78}, //78 //38 //18 //Analog Mode2,txlow
        {0x20, 0x03}, //07 //Analog Mode3,comv,ad_clk mode
        {0x21, 0x40}, //10//20//40 //fix ��?1��o����???
        {0x22, 0xf0}, //d0//f0 //a2 //Vref vpix	FPN????
        {0x24, 0x16}, //Pad drv
        {0x25, 0x01}, //col sel
        {0x26, 0x10}, //Analog PGA gain1
        {0x2d, 0x60}, //40//40 //txl drv mode
        {0x30, 0x01}, //Analog Mode4
        {0x31, 0x90}, //b0//70 // Analog Mode7 [7:5]rsgh_r��?1��o����???[4:3]isp_g
        {0x33, 0x06}, //03//02//01 //EQ_hstart_width
        {0x34, 0x01},
        ///////////////////////////////////////////////////
        ////////////////////  ISP reg  //////////////////////
        //////////////////////////////////////////////////////
        {0x80, 0xff}, //outdoor gamma_en, GAMMA_en, CC_en, EE_en, INTP_en, DN_en, DD_en,LSC_en
        {0x81, 0x24}, //26//24 //BLK dither mode, ll_y_en ,skin_en, edge SA, new_skin_mode, autogray_en,ll_gamma_en,BFF test image
        {0x82, 0xfa}, //FA //auto_SA, auto_EE, auto_DN, auto_DD, auto_LSC, ABS_en, AWB_en, NA
        {0x83, 0x00}, //special_effect
        {0x84, 0x01}, //output format
        {0x86, 0x06}, //c2 //46 //c2 //sync mode
        {0x88, 0x03}, //[1]ctl_auto_gating [0]out_auto_gating
        {0x89, 0x03}, //bypass disable
        {0x85, 0x30}, //60//frame start cut
        {0x8a, 0x00}, //ISP_quiet_mode,close aaa pclk,BLK gate mode,exception,close first pipe clock,close dndd clock,close intp clock,DIV_gatedclk_en
        {0x8b, 0x00}, //[7:6]BFF_gate_mode,[5]BLK switch gain,[4]protect exp,[3:2]pipe gate mode,[1]not split sram,[0]dark current update
        {0xb0, 0x55}, //60 //global gain
        {0xc3, 0x00}, //[7:4]auto_exp_gamma_th1[11:8],[3:0]auto_exp_gamma_th2[11:8]
        {0xc4, 0x80}, //auto_exp_gamma_th1[7:0] into
        {0xc5, 0x90}, //auto_exp_gamma_th2[7:0] out //outdoor gamma
        {0xc6, 0x38}, //auto_gamma_th1
        {0xc7, 0x40}, //auto_gamma_th2
        {0xec, 0x06}, //measure window
        {0xed, 0x04},
        {0xee, 0x60}, //16	col
        {0xef, 0x90}, //8  row
        {0xb6, 0x01}, //[0]aec en
        {0x90, 0x01}, //crop
        {0x91, 0x00},
        {0x92, 0x00},
        {0x93, 0x00},
        {0x94, 0x00}, //08
        {0x95, 0x04},
        {0x96, 0xb0},
        {0x97, 0x06},
        {0x98, 0x40},
        ///////////////////////////////////////////////
        ///////////	BLK ////////////////////////
        ///////////////////////////////////////////////
        {0x18, 0x02},
        {0x40, 0x42},
        {0x41, 0x00}, //80 //dark row sel
        {0x43, 0x54}, //[7:4]BLK start not smooth  [3:0]output start frame
        {0x5e, 0x00}, //00//10 //18
        {0x5f, 0x00}, //00//10 //18
        {0x60, 0x00}, //00//10 //18
        {0x61, 0x00}, //00///10 //18
        {0x62, 0x00}, //00//10 //18
        {0x63, 0x00}, //00//10 //18
        {0x64, 0x00}, //00/10 //18
        {0x65, 0x00}, //00//10 //18
        {0x66, 0x20},
        {0x67, 0x20},
        {0x68, 0x20},
        {0x69, 0x20},
        {0x76, 0x00}, //0f
        {0x6a, 0x00}, //06
        {0x6b, 0x00}, //06
        {0x6c, 0x00}, //06
        {0x6d, 0x00}, //06
        {0x6e, 0x00}, //06
        {0x6f, 0x00}, //06
        {0x70, 0x00}, //06
        {0x71, 0x00}, //06 //manual offset
        {0x76, 0x00},
        {0x72, 0xf0}, //[7:4]BLK DD th [3:0]BLK various th
        {0x7e, 0x3c}, //ndark
        {0x7f, 0x00},
        {0xfe, 0x02},
        {0x48, 0x15},
        {0x49, 0x00}, //04//04 //ASDE OFFSET SLOPE
        {0x4b, 0x0b}, //ASDE y OFFSET SLOPE
        {0xfe, 0x00},
        ///////////////////////////////////////////////
        /////////// AEC ////////////////////////
        ///////////////////////////////////////////////
        {0xfe, 0x01},
        {0x01, 0x04}, //AEC X1
        {0x02, 0xc0}, //AEC X2
        {0x03, 0x04}, //AEC Y1
        {0x04, 0x90}, //AEC Y2
        {0x05, 0x30}, //20 //AEC center X1
        {0x06, 0x90}, //40 //AEC center X2
        {0x07, 0x20}, //30 //AEC center Y1
        {0x08, 0x70}, //60 //AEC center Y2
        {0x09, 0x00}, //AEC show mode
        {0x0a, 0xc2}, //[7]col gain enable
        {0x0b, 0x11}, //AEC every N
        {0x0c, 0x10}, //AEC_mode3 center weight
        {0x13, 0x30},
        {0x17, 0x00}, //AEC ignore mode
        {0x1c, 0x11}, //
        {0x1e, 0x61}, //
        {0x1f, 0x30}, //40//50 //max pre gain
        {0x20, 0x40}, //60//40 //max post gain
        {0x22, 0x80}, //AEC outdoor THD
        {0x23, 0x20}, //target_Y_low_limit
        {0xfe, 0x02},
        {0x0f, 0x04}, //05
        {0xfe, 0x01},
        {0x12, 0x00}, //35 //[5:4]group_size [3]slope_disable [2]outdoor_enable [0]histogram_enable
        {0x15, 0x50}, //target_Y_high_limit
        {0x10, 0x31}, //num_thd_high
        {0x3e, 0x28}, //num_thd_low
        {0x3f, 0xe0}, //luma_thd
        {0x40, 0xe0}, //luma_slope
        {0x41, 0x0f}, //color_diff
        /////////////////////////////
        //////// INTPEE /////////////
        /////////////////////////////
        {0xfe, 0x02}, //page2
        {0x90, 0x6c}, //ac //eeintp mode1
        {0x91, 0x03}, //02 ////eeintp mode2
        {0x92, 0xc8}, //44 //low criteria for direction
        {0x94, 0x66},
        {0x95, 0xb5},
        {0x97, 0x64}, //78 ////edge effect
        {0xa2, 0x11}, //fix direction
        {0xfe, 0x00},
        /////////////////////////////
        //////// DNDD///////////////
        /////////////////////////////
        {0xfe, 0x02},
        {0x80, 0xc1}, //c1 //[7]share mode [6]skin mode  [5]is 5x5 mode [1:0]noise value select 0:2  1:2.5	2:3  3:4
        {0x81, 0x08}, //
        {0x82, 0x08}, //signal a 0.6
        {0x83, 0x05}, //04 //signal b 2.5
        {0x84, 0x0a}, //10 //05 dark_DD_TH
        {0x86, 0x50}, //a0 Y_value_dd_th2
        {0x87, 0x30}, //90 Y_value_dd_th3
        {0x88, 0x15}, //60 Y_value_dd_th4
        {0x89, 0x80}, //80	// asde th2
        {0x8a, 0x60}, //60	// asde th3
        {0x8b, 0x30}, //30	// asde th4
        /////////////////////////////////////////////////
        ///////////// ASDE ////////////////////////
        /////////////////////////////////////////////////
        {0xfe, 0x01}, //page 1
        {0x21, 0x14}, //luma_value_div_sel(��??�̡�?��?0xef3��2��?1??�̡�???�䨮1��?0xef��??��??D?1��?)
        //ff  ef  luma_value read_only
        {0xfe, 0x02}, //page2
        {0xa3, 0x40}, //ASDE_low_luma_value_LSC_th_H
        {0xa4, 0x20}, //ASDE_low_luma_value_LSC_th_L
        {0xa5, 0x40}, //80 //ASDE_LSC_gain_dec_slope_H
        {0xa6, 0x80}, // 80 //ASDE_LSC_gain_dec_slope_L
        //ff  a7  ASDE_LSC_gain_dec	//read only
        {0xab, 0x40}, //50 //ASDE_low_luma_value_OT_th
        {0xae, 0x0c}, //[3]EE1_effect_inc_or_dec_high,[2]EE2_effect_inc_or_dec_high,
                      //[1]EE1_effect_inc_or_dec_low,[0]EE2_effect_inc_or_dec_low,	1:inc  0:dec
        {0xb3, 0x44}, //ASDE_EE1_effect_slope_low,ASDE_EE2_effect_slope_low
        {0xb4, 0x44}, //12 //ASDE_EE1_effect_slope_high,ASDE_EE2_effect_slope_high
        {0xb6, 0x40}, //40 //ASDE_auto_saturation_dec_slope
        {0xb7, 0x02}, //04 //ASDE_sub_saturation_slope
        {0xb9, 0x32}, //[7:0]ASDE_auto_saturation_low_limit
        {0x3c, 0x08}, //[3:0]auto gray_dec_slope
        {0x3d, 0x30}, //[7:0]auto gray_dec_th
        {0x4b, 0x0d}, //y offset slope
        {0x4c, 0x20}, //y offset limit
        {0xfe, 0x00},
        ///////////////////gamma1////////////////////
        ////Gamma
        {0xfe, 0x02},
        {0x10, 0x0a},
        {0x11, 0x12},
        {0x12, 0x19},
        {0x13, 0x1f},
        {0x14, 0x2c},
        {0x15, 0x38},
        {0x16, 0x42},
        {0x17, 0x4e},
        {0x18, 0x63},
        {0x19, 0x76},
        {0x1a, 0x87},
        {0x1b, 0x96},
        {0x1c, 0xa2},
        {0x1d, 0xb8},
        {0x1e, 0xcb},
        {0x1f, 0xd8},
        {0x20, 0xe2},
        {0x21, 0xe9},
        {0x22, 0xf0},
        {0x23, 0xf8},
        {0x24, 0xfd},
        {0x25, 0xff},
        {0xfe, 0x00},
        {0xc6, 0x0d}, //20//40//auto gammath1
        {0xc7, 0x14},
        {0xfe, 0x02},
        {0x26, 0x0f},
        {0x27, 0x14},
        {0x28, 0x19},
        {0x29, 0x1e},
        {0x2a, 0x27},
        {0x2b, 0x33},
        {0x2c, 0x3b},
        {0x2d, 0x45},
        {0x2e, 0x59},
        {0x2f, 0x69},
        {0x30, 0x7c},
        {0x31, 0x89},
        {0x32, 0x98},
        {0x33, 0xae},
        {0x34, 0xc0},
        {0x35, 0xcf},
        {0x36, 0xda},
        {0x37, 0xe2},
        {0x38, 0xe9},
        {0x39, 0xf3},
        {0x3a, 0xf9},
        {0x3b, 0xff},
        ///////////////////////////////////////////////
        ///////////	 YCP	   ///////////////////////
        ///////////////////////////////////////////////
        {0xfe, 0x02},
        {0xd1, 0x30}, //32 //
        {0xd2, 0x30}, //32 //
        {0xd3, 0x45},
        {0xdd, 0x14}, //edge sa
        {0xde, 0x86}, //asde auto gray
        {0xed, 0x01}, //
        {0xee, 0x28},
        {0xef, 0x30},
        {0xd8, 0xd8}, //autogray protecy
        ////////////////////////////
        //////// LSC  0.8///////////////
        ////////////////////////////
        {0xfe, 0x01},
        {0xc2, 0x18},
        {0xc3, 0x06},
        {0xc4, 0x06},
        {0xc8, 0x10},
        {0xc9, 0x06},
        {0xca, 0x02},
        {0xbc, 0x43},
        {0xbd, 0x2e},
        {0xbe, 0x2b},
        {0xb6, 0x39},
        {0xb7, 0x2e},
        {0xb8, 0x28},
        {0xc5, 0x0b},
        {0xc6, 0x29},
        {0xc7, 0x1d},
        {0xcb, 0x0b},
        {0xcc, 0x15},
        {0xcd, 0x16},
        {0xbf, 0x00},
        {0xc0, 0x00},
        {0xc1, 0x00},
        {0xb9, 0x00},
        {0xba, 0x00},
        {0xbb, 0x00},
        {0xaa, 0x00},
        {0xab, 0x0d},
        {0xac, 0x0d},
        {0xad, 0x00},
        {0xae, 0x01},
        {0xaf, 0x07},
        {0xb0, 0x0c},
        {0xb1, 0x12},
        {0xb2, 0x12},
        {0xb3, 0x18},
        {0xb4, 0x1e},
        {0xb5, 0x19},
        {0xd0, 0x00},
        {0xd1, 0x07},
        {0xd2, 0x09},
        {0xd6, 0x1a},
        {0xd7, 0x00},
        {0xd8, 0x06},
        {0xd9, 0x00},
        {0xda, 0x00},
        {0xdb, 0x00},
        {0xd3, 0x1f},
        {0xd4, 0x21},
        {0xd5, 0x11},
        {0xa4, 0x00},
        {0xa5, 0x00},
        {0xa6, 0x56},
        {0xa7, 0x00},
        {0xa8, 0x77},
        {0xa9, 0x77},
        {0xa1, 0x80},
        {0xa2, 0x80},
        {0xfe, 0x01},
        {0xdf, 0x00}, //0c
        {0xdc, 0x80},
        {0xdd, 0x30},
        {0xe0, 0x6b}, //70
        {0xe1, 0x70}, //80
        {0xe2, 0x6b}, //80
        {0xe3, 0x70}, //80
        {0xe6, 0xa0}, //90
        {0xe7, 0x60}, //50
        {0xe8, 0xa0}, //90/
        {0xe9, 0x60}, //60
        {0xfe, 0x00},
        /////////////////////////////////////////////////
        /////////////	AWB 	////////////////////////
        /////////////////////////////////////////////////
        {0xfe, 0x01},
        {0x4f, 0x00},
        {0x4f, 0x00},
        {0x4b, 0x01},
        {0x4f, 0x00},
        {0x4c, 0x01}, // D75
        {0x4d, 0x71},
        {0x4e, 0x01},
        {0x4c, 0x01},
        {0x4d, 0x91},
        {0x4e, 0x01},
        {0x4c, 0x01},
        {0x4d, 0x70},
        {0x4e, 0x01},
        {0x4c, 0x01}, // D65
        {0x4d, 0x90},
        {0x4e, 0x02},
        {0x4c, 0x01},
        {0x4d, 0x8f},
        {0x4e, 0x02},
        {0x4c, 0x01},
        {0x4d, 0xb0},
        {0x4e, 0x02},
        {0x4c, 0x01},
        {0x4d, 0xaf},
        {0x4e, 0x02},
        {0x4c, 0x01},
        {0x4d, 0x6f},
        {0x4e, 0x02},
        {0x4c, 0x01}, // D50
        {0x4d, 0xad},
        {0x4e, 0x33}, //03
        {0x4c, 0x01},
        {0x4d, 0xae},
        {0x4e, 0x33}, //03
        {0x4c, 0x01},
        {0x4d, 0x8c},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0xac},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0xcd},
        {0x4e, 0x03}, //03
        {0x4c, 0x01},
        {0x4d, 0x8e},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0x8d},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0x8b},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0x6a},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0x6b},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0x6c},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0x6d},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0x6e},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0xab},
        {0x4e, 0x03},
        {0x4c, 0x01},
        {0x4d, 0xcb},
        {0x4e, 0x03},
        {0x4c, 0x01}, // CWF
        {0x4d, 0xaa},
        {0x4e, 0x04},
        {0x4c, 0x01}, // CWF
        {0x4d, 0xa9},
        {0x4e, 0x04},
        {0x4c, 0x01},
        {0x4d, 0xca},
        {0x4e, 0x04},
        {0x4c, 0x01},
        {0x4d, 0xc9},
        {0x4e, 0x04},
        {0x4c, 0x01},
        {0x4d, 0x8a},
        {0x4e, 0x04},
        {0x4c, 0x01},
        {0x4d, 0x89},
        {0x4e, 0x04},
        {0x4c, 0x02},
        {0x4d, 0x0b},
        {0x4e, 0x05},
        {0x4c, 0x02},
        {0x4d, 0x0a},
        {0x4e, 0x05},
        {0x4c, 0x02},
        {0x4d, 0x2a},
        {0x4e, 0x05},
        {0x4c, 0x02}, // A
        {0x4d, 0x6a},
        {0x4e, 0x06},
        {0x4c, 0x02},
        {0x4d, 0x69},
        {0x4e, 0x06},
        {0x4c, 0x02},
        {0x4d, 0x29},
        {0x4e, 0x06},
        {0x4c, 0x02},
        {0x4d, 0x09},
        {0x4e, 0x06},
        {0x4c, 0x02},
        {0x4d, 0x49},
        {0x4e, 0x06},
        {0x4c, 0x02}, // H
        {0x4d, 0xc9},
        {0x4e, 0x07},
        {0x4c, 0x02},
        {0x4d, 0xc8},
        {0x4e, 0x07},
        {0x4c, 0x02},
        {0x4d, 0x68},
        {0x4e, 0x07},
        {0x4c, 0x02},
        {0x4d, 0xa8},
        {0x4e, 0x07},
        {0x4c, 0x02},
        {0x4d, 0x88},
        {0x4e, 0x07},
        {0x4f, 0x01},
        {0x50, 0x80}, //AWB_PRE_mode
        {0x51, 0xa8}, //AWB_pre_THD_min[7:0]
        {0x52, 0x57}, //AWB_pre_THD_min[15:8] Dominiate luma 0.25=639c 0.22=57a8
        {0x53, 0x38}, //AWB_pre_THD_min_MIX[7:0]
        {0x54, 0xc7}, //AWB_pre_THD_min_MIX[15:8] Mix luma 0.5
        {0x56, 0x0e}, //AWB_tone mode
        {0x58, 0x09}, //08 //AWB_C_num_sel,AWB_D_num_sel
        {0x5b, 0x00}, //AWB_mix_mode
        {0x5c, 0x74}, //green_num0[7:0]
        {0x5d, 0x8b}, //green_num0[15:8] 0.35
        {0x61, 0xa7}, //R2G_stand0
        {0x62, 0xb5}, //B2G_stand0
        {0x63, 0xaa}, //00//88//a4 //AWB gray mode [7]enable
        {0x64, 0x80},
        {0x65, 0x04}, //AWB margin
        {0x67, 0xa4}, //R2G_stand3[7:0]  FF/CWF
        {0x68, 0xb0}, //B2G_stand3[7:0]
        {0x69, 0x00}, //R2G_stand4[9:8] B2G_stand4[9:8] R2G_stand3[9:8] B2G_stand3[9:8]
        {0x6a, 0xa4}, //R2G_stand4[7:0]  TL84/TL84&CWF
        {0x6b, 0xb0}, //B2G_stand4[7:0]
        {0x6c, 0xb2}, //R2G_stand5[7:0]  A
        {0x6d, 0xac}, //B2G_stand5[7:0]
        {0x6e, 0x60}, //AWB_skin_weight R2G_stand5[9:8] B2G_stand5[9:8]
        {0x6f, 0x15}, //18 //AWB_indoor_THD (0x21=17 caculate)
        {0x73, 0x0f}, //AWB_indoor_mode
        {0x70, 0x10}, //AWB low luma TH
        {0x71, 0xe8}, //AWB outdoor TH
        {0x72, 0xc0}, //outdoor mode
        {0x74, 0x01}, //[2:0]AWB skip mode 2x2,4x4,4x8,8x8
        {0x75, 0x01}, //[1:0]AWB_every_N
        {0x7f, 0x08}, //[3]gray world frame start
        {0x76, 0x70}, //R limit
        {0x77, 0x58}, //G limit
        {0x78, 0xa0}, //d8 //B limit
        {0xfe, 0x00},
        //////////////////////////////////////////
        ///////////	CC	 ////////////////////////
        //////////////////////////////////////////
        {0xfe, 0x02},
        {0xc0, 0x01}, //[5:4] CC mode [0]CCT enable
        {0xC1, 0x44}, //50 //D50/D65
        {0xc2, 0xf4},
        {0xc3, 0x02}, //0
        {0xc4, 0xf2}, //e8 //e0
        {0xc5, 0x44}, //48
        {0xc6, 0xf8}, //f0
        {0xc7, 0x50},
        {0xc8, 0xf2},
        {0xc9, 0x00},
        {0xcA, 0xE0},
        {0xcB, 0x45},
        {0xcC, 0xec},
        {0xCd, 0x45},
        {0xce, 0xf0},
        {0xcf, 0x00},
        {0xe3, 0xf0},
        {0xe4, 0x45},
        {0xe5, 0xe8},
        {0xfe, 0x00},
        {0xf2, 0x0f},
        //////////////frame rate   50Hz
        {0xfe, 0x00},
        {0xf7, 0x1d},
        {0xf8, 0x82},
        {0x03, 0x04},
        {0x04, 0xe2},
        {0x05, 0x01}, //hb
        {0x06, 0x56},
        {0x07, 0x00}, //Vb
        {0x08, 0x14},
        {0xfe, 0x01},
        {0x25, 0x00},
        {0x26, 0xfa}, //step
        {0x27, 0x04}, //20fps
        {0x28, 0xe2},
        {0x29, 0x05}, //16.7fps
        {0x2a, 0xdc},
        {0x2b, 0x07}, //12.5fps
        {0x2c, 0xd0},
        {0x2d, 0x0b}, //night mode 8.3fps
        {0x2e, 0xb8},
        {0xfe, 0x00},
        /////////dark  sun//////
        {0x18, 0x22},
        {0xfe, 0x02},
        {0x40, 0xbf},
        {0x46, 0xcf},
        {0xfe, 0x00},
        //******************MIPI*******************
        {0xfe, 0x03}, //page 3
        {0x01, 0x83}, //07
        {0x02, 0x01}, //37//07
        {0x03, 0x10}, //10
        {0x04, 0x20}, //01	// fifo_prog
        {0x05, 0x00}, //fifo_prog
        {0x06, 0x88}, //YUV ISP data
        {0x11, 0x1e}, //LDI set YUV422
        {0x12, 0x80}, //04 //00 //04//00 //LWC[7:0]  //
        {0x13, 0x0c}, //05 //LWC[15:8]
        {0x15, 0x12}, //DPHYY_MODE read_ready
        {0x17, 0xf0}, //ff  01wdiv set
        {0xfe, 0x00},
        //size:1280x720
        //// crop window
        {0xfe, 0x00},
        {0x99, 0x55},
        {0x9a, 0x06},
        {0x9b, 0x00},
        {0x9c, 0x00},
        {0x9d, 0x01},
        {0x9e, 0x23},
        {0x9f, 0x00},
        {0xa0, 0x00},
        {0xa1, 0x01},
        {0xa2, 0x23},
        {0x90, 0x01},
        {0x91, 0x00},
        {0x92, 0x78},
        {0x93, 0x00},
        {0x94, 0x00}, // crop start:
        {0x95, 0x01},
        {0x96, 0xe0}, // 480
        {0x97, 0x02},
        {0x98, 0x80},
        //// AWB
        {0xfe, 0x00},
        {0xec, 0x06},
        {0xed, 0x04},
        {0xee, 0x60},
        {0xef, 0x90},
        {0xfe, 0x01},
        {0x74, 0x01},
        //// AEC
        {0xfe, 0x01},
        {0x01, 0x04},
        {0x02, 0xc0},
        {0x03, 0x14},
        {0x04, 0x90},
        {0x05, 0x30},
        {0x06, 0x90},
        {0x07, 0x30},
        {0x08, 0x80},
        {0x0a, 0x82},
        {0x21, 0x15},
        {0xfe, 0x00},
        {0x20, 0x15},
        //// mipi
        {0xfe, 0x03},
        {0x12, 0x00},
        {0x13, 0x05},
        {0x04, 0x40},
        {0x05, 0x01},
        {0x29, 0x02},
        {0x2b, 0x04},
        {0xfe, 0x00},

};

sensorInfo_t gc2145Info =
    {
        "gc2145",         //		const char *name; // name of sensor
        DRV_I2C_BPS_100K, //		drvI2cBps_t baud;
        0x78 >> 1,        //		uint8_t salve_i2c_addr_w;	 // salve i2c write address
        0x79 >> 1,        //		uint8_t salve_i2c_addr_r;	 // salve i2c read address
        0,                //		uint8_t reg_addr_value_bits; // bit0: 0: i2c register value is 8 bit, 1: i2c register value is 16 bit
        {0x21, 0x45},     //		uint8_t sensorid[2];
        640,              //		uint16_t spi_pixels_per_line;	// max width of source image
        480,              //		uint16_t spi_pixels_per_column; // max height of source image
        1,                //		uint16_t rstActiveH;	// 1: high level valid; 0: low level valid
        100,              //		uint16_t rstPulseWidth; // Unit: ms. Less than 200ms
        1,                //		uint16_t pdnActiveH;	// 1: high level valid; 0: low level valid
        0,                //		uint16_t dstWinColStart;
        640,              //		uint16_t dstWinColEnd;
        0,                //		uint16_t dstWinRowStart;
        480,              //		uint16_t dstWinRowEnd;
        1,                //		uint16_t spi_ctrl_clk_div;
#ifdef CONFIG_8850_FPGA_BOARD
        DRV_NAME_I2C2, //		uint32_t i2c_name;
#else
        DRV_NAME_I2C1, //		uint32_t i2c_name;
#endif
        0, //		uint32_t nPixcels;
        2, //		uint8_t captureDrops;
        0,
        0,
        NULL, //		uint8_t *buffer;
        {NULL, NULL},
        false,
        true,
        true,
        false, //		bool isCamIfcStart;
        false, //		bool scaleEnable;
        false, //		bool cropEnable;
        false, //		bool dropFrame;
        false, //		bool spi_href_inv;
        false, //		bool spi_little_endian_en;
        false,
        false,
        true,
        SENSOR_VDD_2800MV,     //		cameraAVDD_t avdd_val; // voltage of avdd
        SENSOR_VDD_1800MV,     //		cameraAVDD_t iovdd_val;
        CAMA_CLK_OUT_FREQ_24M, //		cameraClk_t sensorClk;
        ROW_RATIO_1_1,         //		camRowRatio_t rowRatio;
        COL_RATIO_1_1,         //		camColRatio_t colRatio;
        CAM_FORMAT_YUV,        //		cameraImageFormat_t image_format; // define in SENSOR_IMAGE_FORMAT_E enum,
        SPI_MODE_MASTER2_2,    //		camSpiMode_t camSpiMode;
        SPI_OUT_U0_Y1_V0_Y0,   //		camSpiYuv_t camYuvMode;
        camCaptureIdle,        //		camCapture_t camCapStatus;
        camCsi_In,
        NULL, //		drvIfcChannel_t *camp_ipc;
        NULL, //		drvI2cMaster_t *i2c_p;
        NULL, //		CampCamptureCB captureCB;
        NULL,
};

static void prvCamGc2145PowerOn(void)
{
    sensorInfo_t *p = &gc2145Info;
    halPmuSetPowerLevel(HAL_POWER_CAMD, p->dvdd_val);
    halPmuSwitchPower(HAL_POWER_CAMD, true, false);
    osiDelayUS(1000);
    halPmuSetPowerLevel(HAL_POWER_CAMA, p->avdd_val);
    halPmuSwitchPower(HAL_POWER_CAMA, true, false);
    osiDelayUS(1000);
}

static void prvCamGc2145PowerOff(void)
{
    halPmuSwitchPower(HAL_POWER_CAMA, false, false);
    osiDelayUS(1000);
    halPmuSwitchPower(HAL_POWER_CAMD, false, false);
    osiDelayUS(1000);
}

static bool prvCamGc2145I2cOpen(uint32_t name, drvI2cBps_t bps)
{
    sensorInfo_t *p = &gc2145Info;
#ifdef CONFIG_FIBOCOM_BASE
    if (name == 0 || p->i2c_p != NULL)
#else
    if (name == 0 || bps != DRV_I2C_BPS_100K || p->i2c_p != NULL)
#endif
    {
        return false;
    }
    p->i2c_p = drvI2cMasterAcquire(name, bps);
    if (p->i2c_p == NULL)
    {
        OSI_LOGE(0x10007d55, "cam i2c open fail");
        return false;
    }
    return true;
}

static void prvCamGc2145I2cClose()
{
    sensorInfo_t *p = &gc2145Info;
    if (p->i2c_p != NULL)
        drvI2cMasterRelease(p->i2c_p);
    p->i2c_p = NULL;
}

static void prvCamWriteOneReg(uint8_t addr, uint8_t data)
{
    sensorInfo_t *p = &gc2145Info;
#ifdef CONFIG_FIBOCOM_BASE
    drvI2cSlave_t idAddress = {p->salve_i2c_addr_w, addr, 0, false, true};
#else
    drvI2cSlave_t idAddress = {p->salve_i2c_addr_w, addr, 0, false};
#endif
    if (p->i2c_p != NULL)
    {
        drvI2cWrite(p->i2c_p, &idAddress, &data, 1);
    }
    else
    {
        OSI_LOGE(0x10007d56, "i2c is not open");
    }
}

static void prvCamReadReg(uint8_t addr, uint8_t *data, uint32_t len)
{
    sensorInfo_t *p = &gc2145Info;
#ifdef CONFIG_FIBOCOM_BASE
    drvI2cSlave_t idAddress = {p->salve_i2c_addr_w, addr, 0, false, true};
#else
    drvI2cSlave_t idAddress = {p->salve_i2c_addr_w, addr, 0, false};
#endif
    if (p->i2c_p != NULL)
    {
        drvI2cRead(p->i2c_p, &idAddress, data, len);
    }
    else
    {
        OSI_LOGE(0x10007d56, "i2c is not open");
    }
}

static bool prvCamWriteRegList(const cameraReg_t *regList, uint16_t len)
{
    sensorInfo_t *p = &gc2145Info;
    uint16_t regCount;
#ifdef CONFIG_FIBOCOM_BASE
    drvI2cSlave_t wirte_data = {p->salve_i2c_addr_w, 0, 0, false, true};
#else
    drvI2cSlave_t wirte_data = {p->salve_i2c_addr_w, 0, 0, false};
#endif
    for (regCount = 0; regCount < len; regCount++)
    {
        OSI_LOGI(0x10007d57, "cam write reg %x,%x", regList[regCount].addr, regList[regCount].data);
        wirte_data.addr_data = regList[regCount].addr;
        if (drvI2cWrite(p->i2c_p, &wirte_data, &regList[regCount].data, 1))
            osiDelayUS(5);
        else
            return false;
    }
    return true;
}

static bool prvCamGc2145Rginit(sensorInfo_t *info)
{
    OSI_LOGI(0x10009fed, "cam prvCamGC2145Rginit %d", info->sensorType);
    switch (info->sensorType)
    {
    case camCsi_In:
        if (!prvCamWriteRegList(RG_InitVgaMIPI, sizeof(RG_InitVgaMIPI) / sizeof(cameraReg_t)))
            return false;
        break;
    default:
        return false;
    }
    return true;
}

static void prvCamIsrCB(void *ctx)
{
    static uint8_t pictureDrop = 0;
    static uint8_t prev_ovf = 0;
    static uint8_t cap_ovf = 0;
    sensorInfo_t *p = &gc2145Info;
    REG_CAMERA_IRQ_CAUSE_T cause;
    cameraIrqCause_t mask = {0, 0, 0, 0};
    cause.v = hwp_camera->irq_cause;
    hwp_camera->irq_clear = cause.v;
    OSI_LOGI(0x10009fee, "cam gc2145 prvCsiCamIsrCB %d,%d,%d,%d", cause.b.vsync_f, cause.b.ovfl, cause.b.dma_done, cause.b.vsync_r);
    OSI_LOGI(0x10007d5b, "p->camCapStatus %d", p->camCapStatus);
    switch (p->camCapStatus)
    {
    case camCaptureState1:
        if (cause.b.vsync_f)
        {
            drvCamClrIrqMask();
            drvCamCmdSetFifoRst();
            drvCampStartTransfer(p->nPixcels, p->previewdata[p->page_turn]);
            p->isCamIfcStart = true;
            mask.overflow = 1;
            mask.dma = 1;
            drvCamSetIrqMask(mask);
            p->camCapStatus = camCaptureState2;
        }
        break;
    case camCaptureState2:
        if (cause.b.ovfl)
        {
            OSI_LOGI(0x10007d5c, "cam ovfl ");
            drvCameraControllerEnable(false);
            drvCamCmdSetFifoRst();
            drvCameraControllerEnable(true);
            cap_ovf = 1;
        }
        if (cause.b.dma_done)
        {
            drvCamClrIrqMask();
            if (cap_ovf == 1)
            {
                cap_ovf = 0;
                if (p->isCamIfcStart)
                    drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                p->camCapStatus = camCaptureState1;
                mask.fend = 1;
                drvCamSetIrqMask(mask);
                return;
            }
            if (p->isCamIfcStart == true)
            {
                p->isCamIfcStart = false;
                if (drvCampStopTransfer(p->nPixcels, p->capturedata))
                {
                    OSI_LOGI(0x10007d5d, "cam p->captureDrops %d ", p->captureDrops);
                    OSI_LOGI(0x10007d5e, "cam pictureDrop %d ", pictureDrop);
                    if (pictureDrop < p->captureDrops)
                    {
                        mask.fend = 1;
                        drvCamSetIrqMask(mask);
                        p->camCapStatus = camCaptureState1;
                        pictureDrop++;
                    }
                    else
                    {
                        p->camCapStatus = camCaptureIdle;
                        osiSemaphoreRelease(p->cam_sem_capture);
                        if (pictureDrop >= p->captureDrops)
                            pictureDrop = 0;
                        p->isFirst = false;
                    }
                }
            }
        }
        break;
    case campPreviewState1:
        if (cause.b.vsync_f)
        {
            drvCamClrIrqMask();
            OSI_LOGI(0x10007d5f, "cam mask ovf %d, firstframe %d", prev_ovf, p->isFirstFrame);
            if (prev_ovf || p->isFirstFrame)
            {
                drvCamCmdSetFifoRst();
                drvCampStartTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                p->isCamIfcStart = true;
                p->isFirstFrame = false;
                prev_ovf = 0;
                hwp_camera->irq_clear |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;
            }
            hwp_camera->irq_clear |= 1 << 0;
            mask.overflow = 1;
            mask.dma = 1;
            drvCamSetIrqMask(mask);
            p->camCapStatus = campPreviewState2;
        }
        break;
    case campPreviewState2:
        if (cause.b.ovfl)
        {
            OSI_LOGI(0x10007d5c, "cam ovfl ");
            prev_ovf = 1;

            drvCamClrIrqMask();
            mask.fend = 1;
            drvCamSetIrqMask(mask);
            //stop
            p->camCapStatus = campPreviewState1;
            return;
        }
        if (cause.b.dma_done)
        {
            drvCamClrIrqMask();
            if (prev_ovf == 1)
            {
                prev_ovf = 0;
                if (p->isCamIfcStart)
                    drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                p->camCapStatus = campPreviewState1;
                mask.fend = 1;
                drvCamSetIrqMask(mask);
                return;
            }
            if (p->isCamIfcStart == true)
            {
                p->isCamIfcStart = false;
                if (drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]))
                {
                    OSI_LOGI(0x10009fef, "cam 2145 preview release data %x", p->previewdata[p->page_turn]);
#ifndef CONFIG_CAMERA_SINGLE_BUFFER
                    p->page_turn = 1 - p->page_turn;
#endif
                    osiSemaphoreRelease(p->cam_sem_preview);
                    p->isFirst = false;
                    OSI_LOGI(0x10009ff0, "cam 2145 preview_page data %d", p->preview_page);
                    if (--p->preview_page)
                    {
                        p->camCapStatus = campPreviewState1;
                        mask.fend = 1;
                        drvCampStartTransfer(p->nPixcels, p->previewdata[p->page_turn]);
                        p->isCamIfcStart = true;
                        drvCamSetIrqMask(mask);
                    }
                    else
                    {
                        OSI_LOGI(0x1000910d, "cam go to idle");
                        p->camCapStatus = camCaptureIdle;
                        drvCameraControllerEnable(false);
                    }
                }
                else
                {
                    drvCamClrIrqMask();
                    p->camCapStatus = campPreviewState1;
                    mask.fend = 1;
                    drvCamSetIrqMask(mask);
                    OSI_LOGI(0x10007d62, "cam dma stop error");
                }
            }
        }
        break;
    default:
        break;
    }
}

void camGc2145TestMode(bool on)
{
    sensorInfo_t *p = &gc2145Info;
    if (p->isCamOpen)
    {
        if (on)
        {
            prvCamWriteOneReg(0x8c, 0x01);
            osiDelayUS(100000);
            prvCamWriteOneReg(0xfe, 0x03);
            prvCamWriteOneReg(0x10, 0x94); //open sensor
            prvCamWriteOneReg(0xfe, 0x00);
            osiDelayUS(10000);
        }
        else
        {
            osiDelayUS(100000);
            prvCamWriteOneReg(0xfe, 0x03);
            prvCamWriteOneReg(0x10, 0x94);
            prvCamWriteOneReg(0xfe, 0x00);
            osiDelayUS(10000);
        }
    }
}

bool camGc2145Open(void)
{
    OSI_LOGI(0x10009ff1, "camGc2145Open");
    sensorInfo_t *p = &gc2145Info;
    osiRequestSysClkActive(&gcCamCLK);
    drvCamSetPdn(true);
#ifdef CONFIG_SOC_8850
    drvCamGc2145SetRst(false);
#endif
    osiDelayUS(1000);
    prvCamGc2145PowerOn();
    drvCamSetMCLK(p->sensorClk);
    osiDelayUS(100);
    drvCamSetPdn(false);
    osiDelayUS(100);
#ifdef CONFIG_SOC_8850
    drvCamGc2145SetRst(true);
#endif
    if (!prvCamGc2145I2cOpen(p->i2c_name, p->baud))
    {
        OSI_LOGE(0x10009ff2, "cam prvCamGc2145I2cOpen fail");
        prvCamGc2145I2cClose();
        prvCamGc2145PowerOff();
        return false;
    }
    if (!prvCamGc2145Rginit(p))
    {
        OSI_LOGE(0x10009ff3, "cam prvCamGc2145Rginit fail");
        prvCamGc2145I2cClose();
        prvCamGc2145PowerOff();
        return false;
    }
    drvCampRegInit(p);
    drvCamSetIrqHandler(prvCamIsrCB, NULL);

    p->isCamOpen = true;
    return true;
}

void camGc2145Close(void)
{
    sensorInfo_t *p = &gc2145Info;
    if (p->isCamOpen)
    {
        osiReleaseClk(&gcCamCLK);
        osiDelayUS(1000);
        drvCamSetPdn(false);
#ifdef CONFIG_SOC_8850
        drvCamGc2145SetRst(true);
#endif
        osiDelayUS(1000);
        drvCamSetPdn(true);
        osiDelayUS(100);
#ifdef CONFIG_SOC_8850
        drvCamGc2145SetRst(false);
#endif
        osiDelayUS(100);
        drvCamDisableMCLK();
        osiDelayUS(1000);
        prvCamGc2145PowerOff();
        osiDelayUS(1000);
        drvCamSetPdn(false);
        prvCamGc2145I2cClose();
        drvCampRegDeInit();
        drvCamDisableIrq();
        p->isFirst = true;
        p->isCamOpen = false;
    }
    else
    {
        p->isCamOpen = false;
    }
}

void camGc2145GetId(uint8_t *data, uint8_t len)
{
    sensorInfo_t *p = &gc2145Info;
    if (p->isCamOpen)
    {
        if (data == NULL || len < 1)
            return;
        prvCamWriteOneReg(0xf3, 0x1);
        osiDelayUS(2);
        prvCamReadReg(0xf0, data, len);
    }
}

bool camGc2145CheckId(void)
{
    sensorInfo_t *p = &gc2145Info;
    uint8_t sensorID[2] = {0, 0};
    if (!p->isCamOpen)
    {
        drvCamSetPdn(true);
#ifdef CONFIG_SOC_8850
        drvCamGc2145SetRst(false);
#endif
        osiDelayUS(1000);
        prvCamGc2145PowerOn();
        drvCamSetMCLK(p->sensorClk);
        osiDelayUS(1000);
        drvCamSetPdn(false);
        osiDelayUS(1000);
#ifdef CONFIG_SOC_8850
        drvCamGc2145SetRst(true);
#endif
        if (!prvCamGc2145I2cOpen(p->i2c_name, p->baud))
        {
            OSI_LOGE(0x10009ff4, "cam prvCamGc2145I2cOpen failed");
            return false;
        }
        if (!p->isCamOpen)
        {
            p->isCamOpen = true;
        }
        camGc2145GetId(sensorID, 2);
        OSI_LOGI(0x10007d67, "cam get id 0x%x,0x%x", sensorID[0], sensorID[1]);
        if ((p->sensorid[0] == sensorID[0]) && (p->sensorid[1] == sensorID[1]))
        {
            OSI_LOGI(0x10007d68, "check id successful");
            camGc2145Close();
            return true;
        }
        else
        {
            camGc2145Close();
            OSI_LOGE(0x10007d69, "check id error");
            return false;
        }
    }
    else
    {
        OSI_LOGE(0x10007d6a, "camera already opened !");
        return false;
    }
}

void camGc2145CaptureImage(uint32_t size)
{
    sensorInfo_t *p = &gc2145Info;
    if (size != 0)
    {
        cameraIrqCause_t mask = {0, 0, 0, 0};
        if (p->isCamIfcStart == true)
        {
            drvCampStopTransfer(p->nPixcels, p->capturedata);
            p->isCamIfcStart = false;
        }
        p->nPixcels = size;
        p->camCapStatus = camCaptureState1;
        mask.fend = 1;
        drvCamSetIrqMask(mask);
        drvCameraControllerEnable(true);
    }
}

void camGc2145PrevStart(uint32_t size)
{
    sensorInfo_t *p = &gc2145Info;
    OSI_LOGI(0x10009ff5, "camGc2145PrevStart p->preview_page=%d ", p->preview_page);
    if (p->preview_page == 0)
    {
        cameraIrqCause_t mask = {0, 0, 0, 0};
        drvCamSetIrqMask(mask);
        if (p->isCamIfcStart == true)
        {
            drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]);
            p->isCamIfcStart = false;
        }
        p->nPixcels = size;
        p->camCapStatus = campPreviewState1;
        p->isStopPrev = false;
#ifdef CONFIG_CAMERA_SINGLE_BUFFER
        p->preview_page = 1;
#else
        p->preview_page = 2;
#endif
        mask.fend = 1;
        p->isFirstFrame = true;
        drvCamSetIrqMask(mask);
        drvCameraControllerEnable(true);
    }
}

void camGc2145PrevNotify(void)
{
    sensorInfo_t *p = &gc2145Info;
    uint32_t critical = osiEnterCritical();
    if (p->isCamOpen && !p->isStopPrev)
    {
        if (p->preview_page == 0)
        {
            cameraIrqCause_t mask = {0, 0, 0, 0};

            p->camCapStatus = campPreviewState1;
            p->preview_page++;
            p->isFirstFrame = true;
            mask.fend = 1;
            drvCamSetIrqMask(mask);
            drvCameraControllerEnable(true);
        }
#ifndef CONFIG_CAMERA_SINGLE_BUFFER
        else
        {
            if (p->preview_page < 2)
                p->preview_page++;
        }
#endif
    }
    osiExitCritical(critical);
}

void camGc2145StopPrev(void)
{
    sensorInfo_t *p = &gc2145Info;
    drvCamClrIrqMask();
    if (p->isCamIfcStart == true)
    {
        drvCampStopTransfer(p->nPixcels, p->previewdata[p->page_turn]);
        p->isCamIfcStart = false;
    }
    drvCameraControllerEnable(false);
    p->camCapStatus = camCaptureIdle;
    p->isStopPrev = true;
    p->preview_page = 0;
}

void camGc2145SetFalsh(uint8_t level)
{
    if (level >= 0 && level < 16)
    {
        if (level == 0)
        {
            halPmuSwitchPower(HAL_POWER_CAMFLASH, false, false);
        }
        else
        {
            halPmuSetCamFlashLevel(level);
            halPmuSwitchPower(HAL_POWER_CAMFLASH, true, false);
        }
    }
}

void camGc2145Brightness(uint8_t level)
{
}

void camGc2145Contrast(uint8_t level)
{
}

void camGc2145ImageEffect(uint8_t effect_type)
{
}
void camGc2145Ev(uint8_t level)
{
}

void camGc2145AWB(uint8_t mode)
{
}

void camGc2145GetSensorInfo(sensorInfo_t **pSensorInfo)
{
    OSI_LOGI(0x10009ff6, "CamGc2145GetSensorInfo %08x", &gc2145Info);
    *pSensorInfo = &gc2145Info;
}

bool camGc2145Reg(SensorOps_t *pSensorOpsCB)
{
    if (camGc2145CheckId())
    {
        pSensorOpsCB->cameraOpen = camGc2145Open;
        pSensorOpsCB->cameraClose = camGc2145Close;
        pSensorOpsCB->cameraGetID = camGc2145GetId;
        pSensorOpsCB->cameraCaptureImage = camGc2145CaptureImage;
        pSensorOpsCB->cameraStartPrev = camGc2145PrevStart;
        pSensorOpsCB->cameraPrevNotify = camGc2145PrevNotify;
        pSensorOpsCB->cameraStopPrev = camGc2145StopPrev;
        pSensorOpsCB->cameraSetAWB = camGc2145AWB;
        pSensorOpsCB->cameraSetBrightness = camGc2145Brightness;
        pSensorOpsCB->cameraSetContrast = camGc2145Contrast;
        pSensorOpsCB->cameraSetEv = camGc2145Ev;
        pSensorOpsCB->cameraSetImageEffect = camGc2145ImageEffect;
        pSensorOpsCB->cameraGetSensorInfo = camGc2145GetSensorInfo;
        pSensorOpsCB->cameraFlashSet = camGc2145SetFalsh;
        pSensorOpsCB->cameraTestMode = camGc2145TestMode;
        return true;
    }
    return false;
}
#endif
