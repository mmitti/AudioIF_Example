# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "CLK_BUS_NUM" -parent ${Page_0}
  ipgui::add_param $IPINST -name "SYS_CLK_BASE" -parent ${Page_0}
  ipgui::add_param $IPINST -name "MIDI_CLK_BASE" -parent ${Page_0}
  ipgui::add_param $IPINST -name "KEEP_ARRIVE_BASE" -parent ${Page_0}


}

proc update_PARAM_VALUE.CLK_BUS_NUM { PARAM_VALUE.CLK_BUS_NUM } {
	# Procedure called to update CLK_BUS_NUM when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.CLK_BUS_NUM { PARAM_VALUE.CLK_BUS_NUM } {
	# Procedure called to validate CLK_BUS_NUM
	return true
}

proc update_PARAM_VALUE.KEEP_ARRIVE_BASE { PARAM_VALUE.KEEP_ARRIVE_BASE } {
	# Procedure called to update KEEP_ARRIVE_BASE when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.KEEP_ARRIVE_BASE { PARAM_VALUE.KEEP_ARRIVE_BASE } {
	# Procedure called to validate KEEP_ARRIVE_BASE
	return true
}

proc update_PARAM_VALUE.MIDI_CLK_BASE { PARAM_VALUE.MIDI_CLK_BASE } {
	# Procedure called to update MIDI_CLK_BASE when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.MIDI_CLK_BASE { PARAM_VALUE.MIDI_CLK_BASE } {
	# Procedure called to validate MIDI_CLK_BASE
	return true
}

proc update_PARAM_VALUE.SYS_CLK_BASE { PARAM_VALUE.SYS_CLK_BASE } {
	# Procedure called to update SYS_CLK_BASE when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.SYS_CLK_BASE { PARAM_VALUE.SYS_CLK_BASE } {
	# Procedure called to validate SYS_CLK_BASE
	return true
}


proc update_MODELPARAM_VALUE.SYS_CLK_BASE { MODELPARAM_VALUE.SYS_CLK_BASE PARAM_VALUE.SYS_CLK_BASE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.SYS_CLK_BASE}] ${MODELPARAM_VALUE.SYS_CLK_BASE}
}

proc update_MODELPARAM_VALUE.MIDI_CLK_BASE { MODELPARAM_VALUE.MIDI_CLK_BASE PARAM_VALUE.MIDI_CLK_BASE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.MIDI_CLK_BASE}] ${MODELPARAM_VALUE.MIDI_CLK_BASE}
}

proc update_MODELPARAM_VALUE.KEEP_ARRIVE_BASE { MODELPARAM_VALUE.KEEP_ARRIVE_BASE PARAM_VALUE.KEEP_ARRIVE_BASE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.KEEP_ARRIVE_BASE}] ${MODELPARAM_VALUE.KEEP_ARRIVE_BASE}
}

proc update_MODELPARAM_VALUE.CLK_BUS_NUM { MODELPARAM_VALUE.CLK_BUS_NUM PARAM_VALUE.CLK_BUS_NUM } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.CLK_BUS_NUM}] ${MODELPARAM_VALUE.CLK_BUS_NUM}
}

