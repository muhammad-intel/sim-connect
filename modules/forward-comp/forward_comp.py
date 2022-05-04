# empty_component.py - sample code for a Simics configuration component
# Use this file as a skeleton for your own component implementations.

from comp import (StandardConnectorComponent, PciBusUpConnector,
                  ConfigAttribute, SimpleConfigAttribute)

import connectors


class forward_comp(StandardConnectorComponent):
    """The empty component class."""
    _class_desc      = "empty component"
    _help_categories = ()

    class basename(StandardConnectorComponent.basename):
        ## If no name is given when creating a
        ## new component, this will be used to generate
        ## a name. 
        val = "forward"

    #
    # Configuration
    #
    class use_rtl_mode(SimpleConfigAttribute(0, 'i')):
        """Configure how RTL is connected to the forwarder.
           0 means not at all. 1 means all objects go to RTL.
           2 means something else. """

    #
    # Setup code
    #
    def setup(self):
        super().setup()
        if not self.instantiated.val:
            self.add_objects()
            # PCIe connector
        self.add_connector('pci', PciBusUpConnector(0, 'main'))

    def add_objects(self):
        # Create the device with the PCIe interface
        main = self.add_pre_obj('main', 'forward_pcie_main')
        # Create the forwarder objects
        fpc  = self.add_pre_obj('pci_config_fwd', 'forward_receiver')
        f0   = self.add_pre_obj('bar0_fwd', 'forward_receiver')
        f1   = self.add_pre_obj('bar1_fwd', 'forward_receiver')
        f2   = self.add_pre_obj('bar2_fwd', 'forward_receiver')
        f3   = self.add_pre_obj('bar3_fwd', 'forward_receiver')
        f4   = self.add_pre_obj('bar4_fwd', 'forward_receiver')                                

        # Set up attributes in main object
        fpc.attr.mem_access_type = False
        f0.attr.mem_access_type  = True
        f1.attr.mem_access_type  = True
        f2.attr.mem_access_type  = True
        f3.attr.mem_access_type  = True
        f4.attr.mem_access_type  = True


        main.attr.pci_config_forward = fpc
        main.attr.bar0_object        = f0
        main.attr.bar1_object        = f1
        main.attr.bar2_object        = f2
        main.attr.bar3_object        = f3
        main.attr.bar4_object        = f4

        # Configure RTL
        if(self.use_rtl_mode.val == 0):
            main.attr.use_rtl = False
            fpc.attr.use_rtl  = False
            f0.attr.use_rtl   = False
            f1.attr.use_rtl   = False
            f2.attr.use_rtl   = False
            f3.attr.use_rtl   = False
            f4.attr.use_rtl   = False
        elif(self.use_rtl_mode.val == 1):
            main.attr.use_rtl = True
            fpc.attr.use_rtl  = True
            f0.attr.use_rtl   = True
            f1.attr.use_rtl   = True
            f2.attr.use_rtl   = True
            f3.attr.use_rtl   = True
            f4.attr.use_rtl   = True
        else:
            main.attr.use_rtl = False
            fpc.attr.use_rtl  = False
            f0.attr.use_rtl   = False
            f1.attr.use_rtl   = False
            f2.attr.use_rtl   = False
            f3.attr.use_rtl   = False
            f4.attr.use_rtl   = False
            ##
            ## NOTE - this hard-codes the number of objects
            ##        and their class.  You could imagine making it
            ##        all run-time configurable... but really that 
            ##  
            ## 
