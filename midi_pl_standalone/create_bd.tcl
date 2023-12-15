################################################################
# This is a generated script based on design: midi_pl_standalone
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

set_property  ip_repo_paths  [get_property DIRECTORY [current_project]]/../../ip [current_project]
update_ip_catalog
add_files -norecurse [get_property DIRECTORY [current_project]]/../hdl/transpose_midi.v


namespace eval _tcl {
proc get_script_folder {} {
   set script_path [file normalize [info script]]
   set script_folder [file dirname $script_path]
   return $script_folder
}
}
variable script_folder
set script_folder [_tcl::get_script_folder]

################################################################
# Check if script is running in correct Vivado version.
################################################################
set scripts_vivado_version 2023.2
set current_vivado_version [version -short]

if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
   puts ""
   catch {common::send_gid_msg -ssname BD::TCL -id 2041 -severity "ERROR" "This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Please run the script in Vivado <$scripts_vivado_version> then open the design in Vivado <$current_vivado_version>. Upgrade the design by running \"Tools => Report => Report IP Status...\", then run write_bd_tcl to create an updated script."}

   return 1
}

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source midi_pl_standalone_script.tcl


# The design that will be created by this Tcl script contains the following 
# module references:
# transpose_midi

# Please add the sources of those modules before sourcing this Tcl script.

# If there is no project opened, this script will create a
# project, but make sure you do not have an existing project
# <./myproj/project_1.xpr> in the current working folder.

set list_projs [get_projects -quiet]
if { $list_projs eq "" } {
   create_project project_1 myproj -part xc7z020clg400-1
   set_property BOARD_PART www.digilentinc.com:pynq-z1:part0:1.0 [current_project]
}


# CHANGE DESIGN NAME HERE
variable design_name
set design_name midi_pl_standalone

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design $design_name

# Creating design if needed
set errMsg ""
set nRet 0

set cur_design [current_bd_design -quiet]
set list_cells [get_bd_cells -quiet]

if { ${design_name} eq "" } {
   # USE CASES:
   #    1) Design_name not set

   set errMsg "Please set the variable <design_name> to a non-empty value."
   set nRet 1

} elseif { ${cur_design} ne "" && ${list_cells} eq "" } {
   # USE CASES:
   #    2): Current design opened AND is empty AND names same.
   #    3): Current design opened AND is empty AND names diff; design_name NOT in project.
   #    4): Current design opened AND is empty AND names diff; design_name exists in project.

   if { $cur_design ne $design_name } {
      common::send_gid_msg -ssname BD::TCL -id 2001 -severity "INFO" "Changing value of <design_name> from <$design_name> to <$cur_design> since current design is empty."
      set design_name [get_property NAME $cur_design]
   }
   common::send_gid_msg -ssname BD::TCL -id 2002 -severity "INFO" "Constructing design in IPI design <$cur_design>..."

} elseif { ${cur_design} ne "" && $list_cells ne "" && $cur_design eq $design_name } {
   # USE CASES:
   #    5) Current design opened AND has components AND same names.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 1
} elseif { [get_files -quiet ${design_name}.bd] ne "" } {
   # USE CASES: 
   #    6) Current opened design, has components, but diff names, design_name exists in project.
   #    7) No opened design, design_name exists in project.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 2

} else {
   # USE CASES:
   #    8) No opened design, design_name not in project.
   #    9) Current opened design, has components, but diff names, design_name not in project.

   common::send_gid_msg -ssname BD::TCL -id 2003 -severity "INFO" "Currently there is no design <$design_name> in project, so creating one..."

   create_bd_design $design_name

   common::send_gid_msg -ssname BD::TCL -id 2004 -severity "INFO" "Making design <$design_name> as current_bd_design."
   current_bd_design $design_name

}

common::send_gid_msg -ssname BD::TCL -id 2005 -severity "INFO" "Currently the variable <design_name> is equal to \"$design_name\"."

if { $nRet != 0 } {
   catch {common::send_gid_msg -ssname BD::TCL -id 2006 -severity "ERROR" $errMsg}
   return $nRet
}

set bCheckIPsPassed 1
##################################################################
# CHECK IPs
##################################################################
set bCheckIPs 1
if { $bCheckIPs == 1 } {
   set list_check_ips "\ 
mmitti:user:Bus2MIDI:1.0\
mmitti:user:MIDI2Bus:1.0\
mmitti:user:MIDI_Clock:1.0\
xilinx.com:ip:clk_wiz:6.0\
"

   set list_ips_missing ""
   common::send_gid_msg -ssname BD::TCL -id 2011 -severity "INFO" "Checking if the following IPs exist in the project's IP catalog: $list_check_ips ."

   foreach ip_vlnv $list_check_ips {
      set ip_obj [get_ipdefs -all $ip_vlnv]
      if { $ip_obj eq "" } {
         lappend list_ips_missing $ip_vlnv
      }
   }

   if { $list_ips_missing ne "" } {
      catch {common::send_gid_msg -ssname BD::TCL -id 2012 -severity "ERROR" "The following IPs are not found in the IP Catalog:\n  $list_ips_missing\n\nResolution: Please add the repository containing the IP(s) to the project." }
      set bCheckIPsPassed 0
   }

}

##################################################################
# CHECK Modules
##################################################################
set bCheckModules 1
if { $bCheckModules == 1 } {
   set list_check_mods "\ 
transpose_midi\
"

   set list_mods_missing ""
   common::send_gid_msg -ssname BD::TCL -id 2020 -severity "INFO" "Checking if the following modules exist in the project's sources: $list_check_mods ."

   foreach mod_vlnv $list_check_mods {
      if { [can_resolve_reference $mod_vlnv] == 0 } {
         lappend list_mods_missing $mod_vlnv
      }
   }

   if { $list_mods_missing ne "" } {
      catch {common::send_gid_msg -ssname BD::TCL -id 2021 -severity "ERROR" "The following module(s) are not found in the project: $list_mods_missing" }
      common::send_gid_msg -ssname BD::TCL -id 2022 -severity "INFO" "Please add source files for the missing module(s) above."
      set bCheckIPsPassed 0
   }
}

if { $bCheckIPsPassed != 1 } {
  common::send_gid_msg -ssname BD::TCL -id 2023 -severity "WARNING" "Will not continue with creation of design due to the error(s) above."
  return 3
}

##################################################################
# DESIGN PROCs
##################################################################



# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  variable script_folder
  variable design_name

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports

  # Create ports
  set clock [ create_bd_port -dir I -type clk clock ]
  set midi_uart_in [ create_bd_port -dir I midi_uart_in ]
  set midi_uart_out [ create_bd_port -dir O midi_uart_out ]

  # Create instance: Bus2MIDI_0, and set properties
  set Bus2MIDI_0 [ create_bd_cell -type ip -vlnv mmitti:user:Bus2MIDI:1.0 Bus2MIDI_0 ]

  # Create instance: MIDI2Bus_0, and set properties
  set MIDI2Bus_0 [ create_bd_cell -type ip -vlnv mmitti:user:MIDI2Bus:1.0 MIDI2Bus_0 ]

  # Create instance: MIDI_Clock_0, and set properties
  set MIDI_Clock_0 [ create_bd_cell -type ip -vlnv mmitti:user:MIDI_Clock:1.0 MIDI_Clock_0 ]
  set_property CONFIG.CLK_BUS_NUM {2} $MIDI_Clock_0


  # Create instance: clk_wiz_0, and set properties
  set clk_wiz_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0 ]
  set_property -dict [list \
    CONFIG.CLKOUT1_JITTER {365.761} \
    CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {8} \
    CONFIG.MMCM_CLKOUT0_DIVIDE_F {118.750} \
    CONFIG.RESET_PORT {resetn} \
    CONFIG.RESET_TYPE {ACTIVE_LOW} \
    CONFIG.USE_RESET {false} \
  ] $clk_wiz_0


  # Create instance: transpose_midi_0, and set properties
  set block_name transpose_midi
  set block_cell_name transpose_midi_0
  if { [catch {set transpose_midi_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2095 -severity "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $transpose_midi_0 eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2096 -severity "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create interface connections
  connect_bd_intf_net -intf_net MIDI2Bus_0_MIDI_OUT [get_bd_intf_pins MIDI2Bus_0/MIDI_OUT] [get_bd_intf_pins transpose_midi_0/midi_in]
  connect_bd_intf_net -intf_net MIDI_Clock_0_MidiClock_1 [get_bd_intf_pins Bus2MIDI_0/MidiClock] [get_bd_intf_pins MIDI_Clock_0/MidiClock_1]
  connect_bd_intf_net -intf_net MIDI_Clock_0_MidiClock_2 [get_bd_intf_pins MIDI2Bus_0/MidiClock] [get_bd_intf_pins MIDI_Clock_0/MidiClock_2]
  connect_bd_intf_net -intf_net transpose_midi_0_midi_out [get_bd_intf_pins Bus2MIDI_0/MIDI_IN] [get_bd_intf_pins transpose_midi_0/midi_out]

  # Create port connections
  connect_bd_net -net Bus2MIDI_0_midi_uart_out [get_bd_ports midi_uart_out] [get_bd_pins Bus2MIDI_0/midi_uart_out]
  connect_bd_net -net clk_in1_0_1 [get_bd_ports clock] [get_bd_pins Bus2MIDI_0/aclk] [get_bd_pins MIDI2Bus_0/aclk] [get_bd_pins clk_wiz_0/clk_in1] [get_bd_pins transpose_midi_0/aclk]
  connect_bd_net -net clk_wiz_0_clk_out1 [get_bd_pins MIDI_Clock_0/pll_clk_in] [get_bd_pins clk_wiz_0/clk_out1]
  connect_bd_net -net clk_wiz_0_locked [get_bd_pins Bus2MIDI_0/aresetn] [get_bd_pins MIDI2Bus_0/aresetn] [get_bd_pins MIDI_Clock_0/locked] [get_bd_pins clk_wiz_0/locked] [get_bd_pins transpose_midi_0/aresetn]
  connect_bd_net -net midi_uart_in_0_1 [get_bd_ports midi_uart_in] [get_bd_pins MIDI2Bus_0/midi_uart_in]

  # Create address segments


  # Restore current instance
  current_bd_instance $oldCurInst

  save_bd_design
}
# End of create_root_design()


##################################################################
# MAIN FLOW
##################################################################
create_root_design ""
common::send_gid_msg -ssname BD::TCL -id 2053 -severity "WARNING" "This Tcl script was generated from a block design that has not been validated. It is possible that design <$design_name> may result in errors during validation."

