import cv2
import torch
import sys
from torchvision.transforms import ToTensor
from LedDetection import LedDetector

FOCAL_LEN = (3.230422251762199153 + 3.217988176791898809) * 50 
REAL_DIST = 0.16
INTRINSIC_PATH = "scripts/calibration/20241203_130810/camera_matrix.npy"
DEFAULT_PORT = 554
DEFAULT_STREAM = 'main_stream'

if __name__=='__main__':

    try:
        port = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_PORT
        stream = sys.argv[2] if len(sys.argv) > 2 else DEFAULT_STREAM
    except:
        print("Usage: python stream2.py [port] [stream]")
        print("args:")
        print("\tport: RTSP port number (default: 554)")
        print("\tstream: RTSP stream name (default: main_stream)")
        exit()

    ld = LedDetector()
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
        print(led_coords[0])
        cv2.circle(frame, tuple(led_coords[0].numpy()), radius=4, color=(0, 0, 255), thickness=4)
        cv2.circle(frame, tuple(led_coords[1].numpy()), radius=4, color=(0, 255, 0), thickness=5)
        cv2.circle(frame, tuple(led_coords[2].numpy()), radius=4, color=(255, 0, 0), thickness=4)
        pixel_dist = torch.sqrt((led_coords[0][0] - led_coords[2][0])**2 + (led_coords[0][1] - led_coords[2][1])**2)
        distance = REAL_DIST * FOCAL_LEN / pixel_dist
        cv2.putText(frame,  f"Distance: {distance:.2f}", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
        cv2.imshow('Webcam Stream', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
