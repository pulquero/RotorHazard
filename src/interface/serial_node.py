'''RotorHazard serial interface layer.'''
import logging
import serial # For serial comms
import gevent
import time

from interface import pack_8
from . import RHInterface as rhi

BOOTLOADER_CHILL_TIME = 2 # Delay for USB to switch from bootloader to serial mode
SERIAL_BAUD_RATES = [921600, 500000, 115200]
DEF_S32BPILL_SERIAL_PORT = "/dev/serial0"

logger = logging.getLogger(__name__)


class SerialNodeManager(rhi.RHNodeManager):
    def __init__(self, serial_obj):
        super().__init__()
        self.serial_io = serial_obj
        self.addr = 'serial:'+self.serial_io.port

    def _read_command(self, command, size):
        self.serial_io.flushInput()
        self.serial_io.write(bytearray([command]))
        return bytearray(self.serial_io.read(size + 1))

    def _write_command(self, command, data):
        data_with_cmd = bytearray()
        data_with_cmd.append(command)
        data_with_cmd.extend(data)
        self.serial_io.write(data_with_cmd)

    def close(self):
        self.serial_io.close()

    def jump_to_bootloader(self):
        try:
            if self.api_level >= 32:
                logger.info('Sending JUMP_TO_BOOTLOADER message to serial node {0}'.format(self))
                self.write_command(rhi.JUMP_TO_BOOTLOADER, pack_8(0))
                self.serial_io.flushInput()
                time.sleep(0.1)
                self.serial_io.flushInput()
                self.serial_io.flushOutput()
                self.serial_io.close()
        except Exception as ex:
            logger.error('Error sending JUMP_TO_BOOTLOADER message to serial node {0}: {1}'.format(self, ex))


def discover(idxOffset, config, isS32BPillFlag=False, *args, **kwargs):
    node_managers = []
    config_ser_ports = getattr(config, 'SERIAL_PORTS', [])
    if isS32BPillFlag and len(config_ser_ports) == 0:
        config_ser_ports.append(DEF_S32BPILL_SERIAL_PORT)
        logger.debug("Using default serial port ('{}') for S32_BPill board".format(DEF_S32BPILL_SERIAL_PORT))
    if config_ser_ports:
        next_index = idxOffset
        for comm in config_ser_ports:
            for baudrate in SERIAL_BAUD_RATES:
                logger.info("Trying {} with baud rate {}".format(comm, baudrate))
                serial_obj = serial.Serial(port=None, baudrate=baudrate, timeout=0.25)
                serial_obj.setDTR(0)  # clear in case line is tied to node-processor reset
                serial_obj.setRTS(0)
                serial_obj.setPort(comm)
                serial_obj.open()  # open port (now that DTR is configured for no change)
                gevent.sleep(BOOTLOADER_CHILL_TIME)  # delay needed for Arduino USB
                node_manager = SerialNodeManager(serial_obj)
                if node_manager.discover_nodes(next_index):
                    logger.info('{} node(s) with API level {} found at baudrate {}'.format(len(node_manager.nodes), node_manager.api_level, baudrate))
                    next_index += len(node_manager.nodes)
                    node_managers.append(node_manager)
                else:
                    serial_obj.close()
    return node_managers
