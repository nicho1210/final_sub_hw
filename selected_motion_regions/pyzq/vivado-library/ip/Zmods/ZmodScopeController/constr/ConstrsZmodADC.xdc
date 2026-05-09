#$kSamplingPeriod
# Disable timing analysis for clock domain crossing dedicated modules
set_false_path -through [get_pins -filter {NAME =~ *SyncAsync*/oSyncStages_reg[*]/D} -hier]
set_false_path -through [get_pins -filter {NAME =~ *SyncAsync*/oSyncStages*/PRE || NAME =~ *SyncAsync*/oSyncStages*/CLR} -hier]

set_false_path -through [get_pins -filter {NAME =~ *InstHandshake*/*/CLR} -hier]
set_false_path -from [get_cells -hier -filter {NAME =~ *InstHandshake*/iData_int_reg[*]}] -to [get_cells -hier -filter {NAME=~ *InstHandshake*/oData_reg[*]}]

# Disable timing analysis on the InstADC_ClkODDR primitive reset input. 
set_false_path -rise_from [get_pins -hier -filter {NAME =~ *InstADC_InClkReset*/SyncAsyncx/oSyncStages_reg[1]/C}] -fall_to [get_pins -hier -filter {NAME=~ *InstADC_ClkODDR*/R}]


#
create_generated_clock -name ZmodAdcClkIn -source [get_pins InstADC_ClkODDR/C] -add -master_clock [get_clocks -of [get_ports ADC_InClk]] -divide_by 1 [get_ports ZmodAdcClkIn_p]

#DCO Clock period  
set tDCO [get_property CLKIN1_PERIOD [get_cells InstDataPath/MMCME2_ADV_inst]];   
set tDCO_half [expr $tDCO/2];
create_clock -period $tDCO -name ZmodDcoClk -waveform "0.000 $tDCO_half" [get_ports ZmodDcoClk]


#Specify timing parameters for AD9648 in CMOS mode
set tskew_max 1.000;      
set tskew_min  -1.200;  
#Reg 0x17 setting 
set OutputDelay  1.12;     

set_input_delay -clock [get_clocks ZmodDcoClk] -clock_fall -min -add_delay [expr $tskew_min - $OutputDelay + $tDCO_half] [get_ports {dZmodADC_Data[*]} -prop_thru_buffers]
set_input_delay -clock [get_clocks ZmodDcoClk] -clock_fall -max -add_delay [expr $tskew_max - $OutputDelay + $tDCO_half] [get_ports {dZmodADC_Data[*]} -prop_thru_buffers]
set_input_delay -clock [get_clocks ZmodDcoClk] -min -add_delay [expr $tskew_min - $OutputDelay + $tDCO_half] [get_ports {dZmodADC_Data[*]} -prop_thru_buffers]
set_input_delay -clock [get_clocks ZmodDcoClk] -max -add_delay [expr $tskew_max - $OutputDelay + $tDCO_half] [get_ports {dZmodADC_Data[*]} -prop_thru_buffers]

set_false_path -fall_from [get_pins -hier -filter {NAME =~ *InstADC_InClkReset*/SyncAsyncx/oSyncStages_reg[1]/C}] -to [get_pins -hier -filter {NAME=~ *InstADC_ClkODDR*/R}]
set_false_path -rise_from [get_pins -hier -filter {NAME =~ *InstADC_InClkReset*/SyncAsyncx/oSyncStages_reg[1]/C}] -fall_to [get_pins -hier -filter {NAME=~ *InstADC_ClkODDR*/R}]
set_false_path -setup -rise_from [get_pins -hier -filter {NAME =~ *InstADC_InClkReset*/SyncAsyncx/oSyncStages_reg[1]/C}] -fall_to [get_pins -hier -filter {NAME=~ *InstADC_ClkODDR*/R}]
