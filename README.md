# STM32G0B1CEU6 USB-CAN Converter

This project implements a SLCAN USB-CAN converter using the STM32G0B1CEU6 microcontroller. The device provides two CAN interfaces (FDCAN1 and FDCAN2) accessible via USB CDC (Virtual COM Port). 

## Features

- **Dual CAN Interfaces**: Two independent CAN controllers (FDCAN1 and FDCAN2)
- **USB Communication**: CDC (Virtual COM Port) interface for easy integration
- **LED Status Indicators**: Visual feedback for device status and CAN activity
- **Error Handling**: Comprehensive error detection and reporting

## SLCAN
Modification:
- 'F' -- Read status.
Returns:
An F with 2 bytes BCD hex value plus CR (Ascii 13)
for OK. If CAN channel isn’t open it returns BELL
(Ascii 7). This command also clear the RED Error
LED. See availible errors below. E.g. F01[CR]

|Bit | Description |
| ---| ----|
| 0 | CAN receive FIFO queue full|
| 1 | CAN transmit FIFO queue full|
| 2 | Error warning (EI), see SJA1000 datasheet|
| 3 | Data Overrun (DOI), see SJA1000 datasheet, not used |
| 4 | No change (probably, disconnected), see [register viz](https://regviz.com/r/stm32/stm32g0/STM32G0B1/FDCAN1/FDCAN_PSR/)|
| 5 | Error Passive (EPI), see SJA1000 datasheet |
| 6 | Arbitration Lost (ALI), see SJA1000 datasheet * |
| 7 | Bus Error (BEI), see SJA1000 datasheet ** |

## License

This project is provided as-is for educational and development purposes. Modify as needed for your specific requirements. 


# TODO:

- make two usb devices from here
  https://community.st.com/t5/stm32-mcus-embedded-software/stm32f4-multi-instance-usb-cdc-device/td-p/357609
