##
# @file serial_recv.py
# @brief
# @author Red Li
# @version
# @date 2013-07-16



import serial, sys
import time

def sft(time):
    h = int(time / 3600)
    m = (int(time) % 3600) / 60
    s = int(time) % 60
    return "%02d:%02d:%02d" % (h, m, s)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print "%s COMx Baudrate" % sys.argv[0]
        sys.exit(1)

    ofp = sys.stdout
    if len(sys.argv) > 3:
        ofp = open(sys.argv[3], "wb")


    com = sys.argv[1]
    baudrate = sys.argv[2]
    total = 0
    data = None
    start_time = None

    ps = serial.Serial(com, baudrate)
    while True:
        data = ps.read()
        if start_time == None:
            start_time = time.time()
        total += len(data)
        ofp.write(data)
        ofp.flush()

        elapse = time.time() - start_time
        sys.stderr.write("-->Time:%s Recv: %d Speed: %.2f B/s\r"
                % (sft(elapse), total, total / (elapse + 0.00001)))


