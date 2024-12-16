from multiprocessing import Process, Queue
import numpy as np
import cv2
import math

class Map(object):
    def __init__(self):
        self.frames = []
        self.points = []
        self.state = None
        self.video_writer = None
        self.frame_count = 0
        
    def create_viewer(self):
        self.video_writer = cv2.VideoWriter(
            'big-top-down.mp4',
            cv2.VideoWriter_fourcc(*'mp4v'),
            10,
            (800, 600)
        )
        self.current_points = None
        self.current_poses = None
        
    def project_top_down(self, points):
        # For top-down view, we just use X and Z coordinates
        points_2d = points[:, [0, 2]]  # Using X and Z for top-down view
        return points_2d

    def render_frame(self):
        frame = np.ones((600, 800, 3), dtype=np.uint8) * 64  # dark gray background

        if self.current_points is not None and self.current_poses is not None:
            points = self.current_points
            poses = self.current_poses

            # Calculate scale based on camera positions only
            if len(poses) > 0:
                camera_positions = np.array([pose[:3, 3] for pose in poses])
                camera_positions_2d = self.project_top_down(camera_positions)
                
                # Find bounds of camera trajectory
                min_vals = np.min(camera_positions_2d, axis=0)
                max_vals = np.max(camera_positions_2d, axis=0)
                
                # Add padding
                padding = (max_vals - min_vals) * 0.1
                min_vals -= padding
                max_vals += padding
                
                # Calculate scale to fit camera trajectory
                scale_x = 700 / (max_vals[0] - min_vals[0]) if max_vals[0] != min_vals[0] else 1
                scale_z = 500 / (max_vals[1] - min_vals[1]) if max_vals[1] != min_vals[1] else 1
                scale = min(scale_x, scale_z)
                
                # Calculate center based on camera trajectory
                center_x = (min_vals[0] + max_vals[0]) / 2
                center_z = (min_vals[1] + max_vals[1]) / 2
                
                # Draw points using camera-based scaling
                if len(points) > 0:
                    points_2d = self.project_top_down(points)
                    points_screen = np.zeros_like(points_2d)
                    points_screen[:, 0] = (points_2d[:, 0] - center_x) * scale + 400
                    points_screen[:, 1] = (points_2d[:, 1] - center_z) * scale + 300
                    
                    points_screen = points_screen.astype(np.int32)
                    
                    # Draw points
                    for point in points_screen:
                        if 0 <= point[0] < 800 and 0 <= point[1] < 600:
                            cv2.circle(frame, tuple(point), 1, (210, 210, 210), -1)
                
                # Draw camera positions
                for pose in poses:
                    pos = pose[:3, 3]
                    pos_2d = self.project_top_down(pos.reshape(1, 3))[0]
                    
                    # Transform to screen coordinates
                    screen_x = int((pos_2d[0] - center_x) * scale + 400)
                    screen_y = int((pos_2d[1] - center_z) * scale + 300)
                    
                    if 0 <= screen_x < 800 and 0 <= screen_y < 600:
                        cv2.circle(frame, (screen_x, screen_y), 3, (0, 255, 0), -1)

        # Add frame counter and compass directions
        cv2.putText(frame, f"Frame: {self.frame_count}", (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
        
        cv2.putText(frame, "N", (400, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
        cv2.putText(frame, "S", (400, 570), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
        cv2.putText(frame, "E", (760, 300), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
        cv2.putText(frame, "W", (40, 300), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

        # Write frame to video
        if self.video_writer is not None:
            self.video_writer.write(frame)

        self.frame_count += 1

    def display(self):
        poses = [f.pose for f in self.frames if hasattr(f, 'pose')]
        pts = [p.pt for p in self.points if hasattr(p, 'pt')]

        if poses or pts:
            self.current_poses = np.array(poses)
            self.current_points = np.array(pts)
            self.render_frame()

    def display_image(self, ip_image):
        self.render_frame()

    def __del__(self):
        if hasattr(self, 'video_writer') and self.video_writer is not None:
            self.video_writer.release()

    def release(self):
        if self.video_writer is not None:
            self.video_writer.release()

class Point(object):
    def __init__(self, mapp, loc):
        self.frames = []
        self.pt = loc
        self.idxs = []
        self.id = len(mapp.points)
        mapp.points.append(self)

    def add_observation(self, frame, idx):
        self.frames.append(frame)
        self.idxs.append(idx)