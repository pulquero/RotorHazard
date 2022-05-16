import gevent.monkey 
gevent.monkey.patch_all()

import logging
import sys
from . import get_interface
from rh.util import ms_counter


def start(port, freq):
    INTERFACE = get_interface(port)

    for node in INTERFACE.nodes:
        INTERFACE.set_frequency(node.index, freq)

    while True:
        for node in INTERFACE.nodes:
            adc_val = node.read_adc()
            rssi_val = node.read_rssi()
            print("{}, {}, {}, {}".format(ms_counter(), node, adc_val, rssi_val))


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if len(sys.argv) < 3:
        print('Please specify a serial port, e.g. COM12 (or I2C address, e.g. i2c:1/0x08, or socket port, e.g. :5005), and a frequency.')
        exit()
    port = sys.argv[1]
    freq = int(sys.argv[2])
    start(port, freq)
