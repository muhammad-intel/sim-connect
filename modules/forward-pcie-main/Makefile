# -*- makefile -*-
#
# Sample makefile for the DML device: "empty-dml-device"
# See Simics Model Builder User's Guide for more details on what to put here.
#

MODULE_CLASSES = forward_pcie_main
SRC_FILES = forward-pcie-main.dml
PYTHON_FILES = module_load.py

SIMICS_API := 6
THREAD_SAFE := yes

# Extra for PCIe 
EXTRA_MODULE_VPATH += dml-lib

ifeq ($(MODULE_MAKEFILE),)
$(error Make sure you compile your module from the project directory)
else
include $(MODULE_MAKEFILE)
endif
