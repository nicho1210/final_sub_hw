set ADC_InClk_period 2.500; # 400MHz max frequency expecte for ADC_InClk_period
create_clock -period $ADC_InClk_period -name ADC_InClk -waveform {0.000 1.250} -add [get_ports ADC_InClk]
