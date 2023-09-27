/************************************************
Copyright (c) 2019, Systems Group, ETH Zurich.
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
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************/
#include "echo_server_application.hpp"
#include <iostream>
#include "ap_axi_sdata.h"

// Buffers responses coming from the TCP stack
void status_handler(hls::stream<appTxRsp> &txStatus,
					hls::stream<appTxRsp> &txStatusBuffer)
{
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

	if (!txStatus.empty())
	{
		appTxRsp resp = txStatus.read();
		txStatusBuffer.write(resp);
	}
}

// Buffers open status coming from the TCP stack
void openStatus_handler(hls::stream<openStatus> &openConStatus,
						hls::stream<openStatus> &openConStatusBuffer)
{
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

	if (!openConStatus.empty())
	{
		openStatus resp = openConStatus.read();
		openConStatusBuffer.write(resp);
	}
}

void txMetaData_handler(hls::stream<appTxMeta> &txMetaDataBuffer,
						hls::stream<appTxMeta> &txMetaData)
{
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

	if (!txMetaDataBuffer.empty())
	{
		appTxMeta metaDataReq = txMetaDataBuffer.read();
		txMetaData.write(metaDataReq);
	}
}

template <int WIDTH>
void txDataBuffer_handler(hls::stream<net_axis<WIDTH>> &txDataBuffer,
						  hls::stream<net_axis<WIDTH>> &txData)
{
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

	if (!txDataBuffer.empty())
	{
		net_axis<WIDTH> word = txDataBuffer.read();
		txData.write(word);
	}
}

template <int WIDTH>
void client(
	hls::stream<appTxMeta> &txMetaDataBuffer,
	hls::stream<net_axis<WIDTH>> &txDataBuffer,
	// hls::stream<appTxRsp> &txStatus,
	hls::stream<ap_uint<32>> &sessionIdFifo,
	hls::stream<ap_uint<32>> &lengthFifo,
	hls::stream<net_axis<DATA_WIDTH>> &dataFifo)
{
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

	enum txHandlerStateType
	{
		WAIT_CMD,
		CHECK_REQ,
		WRITE_PKG
	};
	static txHandlerStateType txHandlerState = WAIT_CMD;

	// static ap_uint<32> sessionID;
	// static ap_uint<32> transferSize;
	// static ap_uint<32> pkgWordCount;
	static appTxMeta tx_meta_pkt;

	switch (txHandlerState)
	{
	case WAIT_CMD:
		// if (runTx)
		if (!sessionIdFifo.empty() && !lengthFifo.empty() && !txMetaDataBuffer.full())
		{
			tx_meta_pkt.sessionID = sessionIdFifo.read();
			tx_meta_pkt.length = lengthFifo.read();

			txMetaDataBuffer.write(tx_meta_pkt);

			txHandlerState = WRITE_PKG;
		}
		break;
	case WRITE_PKG:
		if (!dataFifo.empty() && !txDataBuffer.full())
		{
			net_axis<WIDTH> currPkt = dataFifo.read();
			txDataBuffer.write(currPkt);
			if (currPkt.last)
			{
				txHandlerState = WAIT_CMD;
			}
		}
		break;
	}
}

template <int WIDTH>
void server(hls::stream<ap_uint<16>> &listenPort,
			hls::stream<bool> &listenPortStatus,
			hls::stream<appNotification> &notifications,
			hls::stream<appReadRequest> &readRequest,
			hls::stream<ap_uint<16>> &rxMetaData,
			hls::stream<net_axis<WIDTH>> &rxData,
			hls::stream<ap_uint<32>> &sessionIdFifo,
			hls::stream<ap_uint<32>> &lengthFifo,
			hls::stream<net_axis<DATA_WIDTH>> &dataFifo)
{
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

	enum listenFsmStateType
	{
		OPEN_PORT,
		WAIT_PORT_STATUS
	};
	static listenFsmStateType listenState = OPEN_PORT;
	enum consumeFsmStateType
	{
		WAIT_PKG,
		CONSUME
	};
	static consumeFsmStateType serverFsmState = WAIT_PKG;
#pragma HLS RESET variable = listenState

	switch (listenState)
	{
	case OPEN_PORT:
		// Open Port 7
		listenPort.write(7);
		listenState = WAIT_PORT_STATUS;
		break;
	case WAIT_PORT_STATUS:
		if (!listenPortStatus.empty())
		{
			bool open = listenPortStatus.read();
			if (!open)
			{
				listenState = OPEN_PORT;
			}
		}
		break;
	}

	if (!notifications.empty())
	{
		appNotification notification = notifications.read();

		if (notification.length != 0)
		{
			readRequest.write(appReadRequest(notification.sessionID, notification.length));
			lengthFifo.write(notification.length);
		}
	}

	switch (serverFsmState)
	{
	case WAIT_PKG:
		if (!rxMetaData.empty() && !rxData.empty())
		{
			ap_uint<32> sessionID = rxMetaData.read();
			net_axis<WIDTH> receiveWord = rxData.read();

			sessionIdFifo.write(sessionID);
			dataFifo.write(receiveWord);
			if (!receiveWord.last)
			{
				serverFsmState = CONSUME;
			}
		}
		break;
	case CONSUME:
		if (!rxData.empty())
		{
			net_axis<WIDTH> receiveWord = rxData.read();
			dataFifo.write(receiveWord);
			if (receiveWord.last)
			{
				serverFsmState = WAIT_PKG;
			}
		}
		break;
	}
}

void echo_server_application(hls::stream<ap_uint<16>> &listenPort,
							 hls::stream<bool> &listenPortStatus,
							 hls::stream<appNotification> &notifications,
							 hls::stream<appReadRequest> &readRequest,
							 hls::stream<ap_uint<16>> &rxMetaData,
							 hls::stream<ap_axiu<DATA_WIDTH, 0, 0, 0>> &rxData,
							 hls::stream<appTxMeta> &txMetaData,
							 hls::stream<ap_axiu<DATA_WIDTH, 0, 0, 0>> &txData,
							 hls::stream<appTxRsp> &txStatus)

{
#pragma HLS DATAFLOW disable_start_propagation
#pragma HLS INTERFACE ap_ctrl_none port = return

#pragma HLS INTERFACE axis register port = listenPort name = m_axis_listen_port
#pragma HLS INTERFACE axis register port = listenPortStatus name = s_axis_listen_port_status

#pragma HLS INTERFACE axis register port = notifications name = s_axis_notifications
#pragma HLS INTERFACE axis register port = readRequest name = m_axis_read_package
#pragma HLS aggregate compact = bit variable = notifications
#pragma HLS aggregate compact = bit variable = readRequest

#pragma HLS INTERFACE axis register port = rxMetaData name = s_axis_rx_metadata
#pragma HLS INTERFACE axis register port = rxData name = s_axis_rx_data

#pragma HLS INTERFACE axis register port = txMetaData name = m_axis_tx_metadata
#pragma HLS INTERFACE axis register port = txData name = m_axis_tx_data
#pragma HLS INTERFACE axis register port = txStatus name = s_axis_tx_status
#pragma HLS aggregate compact = bit variable = txMetaData
#pragma HLS aggregate compact = bit variable = txStatus

	// This is required to buffer up to 1024 reponses => supporting up to 1024 connections
	static hls::stream<appTxRsp> txStatusBuffer("txStatusBuffer");
#pragma HLS STREAM variable = txStatusBuffer depth = 512

	// This is required to buffer up to 512 tx_meta_data => supporting up to 512 connections
	static hls::stream<appTxMeta> txMetaDataBuffer("txMetaDataBuffer");
#pragma HLS STREAM variable = txMetaDataBuffer depth = 512

	// This is required to buffer up to MAX_SESSIONS txData
	static hls::stream<net_axis<DATA_WIDTH>> txDataBuffer("txDataBuffer");
#pragma HLS STREAM variable = txDataBuffer depth = 512

	static hls::stream<net_axis<DATA_WIDTH>> rxData_internal;
#pragma HLS STREAM depth = 2 variable = rxData_internal

	static hls::stream<net_axis<DATA_WIDTH>> txData_internal;
#pragma HLS STREAM depth = 2 variable = txData_internal

	static hls::stream<ap_uint<32>> esa_sessionidFifo("esa_sessionidFifo");
#pragma HLS stream variable = esa_sessionidFifo depth = 64
	static hls::stream<ap_uint<32>> esa_lengthFifo("esa_lengthFifo");
#pragma HLS stream variable = esa_lengthFifo depth = 64
	static hls::stream<net_axis<DATA_WIDTH>> esa_dataFifo("esa_dataFifo");
#pragma HLS stream variable = esa_dataFifo depth = 2048

	/*
	 * Client
	 */
	// status_handler(txStatus, txStatusBuffer);
	txMetaData_handler(txMetaDataBuffer, txMetaData);
	txDataBuffer_handler(txDataBuffer, txData_internal);

	convert_axis_to_net_axis<DATA_WIDTH>(rxData,
										 rxData_internal);

	convert_net_axis_to_axis<DATA_WIDTH>(txData_internal,
										 txData);

	client<DATA_WIDTH>(
		txMetaDataBuffer,
		txDataBuffer,
		// txStatusBuffer,
		esa_sessionidFifo,
		esa_lengthFifo,
		esa_dataFifo);

	if (!txStatus.empty()) // Make Checks
	{
		txStatus.read();
	}

	/*
	 * Server
	 */
	server<DATA_WIDTH>(listenPort,
					   listenPortStatus,
					   notifications,
					   readRequest,
					   rxMetaData,
					   rxData_internal,
					   esa_sessionidFifo,
					   esa_lengthFifo,
					   esa_dataFifo);
}
