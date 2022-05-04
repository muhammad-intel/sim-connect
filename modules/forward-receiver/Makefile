#                                                              -*- Makefile -*-
# Simics module makefile
#

# USER-TODO: specify the class(es) implemented in this module
MODULE_CLASSES=forward_receiver
#CXL_RELAY_ROOT=/nfs/site/disks/xpg.sprmccfe.0019/rahman3x/simics_training/cxl_relay_new
CXL_RELAY_SRC_DIR=$(CXL_RELAY_ROOT)
CXL_RELAY_SRC=$(CXL_RELAY_SRC_DIR)/dml_cxl_relay_interface.c $(CXL_RELAY_SRC_DIR)/pcie_tlp_pkt_lib.c $(CXL_RELAY_SRC_DIR)/cxl_tlp_fifo.c $(CXL_RELAY_SRC_DIR)/named_pipe.c $(CXL_RELAY_SRC_DIR)/dml_debug.c 

# USER-TODO: set file-names
SRC_FILES = forward-receiver.c $(CXL_RELAY_SRC)
#PCIE_INCS := -I/nfs/site/disks/xpg.sprmccfe.0019/rahman3x/simics_training/cxl_relay_new -I${VCS_HOME}/include
PCIE_INCS := -I$(CXL_RELAY_SRC_DIR) -I${VCS_HOME}/include
CFLAGS += -Wall  $(PCIE_INCS)
PYTHON_FILES = module_load.py

MODULE_CFLAGS =

SIMICS_API := 6
THREAD_SAFE:=yes

ifeq ($(MODULE_MAKEFILE),)
$(error Make sure you compile your module from the project directory)
else
include $(MODULE_MAKEFILE)
endif
