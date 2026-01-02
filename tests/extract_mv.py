import sys
import numpy as np
import os
import gc
import cv2

so_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../build"))
sys.path.insert(0, so_dir)

import _ffmpeg_mv as mv

cap = mv.VideoReader("/home/vitalygalanov/DJI_0571.MP4")

n = 0
while True:
    n += 1
    ret, typ, frame, mvs = cap.read()

    print("________________________________________________")
    print("ret: ", ret)
    print("type: ", typ)
    print("frame: ", frame[0, 0])
    print("mvs: ", mvs.shape)
    

    cv2.imwrite(f"frame{n}.jpg", frame)