"""
Calibration sript
"""

import cv2 
import numpy as np 
import os 
import glob
from datetime import datetime


# checkboard dimensions
CHECKERBOARD = (6, 8) 

criteria = (cv2.TERM_CRITERIA_EPS +
      cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001) 

threedpoints = [] 

twodpoints = [] 

objectp3d = np.zeros((1, CHECKERBOARD[0] 
          * CHECKERBOARD[1], 
          3), np.float32) 
objectp3d[0, :, :2] = np.mgrid[0:CHECKERBOARD[0], 
              0:CHECKERBOARD[1]].T.reshape(-1, 2) 
prev_img_shape = None

images = glob.glob('*.jpg') 
print(images)

for filename in images: 
  image = cv2.imread(filename) 
  grayColor = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY) 

  ret, corners = cv2.findChessboardCorners( 
          grayColor, CHECKERBOARD, 
          cv2.CALIB_CB_ADAPTIVE_THRESH 
          + cv2.CALIB_CB_FAST_CHECK +
          cv2.CALIB_CB_NORMALIZE_IMAGE) 

  if ret == True: 
    threedpoints.append(objectp3d) 

    corners2 = cv2.cornerSubPix( 
      grayColor, corners, (11, 11), (-1, -1), criteria) 

    twodpoints.append(corners2)  
    image = cv2.drawChessboardCorners(image, 
                    CHECKERBOARD, 
                    corners2, ret) 

  cv2.imshow('img', image) 
  cv2.waitKey(0) 

cv2.destroyAllWindows() 

h, w = image.shape[:2] 

ret, matrix, distortion, r_vecs, t_vecs = cv2.calibrateCamera( 
  threedpoints, twodpoints, grayColor.shape[::-1], None, None) 

print(" Camera matrix:") 
print(matrix) 

print("\n Distortion coefficient:") 
print(distortion) 

print("\n Rotation Vectors:") 
print(r_vecs) 

print("\n Translation Vectors:") 
print(t_vecs) 

if not os.path.exists('calibration'):
    os.makedirs('calibration')

current_time = datetime.now().strftime('%Y%m%d_%H%M%S')
output_dir_np = os.path.join('calibration', current_time)
output_dir_txt = os.path.join('calibration', current_time)

os.makedirs(output_dir_np, exist_ok=True)
os.makedirs(output_dir_txt, exist_ok=True)

np.save(os.path.join(output_dir_np, 'camera_matrix.npy'), matrix)
np.save(os.path.join(output_dir_np, 'distortion_coefficients.npy'), distortion)
np.save(os.path.join(output_dir_np, 'rotation_vectors.npy'), r_vecs)
np.save(os.path.join(output_dir_np, 'translation_vectors.npy'), t_vecs)

np.savetxt(os.path.join(output_dir_txt, 'camera_matrix.txt'), matrix)
np.savetxt(os.path.join(output_dir_txt, 'distortion_coefficients.txt'), distortion)
np.savetxt(os.path.join(output_dir_txt, 'rotation_vectors.txt'), r_vecs)
np.savetxt(os.path.join(output_dir_txt, 'translation_vectors.txt'), t_vecs)
