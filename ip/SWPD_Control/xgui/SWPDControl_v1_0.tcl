# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "MCU_CLK_DIVIDE" -parent ${Page_0}
  ipgui::add_param $IPINST -name "MCU_WAIT_CYCLE" -parent ${Page_0}
  ipgui::add_static_text $IPINST -name "Note" -parent ${Page_0} -text {MCU Clock[Hz] = ACLK[Hz] / 2^MCU_CLK_DIVIDE

MCU Wait Delay[s] = MCU_WAIT_CYCLE/MCU Clock[Hz]

}


}

proc update_PARAM_VALUE.MCU_CLK_DIVIDE { PARAM_VALUE.MCU_CLK_DIVIDE } {
	# Procedure called to update MCU_CLK_DIVIDE when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.MCU_CLK_DIVIDE { PARAM_VALUE.MCU_CLK_DIVIDE } {
	# Procedure called to validate MCU_CLK_DIVIDE
	return true
}

proc update_PARAM_VALUE.MCU_WAIT_CYCLE { PARAM_VALUE.MCU_WAIT_CYCLE } {
	# Procedure called to update MCU_WAIT_CYCLE when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.MCU_WAIT_CYCLE { PARAM_VALUE.MCU_WAIT_CYCLE } {
	# Procedure called to validate MCU_WAIT_CYCLE
	return true
}

proc update_PARAM_VALUE.C_S_AXI_DATA_WIDTH { PARAM_VALUE.C_S_AXI_DATA_WIDTH } {
	# Procedure called to update C_S_AXI_DATA_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_S_AXI_DATA_WIDTH { PARAM_VALUE.C_S_AXI_DATA_WIDTH } {
	# Procedure called to validate C_S_AXI_DATA_WIDTH
	return true
}

proc update_PARAM_VALUE.C_S_AXI_ADDR_WIDTH { PARAM_VALUE.C_S_AXI_ADDR_WIDTH } {
	# Procedure called to update C_S_AXI_ADDR_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_S_AXI_ADDR_WIDTH { PARAM_VALUE.C_S_AXI_ADDR_WIDTH } {
	# Procedure called to validate C_S_AXI_ADDR_WIDTH
	return true
}

proc update_PARAM_VALUE.C_S_AXI_BASEADDR { PARAM_VALUE.C_S_AXI_BASEADDR } {
	# Procedure called to update C_S_AXI_BASEADDR when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_S_AXI_BASEADDR { PARAM_VALUE.C_S_AXI_BASEADDR } {
	# Procedure called to validate C_S_AXI_BASEADDR
	return true
}

proc update_PARAM_VALUE.C_S_AXI_HIGHADDR { PARAM_VALUE.C_S_AXI_HIGHADDR } {
	# Procedure called to update C_S_AXI_HIGHADDR when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_S_AXI_HIGHADDR { PARAM_VALUE.C_S_AXI_HIGHADDR } {
	# Procedure called to validate C_S_AXI_HIGHADDR
	return true
}


proc update_MODELPARAM_VALUE.C_S_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_S_AXI_DATA_WIDTH PARAM_VALUE.C_S_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.C_S_AXI_DATA_WIDTH}] ${MODELPARAM_VALUE.C_S_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_S_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_S_AXI_ADDR_WIDTH PARAM_VALUE.C_S_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.C_S_AXI_ADDR_WIDTH}] ${MODELPARAM_VALUE.C_S_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.MCU_CLK_DIVIDE { MODELPARAM_VALUE.MCU_CLK_DIVIDE PARAM_VALUE.MCU_CLK_DIVIDE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.MCU_CLK_DIVIDE}] ${MODELPARAM_VALUE.MCU_CLK_DIVIDE}
}

proc update_MODELPARAM_VALUE.MCU_WAIT_CYCLE { MODELPARAM_VALUE.MCU_WAIT_CYCLE PARAM_VALUE.MCU_WAIT_CYCLE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.MCU_WAIT_CYCLE}] ${MODELPARAM_VALUE.MCU_WAIT_CYCLE}
}

