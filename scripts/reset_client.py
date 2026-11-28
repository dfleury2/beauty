# Reset test
# python3 rst_client.py 127.0.0.1 8085
# python.exe .\reset_client.py 127.0.0.1 8085

import socket
import struct
import sys
import time

host = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
port = int(sys.argv[2]) if len(sys.argv) > 2 else 8085
request = b"GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"

client = socket.create_connection((host, port), timeout=3)
client.sendall(request)
client.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack("ii", 1, 0))
client.close()
time.sleep(1.5)

try:
    with socket.create_connection((host, port), timeout=3) as check:
        check.sendall(request)
        status = check.recv(4096).split(b"\r\n", 1)[0].decode("ascii", "replace")
    print("server still up:", status)
except OSError as error:
    print("server gone:", error)
