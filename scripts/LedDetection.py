import torch

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
