from em_sim import EmSim
from modbus_server import ModbusServer
import serial
import struct
import time


class Em24Server(ModbusServer):
    def __init__(self, serial, address, em_sim):
        ModbusServer.__init__(self, serial, address)
        self.em_sim = em_sim
        self.values = {
            0x000B : 71, # Device ID
            0x0302 : 75, # Device subtype
            0x0303 : 2, # Version code
            0x0304 : 0, # Front selector
            0x1101 : 0, # Application
            0x1102 : 0  # Measurement system
        }
        serial = 'BO24700610298\0'
        for i in range(7):
            self.values[0x1300 + i] = (ord(serial[2 * i]) << 8) | ord(serial[2 * i + 1])
        self.update_values()

    def update_values(self):
        self._lastTimeStamp = time.clock()
        self.update_value(self.em_sim.l1.voltage, 0x0000, 10, 2)
        self.update_value(self.em_sim.l2.voltage, 0x0002, 10, 2)
        self.update_value(self.em_sim.l3.voltage, 0x0004, 10, 2)
        self.update_value(self.em_sim.l1.current, 0x000C, 1000, 2)
        self.update_value(self.em_sim.l2.current, 0x000E, 1000, 2)
        self.update_value(self.em_sim.l3.current, 0x0010, 1000, 2)
        self.update_value(self.em_sim.l1.power, 0x0012, 10, 2)
        self.update_value(self.em_sim.l2.power, 0x0014, 10, 2)
        self.update_value(self.em_sim.l3.power, 0x0016, 10, 2)
        self.update_value(self.em_sim.l1.energy, 0x0046, 10, 2)
        self.update_value(self.em_sim.l2.energy, 0x0048, 10, 2)
        self.update_value(self.em_sim.l3.energy, 0x004A, 10, 2)
        self.update_value((
            self.em_sim.l1.voltage +
            self.em_sim.l2.voltage +
            self.em_sim.l3.voltage)/3, 0x0024, 10, 2)
        self.update_value(self.em_sim.l2.voltage, 0x0002, 10, 2)
        self.update_value(self.em_sim.l3.voltage, 0x0004, 10, 2)
        self.update_value(
            self.em_sim.l1.power + \
            self.em_sim.l2.power + \
            self.em_sim.l3.power, 0x0028, 10, 2)
        self.update_value(
            self.em_sim.l1.energy + \
            self.em_sim.l2.energy + \
            self.em_sim.l3.energy, 0x0042, 10, 2)
        self.update_value(
            self.em_sim.l1.energy_reverse + \
            self.em_sim.l2.energy_reverse + \
            self.em_sim.l3.energy_reverse, 0x005C, 10, 2)

    def update_value(self, value, addr, factor, length):
        iv = int(value * factor)
        shift = 0
        for i in range(length):
            self.values[addr + i] = (iv >> shift) & 0xFFFF
            shift += 16

    def handle_read(self, fun, start_addr, size):
        if fun != 3 and fun != 4:
            return None
        t = time.clock()
        if t - self._lastTimeStamp > 0.5:
            self.update_values()
        return [self.values.get(x, 0) for x in range(start_addr, start_addr + size)]

    def handle_write(self, fun, address, value):
        if fun != 6:
            return None
        self.values[address] = value
        return [address, value]


class Em112Server(ModbusServer):
    def __init__(self, serial, address, em_sim):
        ModbusServer.__init__(self, serial, address)
        self.em_sim = em_sim
        self.values = {
            0x000B : 104, # Device ID
            #0x0302 : 75, # Device subtype
            #0x0303 : 2, # Version code
            #0x0304 : 0, # Front selector
            #0x1101 : 0, # Application
            #0x1102 : 0  # Measurement system
        }
        serial = '330002A'
        for i in range(7):
            self.values[0x5000 + i] = ord(serial[i])
        self.update_values()

    def update_values(self):
        self._lastTimeStamp = time.time()
        self.update_value(self.em_sim.l1.voltage, 0x0000, 10, 2)
        self.update_value(self.em_sim.l1.current, 0x0002, 1000, 2)
        self.update_value(self.em_sim.l1.power, 0x0004, 10, 2)
        ef = self.em_sim.l1.energy
        self.update_value(ef, 0x0010, 10, 2)
        self.update_value(ef, 0x0112, 10, 2)
        er = self.em_sim.l1.energy_reverse
        self.update_value(er, 0x0020, 10, 2)
        self.update_value(er, 0x0116, 10, 2)

    def update_value(self, value, addr, factor, length):
        iv = int(value * factor)
        shift = 0
        for i in range(length):
            self.values[addr + i] = (iv >> shift) & 0xFFFF
            shift += 16

    def handle_read(self, fun, start_addr, size):
        if fun != 3 and fun != 4:
            return None
        t = time.time()
        print(t - self._lastTimeStamp)
        if t - self._lastTimeStamp > 0.5:
            self.update_values()
        return [self.values.get(x, 0) for x in range(start_addr, start_addr + size)]

    def handle_write(self, fun, address, value):
        if fun != 6:
            return None
        self.values[address] = value
        return [address, value]


s = serial.Serial(port='/dev/pts/13', baudrate=9600, bytesize=serial.EIGHTBITS, \
                  parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE)
# server = Em24Server(s, 1, EmSim())
server = Em112Server(s, 1, EmSim())
server.start()
