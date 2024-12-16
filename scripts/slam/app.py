import numpy as np

from readers import LocalReader
from slam import SLAM


K = np.array(
    [[3.230422251762199153e+02, 0.000000000000000000e+00, 2.508429145029543577e+02],
     [0.000000000000000000e+00, 3.217988176791898809e+02, 1.418251136300655446e+02],
     [0.000000000000000000e+00, 0.000000000000000000e+00, 1.000000000000000000e+00]]
)

D = np.array([-4.102526990617688663e-01, 7.005934167599912055e-01, -1.200855971916640723e-03, 1.166773271368007564e-03, -1.418690555704640754e+00])

P = K.copy()
P[0, 0] *= 0.8
P[1, 1] *= 0.8


def main():
    slam = SLAM(K)

    reader = LocalReader("../../data")

    while True:
        success, pts, desc = reader.get_next_frame()
        if not success:
            print("No more frames to read")
            break
        
        matched_ids, new_ids = slam.update(pts, desc)
        print(f"Matched: {len(matched_ids)}, New: {len(new_ids)}")


    # Cleanup
    reader.close()


if __name__ == "__main__":
    main()