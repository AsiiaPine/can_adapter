#!/usr/bin/env python3
# This software is distributed under the terms of the MIT License.
# Copyright (c) 2024 Dmitry Ponomarev.
# Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
from time import sleep, time
import dronecan
from raccoonlab_tools.dronecan.utils import ParametersInterface, NodeFinder
from raccoonlab_tools.common.device_manager import DeviceManager
from raccoonlab_tools.dronecan.global_node import DronecanNode
import serial

def test_dronecan():
    node = DronecanNode()
    msg = dronecan.uavcan.equipment.indication.BeepCommand(frequency=100, duration=10)
    while True:
        node.publish(msg)
        node.node.spin(0.1)

def test_simple():
    message = "T123456789ABCDEF\r"
    # get serial port

    port = DeviceManager.get_device_port(True)
    # open serial port
    print(f"Opening serial port {port}")
    ser = serial.Serial(port, baudrate=1000000, timeout=1)
    sleep(2)
    while True:
        message = f"T{int(time())}\r"
        ser.write(message.encode(errors='ignore'))
            # # read response
        response = ser.read_all()
        if response:
            print(response)
            # send message
        sleep(0.1)

if __name__ =="__main__":
    test_simple()
