from rh.app.config import Config
from rh.interface import RHInterface, MockInterface


def get_interface(port):
    if port == 'MOCK':
        INTERFACE = MockInterface.get_hardware_interface()
    elif port.startswith('COM') or port.startswith('/dev/'):
        config = Config()
        config.SERIAL_PORTS = [port]
        INTERFACE = RHInterface.get_hardware_interface(config=config)
    elif port.startswith('i2c:'):
        from rh.helpers.i2c_helper import I2CBus
        from rh.helpers import parse_i2c_url

        bus_addr = parse_i2c_url(port)
        params = {}
        params['idxOffset'] = 0
        params['i2c_helper'] = [I2CBus(bus_addr[0])]
        params['i2c_addrs'] = [bus_addr[1]]
        INTERFACE = RHInterface.get_hardware_interface(**params)
    elif port.startswith(':'):
        config = Config()
        config.SOCKET_PORTS = [int(port[1:])]
        INTERFACE = RHInterface.get_hardware_interface(config=config)
    else:
        print("Invalid port: {}".format(port))
        exit(1)
    print("Nodes detected: {}".format(len(INTERFACE.nodes)))
    return INTERFACE
