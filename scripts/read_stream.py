import cv2
import sys


DEFAULT_PORT = 554
DEFAULT_STREAM = 'main_stream'


def main(port, stream):
    rtsp_url = f'rtsp://192.168.55.1:{port}/live/{stream}'
    cap = cv2.VideoCapture(rtsp_url)

    if not cap.isOpened():
        print("Cannot open RTSP stream")
        exit()

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame")
            break

        cv2.imshow('RTSP Stream', frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == '__main__':
    try:
        port = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_PORT
        stream = sys.argv[2] if len(sys.argv) > 2 else DEFAULT_STREAM
    except:
        print("Usage: python read_stream.py [port] [stream]")
        print("args:")
        print("\tport: RTSP port number (default: 554)")
        print("\tstream: RTSP stream name (default: main_stream)")
        exit()

    main(port, stream)
