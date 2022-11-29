#include<string.h>
#include<math.h>
#include<hls_math.h>
#include <hls_vector.h>
#include <hls_stream.h>
#include"diffusion_def.hpp"

template<short in_c, short in_c_buf, short out_c, short pp>
void fc_layer_compute(half in[in_c],half w[in_c_buf][out_c], half out[out_c], short bias_in, bool enable_compute)
{
#pragma HLS ARRAY_PARTITION dim=1 factor=pp type=cyclic variable=in
#pragma HLS ARRAY_PARTITION dim=1 factor=pp type=cyclic variable=out
#pragma HLS ARRAY_PARTITION dim=2 factor=pp type=cyclic variable=w
	if(!enable_compute)
		return;

	for (int i = 0; i < in_c_buf; i++)
		for (int j = 0; j < out_c; j++)
		{
#pragma HLS PIPELINE II=1
#pragma HLS UNROLL factor=pp
			half mul = in[i + bias_in] * w[i][j];
#pragma HLS BIND_OP variable=mul op=hmul impl=dsp
			half acc = out[j];
			acc += mul;
#pragma HLS BIND_OP variable=acc op=hadd impl=fabric
			out[j] = acc;
		}
}

template<short in_c_buf, short out_c, short pp>
//void load_w(half w_buf[in_c_buf][out_c], hls::vector<half, pp> *weight, short bias_in)
void load_w(half w_buf[in_c_buf][out_c], data_bus *weight, short bias_in, bool enable_load)
{
#pragma HLS ARRAY_PARTITION dim=2 factor=pp type=cyclic variable=w_buf
//#pragma HLS DEPENDENCE dependent=false type=inter variable=weight
	data_bus weight_temp;
	short loopcnt_2 = out_c / pp;
	short weight_addr_bias = bias_in *loopcnt_2;
	if (!enable_load)
		return;
	for (short i = 0; i < in_c_buf; i++)
		for (short j = 0; j < loopcnt_2; j++)
		{
#pragma HLS PIPELINE II=1
//			weight_temp = weight[weight_addr_bias + j];
			weight_temp = weight[i*loopcnt_2 + weight_addr_bias + j];

//			weight_temp = weight[loopcnt_2 + weight_addr_bias + j];
			short offset = j * pp;
			for (short k = 0; k < pp; k++)
			{
//				w_buf[i][offset + k] = weight_temp[(k*16) : ((k + 1)*16)];.
				w_buf[i][offset + k] = weight_temp((k*half_width), ((k + 1)*half_width));
			}
		}

}

template<short out_c, short pp>
void load_b(half bias_buf[out_c], half *bias)
{
	for (short i = 0; i < out_c; i++)
	{
#pragma HLS PIPELINE II=1
		bias_buf[i] = bias[i];
	}
}

//void fc_layer_coarse(half *in, hls::vector<half, parallelism> *weight, half *bias, half *out)
void fc_layer_coarse(half *in, data_bus *weight, half *bias, half *out)
{
// use same bundle for in_out and weight since they
#pragma HLS INTERFACE mode=m_axi bundle=in_out port=in
#pragma HLS INTERFACE mode=m_axi bundle=in_out port=out
#pragma HLS INTERFACE mode=m_axi bundle=weight port=weight
#pragma HLS INTERFACE mode=m_axi bundle=in_out port=bias
	half in_buf[dim1];
	half out_buf[dim2];
	half w_buf_1[dim1_buf][dim2];
//#pragma HLS ARRAY_PARTITION dim=2 factor=14 type=cyclic variable=w_buf_1
	half w_buf_2[dim1_buf][dim2];
//#pragma HLS ARRAY_PARTITION dim=2 factor=14 type=cyclic variable=w_buf_2
	half bias_buf[dim2];

	for (int i = 0; i < dim1; i++)
	{
		in_buf[i] = in[i];
	}

	for (int i = 0; i < dim2; i++)
	{
		bias_buf[i] = bias[i];
	}
	// implement the double buffer to conduct matrix vector multiplication:
	bool pingpong = 0;
	for (int i = 0; i < (dim1 + dim1_buf); i += dim1_buf)
	{
		if(pingpong == 0)
		{
			bool enable_load = i < dim1;
			bool enable_compute = i > 0;
			load_w<dim1_buf, dim2, parallelism>(w_buf_1, weight, i, enable_load);
			fc_layer_compute<dim1, dim1_buf, dim2, parallelism>(in_buf, w_buf_2, out_buf, i, enable_compute);
			pingpong = ~pingpong;
		}
		else
		{
			bool enable_load = i < dim1;
			bool enable_compute = i > 0;
			load_w<dim1_buf, dim2, parallelism>(w_buf_2, weight, i, enable_load);
			fc_layer_compute<dim1, dim1_buf, dim2, parallelism>(in_buf, w_buf_1, out_buf, i, enable_compute);
			pingpong = ~pingpong;
		}
	}
	for (int i = 0; i < dim2; i++)
	{
		out[i] = out_buf[i] + bias[i];
	}
}
