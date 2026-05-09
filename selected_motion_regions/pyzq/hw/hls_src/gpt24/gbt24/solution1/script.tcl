############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
## Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
############################################################
open_project gbt24
set_top gbt24
add_files ../gbt24/gbt24.cpp
add_files ../gbt24/gbt24.h
add_files -tb ../gbt24/tb_gbt24.cpp -cflags "-Wno-unknown-pragmas"
open_solution "solution1" -flow_target vivado
set_part {xc7z020-clg400-1}
create_clock -period 13.5 -name default
config_export -description {RGB888 grayscale and threshold IP} -display_name gbt24 -format ip_catalog -library hls -output C:/selected_topics_final/PynqZ2Video/hw/ip_repo -rtl verilog -taxonomy /UserIP -vendor user.org -version 1.0
source "./gbt24/solution1/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -rtl verilog -format ip_catalog -output C:/selected_topics_final/PynqZ2Video/hw/ip_repo
