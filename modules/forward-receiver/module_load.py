import cli

class_name = 'forward_receiver'

# info command prints static information
def get_info(obj):
    return []

# status command prints dynamic information
def get_status(obj):
    return []

cli.new_info_command(class_name, get_info)
cli.new_status_command(class_name, get_status)
