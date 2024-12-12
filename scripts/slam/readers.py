import socket
import threading
import queue
from typing import Optional, Tuple, Any
import numpy as np

class NetworkReader:
    def __init__(self, broadcast_ip: str, port: int):

        self.broadcast_ip = broadcast_ip
        self.port = port
        self.packet_queue = queue.Queue()
        self.is_receiving = False
        self.receive_thread: Optional[threading.Thread] = None
        self._stop_event = threading.Event()
        
        # Initialize socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        
    def _receive_loop(self):
        self.sock.bind((self.broadcast_ip, self.port))
        
        self.sock.settimeout(0.5)
        
        while not self._stop_event.is_set():
            try:
                data, addr = self.sock.recvfrom(65535)  # Maximum UDP packet size
                print(f"received from: {addr}")
                self.packet_queue.put((data, addr))
            except socket.timeout:
                continue
            except Exception as e:
                print(f"Error receiving data: {e}")
                break
                
        self.is_receiving = False
        self.packet_queue.put(None)
        
    def start_receive(self):
        if self.is_receiving:
            raise RuntimeError("Already receiving packets")
            
        self.is_receiving = True
        self._stop_event.clear()
        self.receive_thread = threading.Thread(target=self._receive_loop)
        self.receive_thread.daemon = True
        self.receive_thread.start()
        
    def stop_receive(self):
        if not self.is_receiving:
            return
            
        self._stop_event.set()
        if self.receive_thread:
            self.receive_thread.join()
        self.receive_thread = None
        
    def get_next_packet(self) -> Optional[Tuple[bytes, Tuple[str, int]]]:
        try:
            packet = self.packet_queue.get()
            if packet is None:
                return None
            return packet
        except queue.Empty:
            if not self.is_receiving:
                return None
            raise
            
    def __enter__(self):
        self.start_receive()
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop_receive()

import os

class LocalReader:
    def __init__(self, data_folder, zero_point=142, scale=0.146175):
        self.data_folder = data_folder
        self.zero_point = zero_point
        self.scale = scale
        self.frame_idx = 0
        
        if not os.path.exists(data_folder):
            raise ValueError(f"Data folder does not exist: {data_folder}")
    
    def get_next_frame(self):
        ktps_file = os.path.join(self.data_folder, f'keypoints_{self.frame_idx}.bin')
        desc_file = os.path.join(self.data_folder, f'descriptors_{self.frame_idx}.bin')

        if not os.path.exists(ktps_file) or not os.path.exists(desc_file):
            return False, None, None

        with open(ktps_file, 'rb') as file:
            kpt_data = file.read()
        kpts = np.frombuffer(kpt_data, dtype=np.int32)
        kpts = kpts.reshape(2, 200)

        with open(desc_file, 'rb') as file:
                desc_data = file.read()
        desc = np.frombuffer(desc_data, dtype=np.uint8)
        desc = desc.reshape(200, 96) 
        desc = self._dequantize(desc, self.zero_point, self.scale)  
        desc = self._normalize_rows(desc)

        self.frame_idx+=1

        pts = np.array([(kpts[0, idx], kpts[1, idx]) for idx in range(kpts.shape[1])])

        return True, pts, desc

    @staticmethod
    def _dequantize(x, zero_point, scale):
        return (x.astype(np.float32) - zero_point) * scale

    @staticmethod
    def _normalize_rows(arr):
        arr = arr - np.min(arr, axis=1).reshape(-1, 1)
        row_norms = np.sqrt(np.sum(arr ** 2, axis=1))
        row_norms = np.maximum(row_norms, 1e-15)
        row_norms = row_norms.reshape(-1, 1)
        normalized = arr / row_norms
        
        return normalized
