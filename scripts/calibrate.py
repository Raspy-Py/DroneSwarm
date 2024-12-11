"""
Calibration script
"""
import cv2 
import numpy as np 
import os 
import glob
import argparse
from datetime import datetime

def parse_arguments():
    """
    Parse command-line arguments for camera calibration script.
    """
    parser = argparse.ArgumentParser(description='Camera Calibration Script')
    parser.add_argument('-d', '--directory', 
                        type=str, 
                        default='.', 
                        help='Directory containing calibration images (default: current directory)')
    parser.add_argument('-p', '--pattern', 
                        type=str, 
                        default='*.jpg', 
                        help='Image file pattern to match (default: *.jpg)')
    parser.add_argument('-c', '--checkerboard', 
                        nargs=2, 
                        type=int, 
                        default=[6, 8], 
                        help='Checkerboard dimensions (width height, default: 6 8)')
    return parser.parse_args()

def main():
    args = parse_arguments()

    CHECKERBOARD = tuple(args.checkerboard)

    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001) 

    threedpoints = [] 
    twodpoints = [] 

    objectp3d = np.zeros((1, CHECKERBOARD[0] * CHECKERBOARD[1], 3), np.float32) 
    objectp3d[0, :, :2] = np.mgrid[0:CHECKERBOARD[0], 0:CHECKERBOARD[1]].T.reshape(-1, 2) 

    image_path = os.path.join(args.directory, args.pattern)
    images = glob.glob(image_path)
    
    if not images:
        print(f"No images found in {args.directory} matching pattern {args.pattern}")
        return

    print(f"Found {len(images)} images for calibration")

    for filename in images: 
        image = cv2.imread(filename) 
        grayColor = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY) 

        ret, corners = cv2.findChessboardCorners( 
                grayColor, CHECKERBOARD, 
                cv2.CALIB_CB_ADAPTIVE_THRESH + 
                cv2.CALIB_CB_FAST_CHECK +
                cv2.CALIB_CB_NORMALIZE_IMAGE) 

        if ret: 
            threedpoints.append(objectp3d) 


            corners2 = cv2.cornerSubPix( 
                grayColor, corners, (11, 11), (-1, -1), criteria) 

            twodpoints.append(corners2)  

            image_with_corners = cv2.drawChessboardCorners(image, 
                                CHECKERBOARD, 
                                corners2, ret) 

            cv2.imshow('Detected Corners', image_with_corners) 
            cv2.waitKey(0) 

    cv2.destroyAllWindows() 

    if not threedpoints:
        print("No valid calibration images found.")
        return

    h, w = image.shape[:2] 

    ret, matrix, distortion, r_vecs, t_vecs = cv2.calibrateCamera( 
        threedpoints, twodpoints, grayColor.shape[::-1], None, None) 

    print("\nCamera Calibration Results:")
    print(" Camera matrix:") 
    print(matrix) 
    print("\n Distortion coefficient:") 
    print(distortion) 
    print("\n Rotation Vectors:") 
    print(r_vecs) 
    print("\n Translation Vectors:") 
    print(t_vecs) 

    calibration_dir = 'calibration'
    if not os.path.exists(calibration_dir):
        os.makedirs(calibration_dir)


    current_time = datetime.now().strftime('%Y%m%d_%H%M%S')
    output_dir_np = os.path.join(calibration_dir, current_time)
    output_dir_txt = os.path.join(calibration_dir, current_time)

    os.makedirs(output_dir_np, exist_ok=True)
    os.makedirs(output_dir_txt, exist_ok=True)

    np.save(os.path.join(output_dir_np, 'camera_matrix.npy'), matrix)
    np.save(os.path.join(output_dir_np, 'distortion_coefficients.npy'), distortion)
    np.save(os.path.join(output_dir_np, 'rotation_vectors.npy'), r_vecs)
    np.save(os.path.join(output_dir_np, 'translation_vectors.npy'), t_vecs)

    np.savetxt(os.path.join(output_dir_txt, 'camera_matrix.txt'), matrix)
    np.savetxt(os.path.join(output_dir_txt, 'distortion_coefficients.txt'), distortion)

    print(f"\nCalibration results saved in {output_dir_np}")

if __name__ == "__main__":
    main()