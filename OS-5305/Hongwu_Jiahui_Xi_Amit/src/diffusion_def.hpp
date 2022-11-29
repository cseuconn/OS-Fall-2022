#include<string.h>
#include<math.h>
#include<hls_math.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <string.h>  // memcpy
#include <iostream>
// FC layer parameter
#define dim1 784
#define dim1_buf 14
#define dim1_cnt (784 / 14)
#define dim2 784
#define dim2_buf 4
#define dim2_cnt (784 / 4)
#define parallelism 14
#define half_width 16

typedef ap_uint<224> data_bus;


// Conv layer parameter
typedef ap_uint<16> ushort;


// Conv layer parameter

typedef ap_uint<1> u1;
typedef ap_uint<2> u2;
typedef ap_uint<3> u3;
typedef ap_uint<5> u5;
typedef ap_uint<6> u6;
typedef ap_uint<7> u7;
typedef ap_int<6> s6;
typedef ap_uint<8> u8;
typedef ap_uint<9> u9;
typedef ap_int<10> s10;
typedef ap_uint<12> u12;
typedef ap_uint<13> u13;

//typedef float op_t;
typedef ap_fixed<16, 8, AP_RND, AP_SAT> op_t;

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

const int TI = 8;  // Input channel tile size
const int TO = 64; // Output channel tile size



const int TH = 14; // Output height tile size
const int TW = 14; // Output width tile size
//const int IFM_BUF_MAX = ((TD-1)+3) * ((TH-1)*2+5) * ((TW-1)*2+5);
const int IFM_BUF_MAX_H = (TH-1)*2+7; // Input height tile size
const int IFM_BUF_MAX_W = (TW-1)*2+7; // Input width tile size

const int K_BUF_MAX_H = 7; // Kernel height tile size
const int K_BUF_MAX_W = 7; // Kernel width tile size

const int PP_in = 8;  //Input parallelism
const int PP_out = 8; //Output parallelism

const int BLOCK_MAX = 1152;  // 1152*512/64/8

const int KD_CNT = 3;
const int KH_CNT = 3;
const int KW_CNT = 3;



void conv3d_zcu102(u12 CI,
        u5 DI, u9 HI, u9 WI,
        u12 CO,
        u2 KD, u3 KH, u3 KW,
        u2 SD, u2 SH, u2 SW,
        u1 mode, u1 bn_en, u2 relu_mode,
        op_t *wgt_conv,
        op_t *wgt_bn, op_t *bias,
        op_t *ifm,
        op_t *ofm,
        u8 *block_en, u13 block_num,
        u1 shortcut_en, op_t *shortcut_x,
        u2 pool_mode, u2 pool_KD, u3 pool_KH, u3 pool_KW,
        u2 pool_padD, u2 pool_padH, u2 pool_padW,
        u1 test
        );
