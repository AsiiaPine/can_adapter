#!/usr/bin/env python3
"""
USB-CAN Converter Test Script
This script communicates with the STM32G0B1CEU6 USB-CAN converter
"""

import serial
import struct
import time
import sys

class USBCANConverter:
    def __init__(self, port='/dev/ttyACM0', baudrate=115200):
        self.serial = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # Wait for device to initialize
        
    def send_can_message(self, channel, can_id, data, dlc=None):
        """Send a CAN message"""
        if dlc is None:
            dlc = len(data)
        
        # Protocol frame: [CMD, CH, ID(4), DLC, DATA(8)]
        frame = struct.pack('<BBIB8s', 0x01, channel, can_id, dlc, data.ljust(8, b'\x00'))
        self.serial.write(frame)
        
    def get_status(self):
        """Get CAN status"""
        frame = struct.pack('<BBIB8s', 0x03, 0, 0, 0, b'\x00' * 8)
        self.serial.write(frame)
        response = self.serial.readline()
        return response.decode('utf-8').strip()
        
    def reset_counters(self):
        """Reset message counters"""
        frame = struct.pack('<BBIB8s', 0x05, 0, 0, 0, b'\x00' * 8)
        self.serial.write(frame)
        response = self.serial.readline()
        return response.decode('utf-8').strip()
        
    def configure_terminator(self, channel, enabled):
        """Configure CAN terminator"""
        frame = struct.pack('<BBIB8s', 0x04, channel, 0, 0, bytes([enabled]) + b'\x00' * 7)
        self.serial.write(frame)
        
    def read_messages(self, timeout=1):
        """Read received CAN messages"""
        messages = []
        start_time = time.time()
        
        while time.time() - start_time < timeout:
            if self.serial.in_waiting:
                data = self.serial.read(self.serial.in_waiting)
                # Process received data for CAN messages
                if len(data) >= 14:  # Minimum frame size
                    try:
                        cmd, ch, can_id, dlc, can_data = struct.unpack('<BBIB8s', data[:14])
                        if cmd == 0x02:  # CAN_RECEIVE
                            messages.append({
                                'channel': ch,
                                'id': can_id,
                                'dlc': dlc,
                                'data': can_data[:dlc]
                            })
                    except struct.error:
                        pass
            time.sleep(0.01)
            
        return messages

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 can_test.py <serial_port>")
        print("Example: python3 can_test.py /dev/ttyACM0")
        sys.exit(1)
        
    port = sys.argv[1]
    
    try:
        can = USBCANConverter(port)
        print("Connected to USB-CAN converter")
        
        # Get initial status
        print("Initial status:", can.get_status())
        
        # Test sending a message on CAN1
        print("\nSending test message on CAN1...")
        test_data = b'Hello CAN!'
        can.send_can_message(1, 0x123, test_data)
        
        # Wait and read any received messages
        print("Reading received messages...")
        messages = can.read_messages(timeout=2)
        
        for msg in messages:
            print(f"Received: Channel={msg['channel']}, ID=0x{msg['id']:03X}, "
                  f"DLC={msg['dlc']}, Data={msg['data']}")
        
        # Get final status
        print("\nFinal status:", can.get_status())
        
        # Test terminator configuration
        print("\nConfiguring CAN1 terminator...")
        can.configure_terminator(1, 1)  # Enable terminator
        time.sleep(1)
        can.configure_terminator(1, 0)  # Disable terminator
        
        print("Test completed successfully!")
        
    except serial.SerialException as e:
        print(f"Serial error: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\nTest interrupted by user")
    finally:
        if 'can' in locals():
            can.serial.close()

if __name__ == "__main__":
    main()
