import cv2
import torch
from torchvision.transforms import ToTensor
from numpy import ndarray

class LedDetector(torch.nn.Module):
    RED = 1
    GREEN = 0
    BLUE = 0
    def forward(self, X: ndarray) -> torch.tensor:
        img = ToTensor()(X)
        self.width = img.shape[2]
        colors = [Color(1, 1, 1), Color(0, 1, 1), Color(1, 0, 0)]
        return [self.detect_led(img, i) for i in colors]

    def detect_led(self, X: ndarray, colors: tuple[int, int, int], red: bool = False) -> torch.tensor:
        # if red:
        #     mask = (img_tensor[1] < 0.3) & (img_tensor[2] < 0.3)
        #     img_tensor[:, mask] = 1
        result = torch.argmin(abs(img_tensor[0] - colors.red) + abs(img_tensor[1] - colors.green) 
                               + abs(img_tensor[2] - colors.blue) )
        return (result // self.width, result % self.width)

class Color:
    def __init__(self, r: float, g: float, b: float):
        self.red = r
        self.green = g
        self.blue = b
    
    def __eq__(self, other):
        return self.red == other.red and self.green == other.green and self.blue == other.blue 

if __name__=='__main__':
    img = cv2.imread("data/led/-FF0000.webp")
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img_tensor = ToTensor()(img)
    ld = LedDetector()
    img = cv2.imread("data/led/1.jpg")
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    print(ld.forward(img))
