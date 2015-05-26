import struct
from crc16 import crc16
from traceback import print_exc

class ModbusServer:
    def __init__(self, serial, address):
        self.serial = serial
        self.address = address

    def start(self):
        while True:
            try:
                b = self.serial.read(8)
                addr,func, v0, v1, crc = struct.unpack('>BBHHH', b)
                if addr != 0x01:
                    continue
                crc2 = crc16(b[:-2])
                if crc != crc2:
                    raise Exception('CRC error')
                if func == 0x03 or func == 0x04:
                    r = self.handle_read(func, v0, v1)
                    b = struct.pack('>BBB', self.address, func, 2 * len(r))
                elif func == 0x06:
                    r = self.handle_write(func, v0, v1)
                    b = struct.pack('>BB', self.address, func)
                if r == None:
                    b = struct.pack('>BBB', self.address, 0x83, 0xFF)
                else:
                    for c in r:
                        b += struct.pack('>H', c)
                crc = crc16(b)
                b += struct.pack('>H', crc)
                self.serial.write(b)
            except KeyboardInterrupt:
                raise
            except:
                print_exc()

    def handle_read(self, fun, start_addr, size):
        return None

    def handle_write(self, fun, address, value):
        return None
