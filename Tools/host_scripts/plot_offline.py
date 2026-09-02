import argparse
import csv
import pandas as pd
import numpy as np
from pathlib import Path
from collections import deque
import matplotlib.pyplot as plt
from enum import Enum
from calibrate import MagnetometerCalibrator


class Sensor(Enum):
    ACCEL = 0
    GYRO = 1
    MAG = 2

plt.rcParams["font.size"] = 12
plt.rcParams["axes.grid"] = True
plt.rcParams["figure.autolayout"] = True


        
def plot_3d(data: np.ndarray, title = "Magnitometer", label = "Raw", color = 'red'):
    """
    Build 3D scatter plot only for one sensor (accelerometer/magnetometer)
    using raw/calibrated (specify in title) measurements.
    Input: numpy array of n by 3 size
    """
    FIG_3D_SIZE = (7,5)
    dot_size = 15
    alpha = 0.8
    fig_3d = plt.figure(figsize=FIG_3D_SIZE)
    ax1 = fig_3d.add_subplot(projection='3d')
    ax1 = fig_3d.add_subplot(projection='3d')
    ax1.scatter(data[:, 0], data[:, 1], data[:, 2], 
                    s = dot_size, alpha=alpha, c = color)
    x_max, y_max, z_max = np.max(np.abs(data[:, 0])), np.max(np.abs(data[:, 1])), np.max(np.abs(data[:, 2]))
    #Set proper limit
    offset = max(x_max, y_max, z_max)/2
    x_max, y_max, z_max = x_max + offset, y_max + offset,  z_max + offset
    ax1.set_xlim(-x_max, x_max)
    ax1.set_ylim(-y_max, y_max)
    ax1.set_zlim(-z_max, z_max)
    ax1.set_xlabel(f'X')
    ax1.set_ylabel(f'Y')
    ax1.set_zlabel(f'Z')
    ax1.set_title(f'{title} {label} measurements')


def merge_plot_3d(raw_data: np.ndarray, calibrated_data: np.ndarray):
    """
    Build merged 3D scatter plot only for one sensor (accelerometer/magnetometer)
    using both measurements raw and calibrated.
    Input: numpy arrays of n by 3 size
    """
    FIG_3D_SIZE = (7,5)
    dot_size = 15
    raw_alpha, calib_alpha = 0.5, 0.5
    raw_color, calib_color = 'red', "blue" 
    # 3D trajectory
    fig_3d = plt.figure(figsize=FIG_3D_SIZE)
    ax1 = fig_3d.add_subplot(111, projection='3d')
    ax1.scatter(raw_data[:, 0], raw_data[:, 1], raw_data[:, 2], 
                s = dot_size, alpha=raw_alpha, c = raw_color)
    ax1.scatter(calibrated_data[:, 0], calibrated_data[:, 1], calibrated_data[:, 2],
                s = dot_size, alpha=calib_alpha, c = calib_color)
    ax1.set_title('Trajectory 3D')
    ax1.set_xlabel('X')
    ax1.set_ylabel('Y')
    ax1.set_zlabel('Z')
    ax1.set_aspect('equal')
    


def plot_all_sensor_raw_measurements(df: pd.DataFrame):
    """ 
    Plot raw measurements for all sensors against time:
    (time, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z) 
    Input: DataFrame of (nx10) format
    """
    column_arr = df.columns[1:] #except time
    for sensor_group in range(3): #TODO: examine grouping algorithms
        fig, ax = plt.subplots(3,1, sharex=True, figsize=(12,6))
        df.plot(ax=ax[0],x='time_ms', y=column_arr[sensor_group*3], color='blue', linewidth = 2)
        df.plot(ax=ax[1],x='time_ms', y=column_arr[sensor_group*3+1], color='blue', linewidth = 2)
        df.plot(ax=ax[2],x='time_ms', y=column_arr[sensor_group*3+2], color='blue', linewidth = 2)
    plt.tight_layout()
    plt.show()


def plot_sensor_measurements(time: np.ndarray, data: np.ndarray, 
                                 title: str = 'Gyro', label = 'raw', unit: str = 'DPS'):
    """ 
    Plot measurements only for one sensor against time 
    Input: 
    time is (n x 1) size numpy array, 
    data is (n x 3) size numpy array,
    title is the name of sensor (default: 'Gyro') 
    unit: 'DPS'or 'G' or 'uT'
    """
    fig, ax = plt.subplots(3,1, sharex=True, figsize=(12,6), layout = 'tight')
    fig.suptitle(f'{title} {label}  measurements ({unit})')
    ax[0].plot(time, data[:, 0], color='black', linewidth = 2, label = label)
    ax[0].set(ylabel = 'X')
    ax[0].autoscale(enable=True, axis='both', tight=True)
    ax[1].plot(time, data[:, 1], color='black', linewidth = 2, label = label)
    ax[1].set(ylabel = 'Y')
    ax[1].autoscale(enable=True, axis='both', tight=True)
    ax[2].plot(time, data[:, 2], color='black', linewidth = 2, label = label)
    ax[2].set(ylabel = 'Z')
    ax[2].set(xlabel = 'time (s)')
    ax[2].autoscale(enable=True, axis='both', tight=True)
    plt.show()
    
    
def plot_sensor_merged_measurements(time: np.ndarray, raw_data: np.ndarray, calib_data: np.ndarray,
                                    title:str = 'Magnetometer', unit: str = 'uT'):
    """ 
    Plot MERGED raw and calibrated sensor measurements for one sensor against time.
    Input: 
        time (n x 1 numpy array), 
        raw data (n x 3 numpy array),
        calibrated data  (n x 3 numpy array)
    """
    raw_color, calib_color = 'red', 'blue'
    fig, ax = plt.subplots(3, 1, sharex=True, figsize=(12,6))
    fig.suptitle(f'{title} raw and calib measurements ({unit})')
    ax[0].plot(time, raw_data[:,  0], color=raw_color, linewidth = 2, label = 'raw')
    ax[0].plot(time, calib_data[:, 0], color=calib_color, linewidth = 2, label = 'calib')
    ax[0].set(ylabel = 'X')
    ax[0].legend()
    ax[0].autoscale(enable=True, axis='both', tight=True)

    ax[1].plot(time, raw_data[:, 1], color=raw_color, linewidth = 2, label = 'raw')
    ax[1].plot(time, calib_data[:, 1], color=calib_color, linewidth = 2, label = 'calib')
    ax[1].set(ylabel = 'Y')
    ax[1].legend()
    ax[1].autoscale(enable=True, axis='both', tight=True)
 
    ax[2].plot(time, raw_data[:, 2], color=raw_color, linewidth = 2, label = 'raw')
    ax[2].plot(time, calib_data[:, 2], color=calib_color, linewidth = 2, label = 'calib')
    ax[2].set(ylabel = 'Z')
    ax[2].set(xlabel = 'time (s)')
    ax[2].legend()
    ax[2].autoscale(enable=True, axis='both', tight=True)

    plt.show()
 

def hampel_filtration(meas: pd.DataFrame):
    """
    Args:
        pd.DataFrame (n x 3): raw meas
    Returns:
        pd.DataFrame (n x 3): cleared meas frame from outliers 
        
    For mor info check outlier_rejection_analysis.ipynb script
    """
    # calc median for each axis
    meas_median = meas.median()
    # absolute deviation from median value
    abs_dev = abs(meas - meas_median)
    # MAD 
    MAD = 1.4826 * abs_dev.median()
    # threshold ~ 3sigma?
    T = 3 * MAD
    # filter only valid meas: if there is any axis meas that is greater then thres -> row of meas (x,y,z) is banned 
    mask =  abs_dev < T
    #filter out rows if any value is false
    return meas[mask.all(axis=1)] #axis = 1 -> horizontally


def get_calibrated(sensor: Sensor, data: pd.DataFrame, norm: float | int) -> np.ndarray:
    """ 
    Execute calibration pipeline from calibrate.py for magnetometer OR accelerometer
    Input: 
        raw meas (n x 3): pd.DataFrame numpy array 
        norm - used to define normalization coef during calibration (see calibrate.py) - only important for scaling???
    Output: calibrated  meas(n x 3) numpy array   
    """
    if sensor in [Sensor.ACCEL, Sensor.MAG]:
        # outlier rejection before firstly
        cleared_raw_meas = hampel_filtration(data).to_numpy()
        # calibration object fro magnetometer and accel
        calibrator = MagnetometerCalibrator(norm)
        # find calib params
        calibrator.calibrate(cleared_raw_meas) 
        print("\nCalibration completed!")
        print("Hard iron bias (microTesla) for (X, Y, Z):")
        print(f"{{ {np.real(calibrator.b[0,0]):.6f},\
                   {np.real(calibrator.b[1,0]):.6f},\
                   {np.real(calibrator.b[2,0]):.6f} }}")
        print("\nSoft iron transformation matrix:")
        print(f'{{{calibrator.A_1[0, 0]:.6f}, {calibrator.A_1[0, 1]:.6f}, {calibrator.A_1[0, 2]:.6f}}},')
        print(f'{{{calibrator.A_1[1, 0]:.6f}, {calibrator.A_1[1, 1]:.6f}, {calibrator.A_1[1, 2]:.6f}}},')
        print(f'{{{calibrator.A_1[2, 0]:.6f}, {calibrator.A_1[2, 1]:.6f}, {calibrator.A_1[2, 2]:.6f}}}')
        # apply params to RAW DATA! 
        raw_data = data.to_numpy()
        corrected_data = calibrator.apply_calibration(raw_data)
        # cut complex part for convenience (error-free) of plotting
        corrected_data = np.real(corrected_data) 
        
        # if save_params:
        #     calibrator.save_calibration(filename)
        # print("Corrected data size: ", corrected_data.shape)
        # print(f"\nFirst 5 calibrated values of {sensor}:")
        # print(corrected_data[:5, :])
    return corrected_data


def show_sensor_plots(sensor: Sensor, time: np.ndarray, sensor_raw_data: np.ndarray, calibrated_sensors: dict,
                      merge_plots = True, title = 'Accel', unit = 'G'):
    if sensor in calibrated_sensors:
        MagnetometerCalibrator.plot_data(sensor_raw_data, title, label = 'raw', unit = unit)
        MagnetometerCalibrator.plot_data(calibrated_sensors[sensor], title, label = 'calib', unit = unit)
        if not merge_plots:
            plot_sensor_measurements(time, sensor_raw_data, title, unit) 
            plot_sensor_measurements(time, calibrated_sensors[sensor], title, label = 'calib', unit = unit)
        else:
            # merge_plot_3d(raw_data[:,1:], calibrated_data) #except time
            plot_sensor_merged_measurements(time, sensor_raw_data, calibrated_sensors[sensor], title, unit)
    else:
        print("\nPlot only raw measurements")
        # Plot only raw measurements
        plot_3d(sensor_raw_data, title, 'raw')
        MagnetometerCalibrator.plot_data(sensor_raw_data, title, label = 'raw', unit = unit)
        plot_sensor_measurements(time, sensor_raw_data, title, label = 'raw', unit = unit) 

def main():
    print(('-'*50 + '\n')*2)
    # Set args imu_log_2026-08-31_01-54-21 imu_log_2026-08-28_02-33-36
    parser = argparse.ArgumentParser(prog = 'Plot building and calibration script')
    parser.add_argument('-f','--filename', default='imu_log_2026-08-28_02-33-36.csv', 
                        help='Choose file to be processed in log_data folder!')
    parser.add_argument('-ps','--plot-sensor', dest='plot_sensor', choices=['accel', 'mag', 'gyro', 'all'], default='all',
                            help='For which sensor measurements plot should be build (default: all)')
    parser.add_argument('--apply', type=str, nargs='?', const='default',
                    help='Apply existing calibration from JSON file instead of calibrating. Optionally specify the JSON file path.')
    sub_parser = parser.add_subparsers(dest='command', required=False, 
                                   help='Sub-commands (calibrate)')
    calib_parser = sub_parser.add_parser('calibrate', 
                                         help='Calibration command parser')
    calib_parser.add_argument('-s', '--sensor', choices=['accel', 'mag', 'gyro', 'all'], default='all',
                                  help='Choose sensor to calibrate or calibrate both (default)')
    # calib_parser.add_argument('--sj', dest='save_json', action='store_true', default=False,
    #                         help='Whetрer to save calibration params after calibration (default: False)')
    calib_parser.add_argument('--save', dest='save', action='store_true', default=False,
                                help='Whetрer to save corrected measurements after calibration (default: False)')
    calib_parser.add_argument('--no-merge_plot', dest='nomerge', action='store_true', default=False,
                                      help='Whether to exclude calibrated data from the figure of raw data (default: no exlude.)')
    # calib_parser.add_argument('-p', '--plot', action='store_true', 
    #                           help='Whether to build plot after calibration (default: False)')
    
    args = parser.parse_args()
    
    # Choose file 
    filename = Path("log_data")/args.filename
    print("Processing file: ", filename)
    new_header = ["time_ms",
                "acc_x", "acc_y", "acc_z",
                "gyro_x", "gyro_y", "gyro_z",
                "mag_x", "mag_y", "mag_z"]
    df = pd.read_csv(filename,
                    header=0,
                    names=new_header)
    
    df["time_s"] = df["time_ms"] / 1000
    df.drop(columns = "time_ms", inplace = True)
    
    print("\nFirst lines of raw data measurements file:\n", df.head(5))
    print("\nData info:\n", df.describe())
    
    # Converting df to numpy arr
    # raw_data = df.to_numpy()
    time =  df["time_s"].to_numpy()
    acc_raw_data = df.loc[:, ["acc_x", "acc_y", "acc_z"]].to_numpy()
    gyro_raw_data = df.loc[:, ["gyro_x", "gyro_y", "gyro_z"]].to_numpy()
    mag_raw_data = df.loc[:, ["mag_x", "mag_y", "mag_z"]].to_numpy()

    # CALIBRATION
    calibrated_sensors = {}
    # Execute plotting with calibrated data
    if args.command == 'calibrate':
        # MY_LOCAL_FIELD_uT = 0.0521746 #norm coef for magnetometer (dependent on location) 
        # CUSTOM_SCALER = 30 #???
        MY_LOCAL_FIELD_uT = 20
        sensor = args.sensor
        if sensor == 'accel':  
            calibrated_sensors[Sensor.ACCEL] = get_calibrated(Sensor.ACCEL, df.loc[:, ["acc_x", "acc_y", "acc_z"]], norm = 1) 
        elif sensor == 'mag':  
            calibrated_sensors[Sensor.MAG] = get_calibrated(Sensor.MAG, df.loc[:, ["mag_x", "mag_y", "mag_z"]], norm = MY_LOCAL_FIELD_uT)
        # elif sensor == 'gyro':
            # for gyro still no calibration used
            # plot_sensor_raw_measurements(time = raw_data[:, 0], data =  raw_data[:, 1:4])  
        elif sensor == 'all':
            calibrated_sensors[Sensor.ACCEL] = get_calibrated(Sensor.ACCEL, df.loc[:, ["acc_x", "acc_y", "acc_z"]], norm = 1) 
            calibrated_sensors[Sensor.MAG] = get_calibrated(Sensor.MAG, df.loc[:, ["mag_x", "mag_y", "mag_z"]], norm = MY_LOCAL_FIELD_uT)
        else:
            raise ValueError("No such sensor! Should be accel, mag, gyro (default all)")
    
    # SAVE CORRECTED MEAS
    if 'save' in vars(args) and args.save:
        new_df = df.loc[:,['time_s']]
        if Sensor.ACCEL in calibrated_sensors:
            new_df.loc[:,["acc_x", "acc_y", "acc_z"]] = calibrated_sensors[Sensor.ACCEL]
        else:
            new_df.loc[:,["acc_x", "acc_y", "acc_z"]] = df.loc[:,["acc_x", "acc_y", "acc_z"]]       
        new_df.loc[:,["gyro_x", "gyro_y", "gyro_z"]] = df.loc[:,["gyro_x", "gyro_y", "gyro_z"]]
        if Sensor.MAG in calibrated_sensors:
            new_df.loc[:,["mag_x", "mag_y", "mag_z"]]= calibrated_sensors[Sensor.MAG]
        else:
            new_df.loc[:,["mag_x", "mag_y", "mag_z"]] = df.loc[:,["mag_x", "mag_y", "mag_z"]]
        
        new_name = filename.stem + '_corrected' + '.csv'
        new_df.to_csv(path_or_buf=Path("log_data")/new_name, index = False, lineterminator ='\n')
        
            
    # BUILDING PLOTS
    nomerge_flag = vars(args).get('nomerge', False) 
    merge_plots = not nomerge_flag
    if args.plot_sensor == "accel":
        show_sensor_plots(Sensor.ACCEL, time, acc_raw_data, calibrated_sensors=calibrated_sensors,
                          merge_plots=merge_plots, title='Accel', unit='G')
    elif args.plot_sensor == 'gyro':
        show_sensor_plots(Sensor.GYRO, time, gyro_raw_data, calibrated_sensors=calibrated_sensors,
                                  merge_plots=merge_plots, title='Gyro', unit='DPS')
    elif args.plot_sensor == 'mag':
        show_sensor_plots(Sensor.MAG, time, mag_raw_data, calibrated_sensors=calibrated_sensors,
                                  merge_plots=merge_plots, title='Mag', unit='mT')
    elif args.plot_sensor == 'all':
        show_sensor_plots(Sensor.ACCEL, time, acc_raw_data, calibrated_sensors=calibrated_sensors, merge_plots=merge_plots, title='Accel', unit='G')
        show_sensor_plots(Sensor.GYRO, time, gyro_raw_data, calibrated_sensors=calibrated_sensors,  merge_plots=merge_plots, title='Gyro', unit='DPS')
        show_sensor_plots(Sensor.MAG, time, mag_raw_data, calibrated_sensors=calibrated_sensors, merge_plots=merge_plots, title='Mag', unit='mT')
    else:
        raise ValueError("No such sensor! Should be accel, mag, gyro (default all)")

            
    # data from calibration (calibrate.py)
    # A_inv =np.array([[ 2.56200169, 0.12097418 , 0.0409529],
    #             [0.12097418,  2.44772822, -0.03086408],
    #             [0.0409529,  -0.03086408,  2.6015588]])

    # bias = np.array([-39.042610,  9.377833, 9.807292])
    
    
    
if __name__ == "__main__":
    main()