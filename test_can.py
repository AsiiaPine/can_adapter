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
    message = "T0000010121133\r"
    message_standard = "t10221133\r"
    message_remote = "R000001032\r"
    message_remote_standard = "r1042\r"
    # get serial port
    messages = [message, message_standard, message_remote, message_remote_standard]
    rec_messages = ['Te\r', 'ts\r', 'Ter\r', 'tsr\r']
    port = DeviceManager.get_device_port(True)
    # open serial port
    print(f"Opening serial port {port}")
    ser = serial.Serial(port, baudrate=1000000, timeout=1)
    i = 0

    while True:
        i = (i + 1) % len(messages)

        ser.write(messages[i].encode(errors='ignore'))
        start_time = time()
        got_message = False
        while not got_message and time() - start_time < 0.0004:
            response = ser.read_until(b'\r')
            if response:
                if response == rec_messages[i].encode(errors='ignore'):
                    got_message = True
                    print(f"Got message {response}")
        assert got_message
        sleep(0.1)
        

if __name__ =="__main__":
    test_simple()
