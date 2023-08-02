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

#include "echo_server_application.hpp"

void client(hls::stream<ap_uint<16>> &sessioIdFifo,
			hls::stream<ap_uint<16>> &lengthFifo,
			hls::stream<net_axis<DATA_WIDTH>> &dataFifo,
			hls::stream<appTxMeta> &txMetaData, hls::stream<net_axis<DATA_WIDTH>> &txData)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

	// Reads new data from memory and writes it into fifo
	// Read & write metadata only once per package
	static ap_uint<1> esac_fsmState=0;

	ap_uint<16> sessionID;
	ap_uint<16> length;
	net_axis<DATA_WIDTH> currWord;

	switch (esac_fsmState)
	{
	case 0:
		if (!sessioIdFifo.empty() && !lengthFifo.empty() && !txMetaData.full())
		{
			sessioIdFifo.read(sessionID);
			lengthFifo.read(length);
			txMetaData.write(appTxMeta(sessionID, length));
			esac_fsmState=1;
		}
		break;
	case 1:
		if (!dataFifo.empty() && !txData.full())
		{
			dataFifo.read(currWord);
			txData.write(currWord);
			if (currWord.last)
			{
				esac_fsmState=0;
			}
		}
		break;
	}
}

void dummy(hls::stream<ipTuple> &openConnection, hls::stream<openStatus> &openConStatus,
		   hls::stream<ap_uint<16>> &closeConnection,
		   hls::stream<appTxRsp> &txStatus)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

	openStatus newConn;
	ipTuple tuple;

	// Dummy code should never be executed, this is necessary because every hls::streams has to be written/read
	if (!openConStatus.empty())
	{
		openConStatus.read(newConn);
		tuple.ip_address=0x0a010101;
		tuple.ip_port=0x3412;
		openConnection.write(tuple);
		if (newConn.success)
		{
			closeConnection.write(newConn.sessionID);
			// closePort.write(tuple.ip_port);
		}
	}

	if (!txStatus.empty()) // Make Checks
	{
		txStatus.read();
	}
}

void open_port(hls::stream<ap_uint<16>> &listenPort,
			   hls::stream<bool> &listenSts)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

	static ap_uint<2> state=0;
#pragma HLS reset variable=state

	bool listenDone=false;

	switch (state)
	{
	case 0:
		listenPort.write(7);
		state=1;
		break;
	case 1:
		if (!listenSts.empty())
		{
			listenSts.read(listenDone);
			if (listenDone)
			{
				state=2;
			}
			else
			{
				state=0;
			}
		}
		break;
	case 2:
		// IDLE
		break;
	} // switch
}

void notification_handler(hls::stream<appNotification> &notific,
						  hls::stream<appReadRequest> &readReq,
						  hls::stream<ap_uint<16>> &lenghtFifo)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

	appNotification notification;

	// Receive notifications, about new data which is available
	if (!notific.empty())
	{
		notific.read(notification);
		std::cout << notification.ipAddress << "\t" << notification.dstPort << std::endl;
		if (notification.length != 0)
		{
			readReq.write(appReadRequest(notification.sessionID, notification.length));
			lenghtFifo.write(notification.length);
		}
	}
}

void server(hls::stream<ap_uint<16>> &rxMetaData,
			hls::stream<net_axis<DATA_WIDTH>> &rxData,
			hls::stream<ap_uint<16>> &sessioIdFifo,
			hls::stream<net_axis<DATA_WIDTH>> &dataFifo)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

	// Reads new data from memory and writes it into fifo
	// Read & write metadata only once per package
	static ap_uint<1> ksvs_fsmState=0;

	ap_uint<16> sessionID;
	net_axis<DATA_WIDTH> currWord;

	switch (ksvs_fsmState)
	{
	case 0:
		if (!rxMetaData.empty())
		{
			rxMetaData.read(sessionID);
			sessioIdFifo.write(sessionID);
			ksvs_fsmState=1;
		}
		break;
	case 1:
		if (!rxData.empty())
		{
			rxData.read(currWord);
			dataFifo.write(currWord);
			if (currWord.last)
			{
				ksvs_fsmState=0;
			}
		}
		break;
	}
}

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
							 hls::stream<appTxRsp> &s_axis_tx_status)
{
#pragma HLS DATAFLOW disable_start_propagation
#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INTERFACE axis register port=m_axis_listen_port
#pragma HLS INTERFACE axis register port=s_axis_listen_port_status
	// #pragma HLS INTERFACE axis register port=closePort name=m_axis_close_port

#pragma HLS INTERFACE axis register port=s_axis_notifications
#pragma HLS INTERFACE axis register port=m_axis_read_package
#pragma HLS DATA_PACK variable=s_axis_notifications
#pragma HLS DATA_PACK variable=m_axis_read_package

#pragma HLS INTERFACE axis register port=s_axis_rx_metadata
#pragma HLS INTERFACE axis register port=s_axis_rx_data
	// #pragma HLS DATA_PACK variable=rxMetaData

#pragma HLS INTERFACE axis register port=m_axis_open_connection
#pragma HLS INTERFACE axis register port=s_axis_open_status
#pragma HLS DATA_PACK variable=m_axis_open_connection
#pragma HLS DATA_PACK variable=s_axis_open_status

#pragma HLS INTERFACE axis register port=m_axis_close_connection

#pragma HLS INTERFACE axis register port=m_axis_tx_metadata
#pragma HLS INTERFACE axis register port=m_axis_tx_data
#pragma HLS INTERFACE axis register port=s_axis_tx_status
#pragma HLS DATA_PACK variable=m_axis_tx_metadata
#pragma HLS DATA_PACK variable=s_axis_tx_status

	static hls::stream<net_axis<DATA_WIDTH>> s_axis_rx_data_internal;
	static hls::stream<net_axis<DATA_WIDTH>> m_axis_tx_data_internal;

	convert_axis_to_net_axis<DATA_WIDTH>(s_axis_rx_data, s_axis_rx_data_internal);
	convert_net_axis_to_axis<DATA_WIDTH>(m_axis_tx_data_internal, m_axis_tx_data);

	static hls::stream<ap_uint<16>> esa_sessionidFifo("esa_sessionidFifo");
	static hls::stream<ap_uint<16>> esa_lengthFifo("esa_lengthFifo");
	static hls::stream<net_axis<DATA_WIDTH>> esa_dataFifo("esa_dataFifo");

#pragma HLS stream variable=esa_sessionidFifo depth=64
#pragma HLS stream variable=esa_lengthFifo depth=64
#pragma HLS stream variable=esa_dataFifo depth=2048

	client(esa_sessionidFifo, esa_lengthFifo, esa_dataFifo, m_axis_tx_metadata, m_axis_tx_data_internal);
	server(s_axis_rx_metadata, s_axis_rx_data_internal, esa_sessionidFifo, esa_dataFifo);
	open_port(m_axis_listen_port, s_axis_listen_port_status);
	notification_handler(s_axis_notifications, m_axis_read_package, esa_lengthFifo);
	dummy(m_axis_open_connection, s_axis_open_status, m_axis_close_connection, s_axis_tx_status);
}
