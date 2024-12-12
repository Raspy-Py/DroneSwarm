import socket
import struct
import numpy as np
import open3d as o3d
import threading
from dataclasses import dataclass

class SLAMVisualizer:
    def __init__(self, broadcast_ip="255.255.255.255", port=12345):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.sock.bind(("", port))
        
        self.vis = o3d.visualization.Visualizer()
        self.vis.create_window("SLAM Keypoints Visualizer")
        
        coord_frame = o3d.geometry.TriangleMesh.create_coordinate_frame(size=1.0)
        self.vis.add_geometry(coord_frame)
        
        self.pcd = o3d.geometry.PointCloud()
        self.vis.add_geometry(self.pcd)
        
        self.is_running = True
        
    def receive_frame(self):
        """Receive and decode a frame from UDP broadcast"""
        data, addr = self.sock.recvfrom(65535)  # Max UDP packet size
        
        frame_id = struct.unpack('i', data[:4])[0]
        
        kp_rows = struct.unpack('i', data[4:8])[0]
        kp_cols = struct.unpack('i', data[8:12])[0]
        desc_rows = struct.unpack('i', data[12:16])[0]
        desc_cols = struct.unpack('i', data[16:20])[0]
        
        # Extract keypoints and descriptors
        offset = 20
        keypoints_size = kp_rows * kp_cols * 4  # 4 bytes per int32
        keypoints = np.frombuffer(data[offset:offset+keypoints_size], dtype=np.int32)
        keypoints = keypoints.reshape((kp_rows, kp_cols))
        
        offset += keypoints_size
        descriptors = np.frombuffer(data[offset:], dtype=np.float32)
        descriptors = descriptors.reshape((desc_rows, desc_cols))
        
        return Frame(frame_id, keypoints, descriptors)
    
    def update_visualization(self, frame):
        """Update the 3D visualization with new keypoints"""
        # Convert keypoints to point cloud
        points = frame.keypoints.astype(np.float64)  # Convert to 3D points if necessary
        self.pcd.points = o3d.utility.Vector3dVector(points)
        
        # Add colors based on descriptors (optional)
        colors = np.zeros((len(points), 3))
        colors[:, 0] = 1.0  # Red color for all points, can be modified based on descriptors
        self.pcd.colors = o3d.utility.Vector3dVector(colors)
        
        # Update visualization
        self.vis.update_geometry(self.pcd)
        self.vis.poll_events()
        self.vis.update_renderer()
    
    def network_thread(self):
        """Thread for receiving network data"""
        while self.is_running:
            try:
                frame = self.receive_frame()
                self.update_visualization(frame)
            except Exception as e:
                print(f"Error receiving frame: {e}")
    
    def run(self):
        """Main run loop"""
        # Start network thread
        network_thread = threading.Thread(target=self.network_thread)
        network_thread.start()
        
        # Run visualization
        while self.is_running:
            if not self.vis.poll_events():
                self.is_running = False
            self.vis.update_renderer()
        
        # Cleanup
        self.vis.destroy_window()
        self.sock.close()

if __name__ == "__main__":
    visualizer = SLAMVisualizer()
    visualizer.run()