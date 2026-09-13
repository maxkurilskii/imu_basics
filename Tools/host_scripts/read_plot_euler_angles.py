import argparse
import serial
import struct
import pandas as pd
import numpy as np
from pathlib import Path
import matplotlib.pyplot as plt
from typing import Optional

# from stm32_uart_comm import UartCom
from uart_com import UartCommunicator
from imu_csv_logger import ImuLogger
from base_dataclasses import ReadImuEulerResponce


plt.rcParams["font.size"] = 12
plt.rcParams["axes.grid"] = True
# plt.rcParams["figure.autolayout"] = True


def plot_euler_meas(file):
    
    df = pd.read_csv(file,
                    header=0,
                    names=['Time_s', "Roll", "Pitch", "Yaw"])  
    df['Time_s'] = df['Time_s'] / 1000000
    fig, axes = plt.subplots(3,1, sharex=True, figsize=(10,6))
    tight_par = False
    # Roll
    df.plot(ax=axes[0], x='Time_s', y='Roll', color='blue', linewidth=2, legend=True)
    axes[0].set_ylabel('Roll (deg)')
    axes[0].grid(True, alpha=0.3)
    axes[0].autoscale(enable=None, axis="x", tight=tight_par)
    # Pitch
    df.plot(ax=axes[1], x='Time_s', y='Pitch', color='green', linewidth=2, legend=True)
    axes[1].set_ylabel('Pitch (deg)')
    axes[1].grid(True, alpha=0.3)
    axes[1].autoscale(enable=None, axis="x", tight=tight_par)
    # Yaw
    df.plot(ax=axes[2], x='Time_s', y='Yaw', color='red', linewidth=2, legend=True)
    axes[2].set_ylabel('Yaw (deg)')
    axes[2].set_xlabel('Time (s)')
    axes[2].grid(True, alpha=0.3)
    axes[2].legend(loc='lower left')
    axes[2].autoscale(enable=None, axis="x", tight=tight_par)
    fig.suptitle('Euler Angles After Filtration', fontsize=14, fontweight='bold')
    fig.align_ylabels()
    plt.tight_layout(pad=0.4, w_pad=0.5, h_pad=1.0)
    plt.show()
    

def build_plot(file, a):
    
    file_name = args.get('filename', 0)
    if not file_name: raise ValueError('No file specified')
    file = Path("log_data")/file_name
    print("Processing file: ", file)
    df = pd.read_csv(file,
            header=0,
            names=['Time_s', "Roll", "Pitch", "Yaw"])  
   
def main(args):
    if args.get('cmd', 0) == 'meas':
        print('Getting new measurements, ignoring specified file')
        print('To stop measuremnts collection press Ctrl + C')
        logger = ImuLogger(header = [f"{data:>9}" for data in ['Time_s', 'Roll', 'Pitch', 'Yaw']])
        try:
            # com_master = UartCom("COM4", timeout_sec=0.05)
            # while(com_master._my_serial.is_open):
            #     data: Optional[ReadImuEulerResponce] = com_master.uart_read_imu_euler_data() #blocking!!!
            #     if data is not None: 
            #         logger.save_angle_data(data)
            host_com = UartCommunicator()
            while(True):
                msg: bytes = host_com.uart_read_package() #blocking !!!
                roll, pitch, yaw, timestamp, _ = struct.unpack_from('3f1I1H', msg, offset = 3)
                logger.save_angle_data(ReadImuEulerResponce(roll, pitch, yaw, timestamp))
        except serial.SerialException:
            print("Serial connection lost. Data saved")
        except KeyboardInterrupt:
            print("End event was raised. Data saved")
        finally:  
            logger.flush_buffer()            
            plot_euler_meas(logger.file_name)
    else:
        file_name = args.get('filename', 0)
        if not file_name: raise ValueError('No file specified')
        log_dir = Path("log_data")
        log_dir.mkdir(exist_ok=True)
        file_name = Path(log_dir)/file_name
        if not Path(file_name).exists():
            raise FileNotFoundError(f'No file exists: {file_name} (use "meas" arg to create new)')
        
        print("Processing file: ", file_name)
        plot_euler_meas(file_name)
    

if __name__ == "__main__":
    # Choose file 
    parser = argparse.ArgumentParser(prog = 'Build plots of euler angle measurements')
    parser.add_argument('-f','--filename', default='imu_log_2026-09-09_18-10-56.csv', 
                        help='Choose file to be processed in log_data folder!')
    subparser = parser.add_subparsers(dest='cmd', title='subcommand', help='Collect new measurements from serial (ignore measurements from chosen file)')
    make_meas = subparser.add_parser(name='meas')
    # make_meas.add_argument('-ns', '--no-save', dest='save', action='store_false', default=True, 
    #                     help='Whether to save received measurements (default: True)')
    
    args = vars(parser.parse_args())
    print(args)    
    
    main(args)