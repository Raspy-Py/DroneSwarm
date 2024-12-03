import numpy as np
import cv2
import tqdm


WIDTH = 256
HEIGHT = 160

ZERO_POINT = 142
SCALE = 0.146175

KPT_DIRECTORY = '../tmp/kpts'

def softmax(x):
    """Compute softmax values for each sets of scores in x."""
    e_x = np.exp(x - np.max(x))
    return e_x / e_x.sum()


def sigmoid(z):
    return 1/(1 + np.exp(-z))

fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # or 'avc1' for mp4
out = cv2.VideoWriter(f'nice_video.mp4', fourcc, 10, (256, 160))

def postproc(image):
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
    image = image.reshape(160, 256)
    #image = cv2.cvtColor(image, cv2.COLOR_GRAY2BGR)

    return image


for i in tqdm.tqdm(range(1, 87)):
    with open(f'{KPT_DIRECTORY}/kpts{i}.bin', 'rb') as file:
        kpt_data = file.read()
    kpts = np.frombuffer(kpt_data, dtype=np.int32)
    kpts = kpts.reshape(2, 200)
    # kpts = kpts.reshape(200, 2)

    with open(f'../tmp/scrs/scrs{i}.hm', 'rb') as file:
        scoremap_data = file.read()
    scoremap = np.frombuffer(scoremap_data, dtype=np.uint8)
    scoremap = postproc(scoremap)
    

    with open(f'../tmp/mxpl/mxpl{i}.hm', 'rb') as file:
        maxpooled_data = file.read()
    maxpooled = np.frombuffer(maxpooled_data, dtype=np.uint8)
    #maxpooled = postproc(maxpooled)
    maxpooled = maxpooled.copy()
    maxpooled = maxpooled.reshape(160, 256) * 255


    # mark scoremap on the maxpooled as red
    #mean_score = np.mean(scoremap)
    #maxpooled[scoremap < mean_score] = 0
    image = cv2.cvtColor(scoremap, cv2.COLOR_GRAY2BGR)
    #print(np.min(maxpooled), np.max(maxpooled))
    # maxpooled[scoremap == 0] = [0, 0, 0]
    for i in range(200):
    # for kpt in kpts:
        x = kpts[0, i]
        y = kpts[1, i]
        if x > 256 or y > 160:
            print(f'(x; y): ({x}; {y})')

        image = cv2.circle(image, (x, y), 1, (0, 0, 255), -1)


    out.write(image)


out.release()
