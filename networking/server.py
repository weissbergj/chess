import serial
import requests
from flask import Flask, request, render_template_string
import threading
import time

# Flask app setup
app = Flask(__name__)

# Global variable to store received data
received_data = []

# Configure the serial port for UART communication
ser = serial.Serial('/dev/tty.usbserial-0001', 115200)  # Adjust the baud rate as needed

@app.route('/')
def home():
    # Render the received data and command input form on the homepage
    return render_template_string('''
        <h1>Welcome to the Flask Server</h1>
        <p>This server is running and ready to receive data.</p>
        <h2>Received Data:</h2>
        <ul id="data-list">
            {% for data in received_data %}
                <li>{{ data }}</li>
            {% endfor %}
        </ul>
        <h2>Send Command to UART:</h2>
        <form id="command-form" action="/api/send" method="post">
            <input type="text" name="command" placeholder="Enter command" required>
            <button type="submit">Send</button>
        </form>
        <script>
            // Function to fetch the latest data
            function fetchData() {
                fetch('/api/data')
                    .then(response => response.json())
                    .then(data => {
                        const dataList = document.getElementById('data-list');
                        dataList.innerHTML = ''; // Clear the current list
                        data.forEach(item => {
                            const li = document.createElement('li');
                            li.textContent = item;
                            dataList.appendChild(li);
                        });
                    });
            }

            // Fetch data every 2 seconds
            setInterval(fetchData, 2000);

            // Handle form submission
            document.getElementById('command-form').onsubmit = function(event) {
                event.preventDefault(); // Prevent the default form submission
                const formData = new FormData(this);
                fetch(this.action, {
                    method: 'POST',
                    body: formData
                }).then(response => {
                    if (response.ok) {
                        console.log('Command sent');
                    }
                });
                this.reset(); // Reset the form
            };
        </script>
    ''', received_data=received_data)

@app.route('/api/data', methods=['POST'])
def receive_data():
    data = request.form.get('data')
    if data:
        print(f"Received data: {data}")
        received_data.append(data)  # Store the received data
        return "Data received", 200
    return "No data", 400

@app.route('/api/send', methods=['POST'])
def send_command():
    command = request.form.get('command')
    if command:
        print(f"Sending command to UART: {command}")
        ser.write((command + '\n').encode('utf-8'))  # Send command to UART
        return "Command sent", 200
    return "No command provided", 400

# Function to read from UART and send to Flask server
def uart_to_web():
    url = 'http://localhost:5001/api/data'

    try:
        while True:
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