#test slcan
import time
import slcan

serial_port = '/dev/ttyACM0'
baudrate = 115200
slcan.SLCAN.open_port(serial_port, baudrate)

can_message = slcan.slcan_frame_t()
can_message.command = slcan.SLCANCommand.TRANSMIT_STANDART
can_message.id = 0x123
can_message.dlc = 8
can_message.data = b'Hello CAN!'
while True:
    slcan.SLCAN.transmit_can_frame(can_message)
    slcan.SLCAN.spin()
    time.sleep(1)
