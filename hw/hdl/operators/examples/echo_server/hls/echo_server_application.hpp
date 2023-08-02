/************************************************
Copyright (c) 2016, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.// Copyright (c) 2015 Xilinx, Inc.
************************************************/

#include "axi_utils.hpp"
#include "toe.hpp"

#define DATA_WIDTH 512

/** @defgroup echo_server_application Echo Server Application
 *
 */
// void echo_server_application(hls::stream<ap_uint<16>> &listenPort, hls::stream<bool> &listenPortStatus,
// 							 hls::stream<appNotification> &notifications, hls::stream<appReadRequest> &readRequest,
// 							 hls::stream<ap_uint<16>> &rxMetaData, hls::stream<net_axis<DATA_WIDTH>> &rxData,
// 							 hls::stream<ipTuple> &openConnection, hls::stream<openStatus> &openConStatus,
// 							 hls::stream<ap_uint<16>> &closeConnection,
// 							 hls::stream<appTxMeta> &txMetaData, hls::stream<net_axis<DATA_WIDTH>> &txData,
// 							 hls::stream<appTxRsp> &txStatus);

void echo_server_application(hls::stream<ap_uint<16>> &m_axis_listen_port,
							 hls::stream<bool> &s_axis_listen_port_status,
							 hls::stream<appNotification> &s_axis_notifications,
							 hls::stream<appReadRequest> &m_axis_read_package,
							 hls::stream<ap_uint<16>> &s_axis_rx_metadata,
							 hls::stream<ap_axiu<DATA_WIDTH, 0, 0, 0>> &s_axis_rx_data,
							 hls::stream<ipTuple> &m_axis_open_connection,
							 hls::stream<openStatus> &s_axis_open_status,
							 hls::stream<ap_uint<16>> &m_axis_close_connection,
							 hls::stream<appTxMeta> &m_axis_tx_metadata,
							 hls::stream<ap_axiu<DATA_WIDTH, 0, 0, 0>> &m_axis_tx_data,
							 hls::stream<appTxRsp> &s_axis_tx_status);
