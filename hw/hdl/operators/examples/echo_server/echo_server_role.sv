`timescale 1ns / 1ps
// `default_nettype none

import lynxTypes::*;

module echo_server_role #(
    parameter integer  C_S_AXI_CONTROL_DATA_WIDTH = 32,
    parameter integer  C_S_AXI_CONTROL_ADDR_WIDTH = 12,
    parameter integer  NETWORK_STACK_WIDTH=512
) (
    input wire      ap_clk,
    input wire      ap_rst_n,

    /* NETWORK  - TCP/IP INTERFACE */
    //Network TCP/IP
    output  wire                                   m_axis_tcp_listen_port_tvalid ,
    input wire                                     m_axis_tcp_listen_port_tready ,
    output  wire [16-1:0]                          m_axis_tcp_listen_port_tdata  ,

    input wire                                     s_axis_tcp_port_status_tvalid ,
    output  wire                                   s_axis_tcp_port_status_tready ,
    input wire [8-1:0]                             s_axis_tcp_port_status_tdata  ,

    output  wire                                   m_axis_tcp_open_connection_tvalid ,
    input wire                                     m_axis_tcp_open_connection_tready ,
    output  wire [48-1:0]                          m_axis_tcp_open_connection_tdata  ,

    input wire                                     s_axis_tcp_open_status_tvalid ,
    output  wire                                   s_axis_tcp_open_status_tready ,
    input wire [128-1:0]                            s_axis_tcp_open_status_tdata  ,

    output  wire                                   m_axis_tcp_close_connection_tvalid ,
    input wire                                     m_axis_tcp_close_connection_tready ,
    output  wire [16-1:0]                          m_axis_tcp_close_connection_tdata  ,

    input wire                                     s_axis_tcp_notification_tvalid ,
    output  wire                                   s_axis_tcp_notification_tready ,
    input wire [88-1:0]                            s_axis_tcp_notification_tdata  ,

    output  wire                                   m_axis_tcp_read_pkg_tvalid ,
    input wire                                     m_axis_tcp_read_pkg_tready ,
    output  wire [32-1:0]                          m_axis_tcp_read_pkg_tdata  ,

    input wire                                     s_axis_tcp_rx_meta_tvalid ,
    output  wire                                   s_axis_tcp_rx_meta_tready ,
    input wire [16-1:0]                            s_axis_tcp_rx_meta_tdata  ,

    input wire                                     s_axis_tcp_rx_data_tvalid ,
    output  wire                                   s_axis_tcp_rx_data_tready ,
    input wire [NETWORK_STACK_WIDTH-1:0]           s_axis_tcp_rx_data_tdata  ,
    input wire [NETWORK_STACK_WIDTH/8-1:0]         s_axis_tcp_rx_data_tkeep  ,
    input wire                                     s_axis_tcp_rx_data_tlast  ,

    output  wire                                   m_axis_tcp_tx_meta_tvalid ,
    input wire                                     m_axis_tcp_tx_meta_tready ,
    output  wire [32-1:0]                          m_axis_tcp_tx_meta_tdata  ,

    output  wire                                   m_axis_tcp_tx_data_tvalid ,
    input wire                                     m_axis_tcp_tx_data_tready ,
    output  wire [NETWORK_STACK_WIDTH-1:0]         m_axis_tcp_tx_data_tdata  ,
    output  wire [NETWORK_STACK_WIDTH/8-1:0]       m_axis_tcp_tx_data_tkeep  ,
    output  wire                                   m_axis_tcp_tx_data_tlast  ,

    input wire                                     s_axis_tcp_tx_status_tvalid ,
    output  wire                                   s_axis_tcp_tx_status_tready ,
    input wire [64-1:0]                            s_axis_tcp_tx_status_tdata
);

echo_server_application_ip esa(
	// .m_axis_close_connection_V_V_TVALID(m_axis_tcp_close_connection_tvalid),      // output wire m_axis_close_connection_TVALID
	// .m_axis_close_connection_V_V_TREADY(m_axis_tcp_close_connection_tready),      // input wire m_axis_close_connection_TREADY
	// .m_axis_close_connection_V_V_TDATA(m_axis_tcp_close_connection_tdata),        // output wire [15 : 0] m_axis_close_connection_TDATA
	.m_axis_listen_port_TVALID(m_axis_tcp_listen_port_tvalid),                // output wire m_axis_listen_port_TVALID
	.m_axis_listen_port_TREADY(m_axis_tcp_listen_port_tready),                // input wire m_axis_listen_port_TREADY
	.m_axis_listen_port_TDATA(m_axis_tcp_listen_port_tdata),                  // output wire [15 : 0] m_axis_listen_port_TDATA
	// .m_axis_open_connection_V_TVALID(m_axis_tcp_open_connection_tvalid),        // output wire m_axis_open_connection_TVALID
	// .m_axis_open_connection_V_TREADY(m_axis_tcp_open_connection_tready),        // input wire m_axis_open_connection_TREADY
	// .m_axis_open_connection_V_TDATA(m_axis_tcp_open_connection_tdata),          // output wire [47 : 0] m_axis_open_connection_TDATA
	.m_axis_read_package_TVALID(m_axis_tcp_read_pkg_tvalid),              // output wire m_axis_read_package_TVALID
	.m_axis_read_package_TREADY(m_axis_tcp_read_pkg_tready),              // input wire m_axis_read_package_TREADY
	.m_axis_read_package_TDATA(m_axis_tcp_read_pkg_tdata),                // output wire [31 : 0] m_axis_read_package_TDATA
	.m_axis_tx_data_TVALID(m_axis_tcp_tx_data_tvalid),                        // output wire m_axis_tx_data_TVALID
	.m_axis_tx_data_TREADY(m_axis_tcp_tx_data_tready),                        // input wire m_axis_tx_data_TREADY
	.m_axis_tx_data_TDATA(m_axis_tcp_tx_data_tdata),                          // output wire [63 : 0] m_axis_tx_data_TDATA
	.m_axis_tx_data_TKEEP(m_axis_tcp_tx_data_tkeep),                          // output wire [7 : 0] m_axis_tx_data_TKEEP
	.m_axis_tx_data_TLAST(m_axis_tcp_tx_data_tlast),                          // output wire [0 : 0] m_axis_tx_data_TLAST
	.m_axis_tx_metadata_TVALID(m_axis_tcp_tx_meta_tvalid),                // output wire m_axis_tx_metadata_TVALID
	.m_axis_tx_metadata_TREADY(m_axis_tcp_tx_meta_tready),                // input wire m_axis_tx_metadata_TREADY
	.m_axis_tx_metadata_TDATA(m_axis_tcp_tx_meta_tdata),                  // output wire [15 : 0] m_axis_tx_metadata_TDATA
	.s_axis_listen_port_status_TVALID(s_axis_tcp_port_status_tvalid),  // input wire s_axis_listen_port_status_TVALID
	.s_axis_listen_port_status_TREADY(s_axis_tcp_port_status_tready),  // output wire s_axis_listen_port_status_TREADY
	.s_axis_listen_port_status_TDATA(s_axis_tcp_port_status_tdata),    // input wire [7 : 0] s_axis_listen_port_status_TDATA
	.s_axis_notifications_TVALID(s_axis_tcp_notification_tvalid),            // input wire s_axis_notifications_TVALID
	.s_axis_notifications_TREADY(s_axis_tcp_notification_tready),            // output wire s_axis_notifications_TREADY
	.s_axis_notifications_TDATA(s_axis_tcp_notification_tdata),              // input wire [87 : 0] s_axis_notifications_TDATA
	// .s_axis_open_status_V_TVALID(s_axis_tcp_open_status_tvalid),                // input wire s_axis_open_status_TVALID
	// .s_axis_open_status_V_TREADY(s_axis_tcp_open_status_tready),                // output wire s_axis_open_status_TREADY
	// .s_axis_open_status_V_TDATA(s_axis_tcp_open_status_tdata),                  // input wire [23 : 0] s_axis_open_status_TDATA
	.s_axis_rx_data_TVALID(s_axis_tcp_rx_data_tvalid),                        // input wire s_axis_rx_data_TVALID
	.s_axis_rx_data_TREADY(s_axis_tcp_rx_data_tready),                        // output wire s_axis_rx_data_TREADY
	.s_axis_rx_data_TDATA(s_axis_tcp_rx_data_tdata),                          // input wire [63 : 0] s_axis_rx_data_TDATA
	.s_axis_rx_data_TKEEP(s_axis_tcp_rx_data_tkeep),                          // input wire [7 : 0] s_axis_rx_data_TKEEP
	.s_axis_rx_data_TLAST(s_axis_tcp_rx_data_tlast),                          // input wire [0 : 0] s_axis_rx_data_TLAST
	.s_axis_rx_metadata_TVALID(s_axis_tcp_rx_meta_tvalid),                // input wire s_axis_rx_metadata_TVALID
	.s_axis_rx_metadata_TREADY(s_axis_tcp_rx_meta_tready),                // output wire s_axis_rx_metadata_TREADY
	.s_axis_rx_metadata_TDATA(s_axis_tcp_rx_meta_tdata),                  // input wire [15 : 0] s_axis_rx_metadata_TDATA
	.s_axis_tx_status_TVALID(s_axis_tcp_tx_status_tvalid),                    // input wire s_axis_tx_status_TVALID
	.s_axis_tx_status_TREADY(s_axis_tcp_tx_status_tready),                    // output wire s_axis_tx_status_TREADY
	.s_axis_tx_status_TDATA(s_axis_tcp_tx_status_tdata),                      // input wire [23 : 0] s_axis_tx_status_TDATA
	
	.ap_clk(ap_clk),                                                          // input wire aclk
	.ap_rst_n(ap_rst_n)                                                    // input wire aresetn
);
    
endmodule