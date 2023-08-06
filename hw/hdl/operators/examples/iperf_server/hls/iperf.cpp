/************************************************
Copyright (c) 2018, Systems Group, ETH Zurich.
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

#include "iperf.hpp"

void iperf(hls::stream<ap_uint<16>> &m_axis_listen_port, hls::stream<bool> &s_axis_listen_port_status,
		   // This is disabled for the time being, because it adds complexity/potential issues
		   // stream<ap_uint<16> >& closePort,
		   hls::stream<appNotification> &s_axis_notifications, hls::stream<appReadRequest> &m_axis_read_package,
		   hls::stream<ap_uint<16>> &s_axis_rx_metadata, hls::stream<ap_axiu<DATA_WIDTH, 0, 0, 0>> &s_axis_rx_data,
		   hls::stream<ipTuple> &m_axis_open_connection, hls::stream<openStatus> &s_axis_open_status,
		   hls::stream<ap_uint<16>> &m_axis_close_connection,
		   hls::stream<ap_uint<16>> &m_axis_tx_metadata, hls::stream<ap_axiu<DATA_WIDTH, 0, 0, 0>> &m_axis_tx_data,
		   hls::stream<ap_int<17>> &s_axis_tx_status)
{
#pragma HLS DATAFLOW disable_start_propagation
#pragma HLS INTERFACE ap_ctrl_none port = return

#pragma HLS INTERFACE axis register port = m_axis_listen_port
#pragma HLS INTERFACE axis register port = s_axis_listen_port_status

#pragma HLS INTERFACE axis register port = s_axis_notifications
#pragma HLS INTERFACE axis register port = m_axis_read_package
#pragma HLS DATA_PACK variable = s_axis_notifications
#pragma HLS DATA_PACK variable = m_axis_read_package

#pragma HLS INTERFACE axis register port = s_axis_rx_metadata
#pragma HLS INTERFACE axis register port = s_axis_rx_data

#pragma HLS INTERFACE axis register port = m_axis_open_connection
#pragma HLS INTERFACE axis register port = s_axis_open_status
#pragma HLS DATA_PACK variable = m_axis_open_connection
#pragma HLS DATA_PACK variable = s_axis_open_status

#pragma HLS INTERFACE axis register port = m_axis_close_connection

#pragma HLS INTERFACE axis register port = m_axis_tx_metadata
#pragma HLS INTERFACE axis register port = m_axis_tx_data
#pragma HLS INTERFACE axis register port = s_axis_tx_status
#pragma HLS DATA_PACK variable = m_axis_tx_metadata
#pragma HLS DATA_PACK variable = s_axis_tx_status

	static bool listenDone = false;
	static bool runningExperiment = false;
	static ap_uint<1> listenFsm = 0;

	openStatus newConStatus;
	appNotification notification;
	ipTuple tuple;

	// Open Port 5001
	if (!listenDone)
	{
		switch (listenFsm)
		{
		case 0:
			m_axis_listen_port.write(5001);
			listenFsm++;
			break;
		case 1:
			if (!s_axis_listen_port_status.empty())
			{
				s_axis_listen_port_status.read(listenDone);
				listenFsm++;
			}
			break;
		}
	}

	net_axis<DATA_WIDTH> transmitWord;

	static hls::stream<net_axis<DATA_WIDTH>> s_axis_rx_data_internal;
	static hls::stream<net_axis<DATA_WIDTH>> m_axis_tx_data_internal;

	convert_axis_to_net_axis<DATA_WIDTH>(s_axis_rx_data, s_axis_rx_data_internal);
	convert_net_axis_to_axis<DATA_WIDTH>(m_axis_tx_data_internal, m_axis_tx_data);

	// In case we are connecting back
	if (!s_axis_open_status.empty())
	{
		s_axis_open_status.read(newConStatus);
		m_axis_tx_metadata.write(0);
		transmitWord.data = 0x3736353433323130;
		transmitWord.keep = 0xff;
		transmitWord.last = 1;
		m_axis_tx_data_internal.write(transmitWord);
		if (newConStatus.success)
		{
			m_axis_close_connection.write(newConStatus.sessionID);
		}
	}

	if (!s_axis_notifications.empty())
	{
		s_axis_notifications.read(notification);

		if (notification.length != 0)
		{
			m_axis_read_package.write(appReadRequest(notification.sessionID, notification.length));
		}
		else // closed
		{
			runningExperiment = false;
		}
	}

	enum consumeFsmStateType
	{
		WAIT_PKG,
		CONSUME,
		HEADER_2,
		HEADER_3
	};
	static consumeFsmStateType serverFsmState = WAIT_PKG;
	ap_uint<16> sessionID;
	net_axis<DATA_WIDTH> currWord;
	currWord.last = 0;
	static bool dualTest = false;
	static ap_uint<32> mAmount = 0;

	switch (serverFsmState)
	{
	case WAIT_PKG:
		if (!s_axis_rx_metadata.empty() && !s_axis_rx_data_internal.empty())
		{
			s_axis_rx_metadata.read(sessionID);
			s_axis_rx_data_internal.read(currWord);
			if (!runningExperiment)
			{
				if (currWord.data(31, 0) == 0x00000080) // Dual test
				{
					dualTest = true;
				}
				else
				{
					dualTest = false;
				}

				runningExperiment = true;
				serverFsmState = HEADER_2;
			}
			else
			{
				serverFsmState = CONSUME;
			}
		}
		break;
	case HEADER_2:
		if (!s_axis_rx_data_internal.empty())
		{
			s_axis_rx_data_internal.read(currWord);
			if (dualTest)
			{
				tuple.ip_address = 0x0a010101;
				tuple.ip_port = currWord.data(31, 16);
				m_axis_open_connection.write(tuple);
			}
			serverFsmState = HEADER_3;
		}
		break;
	case HEADER_3:
		if (!s_axis_rx_data_internal.empty())
		{
			s_axis_rx_data_internal.read(currWord);
			mAmount = currWord.data(63, 32);
			serverFsmState = CONSUME;
		}
		break;
	case CONSUME:
		if (!s_axis_rx_data_internal.empty())
		{
			s_axis_rx_data_internal.read(currWord);
		}
		break;
	}
	if (currWord.last == 1)
	{
		serverFsmState = WAIT_PKG;
	}

	if (!s_axis_tx_status.empty())
	{
		s_axis_tx_status.read();
	}
}
