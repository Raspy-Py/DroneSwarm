import cv2
import numpy as np
import sys
from torchvision.transforms import ToTensor
from scipy.optimize import minimize
from LedDetection import LedDetector

INTRINSIC_PATH = "scripts/calibration/20241203_130810/camera_matrix.npy"
DEFAULT_PORT = 554
DEFAULT_STREAM = 'main_stream'

d12 = 0.1 
d13 = 0.14
d23 = 0.1


def get_intrinsics():
    """Load camera intrinsics from file."""
    return np.load(INTRINSIC_PATH)


def calculate_distance(intrinsics, led_coords):
    """Calculate distance between drones using LED positions and camera intrinsics."""
    led_coords_h = np.hstack([led_coords, np.ones((3, 1))])  # Shape: (3, 3)

    rays = np.dot(np.linalg.inv(intrinsics), led_coords_h.T).T  # Shape: (3, 3)

    def objective_function(lambdas):

        p1 = lambdas[0] * rays[0]
        p2 = lambdas[1] * rays[1]
        p3 = lambdas[2] * rays[2]

        dist12 = np.linalg.norm(p1 - p2)
        dist13 = np.linalg.norm(p1 - p3)
        dist23 = np.linalg.norm(p2 - p3)

        error = (dist12 - d12) ** 2 + (dist13 - d13) ** 2 + (dist23 - d23) ** 2
        return error

    initial_lambdas = np.array([1.0, 1.0, 1.0])

    result = minimize(objective_function, initial_lambdas, method='BFGS')
    optimized_lambdas = result.x

    p1 = optimized_lambdas[0] * rays[0]
    p2 = optimized_lambdas[1] * rays[1]
    p3 = optimized_lambdas[2] * rays[2]

    centroid = (p1 + p2 + p3) / 3

    return np.linalg.norm(centroid)



if __name__ == '__main__':
    try:
        port = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_PORT
        stream = sys.argv[2] if len(sys.argv) > 2 else DEFAULT_STREAM
    except:
        print("Usage: python stream3.py [port] [stream]")
        print("args:")
        print("\tport: RTSP port number (default: 554)")
        print("\tstream: RTSP stream name (default: main_stream)")
        exit()
        ld = LedDetector()

    intrinsics = get_intrinsics()

    rtsp_url = f'rtsp://192.168.55.1:{port}/live/{stream}'
    cap = cv2.VideoCapture(rtsp_url)
    if not cap.isOpened():
        print("Cannot open cam stream")
        exit()

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame")
            break

        led_coords = ld.forward(ToTensor()(frame))
        led_coords = [coords.numpy() for coords in led_coords]

        if len(led_coords) != 3:
            print("Error: Expected 3 LEDs, but detected", len(led_coords))
            continue

        distance = calculate_distance(intrinsics, np.array(led_coords))

        for i, coord in enumerate(led_coords):
            color = [(0, 0, 255), (0, 255, 0), (255, 0, 0)][i]  # RGB for 3 LEDs
            cv2.circle(frame, tuple(coord), radius=4, color=color, thickness=4)

        cv2.putText(frame, f"Distance: {distance:.2f} m", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 
                    1, (255, 255, 255), 2, cv2.LINE_AA)

        cv2.imshow('Cam Stream', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
