import serial, struct, time
from collections import deque
from enum import Enum
from typing import List, Union, Optional, Tuple
import numpy as np
from matplotlib import pyplot as plt  
from matplotlib import axis , figure 
from stm32_uart_comm import UartCom
from base_dataclasses import ReadImuScaledMeasResponce
from plot_offline import Sensor
plt.rcParams["font.size"] = 10
plt.rcParams["axes.grid"] = True
plt.rcParams["figure.autolayout"] = True
# plt.rcParams["figure.constrained_layout.use"] = True
# plt.rcParams["figure.constrained_layout.hspace"] = 0.1

class EulerAngle(Enum):
    Roll = 0
    Pitch = 1
    Yaw = 2
    

def plot_figure(): #deepseek (test)
    plt.ion()

    fig, ax = plt.subplots()
    x = np.linspace(0, 10, 100)

    # Первоначальный пустой график
    ax.plot(x, np.sin(x))
    # line, = ax.plot(x, np.sin(x))
    plt.pause(1)
    # fig.canvas.draw()
    
    # Устанавливаем границы, чтобы они не прыгали
    # ax.set_ylim(-1.5, 1.5)

    for i in range(10):
        # Меняем данные (например, сдвигаем фазу)
        ax.clear()
        ax.set_ylim(-1.5, 1.5)
        ax.plot(x, np.sin(x + i * 0.5))
        # line.set_ydata(y)  # Обновляем данные линии
        
        # Перерисовываем фигуру и даем время на отрисовку
        # fig.canvas.draw()
        # fig.canvas.flush_events()  # Для обработки событий (нужно в PyQt/PySide)
        plt.pause(0.2)
        # time.sleep(0.2)  # Имитация работы программы

    # Чтобы окно не закрылось после цикла (если нужно посмотреть финальный результат)
    plt.ioff()  # Выключаем интерактивный режим
    plt.show()  # Держим окно открытым
    
    

class RealTimePlotter:
    sensors = {Sensor.ACCEL: ["ACC_X", "ACC_Y", "ACC_Z"],
               Sensor.GYRO: ["Gyro_X", "Gyro_Y", "Gyro_Z"],
               Sensor.MAG: ["MAG_X", "MAG_Y", "MAG_Z"]}
    
    def __init__(self, window_size, sensor, figure_size = (12, 6)) -> None:
        # number of meas along x axis of any plot
        self._window_size = window_size
        if sensor not in RealTimePlotter.sensors:     
            raise KeyError(f"No such sensor: {sensor}, should be Sensors type")
        
        plt.ion() #interactivre mode is on
        
        self._figure, axes = plt.subplots(3, 1, squeeze= True, sharex = True, 
                                                figsize=figure_size, layout='tight')

        self._subplot_objects = {}
        for name, ax in zip(RealTimePlotter.sensors[sensor], axes):
            # line objects that will be auto-updated by new data in subplot
            line_object, = ax.plot([], [], 'b-', linewidth=2)
            # ax config
            ax.set_xlabel('Time, s')
            ax.set_ylabel(name)
            ax.set_ylim(-1.5, 1.5)
            ax.grid(True, alpha=0.3)
            # save ax and line objects in dict format for convenience
            self._subplot_objects[name] = (ax, line_object)
    
    def plot_data(self, gyro_arr: np.ndarray):  
        # start = time.perf_counter()
        for ind, (ax, line) in enumerate(self._subplot_objects.values()):
            line.set_data(gyro_arr[:, 3], gyro_arr[:, ind])
            ax.set_xlim(gyro_arr[0, 3], gyro_arr[0, 3] + self._window_size)
            ax.relim() 
            ax.autoscale(axis="y", tight = True)
            # -- time counting for each iteration (dbg) --
            # now = time.perf_counter()
            # dt, start = now - start,  now
            # time_cnt += dt
            # print(f"Iter {ind+1} takes: {dt:.3f} sec")
        
        # -- approaches to initate plot upd in gui event loop --
        # self._figure.canvas.draw_idle() #put task in event loop
        # self._figure.canvas.flush_events() #force backend to start event proccessing
        plt.pause(0.01) #the same as fraw_idle() + flush_events()
        
        # print(f"[IN] Plotting takes: {time.perf_counter() - start + time_cnt:.3f} sec") 
         
def main():
    try:
        # stm pubs data wit hUART_TX_PERIOD period 
        UART_TX_PERIOD = 0.02 # (not controlled from this script!)
        
        # 4sec (0.02 * 200) period buffer ovf
        MEAS_BUFFER_SIZE = 200 
    
        # window size = time offset (sec) from left bound = MEAS_BUFFER_SIZE * 0.02
        PLT_WINDOW_SIZE  = MEAS_BUFFER_SIZE*UART_TX_PERIOD + 1
        
        # period of plot building (time.monotonic() and last_upd var)
        PLT_TIMER_PERIOD = 0.2
        last_upd = 0 

        meas_buffer = deque(maxlen=MEAS_BUFFER_SIZE)
        com_master = UartCom("COM4", timeout_sec=0.01)
        sensor = Sensor.ACCEL
        FIGURE_SIZE  = (8, 6)
        plotter = RealTimePlotter(PLT_WINDOW_SIZE, sensor, FIGURE_SIZE)
                                
        while(com_master._my_serial.is_open):
        # while(meas_cnt < TOTAL_MEAS_CNT):
            now = time.monotonic() 
            resp: Optional[ReadImuScaledMeasResponce] = com_master.uart_read_imu_scaled_data() #blocking!!!
            if resp is not None: 
                if  sensor == Sensor.ACCEL:  
                    meas_buffer.append((*resp.accel_meas, resp.timestamp / 1000.0))
                elif  sensor == Sensor.GYRO: 
                    meas_buffer.append((*resp.gyro_meas, resp.timestamp / 1000.0))
                elif  sensor == Sensor.MAG: 
                    meas_buffer.append((*resp.mag_meas,  resp.timestamp / 1000.0))

            if now - last_upd >= PLT_TIMER_PERIOD and meas_buffer:
                start = time.perf_counter()     
                # create gyro numpy data base on meas_cnt records (copy again!)        
                plotter.plot_data(np.array(meas_buffer))
                print(f"[OUT] Plotting takes: {time.perf_counter() - start:.3f} sec")
                last_upd = now
        
                
    except serial.SerialException:
        print("Serial connection lost.")
    except struct.error as e:
        print(f"Struct unpack error: {e}")
    except KeyboardInterrupt:
        print("End event was raised.")
    finally:
        plt.ioff() 
        # plt.show()

if __name__ == "__main__":
    main()