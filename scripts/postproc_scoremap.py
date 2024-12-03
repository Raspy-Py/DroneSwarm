import sys
import numpy as np
import matplotlib.pyplot as plt


WIDTH = 256
HEIGHT = 160

# Dequantization params
ZERO_POINT = 142
SCALE = 0.146175

def read_scoremap(filename):
    with open(filename, 'rb') as file:
        image_data = file.read()

    image = np.frombuffer(image_data, dtype=np.uint8)
    image = image.astype(np.float32)
    image = image - ZERO_POINT
    image /= SCALE


    z = np.linspace(-10, 10, 100)
    def sigmoid(z):
        return 1/(1 + np.exp(-z))

    def norm(x):
        return (x - np.min(x)) / (np.max(x) - np.min(x))

    image_sigm = norm(sigmoid(image))
    image_norm = norm(image)
    gamma = 0.5
    image = norm(image_sigm ** (1 / gamma) + image_norm ** gamma)
    image = image.reshape(HEIGHT, WIDTH)

    return image

def read_and_process_scoremap(filename):
    with open(filename, 'rb') as file:
        image_data = file.read()

    image = np.frombuffer(image_data, dtype=np.uint8)
    image = image.astype(np.float32)
    image = image - ZERO_POINT
    image /= SCALE

    def sigmoid(z):
        return 1/(1 + np.exp(-z))

    image = sigmoid(image) * 255

    # min_val = np.min(image)
    # max_val = np.max(image)

    # image = (image - min_val) / (max_val - min_val) * 255
    image = image.astype(np.uint8)

def plot_binary_heatmap(binary_array, filename='binary_heatmap.png'):
    """
    Plot a binary numpy array as a heatmap and save to file.

    Parameters:
    - binary_array (np.ndarray): 2D numpy array with binary values (0 or 1)
    - filename (str): Output filename for the heatmap image
    """
    # Create a new figure with a specific size
    plt.figure(figsize=(10, 8))

    # Create the heatmap using a yellow-blue colormap
    # The cmap='YlGnBu' provides a yellow to green to blue gradient
    plt.imshow(binary_array, cmap='viridis', interpolation='nearest')

    # Add a color bar
    plt.axis("off")

    # Add title and labels
    # plt.title('Scoremap')
    # Save the plot to a file

    plt.savefig(filename, dpi=300, bbox_inches='tight')

    # Close the plot to free up memory
    plt.close()

    print(f"Heatmap saved to {filename}")

# Example usage
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 postproc_scoremap.py scoremap.hm")
        exit()

    input_file = sys.argv[1]
    scoremap = read_scoremap(input_file)

    if len(sys.argv) > 2:
        output_file = sys.argv[2]
    else:
        output_file = input_file.replace('.hm', '.png')


    # Plot and save the heatmap
    plot_binary_heatmap(binary_array=scoremap, filename=output_file)
