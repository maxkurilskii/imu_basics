import struct
import serial
import time
from enum import Enum

from typing import List, Dict, Tuple, Optional, Union
from base_dataclasses import ReadIMU, ReadACC_GYRO, ReadMAG #, ReadADC, ReadTEMP
from imu_csv_logger import ImuLogger
from crc16 import calculate_crc16


class CommandID(Enum):
    READ_IMU = 1
    READ_ACC_GYRO = 2
    READ_MAG = 3
    
control_commands = {CommandID.READ_IMU: ReadIMU(), 
                    CommandID.READ_ACC_GYRO: ReadACC_GYRO(),
                    CommandID.READ_MAG: ReadMAG()} 

class ParseStates(Enum):
    WAIT_START = 0
    WAIT_CMD = 1
    WAIT_LEN = 2
    WAIT_DATA = 3
    WAIT_CRC = 4 
    FINISHED = 5
    ERROR = 6

class UartStreamParser:
    def __init__(self, control_commands: dict) -> None:    
        self._current_msg_state = ParseStates.WAIT_START
        self.start_byte = 35 #b'\x23' or b'#'
        self._commands: dict = control_commands #database of all current commands
        self._control_cmd = None
        
        #protocol payload bytes: [start, cmd, data_len_byte, data, crc_h, crc_l] 
        self._msg = bytearray() 
        self._data_len_cntr = 0
        self._crc16_len_cntr = 0

    def get_message(self) -> Optional[bytearray]:
        if self._current_msg_state == ParseStates.FINISHED:
            return self._msg
        return None
    
    def reset_all(self):
        self._msg = bytearray() 
        self._data_len_cntr = 0
        self._crc16_len_cntr = 0
        self._current_msg_state = ParseStates.WAIT_START
        # print("Last msg erased!")

    def set_control_cmd(self, cmd_id: CommandID):
        self.reset_all()
        if not isinstance(cmd_id, CommandID):
            raise TypeError("Control command should be CommandID type")
        elif cmd_id not in self._commands:
            raise KeyError("Unknown control command")
        self._control_cmd = self._commands[cmd_id]
        
    def is_done(self):
        return self._current_msg_state in [ParseStates.FINISHED, ParseStates.ERROR]
        
    def process_byte(self, raw_byte: bytes):
        if self._control_cmd is None:
            print("No command was set -> no parsing")
            return
        self._msg += raw_byte
        if  self._current_msg_state == ParseStates.WAIT_START:
            if raw_byte[0] == self.start_byte:
                self._current_msg_state = ParseStates.WAIT_CMD
                
        elif self._current_msg_state == ParseStates.WAIT_CMD:
            if raw_byte[0] == self._control_cmd.cmd_code:
                self._current_msg_state = ParseStates.WAIT_LEN
            else:
                print("Unexpected byte during CMD byte waiting")
                self._current_msg_state = ParseStates.ERROR
                
        elif self._current_msg_state == ParseStates.WAIT_LEN:
            if  raw_byte[0] == self._control_cmd.data_len:
                self._current_msg_state = ParseStates.WAIT_DATA
            else:
                print("WAIT_LEN state NOT succeeded, byte: ", raw_byte)
                self._current_msg_state = ParseStates.ERROR
        
        elif self._current_msg_state == ParseStates.WAIT_DATA:
            self._data_len_cntr += 1
            if  self._data_len_cntr == self._control_cmd.data_len:
                self._current_msg_state = ParseStates.WAIT_CRC
                   
        elif self._current_msg_state == ParseStates.WAIT_CRC:
            self._crc16_len_cntr += 1
            if self._crc16_len_cntr == 2:    
                # print(len(self._msg), list(self._msg),sep='\n') #dbg
                #calculate CRC16 for all payload -> should be equal to zero
                crc16 = calculate_crc16(self._msg, 45) 
                if (crc16 & 0xFFFF) != 0x0000:
                    print("Incorrect CRC!")   
                    self._current_msg_state = ParseStates.ERROR
                    return
                self._current_msg_state = ParseStates.FINISHED


class UartCom:
    def __init__(self, port: str, baud_rate = 115200, timeout_sec = 0.1) -> None:
        self.my_serial  = serial.Serial(port, baud_rate, timeout=timeout_sec)
        self.serial_parser = UartStreamParser(control_commands)
                  
    def uart_read_data(self):
        """
        Infinite reading byte by byte + parsing the received data.
        """
        # self.serial_parser.set_control_cmd(1) DON NOT FORGET!
        while(True):
            raw_byte: bytes = self.my_serial.read(size=1)
            # print("INPUT BYTE: ", raw_byte[0])
            if not raw_byte:
                continue 
            self.serial_parser.process_byte(raw_byte)
   
    def uart_read_imu_data(self) -> Optional[bytearray]:
        """
        Reading is done byte by byte.
        Parsing the performed by MsgParser object until it gets last byte of payload.
        Blocking method!
        Returns None or bytearray o mesaurements data
        with length according to ReadIMU byte_data attribute (40 bytes)
        """        
        self.serial_parser.set_control_cmd(CommandID.READ_IMU)
        while(not self.serial_parser.is_done()):
            raw_byte: bytes = self.my_serial.read(size=1)
            if not raw_byte:  continue 
            self.serial_parser.process_byte(raw_byte)  
        msg = self.serial_parser.get_message()
        if msg is None: return None
        return msg[3:-2] #return only data part (payload)
        

def main():
    try:
        imu_csv_logger = ImuLogger()
        com_master = UartCom("COM4", timeout_sec=0.05)
        meas_cnt = 100
        # while(com_master.my_serial.is_open):
        while(meas_cnt):
            payload = com_master.uart_read_imu_data() #blocking!!!
            if payload is not None: 
                imu_csv_logger.save_data(payload)
                meas_cnt -= 1
    except serial.SerialException:
        print("Serial connection lost. Data saved")
    except KeyboardInterrupt:
        print("End event was raised. Data saved")
    finally:
        imu_csv_logger.flush_buffer()
        print("Completion of program")     
           
           
if __name__ == "__main__":
    main()


