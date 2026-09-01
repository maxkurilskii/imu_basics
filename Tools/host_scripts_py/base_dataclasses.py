from enum import Enum
from dataclasses import dataclass, field

@dataclass(frozen=True)
class ReadADC:
    cmd_code: bytes = b'\x41' #65
    byte_data_len: bytes =  b'\x02' #2

@dataclass(frozen=True)
class ReadIMU:
    cmd_code: bytes = b'\x42' #66
    byte_data_len: bytes =  b'\x28' #40
    
@dataclass(frozen=True)
class ReadACC_GYRO:
    cmd_code: bytes = b'\x42' #66
    byte_data_len: bytes =  b'\x18' #24
    
@dataclass(frozen=True)
class ReadGYRO:
    cmd_code: bytes = b'\x42' #66
    byte_data_len: bytes =  b'\x04' #4

@dataclass(frozen=True)
class ReadMAG:
    cmd_code: bytes = b'\x42' #66
    byte_data_len: bytes =  b'\x0C' #12

@dataclass(frozen=True)
class ReadTEMP:
    cmd_code: bytes = b'\x43' #66
    byte_data_len: bytes =  b'\x04' #4