#include"diffusion_def.hpp"


void conv_pynqz2(
		ushort CI, // Number of input channels
		ushort In_HW, // Input height and width
		ushort CO,  // Number of output channels
		ushort K_HW, // Kernel height and width
		ushort stride, // Stride
        u1 mode, // Mode: Padding or not, if yes padding to same size
		u2 relu_mode, //relu_mode: enable ReLU or not
        op_t *wgt_conv, // conv weight in global memory
		op_t *bias,  // bias in global memory
        op_t *ifm,  // input feature map in global memory
        op_t *ofm,  // output feature map in global memory
        )
{
#pragma HLS INTERFACE m_axi port=wgt_conv offset=slave bundle=wgt_conv_ //depth=6615
#pragma HLS INTERFACE m_axi port=wgt_bn offset=slave bundle=wgt_conv_ //depth=45
#pragma HLS INTERFACE m_axi port=bias offset=slave bundle=wgt_conv_ //depth=45
#pragma HLS INTERFACE m_axi port=ifm offset=slave bundle=ifm_ //depth=37632
#pragma HLS INTERFACE m_axi port=ofm offset=slave bundle=ofm_ //depth=141120
#pragma HLS INTERFACE m_axi port=shortcut_x offset=slave bundle=ofm_ //depth=141120

#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=CI
#pragma HLS INTERFACE s_axilite port=In_HW
#pragma HLS INTERFACE s_axilite port=CO
#pragma HLS INTERFACE s_axilite port=K_HW
#pragma HLS INTERFACE s_axilite port=stride
#pragma HLS INTERFACE s_axilite port=mode
#pragma HLS INTERFACE s_axilite port=bn_en
#pragma HLS INTERFACE s_axilite port=relu_mode


    const ushort pad = mode ? ((K_HW-1)/2) : 0;

    const ushort Out_HW = (In_HW+2*pad-K_HW)/stride+1; // Output height

    const ushort K_SIZE = K_HW*K_HW;  // Kernel size

    const ushort H_BUF_MAX = (TH-1)*stride+K_SIZE; // Input height actual tile size
    const ushort W_BUF_MAX = (TW-1)*stride+K_SIZE; // Input width actual tile size

    static op_t ifm_buf1[TI][IFM_BUF_MAX_H][IFM_BUF_MAX_W];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_in type=cyclic variable=ifm_buf1
    static op_t wgt_conv_buf1[TO][TI][K_BUF_MAX_H][K_BUF_MAX_W];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_out type=cyclic variable=wgt_conv_buf1
#pragma HLS ARRAY_PARTITION dim=2 factor=PP_in type=cyclic variable=wgt_conv_buf1

    static op_t ofm_buf1[TO][TH][TW];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_out type=cyclic variable=ofm_buf1
    static op_t wgt_bn_buf1[TO];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_out type=cyclic variable=wgt_bn_buf1
    static op_t bias_buf1[TO];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_out type=cyclic variable=bias_buf1

    static op_t ifm_buf2[TI][IFM_BUF_MAX_H][IFM_BUF_MAX_W];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_in type=cyclic variable=ifm_buf2
    static op_t wgt_conv_buf2[TO][TI][K_BUF_MAX_H][K_BUF_MAX_W];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_out type=cyclic variable=wgt_conv_buf2
#pragma HLS ARRAY_PARTITION dim=2 factor=PP_in type=cyclic variable=wgt_conv_buf2

    static op_t ofm_buf2[TO][TH][TW];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_out type=cyclic variable=ofm_buf2
    static op_t wgt_bn_buf2[TO];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_out type=cyclic variable=wgt_bn_buf2
    static op_t bias_buf2[TO];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_out type=cyclic variable=bias_buf2

    static op_t shortcut_buf[TO][TD][TH][TW];
#pragma HLS ARRAY_PARTITION dim=1 factor=PP_out type=cyclic variable=shortcut_buf


    bool pingpong = 0;
    height_outer: for (ushort cnt_H = 0; cnt_H < Out_HW; cnt_H += TH)
    {
    	width_outer: for (ushort cnt_W = 0; cnt_W < Out_HW; cnt_W+= TW)
    	{
    		channel_out_outer: for (ushort cnt_CO = 0; cnt_CO < (CO + TO); cnt_CO+= TO)
    		{
    			if(pingpong)
    			{
    				load_compute(ofm_buf1);
    				store_ofm(ofm_buf0);
    				pingpong = ~pingpong;
    			}
    			else
    			{
    				load_compute(ofm_buf0);
    				store_ofm(ofm_buf1);
    				pingpong = ~pingpong;
    			}

    		}
    	}
    }
}
