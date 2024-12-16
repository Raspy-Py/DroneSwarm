import numpy as np
import cv2
import tqdm
import copy


ZERO_POINT=142, 
SCALE=0.146175

class SimpleTracker(object):
    def __init__(self):
        self.pts_prev = None
        self.desc_prev = None

    def update(self, img, pts, desc):
        N_matches = 0
        if self.pts_prev is None:
            self.pts_prev = pts
            self.desc_prev = desc

            out = copy.deepcopy(img)
            for pt1 in pts:
                p1 = (int(round(pt1[0])), int(round(pt1[1])))
                cv2.circle(out, p1, 1, (0, 0, 255), -1, lineType=16)
        else:
            matches = self.mnn_mather(self.desc_prev, desc)
            mpts1, mpts2 = self.pts_prev[matches[:, 0]], pts[matches[:, 1]]
            N_matches = len(matches)

            # if distance between two matches is bigger than avarage in K times throw match away
            K = 2   
            max_dist = K * np.mean(np.linalg.norm(mpts1 - mpts2, axis=1))
            for i in range(len(matches)):
                if np.linalg.norm(mpts1[i] - mpts2[i]) > max_dist:
                    matches[i, :] = [-1, -1]
            matches = matches[matches != [-1, -1]].reshape(-1, 2)
            mpts1, mpts2 = self.pts_prev[matches[:, 0]], pts[matches[:, 1]]
            N_matches = len(matches)


            out = copy.deepcopy(img)
            for pt1, pt2 in zip(mpts1, mpts2):
                p1 = (int(round(pt1[0])), int(round(pt1[1])))
                p2 = (int(round(pt2[0])), int(round(pt2[1])))
                # cv2.line(out, p1, p2, (0, 255, 0), lineType=16)
                cv2.circle(out, p2, 1, (0, 0, 255), -1, lineType=16)

            self.pts_prev = pts
            self.desc_prev = desc



        return out, N_matches

    def mnn_mather(self, desc1, desc2):
        sim = desc1 @ desc2.transpose()
        sim[sim < 0.9] = 0
        nn12 = np.argmax(sim, axis=1)
        nn21 = np.argmax(sim, axis=0)
        ids1 = np.arange(0, sim.shape[0])
        mask = (ids1 == nn21[nn12])
        matches = np.stack([ids1[mask], nn12[mask]])
        return matches.transpose()


WIDTH = 256
HEIGHT = 160



KPT_DIRECTORY = '../data'

def softmax(x):
    """Compute softmax values for each sets of scores in x."""
    e_x = np.exp(x - np.max(x))
    return e_x / e_x.sum()


def sigmoid(z):
    return 1/(1 + np.exp(-z))


def dequantize(x, zero_point, scale):
    return (x.astype(np.float32) - zero_point) * scale


def create_video_writer(output_files, width, height, fps):
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # or 'avc1' for mp4
    return cv2.VideoWriter(output_files, fourcc, fps, (width, height))


def normalize_rows(arr):
    arr = arr - np.min(arr, axis=1).reshape(-1, 1)
    row_norms = np.sqrt(np.sum(arr ** 2, axis=1))
    row_norms = np.maximum(row_norms, 1e-15)
    row_norms = row_norms.reshape(-1, 1)
    normalized = arr / row_norms
    
    return normalized

kpts_out = create_video_writer('only-points.mp4', WIDTH, HEIGHT, 10)
#desc_out = create_video_writer('descriptors.mp4', WIDTH, HEIGHT, 10)

prev_kpts = None
prev_desc = None

tracker = SimpleTracker()

for i in tqdm.tqdm(range(1, 47)):
    with open(f'{KPT_DIRECTORY}/keypoints_{i}.bin', 'rb') as file:
        kpt_data = file.read()
    kpts = np.frombuffer(kpt_data, dtype=np.int32)
    kpts = kpts.reshape(2, 200)


    with open(f'{KPT_DIRECTORY}/descriptors_{i}.bin', 'rb') as file:
        desc_data = file.read()
    desc = np.frombuffer(desc_data, dtype=np.uint8)
    desc = desc.reshape(200, 96) 
    desc = dequantize(desc, ZERO_POINT, SCALE)
    desc = normalize_rows(desc)

    with open(f'{KPT_DIRECTORY}/image_{i * 2 + 1}.bin', 'rb') as file:
        image_data = file.read()
    image = np.frombuffer(image_data, dtype=np.uint8)
    image = image.reshape(HEIGHT, WIDTH, 3).copy()
    # kpts_out.write(image); continue


    # image
    # min_desc = np.min(desc)
    # max_desc = np.max(desc)
    # desc_image = (desc - min_desc) / (max_desc - min_desc)
    # desc_image = desc_image * 255
    # desc_image = desc_image.astype(np.uint8)
    # desc_image = cv2.cvtColor(desc_image, cv2.COLOR_GRAY2BGR)
    # desc_image = cv2.resize(desc_image, (WIDTH, HEIGHT))

    # craete blanck rgb image

    out, n_matches = tracker.update(image, kpts.T, desc)

    # for i in range(200):
    # # for kpt in kpts:
    #     x = kpts[0, i]
    #     y = kpts[1, i]
    #     image = cv2.circle(image, (x, y), 1, (0, 0, 255), -1)
    
    # if prev_kpts is not None:
    #     for i in range(200):
    #         x1 = prev_kpts[0, i]
    #         y1 = prev_kpts[1, i]
    #         x2 = kpts[0, i]
    #         y2 = kpts[1, i]
    #         image = cv2.line(image, (x1, y1), (x2, y2), (0, 255, 0), 1)


    kpts_out.write(out)
    #desc_out.write(desc_image)

kpts_out.release()
#desc_out.release()
