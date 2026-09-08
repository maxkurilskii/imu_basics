# Package/msg protocol description based on uart


## Basic structure (under development)
| Start byte |     CMD    | DATA_BYTES_NUM |  DATA  |  CRC  |   
|:----------:|:----------:|:--------------:|:------:|:-----:|

`START_BYTE` = 35 (0x23)

Possible commands and related hex byte:
Read - 82 (0x52)
Callibrate - 67 (0x43)        
Stop - 83  (0x53)    

## Read only mode (test)

To read ADC data 2 bytes data is enought (ADC data 12 bytes -> 0 - 4095)
| Start byte |     CMD    | DATA_BYTES_NUM |  DATA  |  CRC  |   
|:----------:|:----------:|:--------------:|:------:|:-----:|
|0x52        | Read(0x52) |2               |0-4095  |not used


