import simics

# Extend this function if your device requires any additional attributes to be
# set. It is often sensible to make additional arguments to this function
# optional, and let the function create mock objects if needed.
def create_forward_receiver(name = None):
    '''Create a new forward_receiver object'''
    forward_receiver = simics.pre_conf_object(name, 'forward_receiver')
    simics.SIM_add_configuration([forward_receiver], None)
    return simics.SIM_get_object(forward_receiver.name)
