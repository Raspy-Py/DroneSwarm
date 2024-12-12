import socket

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

# Try binding to different addresses to see which one works
sock.bind(("0.0.0.0", 8888))  # Try this first
# sock.bind(("192.168.4.2", 8888))  # Then this
# sock.bind(("192.168.4.255", 8888))  # And this

print("Listening for UDP packets...")
while True:
    try:

        data, addr = sock.recvfrom(1024)
        print(f"Received packet from {addr}: {data}")
    except Exception as e:
        print(f"Error: {e}")