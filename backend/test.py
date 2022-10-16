from uart import UART
import serial
from time import time

SCORE = 500
PULSE_WIDTH = 750


def mapping(x: int, minIn: int, maxIn: int, minOut: int, maxOut: int) -> int:
    return int((x - minIn) * (maxOut - minOut) / (maxIn - minIn) + minOut)


def convertScoreToDeg(score: int) -> int:
    return mapping(score, 100, 500, 0, 180)

def setServo(deg: int) -> int:
    return mapping(deg, 0, 180, 460, 4010)

def convertPulseWidthToScore(pulseWidth: int) -> int:
    return mapping(pulseWidth, 460, 4010, 100, 500)


score = convertPulseWidthToScore(PULSE_WIDTH)
#print(score)
if __name__ == "__main__":
    data = '177'
    print(data.encode())
    print(b'155'.decode())
    print(time())