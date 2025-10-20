import os
import time
import serial

ports = os.listdir('/dev/')
print(ports)

def get_serials():
    serials = []
    for port in ports:
        if port.startswith('ttyACM'):
            serials.append(port)
    return serials

global i
i = 0

def switch_port():
    while True:
        global i
        serials = get_serials()
        if len(serials) == 0:
            serials = get_serials()
            print("No serial ports found")
            time.sleep(2)
            continue
        i = (i) % len(serials)
        print("Switching to ", serials[i])
        port = serials[i]
        last_rec_time = time.time()
        ser = None
        try:
            print(port)
            ser = serial.Serial('/dev/' + port, 115200, timeout=1)
            last_time = 0.0
            msg = 'HELLO\r'
            while True:
                if time.time() - last_rec_time > 2:
                    i += 1
                    continue
                if time.time() - last_time > 1:
                    ser.write(msg.encode('utf-8'))
                    print(f"sent {msg}\n\n")
                    last_time = time.time()
                if ser.in_waiting:
                    data = ser.read_all()
                    if len(data) != 0:
                        last_rec_time = time.time()
                    try:
                        data_decoded = data.decode('utf-8')
                        print("Got: ", data_decoded)
                    except UnicodeDecodeError:
                        print("Got: ", data)
                        i += 1
                        continue
        except serial.SerialException as e:
            print(e)
            if ser is not None:
                ser.close()
            max_tries = 5
            try_count = 0
            print("Trying to reconnect")
            while len(get_serials()) == 0 and try_count < max_tries:
                try_count += 1
                time.sleep(2)
            if try_count >= max_tries:
                print("Failed to connect")
                return
            continue

        except OSError as e:
            print(e)
            max_tries = 5
            try_count = 0
            while len(get_serials()) == 0 and try_count < max_tries:
                try_count += 1
                time.sleep(2)
            if try_count >= max_tries:
                print("Failed to connect")
                return
            continue

        except KeyboardInterrupt:
            print("Interrupted by user")
            ser.close()
            return


switch_port()
