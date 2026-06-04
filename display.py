import serial
import numpy as np
import cv2

SERIAL_PORT='COM13'
BAUD_RATE=115200
WIDTH=128
HEIGHT=128
FRAME_SIZE=WIDTH*HEIGHT*2  #2B per px
counter=0

def process_frame(raw_data, write_to_file=False):
    global counter
    data=np.frombuffer(raw_data, dtype='>u2')
    r=((data>>11)&0x1F)<<3
    g=((data>>5)&0x3F)<<2
    b=((data)&0x1F)<<3

    rgb888=np.stack((b,g,r), axis=-1).astype(np.uint8)
    rgb888=rgb888.reshape(HEIGHT, WIDTH, 3);

    if write_to_file==True:
        cv2.imwrite(f"img{counter}.bmp", rgb888)
        counter+=1

    rgb888=cv2.resize(rgb888, (4*WIDTH, 4*HEIGHT), interpolation=cv2.INTER_NEAREST)
    cv2.imshow('ESP cam', rgb888)
    cv2.waitKey(1)


def main():
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=5) as sm:
            sm.reset_input_buffer()
            print(f"listening on {SERIAL_PORT}")
            while True:
                cv2.waitKey(1)
                if sm.in_waiting>0:
                    discarded=sm.read_until(b"FRAME")
                    print(f"Rx before/including header: {discarded}")

                    if discarded.endswith(b"FRAME"):
                        print("Header found")
                        raw_frame=sm.read(FRAME_SIZE)

                        if len(raw_frame)==FRAME_SIZE:
                            process_frame(raw_frame, True)
                        else:
                            print(f"Incomplete frame: {len(raw_frame)}/{FRAME_SIZE}")
        
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"Encountered Exception: {e}")
    finally:
        cv2.destroyAllWindows()

if __name__=="__main__":
    main()