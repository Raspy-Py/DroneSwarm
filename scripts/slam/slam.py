import numpy as np
import cv2
from scipy.spatial.transform import Rotation
import copy

class SLAM:
    def __init__(self, K):
        """
        Initialize SLAM system
        Args:
            K: 3x3 camera intrinsic matrix
        """
        self.K = K
        self.K_inv = np.linalg.inv(K)
        
        # Storage for keypoints and their properties
        self.keypoint_ids = []  # List of unique IDs for each keypoint
        self.next_id = 0        # Counter for generating unique IDs
        self.keypoints_3d = {}  # Dictionary mapping IDs to 3D positions
        self.descriptors_db = {} # Dictionary mapping IDs to descriptors
        
        # Previous frame data
        self.pts_prev = None
        self.desc_prev = None
        self.ids_prev = None
        
        # Camera pose (R|t) for each frame
        self.poses = [np.eye(4)]  # Start with identity pose
        self.current_pose = np.eye(4)
    
    def mnn_matcher(self, desc1, desc2):
        """
        Mutual nearest neighbor matching of descriptors
        """
        sim = desc1 @ desc2.transpose()
        sim[sim < 0.9] = 0
        nn12 = np.argmax(sim, axis=1)
        nn21 = np.argmax(sim, axis=0)
        ids1 = np.arange(0, sim.shape[0])
        mask = (ids1 == nn21[nn12])
        matches = np.stack([ids1[mask], nn12[mask]])
        return matches.transpose()
    
    def triangulate_points(self, pts1, pts2, pose1, pose2):
        """
        Triangulate 3D points from two views
        """
        # Convert points to normalized coordinates
        pts1_norm = (self.K_inv @ np.vstack((pts1.T, np.ones(len(pts1)))))[:3]
        pts2_norm = (self.K_inv @ np.vstack((pts2.T, np.ones(len(pts2)))))[:3]
        
        # Create projection matrices
        P1 = pose1[:3]  # 3x4 projection matrix for first camera
        P2 = pose2[:3]  # 3x4 projection matrix for second camera
        
        # Triangulate
        pts4d = cv2.triangulatePoints(P1, P2, pts1_norm[:2], pts2_norm[:2])
        pts3d = pts4d[:3] / pts4d[3:]
        return pts3d.T
    
    def estimate_pose(self, pts1, pts2):
        """
        Estimate relative pose between frames using Essential matrix
        """
        # Convert points to normalized coordinates
        pts1_norm = (self.K_inv @ np.vstack((pts1.T, np.ones(len(pts1)))))[:3].T
        pts2_norm = (self.K_inv @ np.vstack((pts2.T, np.ones(len(pts2)))))[:3].T
        
        # Extract x,y coordinates for findEssentialMat
        pts1_norm = pts1_norm[:, :2]  # Take only x,y coordinates
        pts2_norm = pts2_norm[:, :2]  # Take only x,y coordinates
        
        # Ensure points are in the correct format
        pts1_norm = np.float32(pts1_norm)
        pts2_norm = np.float32(pts2_norm)
        
        # Find essential matrix
        E, mask = cv2.findEssentialMat(
            pts1_norm, pts2_norm,
            focal=1.0, pp=(0., 0.),  # Already normalized coordinates
            method=cv2.RANSAC,
            prob=0.999,
            threshold=0.001
        )
        
        # Recover pose from essential matrix
        _, R, t, mask = cv2.recoverPose(E, pts1_norm, pts2_norm)
        
        # Create 4x4 transformation matrix
        pose = np.eye(4)
        pose[:3, :3] = R
        pose[:3, 3] = t.flatten()
        
        return pose, mask.flatten()
    
    def update(self, pts, desc):
        """
        Update SLAM with new keypoints and descriptors
        Returns:
            matched_ids: IDs of matched keypoints
            new_ids: IDs of new keypoints
        """
        if self.pts_prev is None:
            # First frame - initialize keypoints
            self.ids_prev = np.array([self.next_id + i for i in range(len(pts))])
            self.next_id += len(pts)
            
            # Store descriptors
            for id_, desc_ in zip(self.ids_prev, desc):
                self.descriptors_db[id_] = desc_
            
            self.pts_prev = pts
            self.desc_prev = desc
            
            return self.ids_prev, []
        
        # Match keypoints with previous frame
        matches = self.mnn_matcher(self.desc_prev, desc)
        if len(matches) < 8:  # Need at least 8 points for pose estimation
            return [], []
            
        # Get matched points
        pts1 = self.pts_prev[matches[:, 0]]
        pts2 = pts[matches[:, 1]]
        
        # Estimate relative pose
        relative_pose, inlier_mask = self.estimate_pose(pts1, pts2)
        
        # Update current pose
        self.current_pose = self.current_pose @ relative_pose
        self.poses.append(self.current_pose)
        
        # Filter matches by inliers
        matches = matches[inlier_mask.astype(bool)]
        pts1 = pts1[inlier_mask.astype(bool)]
        pts2 = pts2[inlier_mask.astype(bool)]
        
        # Assign IDs to matched points
        matched_ids = self.ids_prev[matches[:, 0]]
        
        # Create new IDs for unmatched points
        matched_indices = set(matches[:, 1])
        unmatched_indices = set(range(len(pts))) - matched_indices
        new_ids = np.array([self.next_id + i for i in range(len(unmatched_indices))])
        self.next_id += len(unmatched_indices)
        
        # Update descriptors database
        for id_, desc_ in zip(matched_ids, desc[matches[:, 1]]):
            self.descriptors_db[id_] = desc_
        for id_, desc_ in zip(new_ids, desc[list(unmatched_indices)]):
            self.descriptors_db[id_] = desc_
        
        # Triangulate new 3D points
        new_pts3d = self.triangulate_points(pts1, pts2, 
                                          self.poses[-2], self.poses[-1])
        
        # Update 3D points database
        for id_, pt3d in zip(matched_ids, new_pts3d):
            if id_ not in self.keypoints_3d:
                self.keypoints_3d[id_] = pt3d
            else:
                # Simple averaging for updating existing 3D points
                self.keypoints_3d[id_] = (self.keypoints_3d[id_] + pt3d) / 2
        
        # Update previous frame data
        self.pts_prev = pts
        self.desc_prev = desc
        ids_current = np.zeros(len(pts), dtype=int)
        ids_current[matches[:, 1]] = matched_ids
        ids_current[list(unmatched_indices)] = new_ids
        self.ids_prev = ids_current
        
        return matched_ids, new_ids
    
    def get_localized_points(self):
        """
        Returns list of currently localized points as (id, 3D-position) pairs
        """
        return [(id_, pos) for id_, pos in self.keypoints_3d.items()]