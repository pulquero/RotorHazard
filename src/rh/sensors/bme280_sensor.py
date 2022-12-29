# coding=UTF-8
import logging

from . import I2CSensor, Reading
from smbus2 import SMBus
import bme280


logger = logging.getLogger(__name__)


class BME280Sensor(I2CSensor):
    def __init__(self, name, addr, i2c_bus):
        super().__init__(name=name, i2c_addr=addr, i2c_bus=i2c_bus)
        self.description = 'BME280'
        self._readData()

    def _readData(self):
        with SMBus(self.i2c_bus.id) as bus:
            self.data = bme280.sample(bus, self.i2c_address)

    @Reading(units='°C')
    def temperature(self):
        return self.data.temperature

    @Reading(units='hPa')
    def pressure(self):
        return self.data.pressure

    @Reading(units='%rH')
    def humidity(self):
        return self.data.humidity


def discover(config, i2c_helper, *args, **kwargs):
    sensors = []
    supported_bme280_addrs = [0x76, 0x77]
    for i2c_bus in i2c_helper:
        for addr in supported_bme280_addrs:
            url = i2c_bus.url_of(addr)
            sensor_config = config.get(url, {})
            if sensor_config.get('enabled', True):
                name = sensor_config.get('name', url)
                try:
                    sensors.append(BME280Sensor(name, addr, i2c_bus))
                except IOError:
                    lvl = logging.INFO if sensor_config else logging.DEBUG
                    logger.log(lvl, "No BME280 found on bus {} at address {:#04x}".format(i2c_bus.id, addr))
    return sensors
