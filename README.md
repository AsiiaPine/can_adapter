# STM32G0B1CEU6 USB-CAN Converter

This project implements a USB-CAN converter using the STM32G0B1CEU6 microcontroller. The device provides two CAN interfaces (FDCAN1 and FDCAN2) accessible via USB CDC (Virtual COM Port).

## Features

- **Dual CAN Interfaces**: Two independent CAN controllers (FDCAN1 and FDCAN2)
- **USB Communication**: CDC (Virtual COM Port) interface for easy integration
- **Configurable Terminators**: Software-controlled CAN bus terminators
- **Status Monitoring**: Real-time message counters and status reporting
- **LED Status Indicators**: Visual feedback for device status and CAN activity
- **Error Handling**: Comprehensive error detection and reporting

## Hardware Configuration

### Pin Assignment
- **FDCAN1**: PD0 (RX), PD1 (TX)
- **FDCAN2**: PB0 (RX), PB1 (TX)
- **USB**: PA11 (DM), PA12 (DP)
- **LEDs**: 
  - PC13 (Red): Error indicator
  - PC14 (Green): USB connection status
  - PC15 (Blue): CAN activity indicator
- **Terminators**: 
  - PA15 (CAN1 terminator control)
  - PB15 (CAN2 terminator control)

### CAN Configuration
- **Baud Rate**: 1 Mbps
- **Frame Format**: Classic CAN (non-FD)
- **Auto Retransmission**: Enabled
- **Error Handling**: Comprehensive error detection

## USB Protocol

The device communicates using a binary protocol over USB CDC. All messages use little-endian byte order.

### Message Format
```
[CMD][CH][ID(4)][DLC][DATA(8)]
```

### Commands
- `0x01` - CMD_CAN_SEND: Send CAN message
- `0x02` - CMD_CAN_RECEIVE: Received CAN message (from device)
- `0x03` - CMD_CAN_STATUS: Get status and counters
- `0x04` - CMD_CAN_CONFIG: Configure terminators
- `0x05` - CMD_CAN_RESET: Reset message counters

### Example Usage

#### Send CAN Message
```python
# Send message on CAN1, ID 0x123, 8 bytes of data
frame = struct.pack('<BBIB8s', 0x01, 1, 0x123, 8, b'HelloCAN')
serial.write(frame)
```

#### Get Status
```python
# Request status
frame = struct.pack('<BBIB8s', 0x03, 0, 0, 0, b'\x00' * 8)
serial.write(frame)
response = serial.readline()  # Returns: "CAN1: RX=10 TX=5, CAN2: RX=3 TX=2"
```

#### Configure Terminator
```python
# Enable CAN1 terminator
frame = struct.pack('<BBIB8s', 0x04, 1, 0, 0, b'\x01' + b'\x00' * 7)
serial.write(frame)
```

## Building the Firmware

### Prerequisites
- STM32CubeIDE or compatible IDE
- STM32CubeMX (for configuration)
- STM32G0 HAL library

### Build Steps
1. Open the project in STM32CubeIDE
2. Ensure all HAL libraries are properly linked
3. Build the project
4. Flash to the STM32G0B1CEU6 device

### Configuration
The project is pre-configured with:
- System clock: 64 MHz
- USB clock: 48 MHz
- CAN clock: 32 MHz
- Both CAN controllers at 1 Mbps

## Testing

Use the provided Python test script:

```bash
python3 can_test.py /dev/ttyACM0
```

The script will:
1. Connect to the device
2. Send a test message
3. Read any received messages
4. Display status information
5. Test terminator configuration

## LED Status Indicators

- **Red LED (PC13)**: Error indicator (toggles on CAN errors)
- **Green LED (PC14)**: USB connection status (blinks when connected)
- **Blue LED (PC15)**: CAN activity indicator (on when messages are processed)

## Error Handling

The firmware includes comprehensive error handling:
- CAN bus errors are detected and reported
- USB communication errors are handled gracefully
- Watchdog timer prevents system lockup
- Buffer overflow protection for received messages

## Limitations

- Maximum 10 queued CAN messages
- Standard CAN only (no CAN FD)
- Fixed 1 Mbps baud rate
- No extended CAN IDs (11-bit only)

## Troubleshooting

### Device Not Recognized
- Check USB cable and connections
- Verify device is powered correctly
- Check for proper USB descriptors

### CAN Communication Issues
- Verify CAN bus termination
- Check baud rate compatibility
- Ensure proper CAN bus wiring

### No Messages Received
- Check CAN bus activity
- Verify terminator configuration
- Monitor LED indicators for activity

## License

This project is provided as-is for educational and development purposes. Modify as needed for your specific requirements. 


# TODO:

- make two usb devices from here: one for swd, one for can
