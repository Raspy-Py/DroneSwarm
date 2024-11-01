import cv2
import torch
from torchvision.transforms import ToTensor
class LedDetector(torch.nn.Module):
    def forward(self, X: torch.Tensor) -> torch.Tensor:
        self.width = X.shape[2]
        result = torch.stackself.detect_led(X)
        return result

    def detect_led(self, X: torch.Tensor) -> torch.Tensor:
        X = X.squeeze(0)  
        min_point = torch.argmin(torch.sqrt((X[0] - 1) * (X[0] - 1)) + X[1] + X[2])
        row = min_point // self.width
        col = min_point - (row * self.width)
        coordinates = torch.stack([row, col])
        return coordinates

if __name__=='__main__':
    ld = LedDetector()
    img = cv2.imread("data/led/1.jpg")
    img_tensor = ToTensor()(img).unsqueeze(0)
    print(ld.forward(img_tensor))

    rtsp_url = f'rtsp://192.168.55.1:554/live/main_stream'
    cap = cv2.VideoCapture(rtsp_url)

    if not cap.isOpened():
        print("Cannot open RTSP stream")
        exit()

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame")
            break

        # cv2.imshow('RTSP Stream', frame)
        print(ld.forward(ToTensor()(frame)))
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
