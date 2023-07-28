#include "zlib_compress.hpp"

#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "ap_axi_sdata.h"

#define AXI_DATA_BITS 512
#define PID_BITS 6
#define STRATEGY 0
#define NUM_BLOCKS 8
#define BLOCK_SIZE_IN_KB 32
#define TUSER_DWIDTH 0

typedef ap_axiu<GMEM_DWIDTH, 0, PID_BITS, 0> input_t;
typedef ap_axiu<GMEM_DWIDTH, TUSER_DWIDTH, PID_BITS, 0> output_t;

/**
 * User logic
 *
 */

void design_user_hls_c0_0_top(
    // Host streams
    hls::stream<ap_axiu<AXI_DATA_BITS, 0, PID_BITS, 0>> &axis_host_0_sink,
    hls::stream<ap_axiu<AXI_DATA_BITS, TUSER_DWIDTH, PID_BITS, 0>> &axis_host_0_src,

    ap_uint<64> axi_ctrl)
{
#pragma HLS DATAFLOW disable_start_propagation
#pragma HLS INTERFACE ap_ctrl_none port = return

#pragma HLS INTERFACE axis register port = axis_host_0_sink name = s_axis_host_0_sink
#pragma HLS INTERFACE axis register port = axis_host_0_src name = m_axis_host_0_src

#pragma HLS INTERFACE s_axilite port = return bundle = control
// #pragma HLS INTERFACE s_axilite port=axi_ctrl_a bundle=control
// #pragma HLS INTERFACE s_axilite port=axi_ctrl_b bundle=control
// #pragma HLS INTERFACE s_axilite port=axi_ctrl_c bundle=control
#pragma HLS INTERFACE s_axilite port = axi_ctrl bundle = control

    gzipMulticoreCompressAxiStream<STRATEGY, BLOCK_SIZE_IN_KB, NUM_BLOCKS, TUSER_DWIDTH>(axis_host_0_sink, axis_host_0_src);
}
