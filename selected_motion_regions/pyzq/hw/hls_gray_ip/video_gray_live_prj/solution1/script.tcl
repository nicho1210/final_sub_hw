############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
## Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
############################################################
open_project video_gray_live_prj
set_top video_gray_live
add_files video_gray_live.cpp
open_solution "solution1" -flow_target vivado
set_part {xc7z020-clg400-1}
create_clock -period 10 -name default
config_export -format ip_catalog -output C:/selected_topics_hdmi/pyzq/hw/hls_gray_ip/hls_export -rtl verilog
source "./video_gray_live_prj/solution1/directives.tcl"
#csim_design
csynth_design
#cosim_design
export_design -rtl verilog -format ip_catalog -output C:/selected_topics_hdmi/pyzq/hw/hls_gray_ip/hls_export
