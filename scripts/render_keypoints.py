import numpy as np
import cv2
import tqdm
KPT_DIRECTORY = '../tmp/kpts'

def softmax(x):
    """Compute softmax values for each sets of scores in x."""
    e_x = np.exp(x - np.max(x))
    return e_x / e_x.sum()


z = np.linspace(-10, 10, 100) 
def sigmoid(z):
    return 1/(1 + np.exp(-z))

fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # or 'avc1' for mp4
out = cv2.VideoWriter(f'sigm_scores.mp4', fourcc, 15, (256, 160))

for i in tqdm.tqdm(range(1, 149)):
    with open(f'{KPT_DIRECTORY}/kpts{i}.bin', 'rb') as file:
        kpt_data = file.read()
    kpts = np.frombuffer(kpt_data, dtype=np.int32)
    kpts = kpts.reshape(200, 2)

    with open(f'../tmp/scrs/scrs{i}.hm', 'rb') as file:
        image_data = file.read()
    image = np.frombuffer(image_data, dtype=np.uint8)
    image = image.astype(np.float32)
    image = image - 142
    image /= 0.146175

    image = sigmoid(image) * 255


    min_val = np.min(image)
    max_val = np.max(image)

    image = (image - min_val) / (max_val - min_val) * 255
    image = image.astype(np.uint8)
    image = image.reshape(160, 256)
    image = cv2.cvtColor(image, cv2.COLOR_GRAY2BGR)
    image = image.copy()

    # for kpt in kpts:
        # x, y = kpt

        # x = int(x)
        # y = int(y)

        # image = cv2.circle(image, (x, y), 1, (0, 0, 255), -1)

    out.write(image)

out.release()
