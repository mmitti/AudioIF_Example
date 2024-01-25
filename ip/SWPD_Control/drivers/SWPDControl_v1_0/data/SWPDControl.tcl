proc generate {drv_handle} {
	xdefine_include_file $drv_handle "xparameters.h" "SWPDControl" "NUM_INSTANCES" "DEVICE_ID"  "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
	# https://github.com/Xilinx/embeddedsw/blob/master/XilinxProcessorIPLib/drivers/tmrctr/data/tmrctr.tcl
	set file_handle [::hsi::utils::open_include_file "xparameters.h"]
	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
	set periph [lindex $periphs 0]
	puts $file_handle "#define XPAR_SWPD_CONTROL_REG_BASEADDR [::hsi::utils::get_ip_param_name $periph C_S_AXI_BASEADDR]"
	puts $file_handle "\n/******************************************************************/\n"
	close $file_handle
}
