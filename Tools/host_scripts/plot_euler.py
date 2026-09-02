import argparse
import csv
import pandas as pd
import numpy as np
from pathlib import Path
from collections import deque
import matplotlib.pyplot as plt
from enum import Enum

plt.rcParams["font.size"] = 12
plt.rcParams["axes.grid"] = True
plt.rcParams["figure.autolayout"] = True


def plot_euler_angles(time: np.ndarray, data: np.ndarray):
    fig, ax = plt.subplots(3,1, sharex=True, figsize=(12,6), layout = 'tight')
    ax[0].plot(time, data[:, 0], color='black', linewidth = 2)
    ax[0].set(ylabel = 'Roll')
    ax[0].autoscale(enable=True, axis='both', tight=True)
    ax[1].plot(time, data[:, 1], color='black', linewidth = 2)
    ax[1].set(ylabel = 'Pitch')
    ax[1].autoscale(enable=True, axis='both', tight=True)
    ax[2].plot(time, data[:, 2], color='black', linewidth = 2)
    ax[2].set(ylabel = 'Yaw')
    ax[2].set(xlabel = 'time (s)')
    ax[2].autoscale(enable=True, axis='both', tight=True)
    plt.show()
    
def main():
     # Choose file 
    parser = argparse.ArgumentParser(prog = 'Plot building and calibration script')
    parser.add_argument('-f','--filename', default='imu_log_2026-09-02_14-47-42.csv', 
                            help='Choose file to be processed in log_data folder!')
    args = parser.parse_args()
    filename = Path("log_data")/args.filename
    print("Processing file: ", filename)
    new_header = ["time_ms",
                "Roll", "Pitch", "Yaw"]
    df = pd.read_csv(filename,
                    header=0,
                    names=new_header)
    
    df["time_s"] = df["time_ms"] / 1000
    df.drop(columns = "time_ms", inplace = True)
    print(f"/nDesciption:{df.describe()}\n")
    time =  df["time_s"].to_numpy()
    euler_angles = df.loc[:, ["Roll", "Pitch", "Yaw"]].to_numpy()

    plot_euler_angles(time, euler_angles)
        
if __name__ == "__main__":
    main()