#!/usr/bin/env python

# Python 2/3 compatibility
from __future__ import print_function

import numpy as np
import cv2 as cv
import argparse

# built-in modules
import os
import sys
import getopt
from glob import glob

# 罗格利斯旋转向量转旋转矩阵
def Rodriguez(rvecs):
    # 旋转向量模长
    θ = (rvecs[0] * rvecs[0] + rvecs[1] * rvecs[1] + rvecs[2] * rvecs[2])**(1/2)
    # 旋转向量的单位向量
    r = rvecs / θ
    # 旋转向量单位向量的反对称矩阵
    anti_r = np.array([
        [0, -r[2], r[1]],
        [r[2], 0, -r[0]],
        [-r[1], r[0], 0]
    ])
    # 旋转向量转旋转矩阵(Rodriguez公式)     # np.outer(r, r) = r @ r.T 向量外积
    M = np.eye(3) * np.cos(θ) + (1 - np.cos(θ)) * np.outer(r, r) + np.sin(θ) * anti_r
    return M


def main():
    usge = '''
    camera calibration for distorted images with chess board samples
    reads distorted images, calculates the calibration and write undistorted images

    usage:
        calibrate.py <image mask> [--output_dir <output path>] [--square_size <>] [--threads <>]

    example values:
        --output_dir:  ./output/
        --square_size: 1.0
        --pattern_size: (5,9)
        <image mask> defaults to ./data/

    '''

    arg_parser = argparse.ArgumentParser(
        description=usge, formatter_class=argparse.RawTextHelpFormatter)
    arg_parser.add_argument("-o", "--output_dir", default="./output")
    arg_parser.add_argument("-s", "--square_size", default=0.12, type=float)
    arg_parser.add_argument("-t", "--threads", default=1, type=int)
    arg_parser.add_argument("-p", "--pattern_size", default="(5,9)", help="chessboard corners pattern size")
    arg_parser.add_argument("img_dir")
    args = arg_parser.parse_args()

    img_names = [ os.path.join(args.img_dir, n) for n in os.listdir(args.img_dir)]

    print("find image:", len(img_names))

    debug_dir = args.output_dir
    if debug_dir and not os.path.isdir(debug_dir):
        os.mkdir(debug_dir)

    square_size = float(args.square_size)

    pattern_size = (5, 9)
    pattern_size = eval(args.pattern_size)
    pattern_points = np.zeros((np.prod(pattern_size), 3), np.float32)
    pattern_points[:, :2] = np.indices(pattern_size).T.reshape(-1, 2)
    pattern_points *= square_size

    print(square_size, pattern_size)
    print(pattern_points.shape)
    # print(pattern_points)

    obj_points = []
    img_points = []
    # TODO: use imquery call to retrieve results
    h, w = cv.imread(img_names[0], cv.IMREAD_GRAYSCALE).shape[:2]

    def processImage(fn):
        print('processing %s... ' % fn)
        img = cv.imread(fn, 0)
        if img is None:
            print("Failed to load", fn)
            return None

        assert w == img.shape[1] and h == img.shape[0], ("size: %d x %d ... " % (
            img.shape[1], img.shape[0]))
        # found, corners = cv.findChessboardCorners(img, pattern_size, flags=cv.CALIB_CB_ADAPTIVE_THRESH
        #                                           + cv.CALIB_CB_EXHAUSTIVE)

        # found, corners = cv.findCirclesGrid(img, pattern_size)

        found, corners = cv.findChessboardCorners(img, pattern_size, flags=cv.CALIB_CB_ADAPTIVE_THRESH
                                                  )

        if found:
            term = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 30, 0.001)
            corners2 = cv.cornerSubPix(img, corners, (11, 11), (-1, -1), term)
            # print(np.allclose(corners, corners2))
            corners = corners2

        if debug_dir:
            vis = cv.cvtColor(img, cv.COLOR_GRAY2BGR)
            cv.drawChessboardCorners(vis, pattern_size, corners, found)
            name = os.path.basename(fn)
            outfile = os.path.join(debug_dir, name + '_chess.png')
            cv.imwrite(outfile, vis)
            # cv.imshow("img", vis)
            # cv.waitKey(0)

        if not found:
            print('chessboard not found')
            return None

        print('           %s... OK' % fn)
        return (corners.reshape(-1, 2), pattern_points)

    threads_num = int(args.threads)
    if threads_num <= 1:
        chessboards = [processImage(fn) for fn in img_names]
    else:
        print("Run with %d threads..." % threads_num)
        from multiprocessing.dummy import Pool as ThreadPool
        pool = ThreadPool(threads_num)
        chessboards = pool.map(processImage, img_names)

    chessboards = [x for x in chessboards if x is not None]
    for (corners, pattern_points) in chessboards:
        img_points.append(corners)
        obj_points.append(pattern_points)

    # calculate camera distortion
    rms, camera_matrix, dist_coefs, rvecs, tvecs = cv.calibrateCamera(
        obj_points, img_points, (w, h), None, None)

    np.set_printoptions(precision=3, suppress=True)
    camera_matrix = np.around(camera_matrix, 3)
    dist_coefs = np.around(dist_coefs, 3)
    rvecs = np.around(rvecs, 3)
    print("\nRMS:", rms)
    print("camera matrix:\n", camera_matrix)
    print("distortion coefficients: ", dist_coefs.ravel())
    print("trastion matrix:\n", rvecs.ravel(), rvecs.shape)
   
    dst = cv.Rodrigues(rvecs[1])

    for x in rvecs:
        dst = Rodriguez(x)
        print(dst.ravel())

    mean_error = 0
    for i in range(len(obj_points)):
        imgpoints2, _ = cv.projectPoints(
            obj_points[i], rvecs[i], tvecs[i], camera_matrix, dist_coefs)
        # error = cv.norm(img_points[i]*1.0, imgpoints2*1.0, cv.NORM_L2)/len(imgpoints2)
        error = np.linalg.norm(imgpoints2 - img_points[i]) / len(imgpoints2)

        mean_error += error
    print("total error: {}".format(mean_error/len(obj_points)))

    # undistort the image with the calibration
    print('')
    for fn in img_names if debug_dir else []:
        name = os.path.basename(fn)
        img_found = os.path.join(debug_dir, name + '_chess.png')
        outfile = os.path.join(debug_dir, name + '_undistorted.png')

        img = cv.imread(img_found)
        if img is None:
            continue

        h, w = img.shape[:2]
        # print(h, w)
        # newcameramtx, roi = cv.getOptimalNewCameraMatrix(
        #     camera_matrix, dist_coefs, (w, h), 1, (w, h))

        dst = cv.undistort(img, camera_matrix, dist_coefs, None)

        # # crop and save the image
        # x, y, w, h = roi
        # dst = dst[y:y+h, x:x+w]

        # print('Undistorted image written to: %s' % outfile)
        cv.imwrite(outfile, dst)

    print('Done')


if __name__ == '__main__':
    main()
    cv.destroyAllWindows()
