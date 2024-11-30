import cv2
import torch
from torchvision.transforms import ToTensor
FOCAL_LEN = 1
REAL_DIST = 1
class LedDetector(torch.nn.Module):
    def forward(self, X: torch.Tensor) -> torch.Tensor:
        X = X.squeeze(0)
        self.width = X.shape[2]
        # result = torch.stackself.detect_led(X, 1, 0, 0)
        red = torch.argmin(torch.sqrt((X[2] - 1) * (X[2] - 1)) + X[0] + X[1])
        green = torch.argmin(torch.sqrt((X[1] - 1) * (X[1] - 1)) + X[0] + X[2])
        blue = torch.argmin(torch.sqrt((X[0] - 1) * (X[0] - 1)) + X[1] + X[2])

        red_coordintates = self.get_coordinates(red)
        green_coordintates = self.get_coordinates(green)
        blue_coordintates = self.get_coordinates(blue)

        return torch.stack([red_coordintates, green_coordintates, blue_coordintates])

    def get_coordinates(self, X: torch.Tensor) -> torch.Tensor:
        y = X // self.width
        x = X - (y * self.width)
        coordinates = torch.stack([x, y])
        return coordinates

if __name__=='__main__':

    ld = LedDetector()
    cap = cv2.VideoCapture(0)
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
        distance = abs(led_coords[0][0] - led_coords[0][1]) * FOCAL_LEN / REAL_DIST
        cv2.putText(frame,  f"Distance: {distance:.2f}", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
        cv2.imshow('Webcam Stream', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cap.release()
    cv2.destroyAllWindows()
