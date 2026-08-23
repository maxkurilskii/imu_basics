import struct
import serial
import datetime as dt
import csv
from pathlib import Path
from enum import Enum
from typing import List, Dict, Tuple, Optional, Union


class ImuLogger:
    def __init__(self, filename = None):
        self.file_name = self._generate_filename() if filename is None else filename
        self.create_new_csv(self.file_name)
        self.max_buf_size = 100
        self.data_buffer: list = []
    
    def save_data(self, data: bytearray) -> None:
        # unpack 40 bytes: acc(x,y,z) -> gyro(x,y,z) -> mag(x,y,z) -> time_ms; lit endian(LSB first) 
        acc  = struct.unpack('<3f', data[0:12])    # accel in float (12 bytes)
        gyro = struct.unpack('<3f', data[12:24])   # gyro in float (12 bytes)
        mag  = struct.unpack('<3f', data[24:36])   # magnet in float (12 bytes)
        time = struct.unpack('<I', data[36:40])[0] # meas timestamp in uint32_t (4 bytes)
        # print(f"Accel X:{acc[0]:>9.3f} Y:{acc[1]:>9.3f} Z:{acc[2]:>9.3f}", end='')
        # print(f"|| Gyro X:{gyro[0]:>9.3f} Y:{gyro[1]:>9.3f} Z:{gyro[2]:>9.3f}", end='')
        # print(f"|| Mag X:{mag[0]:>9.3f} Y:{mag[1]:>9.3f} Z:{mag[2]:>9.3f}")
        # print(f"|| Timestamp: {time} ms")
        record = [f"{time:>9}", 
                    *[f"{data:>9.3f}" for data in acc],
                    *[f"{data:>9.3f}" for data in gyro],
                    *[f"{data:>9.3f}" for data in mag]]
        self.data_buffer.append(record)
        
        if len(self.data_buffer) >= self.max_buf_size:
            self.flush_buffer()
    
    def flush_buffer(self):
        with open(self.file_name, mode = 'a', encoding='utf-8', newline='') as f:
            writer = csv.writer(f)
            writer.writerows(self.data_buffer)
        self.data_buffer = []
    
    @staticmethod
    def create_new_csv(fname):
        header = ["time_ms",
                        "A_X", "A_Y", "A_Z", 
                        "G_X", "G_Y", "G_Z", 
                        "M_X", "M_Y", "M_Z"]
        header_formatted = [f"{data:>9}" for data in header]
        with open(fname, mode = 'w', encoding='utf-8', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(header_formatted)
    
    @staticmethod
    def _generate_filename():
        timestamp = dt.datetime.today().strftime("%Y-%m-%d_%H-%M-%S")
        log_dir = Path("log_data")
        log_dir.mkdir(exist_ok=True)
        return log_dir / f"imu_log_{timestamp}.csv"
    
        
if __name__ == "__main__":
    pass
    #print(dt.datetime.today().strftime("%d:%m:%Y %H:%M:%S"))







    
