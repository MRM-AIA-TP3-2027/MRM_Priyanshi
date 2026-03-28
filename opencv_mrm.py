import cv2
import numpy as np

cap = cv2.VideoCapture(0)

parameters = cv2.aruco.DetectorParameters()
parameters.cornerRefinementMethod = cv2.aruco.CORNER_REFINE_SUBPIX

aruco_dicts = [
    cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_250),
    cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_5X5_250),
    cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_6X6_250)
]

marker_length = 0.05

while True:
    ret, frame = cap.read()
    if not ret:
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    corners = None
    ids = None

    for dictionary in aruco_dicts:
        detector = cv2.aruco.ArucoDetector(dictionary, parameters)
        corners, ids, rejected = detector.detectMarkers(gray)

        if ids is not None:
            break

    if ids is not None:

        cv2.aruco.drawDetectedMarkers(frame, corners, ids)

        h, w, _ = frame.shape

        camera_matrix = np.array([
            [w, 0, w/2],
            [0, w, h/2],
            [0, 0, 1]
        ], dtype=float)

        dist_coeffs = np.zeros((5,1))

        for i in range(len(ids)):

            marker_points = np.array([
                [-marker_length/2, marker_length/2, 0],
                [marker_length/2, marker_length/2, 0],
                [marker_length/2, -marker_length/2, 0],
                [-marker_length/2, -marker_length/2, 0]
            ], dtype=np.float32)

            success, rvec, tvec = cv2.solvePnP(
                marker_points,
                corners[i],
                camera_matrix,
                dist_coeffs
            )

            if success:

                print("Marker ID:", ids[i][0])
                print("Rotation Vector (rvec):", rvec.flatten())
                print("Translation Vector (tvec):", tvec.flatten())
                print("--------------------------------------")

                cv2.drawFrameAxes(
                    frame,
                    camera_matrix,
                    dist_coeffs,
                    rvec,
                    tvec,
                    marker_length
                )

                x = tvec[0][0]
                y = tvec[1][0]
                z = tvec[2][0]

                depth = np.sqrt(x*x + y*y + z*z)

                text = f"ID:{ids[i][0]} Depth:{depth:.2f}m"

                corner = corners[i][0][0]

                cv2.putText(
                    frame,
                    text,
                    (int(corner[0]), int(corner[1]-10)),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.6,
                    (0,255,0),
                    2
                )

    cv2.imshow("ArUco Pose Estimation", frame)

    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()