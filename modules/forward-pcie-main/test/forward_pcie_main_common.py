import simics

# Extend this function if your device requires any additional attributes to be
# set. It is often sensible to make additional arguments to this function
# optional, and let the function create mock objects if needed.
def create_forward_pcie_main(name = None):
    '''Create a new forward_pcie_main object'''
    forward_pcie_main = simics.pre_conf_object(name, 'forward_pcie_main')
    simics.SIM_add_configuration([forward_pcie_main], None)
    return simics.SIM_get_object(forward_pcie_main.name)
