import cv2
import numpy as np

with open('./data/output.rgb', 'rb') as file:
    image_data = file.read()

image = np.frombuffer(image_data, dtype=np.uint8)
image = image.reshape(1080, 1920, 3)

cv2.imshow('Raw RGB Image', image)
cv2.waitKey(0)
cv2.destroyAllWindows()
