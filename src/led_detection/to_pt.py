import torch
from LedDetection import LedDetector
model = LedDetector()
model.eval()
trace_model = torch.jit.trace(model, torch.Tensor(1, 3, 192, 256))  # (1, 3, 192, 256)
trace_model.save("model.pt")