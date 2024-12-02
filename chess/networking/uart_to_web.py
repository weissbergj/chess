import serial

# Configure the serial port
ser = serial.Serial('/dev/tty.usbserial-0001', 9600)  # Adjust the port and baud rate as needed

try:
    while True:
        # Read a line from the serial port
        raw_line = ser.readline()
        # Decode the line, ignoring errors
        line = raw_line.decode('utf-8', errors='ignore').strip()
        
        # Check if the line contains "White played:"
        if "White played:" in line:
            print(line)
except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()