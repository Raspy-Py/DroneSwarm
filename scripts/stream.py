import cv2 
import numpy as np 
from torchvision.transforms import ToTensor
from LedDetection import LedDetector
INTRINSIC_PATH = "scripts/calibration/20241206_215259/camera_matrix.txt"
DEFAULT_PORT = 554
DEFAULT_STREAM = 'main_stream'

def get_intrinsics():
    return np.loadtxt(INTRINSIC_PATH)

def calculate_delta(x1, x2, x3, cosa, cosb, cosc, ncosa, ncosb, ncosc):

    a1 = x1 - x2 * cosa
    a2 = x2 - x3 * cosb
    a3 = x1 - x3 * cosc

    b1 = x2 - x1 * cosa
    b2 = x3 - x2 * cosb
    b3 = x3 - x1 * cosc

    c1 = x1 * x2 * (ncosa - cosa)
    c2 = x2 * x3 * (ncosb - cosb)
    c3 = x1 * x3 * (ncosc - cosc)

    delta = a1 * a2 * b3 + b1 * b2 * a3

    delta_x1 = (c1 * a2 * b3 + c3 * b1 * b2 - b1 * c2 * b3) / delta
    delta_x2 = (a1 * b3 * c2 + c1 * b2 * a3 - a1 * b2 * c3) / delta
    delta_x3 = (a1 * a2 * c3 + b1 * c2 * a3 - c1 * a2 * a3) / delta

    return delta_x1, delta_x2, delta_x3


def calculate_angle(pixel_1, pixel_2, intrinsics):
    x1, y1 = pixel_1
    x2, y2 = pixel_2

    fx, fy = intrinsics[0][0], intrinsics[1][1]
    cx, cy = intrinsics[0][2], intrinsics[1][2]

    X1, Y1 = (x1 - cx) / fx, (y1 - cy) / fy
    X2, Y2 = (x2 - cx) / fx, (y2 - cy) / fy

    v1 = np.array([X1, Y1, 1])
    v2 = np.array([X2, Y2, 1])

    cos_theta = np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))
    return cos_theta
    # return np.clip(cos_theta, -1.0, 1.0)  # Clip to avoid numerical issues

def main():
    ld = LedDetector()
    intrinsics = get_intrinsics()
    x1prev, x2prev, x3prev = 240, 260, 270
    cosa, cosb, cosc = None, None, None
    rtsp_url = f'rtsp://192.168.55.1:{DEFAULT_PORT}/live/{DEFAULT_STREAM}'
    cap = cv2.VideoCapture(rtsp_url)
    if not cap.isOpened():
        print("Cannot open cam stream")
        exit()

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame")
            break

        ledc = ld.forward(ToTensor()(frame))
        ledc = [coords.numpy() for coords in ledc]

        cv2.circle(frame, tuple(ledc[0]), radius=4, color=(0, 0, 255), thickness=4)
        cv2.circle(frame, tuple(ledc[1]), radius=4, color=(0, 255, 0), thickness=5)
        cv2.circle(frame, tuple(ledc[2]), radius=4, color=(255, 0, 0), thickness=4)

        ncosa = calculate_angle(ledc[2], ledc[1], intrinsics)
        ncosb = calculate_angle(ledc[2], ledc[0], intrinsics)
        ncosc = calculate_angle(ledc[1], ledc[0], intrinsics)
        cv2.imshow('Cam Stream', frame)
        if cosa is None:
            cosa, cosb, cosc = ncosa, ncosb, ncosc
            continue
        deltax1, deltax2, deltax3 = calculate_delta(x1prev, x2prev, x3prev, cosa, cosb, cosc, ncosa, ncosb, ncosc)
        if (abs(deltax1) > 100 or abs(deltax2) > 100 or abs(deltax3) > 100):
            print("NOISE DETECTED")
            continue
        alpha = 0.7  # Smoothing factor
        x1prev = alpha * x1prev + (1 - alpha) * (deltax1 + x1prev)
        x2prev = alpha * x2prev + (1 - alpha) * (deltax2 + x2prev)
        x3prev = alpha * x3prev + (1 - alpha) * (deltax3 + x3prev)
        print(x1prev - deltax1, x2prev - deltax2, x3prev - deltax3)
        cosa = ncosa
        cosb = ncosb
        cosc = ncosc
        
#         print(f"""Angle between red, green = {rg}
# Angle between red, blue = {rb}
# Angle between blue, green = {bg}

# """)
        # print(x1prev, "BLUE")
        # print(x2prev, "GREEN")
        # print(x3prev, "RED")
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cap.release()
    cv2.destroyAllWindows()

if __name__ == '__main__':
    main()