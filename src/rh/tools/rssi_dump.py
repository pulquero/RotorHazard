import gevent.monkey 
gevent.monkey.patch_all()

import logging
import sys
from . import get_interface
from rh.interface import RHInterface

def start(port, freq, write_buffer):
    INTERFACE = get_interface(port)

    for node in INTERFACE.nodes:
        INTERFACE.set_mode(node.index, RHInterface.RSSI_HISTORY_MODE)
        INTERFACE.set_frequency(node.index, freq)

    count = 1
    dataBuffer = []
    try:
        while True:
            gevent.sleep(0.1)

            for node in INTERFACE.nodes:
                data = INTERFACE.read_rssi_history(node.index)
                if data is not None and len(data) > 0:
                    for rssi in data:
                        if rssi == 0xFF:  # end of data or no data
                            if len(dataBuffer) > 0:
                                filename = "rssi_dump_{}.csv".format(count)
                                write_buffer(filename, dataBuffer)
                                dataBuffer = []
                                count += 1
                        else:
                            dataBuffer.append(rssi)
    except:
        filename = 'rssi_dump.csv'
        write_buffer(filename, dataBuffer)
        INTERFACE.close()
        raise

def write_buffer(filename, buf):
    with open(filename, 'w') as f:
        for v in buf:
            f.write('{}\n'.format(v))
    print("Written {} ({})".format(filename, len(buf)))


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if len(sys.argv) < 3:
        print('Please specify a serial port, e.g. COM12 (or I2C address, e.g. i2c:1/0x08, or socket port, e.g. :5005), and a frequency.')
        exit()
    port = sys.argv[1]
    freq = int(sys.argv[2])
    start(port, freq, write_buffer)
