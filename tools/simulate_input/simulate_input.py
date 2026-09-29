# /// script
# dependencies = ["pyserial"]
# ///

import json
import serial.tools.list_ports
import serial
from http.server import BaseHTTPRequestHandler, HTTPServer
import mimetypes
import threading
from pathlib import Path

STATIC_DIR = Path(__file__).parent.resolve()

ESPRESSIF_VID = 0x303A  # pyserial gives vid/pid as ints, not the "303A:1001" string form

BAUD_RATE = 115200

HTTP_PORT = 8000

ser = None

def find_espressif_ports():
    return [p for p in serial.tools.list_ports.comports() if p.vid == ESPRESSIF_VID]

def choose_port():
    candidates = find_espressif_ports()

    if len(candidates) == 1:
        return candidates[0].device

    if not candidates:
        print("No Espressif device found. Available ports:")
        candidates = list(serial.tools.list_ports.comports())

    for i, port in enumerate(candidates):
        print(f"{i}: {port.device}  ({port.description})")

    choice = input("Select a port: ")
    return candidates[int(choice)].device

def read_from_board():
    """Prints every line the board sends over serial. Runs in its own thread."""
    while True:
        try:
            line = ser.readline()  # waits up to `timeout` seconds; returns b"" if nothing came
        except serial.SerialException as e:
            print(f"Serial read failed: {e}")
            return
        if line:
            print(f"[board] {line.decode('utf-8', errors='replace').rstrip()}")

class JsonHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        request_path = self.path.lstrip('/') or 'index.html'
        file_path = (STATIC_DIR / request_path).resolve()

        # Refuse to serve anything outside STATIC_DIR (blocks paths like /../../etc/passwd).
        if STATIC_DIR not in file_path.parents and file_path != STATIC_DIR:
            self.send_error(403)
            return

        if not file_path.is_file():
            self.send_error(404)
            return

        content_type, _ = mimetypes.guess_type(str(file_path))
        self.send_response(200)
        self.send_header('Content-Type', content_type or 'application/octet-stream')
        self.end_headers()
        self.wfile.write(file_path.read_bytes())

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        print(f"Received POST data: {post_data}")

        try:
            data = json.loads(post_data.decode('utf-8'))

            if ser is not None:
                # Write the received JSON data to the serial port
                # Needs to be "BUTTON <button_name> <state>"
                ser.write(f"BUTTON {data['button']} {data['state']}\n".encode('utf-8'))
                ser.flush()

            response_data = {"status": "success", "received": data}
            response_code = 200
        except json.JSONDecodeError:
            response_data = {"status": "error", "message": "Invalid JSON data"}
            response_code = 400

        # 4. Construct and send the HTTP response headers
        self.send_response(response_code)
        self.send_header('Content-Type', 'application/json')
        
        # Convert response back to bytes to calculate length safely
        response_bytes = json.dumps(response_data).encode('utf-8')
        self.send_header('Content-Length', str(len(response_bytes)))
        self.end_headers()

        # 5. Write the response data to the output stream
        self.wfile.write(response_bytes)

def run():
    serial_port = choose_port()
    global ser
    ser = serial.Serial(serial_port, BAUD_RATE, timeout=1)

    # daemon=True lets the program exit on Ctrl+C without waiting for this thread.
    threading.Thread(target=read_from_board, daemon=True).start()

    server_address = ('', HTTP_PORT)
    httpd = HTTPServer(server_address, JsonHandler)
    print(f"Serving HTTP on port {HTTP_PORT}...")

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server.")


if __name__ == "__main__":
    run()