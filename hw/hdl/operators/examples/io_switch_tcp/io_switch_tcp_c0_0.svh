// Tie-off
always_comb axi_ctrl.tie_off_s();
// always_comb axis_card_0_src.tie_off_m();
// always_comb axis_host_0_sink.tie_off_s();

always_comb tcp_0_listen_req.tie_off_m();
always_comb tcp_0_listen_rsp.tie_off_s();
always_comb tcp_0_open_req.tie_off_m();
always_comb tcp_0_open_rsp.tie_off_s();
always_comb tcp_0_close_req.tie_off_m();
always_comb tcp_0_notify.tie_off_s();
always_comb tcp_0_rd_pkg.tie_off_m();
always_comb tcp_0_rx_meta.tie_off_s();
always_comb tcp_0_tx_meta.tie_off_m();
always_comb tcp_0_tx_stat.tie_off_s();

always_comb tcp_1_listen_req.tie_off_m();
always_comb tcp_1_listen_rsp.tie_off_s();
always_comb tcp_1_open_req.tie_off_m();
always_comb tcp_1_open_rsp.tie_off_s();
always_comb tcp_1_close_req.tie_off_m();
always_comb tcp_1_notify.tie_off_s();
always_comb tcp_1_rd_pkg.tie_off_m();
always_comb tcp_1_rx_meta.tie_off_s();
always_comb tcp_1_tx_meta.tie_off_m();
always_comb tcp_1_tx_stat.tie_off_s();


// I/O
AXI4SR axis_sink_int ();
AXI4SR axis_src_int ();

axisr_reg inst_reg_sink (.aclk(aclk), .aresetn(aresetn), .s_axis(axis_switch_mem_0_sink), .m_axis(axis_sink_int));
axisr_reg inst_reg_src (.aclk(aclk), .aresetn(aresetn), .s_axis(axis_src_int), .m_axis(axis_switch_mem_0_src));

// UL
// assign axis_src_int.tvalid  = 1;
// // assign axis_src_int.tdata = {512{1'b1}}; 
// // assign axis_src_int.tdata = {{256{1'b1}}, {256{1'b0}}};
// assign axis_src_int.tdata = axis_sink_int.tdata;
// assign axis_src_int.tkeep   = 64'hffffffffffffffff;
// assign axis_src_int.tid     = 0;
// assign axis_src_int.tlast   = 1;

always_comb begin
    axis_src_int.tvalid  = axis_sink_int.tvalid;
    axis_src_int.tdata = axis_sink_int.tdata + 1;
    axis_src_int.tkeep   = axis_sink_int.tkeep;
    axis_src_int.tid     = axis_sink_int.tid;
    axis_src_int.tlast   = axis_sink_int.tlast;
    axis_sink_int.tready = axis_src_int.tready;
end
