##
# @file serial_send.py
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
    if len(sys.argv) < 4:
        print "%s COMx Baudrate File [speed_limit]" % sys.argv[0]
        sys.exit(1)

    if len(sys.argv) > 4:
        speed_limit = int(sys.argv[4])
    else:
        speed_limit = 0xffffffff;

    com = sys.argv[1]
    baudrate = sys.argv[2]
    fn = sys.argv[3]
    total = 0
    data = None
    start_time = time.time()

    fp = open(fn, "rb")
    ps = serial.Serial(com, baudrate)
    sleep_time = 0.001

    while True:
        data = fp.read(128);
        total += len(data)
        ps.write(data)
        ps.flush()

        elapse = time.time() - start_time
        speed = total / (elapse + 0.00001)
        sys.stderr.write("-->Time:%s Send: %d Speed: %.2f B/s\r"
                % (sft(elapse), total, speed))

        if speed > speed_limit:
            time.sleep(0.05)

        if len(data) < 128:
            break

