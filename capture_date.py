import csv
import serial
import time

ser = serial.Serial('/dev/cu.usbserial-AH03J6GL', 9600)
v = time.time()
writer.writerow("Time (sec)","Battery Voltage (V)","Rotation Enable","Top Photoresistor (Ohm)","Bottom Photoresistor (Ohm)","Left Photoresistor (Ohm)","Right Photoresistor (Ohm)")

with open('arduino_data.csv', 'wb') as f:
    writer = csv.writer(f)

    while True:
        data_values = ser.readline()  
        data_values = str(time.time()-v) + ","+ data_values
        writer.writerow(data_values.split(','))
        
