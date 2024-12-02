# from flask import Flask, request

# app = Flask(__name__)

# @app.route('/')
# def home():
#     return "<h1>Welcome to the Flask Server</h1><p>This server is running and ready to receive data.</p>"

# @app.route('/api/data', methods=['POST'])
# def receive_data():
#     data = request.form.get('data')
#     if data:
#         print(f"Received data: {data}")
#         return "Data received", 200
#     return "No data", 400

# if __name__ == '__main__':
#     app.run(host='0.0.0.0', port=5001)

import serial
import requests
from flask import Flask, request
import threading

# Flask app setup
app = Flask(__name__)

@app.route('/')
def home():
    return "<h1>Welcome to the Flask Server</h1><p>This server is running and ready to receive data.</p>"

@app.route('/api/data', methods=['POST'])
def receive_data():
    data = request.form.get('data')
    if data:
        print(f"Received data: {data}")
        return "Data received", 200
    return "No data", 400

# Function to read from UART and send to Flask server
def uart_to_web():
    # Configure the serial port
    ser = serial.Serial('/dev/tty.usbserial-0001', 9600)  # Adjust the port and baud rate as needed
    url = 'http://localhost:5001/api/data'

    try:
        while True:
            # Read a line from the serial port
            line = ser.readline().decode('utf-8').strip()
            print(f"Received from UART: {line}")

            # Send the data to the local web server
            response = requests.post(url, data={'data': line})
            print(f"Server response: {response.status_code}")
    except KeyboardInterrupt:
        print("Exiting...")
    finally:
        ser.close()

if __name__ == '__main__':
    # Start the UART reading in a separate thread
    uart_thread = threading.Thread(target=uart_to_web)
    uart_thread.start()

    # Run the Flask server
    app.run(host='0.0.0.0', port=5001)