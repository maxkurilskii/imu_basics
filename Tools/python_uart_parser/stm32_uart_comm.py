import struct
import serial
import time
from enum import Enum
from base_dataclasses import ReadIMU, ReadACC_GYRO, ReadMAG, ReadADC, ReadTEMP
from typing import List, Dict, Tuple, Optional, Union


class ControlCommands(Enum):
    ReadADC = 1 #b'\x52'
    ReadIMU = 2
    ReadACC_GYRO = 3
    ReadMAG = 4
    ReadTEMP = 5

class ParseStates(Enum):
    WAIT_START = 0
    WAIT_CMD = 1
    WAIT_LEN = 2
    WAIT_DATA = 3
    WAIT_CRC = 4 

class MsgParser:
    def __init__(self, cmd_type: ControlCommands) -> None:
        self.start_byte =  b'\x23' #35 
        
        if cmd_type == ControlCommands.ReadADC:
            self.control_cmd = ReadADC()
        elif cmd_type == ControlCommands.ReadIMU:
            self.control_cmd = ReadIMU()
        elif cmd_type == ControlCommands.ReadACC_GYRO:
            self.control_cmd = ReadACC_GYRO()
        elif cmd_type == ControlCommands.ReadTEMP:
            self.control_cmd = ReadTEMP()
        elif cmd_type == ControlCommands.ReadMAG:
            self.control_cmd = ReadMAG()
        else:
            raise TypeError("Cmd should be ControlCommands type")
        
        self._msg = bytearray(b'') 
        self._data_len = 0
        self._crc_len = 0
        self._current_msg_state = ParseStates.WAIT_START

    def reset_all(self):
        self._msg = bytearray()  
        self._data_len = 0
        self._crc_len = 0
        self._current_msg_state = ParseStates.WAIT_START
        
    @staticmethod
    def calculate_crc16(data_bytes: list):
        pass
        
    def process_byte(self, raw_byte: bytes):
        if  self._current_msg_state == ParseStates.WAIT_START:
            if raw_byte == self.start_byte:
                # print("WAIT_START state succeeded, byte: ", raw_byte)
                self._current_msg_state = ParseStates.WAIT_CMD
                
        elif self._current_msg_state == ParseStates.WAIT_CMD:
            if raw_byte == self.control_cmd.cmd_code:
                # print("WAIT_CMD state succeeded, byte: ", raw_byte)
                self._current_msg_state = ParseStates.WAIT_LEN
            else:
                # print("Unexpected byte during CMD byte waiting")
                self.reset_all()
                
        elif self._current_msg_state == ParseStates.WAIT_LEN:
            if  raw_byte == self.control_cmd.byte_data_len:
                self._current_msg_state = ParseStates.WAIT_DATA
            else:
                print("WAIT_LEN state NOT succeeded, byte: ", raw_byte)
                # print("Unexpected byte during LEN byte waiting")
                self.reset_all()
        
        elif self._current_msg_state == ParseStates.WAIT_DATA:
            self._msg += raw_byte
            self._data_len+=1
            if  self._data_len == self.control_cmd.byte_data_len[0]:
                if isinstance(self.control_cmd, ReadIMU):
                    # unpack 40 bytes: acc(x,y,z) -> gyro(x,y,z) -> mag(x,y,z) -> time_ms; lit endian(LSB first) 
                    acc  = struct.unpack('<3f', self._msg[0:12])    # accel in float (12 bytes)
                    gyro = struct.unpack('<3f', self._msg[12:24])   # gyro in float (12 bytes)
                    mag  = struct.unpack('<3f', self._msg[24:36])   # magnet in float (12 bytes)
                    time = struct.unpack('<I', self._msg[36:40])[0] # meas timestamp in uint32_t (4 bytes)
                    print(f"Accel X:{acc[0]:>9.3f} Y:{acc[1]:>9.3f} Z:{acc[2]:>9.3f}", end='')
                    print(f"|| Gyro X:{gyro[0]:>9.3f} Y:{gyro[1]:>9.3f} Z:{gyro[2]:>9.3f}", end='')
                    print(f"|| Mag X:{mag[0]:>9.3f} Y:{mag[1]:>9.3f} Z:{mag[2]:>9.3f}")
                    # print(f"|| Timestamp: {time} ms")
                elif isinstance(self.control_cmd, ReadACC_GYRO):
                    # unpack 24 bytes: acc(x,y,z) -> gyro(x,y,z); lit endian(LSB first) 
                    acc =  struct.unpack('<3f', self._msg[0:12]) # accel (first 12 bytes)
                    gyro =  struct.unpack('<3f', self._msg[12:]) # gyro (last 12 bytes)
                    print(f"Accel X:{acc[0]:>10.4f}  Y:{acc[1]:>10.4f}  Z:{acc[2]:>10.4f}", end='\t')
                    print(f"||\tGyro X:{gyro[0]:>10.4f}  Y:{gyro[1]:>10.4f}  Z:{gyro[2]:>10.4f}")
                elif isinstance(self.control_cmd, ReadMAG):
                    # unpack 12 bytes:  mag(x,y,z); lit endian(LSB first) from stm
                    mag =  struct.unpack('<3f', self._msg[3:]) 
                    print(f"Magnit X:{mag[0]:>10.4f}  Y:{mag[1]:>10.4f}  Z:{mag[2]:>10.4f}")                    
                elif isinstance(self.control_cmd, ReadTEMP):
                                    # unpack 12 bytes: acc(x,y,z) -> gyro(x,y,z); lit endian(LSB first)
                                    temper =  struct.unpack('<f', self._msg) 
                                    print(f"Temperature (celcius): {temper[0]:>5.5f}")
                                    # print()
                else:
                    print("Your data: ", struct.unpack('<H', self._msg)[0])
                # print("Your data: ", struct.unpack('<H', self._msg[3:-2]))
                self.reset_all()
                
                # self._current_msg_state = ParseStates.WAIT_CRC
                   
        # elif self._current_msg_state == ParseStates.WAIT_CRC:
        #     self._msg.append(raw_byte)
        #     self._crc_len+=1
        #     if self._crc_len == 2:
        #         crc_bytes =  self._msg[-2:]
        #         #only [cmd + len + data] from msg is calculated by crc algo         
        #         crc16_value = self.calculate_crc16(self._msg[1:-2]) 
        #         if crc16_value == crc_bytes:
        #             print("Your data: ", struct.unpack('<H', *self._msg[3:-2]))
        #         else:
        #             print("Incorrect CRC!")
        #     print("New msg searching ....")
        #     self.reset_all()

class UartCom:
    def __init__(self, port, baud_rate = 115200, timeout_sec = 0.1) -> None:
        self.my_serial = serial.Serial(port, baud_rate, timeout=timeout_sec)
        self.adc_parser = MsgParser(ControlCommands.ReadIMU)
    
    # def uart_read_adc(self):
    #     msg = bytearray()
    #     while(True):
    #         raw_byte: bytes = self.my_serial.read(size=1)
    #         if not raw_byte:
    #             continue
    #         msg.append(raw_byte[0])
    #         if len(msg) >= 2:
    #             print("Your data: ", struct.unpack('<H', msg)[0])
    #             msg = bytearray()
    #             self.my_serial.flush()
                  
    def uart_read_adc(self):
        while(True):
            raw_byte: bytes = self.my_serial.read(size=1)
            if not raw_byte:
                # print("NO RAW BYTE")
                # self.adc_parser.reset_all()
                continue 
            # print("INPUT BYTE: ", raw_byte)
            self.adc_parser.process_byte(raw_byte)
   
    # def uart_read_imu(self):
    #     """
    #     Send (not used yet) ReadIMU cmd -> Reading is done byte by byte, parsing the received data.
    #     Blocking method!
    #     """
    #     imu_msg_bytes = 7  #start(1) + cmd(1) + len(1) + data(2) + crc(2)   
    #     while(adc_msg_bytes):
    #         raw_byte: bytes = self.my_serial.read(size=1)
    #         if not raw_byte:
    #             continue  #or should break ? because msg is likely corrupted 
    #         self.adc_parser.process_byte(raw_byte)
    #         adc_msg_bytes-=1
    
    # def uart_exchange(self):
            
            
def run_comm():
    com_master = UartCom("COM4")
    while(com_master.my_serial.is_open):
        com_master.uart_read_adc()

                        
if __name__ == "__main__":
    run_comm()

    # @staticmethod
    # def visualize_data(data):
    #     # print("Your data as bytes:", data)
    #     print("Your data", data)
        
# while(True):
#     res = com_master.uart_read()
#     if res is not None:
#         print(res)
#     else:
#         time.sleep(0.5)
    
    
