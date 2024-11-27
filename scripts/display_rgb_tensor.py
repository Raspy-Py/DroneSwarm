import cv2
import numpy as np

with open('../tmp/scrs/scrs30.hm', 'rb') as file:
    image_data = file.read()

image = np.frombuffer(image_data, dtype=np.uint8)
image = image.astype(np.float32)
image = image - 142
image /= 0.146175


min_val = np.min(image)
max_val = np.max(image)

image = (image - min_val) / (max_val - min_val) * 255
image = image.astype(np.uint8)
image = image.reshape(160, 256)


cv2.imshow('Raw RGB Image', image)
cv2.waitKey(0)
cv2.destroyAllWindows()
