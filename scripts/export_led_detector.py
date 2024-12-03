import torch
import torch.nn as nn

class LedDetector(nn.Module):
    def __init__(self, colors=None):
        super(LedDetector, self).__init__()
        
        # Define colors as a parameter to ensure it's part of the model state
        if colors is None:
            colors = torch.tensor([
                [1.0, 0.0, 0.0],
                [0.0, 0.0, 1.0],
                [1.0, 0.0, 1.0]
            ], dtype=torch.float32)
        
        # Register colors as a buffer to make it part of the model's state
        self.register_buffer('COLORS', colors)
        
        # Explicitly define dtype and device
        self.dtype = torch.float32
        
    def forward(self, X: torch.Tensor) -> torch.Tensor:
        # Ensure input is float32 and on the same device as colors
        X = X.to(dtype=self.dtype, device=self.COLORS.device)
        
        # Ensure input has batch dimension (assume first dim is batch)
        if X.dim() == 3:
            X = X.unsqueeze(0)  # Add batch dimension if missing
        
        # Batch processing of LED detection
        batch_results = []
        for single_image in X:
            image_results = []
            for color in self.COLORS:
                led_pos = self._detect_led(single_image, color)
                image_results.append(led_pos)
            batch_results.append(torch.stack(image_results))
        
        return torch.stack(batch_results)
    
    def _detect_led(self, image: torch.Tensor, color: torch.Tensor) -> torch.Tensor:
        # Ensure image is 2D (height, width, channels)
        if image.dim() == 3:
            image = image.permute(1, 2, 0)
        
        # Compute color difference
        diff = torch.abs(image - color).sum(dim=-1)
        
        # Find the minimum difference location
        min_idx = torch.argmin(diff)
        
        # Convert to 2D coordinates
        height, width = image.shape[:2]
        y = min_idx // width
        x = min_idx % width
        
        return torch.tensor([y, x], dtype=torch.float32)
    
    def extra_repr(self) -> str:
        return f'Colors: {self.COLORS}'

# Example of how to prepare for RKNN conversion
def prepare_for_rknn_export(model, example_input):
    """
    Prepare the model for RKNN export by tracing or converting to TorchScript
    
    Args:
        model (nn.Module): The trained model
        example_input (torch.Tensor): An example input tensor
    
    Returns:
        Traced or scripted model ready for RKNN conversion
    """
    model.eval()  # Set to evaluation mode
    
    # Ensure input has batch dimension
    if example_input.dim() == 3:
        example_input = example_input.unsqueeze(0)
    
    # Option 1: Tracing (recommended for most cases)
    traced_model = torch.jit.trace(model, example_input)
    
    # Option 2: Scripting (alternative method)
    # traced_model = torch.jit.script(model)
    
    return traced_model

# Example usage
if __name__ == '__main__':
    # Create an instance of the model
    led_detector = LedDetector()
    
    # Example inputs with and without explicit batch dimension
    example_input_with_batch = torch.rand(1, 3, 224, 224)  # Explicit batch
    example_input_without_batch = torch.rand(3, 224, 224)  # No batch dim
    
    # Prepare for RKNN export
    rknn_model_1 = prepare_for_rknn_export(led_detector, example_input_with_batch)
    rknn_model_2 = prepare_for_rknn_export(led_detector, example_input_without_batch)
    
    # Optional: Save the models for RKNN conversion
    torch.jit.save(rknn_model_1, 'led_detector_rknn_1.pt')
    torch.jit.save(rknn_model_2, 'led_detector_rknn_2.pt')