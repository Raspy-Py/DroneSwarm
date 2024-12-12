import numpy as np
import cv2 as cv
from matplotlib import pyplot as plt
 
img = cv.imread('simple.jpg', cv.IMREAD_GRAYSCALE)
 
# Initiate ORB detector
orb = cv.ORB_create()
 
# find the keypoints with ORB
kp = orb.detect(img,None)
 
# compute the descriptors with ORB
kp, des = orb.compute(img, kp)
print(f"des.type: {type(des)}")
print(f"des.shape: {des.shape}")
print(f"des.dtype: {des.dtype}")

print(f"kp.len: {len(kp)}")   
# # draw only keypoints location,not size and orientation
# img2 = cv.drawKeypoints(img, kp, None, color=(0,255,0), flags=0)
# plt.imshow(img2), 
# plt.savefig('simple_orb.jpg')