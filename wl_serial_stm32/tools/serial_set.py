##
# @file serial_send.py
# @brief
# @author Red Li
# @version
# @date 2013-07-16



import serial, sys
import time, argparse

def sft(time):
    h = int(time / 3600)
    m = (int(time) % 3600) / 60
    s = int(time) % 60
    return "%02d:%02d:%02d" % (h, m, s)


def print_usage(parser, msg, exit = True):
    print "Error: %s" % msg
    parser.print_help()
    if exit: sys.exit(1)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = "Configure wireless serial port")
    baud_choices = [1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200,
            230400, 460800]
    rf_speed_choices = ["250K", "1M", "2M"]

    parser.add_argument('port', metavar='COMx', type=str, nargs=1,
            help='Serial port COMx')

    parser.add_argument('--get-stats', dest = 'get_stats', default = False,
            action = 'store_true',
            help = "Get Stats information")

    parser.add_argument('--reset', dest = 'reset', default = False,
            action = 'store_true',
            help = "Reset system")

    parser.add_argument('--remote', dest = 'remote', default = False,
            action = 'store_true',
            help = "The command is operate on remote target")

    parser.add_argument('--set-baud', dest = 'baudrate',
            type = int,
            choices = baud_choices,
            help = "Serial port baudrate")

    parser.add_argument('--get-baud', dest = 'get_baud',
            action = 'store_true',
            help = "Get Serial port baudrate")

    parser.add_argument('--set-rf-chn', dest = 'rf_chn',  type = int,
            help = "RF channel, 0 - 125")

    parser.add_argument('--set-rf-speed', dest = 'rf_speed',
            choices = rf_speed_choices, type=str,
            help = "RF speed")

    parser.add_argument('--get-rf', dest = 'get_rf',
            action = 'store_true',
            help = "Get rf_chn and rf_speed")

    parser.add_argument('--set-local-addr', dest = 'local_addr',
            help = "Local address, format xx:xx:xx:xx:xx")

    parser.add_argument('--get-local-addr', dest = 'get_local_addr',
            action = 'store_true',
            help = "Get local address")

    parser.add_argument('--set-remote-addr', dest = 'remote_addr',
            help = "Remote address, format xx:xx:xx:xx:xx")

    parser.add_argument('--get-remote-addr', dest = 'get_remote_addr',
            action = 'store_true',
            help = "Get remote address")

    parser.add_argument('--set-pwm-duty-cycle', dest = 'set_pwm_duty_cycle',
            nargs = 2,
            type = int,
            help = "Set pwm duty cycle, idx = [0 - 3], duty cycle = [0 - 100]")

    parser.add_argument('--get-pwm-duty-cycle', dest = 'get_pwm_duty_cycle',
            nargs = 1,
            type = int,
            help = "Get pwm duty cycle")

    parser.add_argument('--set-gpio-state', dest = 'set_gpio_state',
            nargs = 2,
            type = int,
            help = "Set gpio state, idx = [0 - 3], state = [0, 1]")

    parser.add_argument('--get-gpio-state', dest = 'get_gpio_state',
            nargs = 1,
            type = int,
            help = "Get GPIO state")

    parser.add_argument('--set-pin-mode', dest = 'set_pin_mode',
            nargs = 2,
            type = int,
            help = "Set pin mode state, idx = [0 - 7], mode = [0, 1]")

    parser.add_argument('--get-pin-mode', dest = 'get_pin_mode',
            nargs = 1,
            type = int,
            help = "Get PIN Mode")

    args = parser.parse_args()

    s2addr = lambda s: [int(ss, 16) for ss in s.split(":")]
    t = lambda vs: "".join([chr(v) for v in vs])

    ps = serial.Serial(args.port[0], 9600, timeout = 0.2)

    remote_flag = 0x0
    if args.remote:
        remote_flag = 0x40

    permanent_flag = 0x80
    op = lambda v: v | remote_flag | permanent_flag

    if args.get_stats:
        ti = lambda vs: ord(vs[0]) | (ord(vs[1]) << 8) | (ord(vs[2]) << 16) | (ord(vs[3]) << 24)
        vn = ['recv', 'recv_fail', 'send', 'send_fail', 'w_recv',
              'w_drop', 'w_send', 'w_ack_send', 'w_send_fail', 'w_retry',
              'ds_rx_drop', 'ds_rx', 'ds_tx']

        for i in range(len(vn)):
            ps.write(t([0xaf, 0xfa, op(0x31), i]))
            ret = ps.read(100)
            if len(ret) < 4:
                print "Counter '%s:%d' - NOK" % (vn[i], i)
            else:
                print "Counter '%s:%d' - %d " % (vn[i], i, ti(ret[2:6]))

    if args.reset:
        ps.write(t([0xaf, 0xfa, op(0x3F)]))
        ret = ps.read(100)
        if len(ret) == 0:
            print "RESET - NOK"
        else:
            print "RESET - " + ret
        sys.exit(0)


    remote_flag = 0x0
    if args.remote:
        remote_flag = 0x40

    permanent_flag = 0x80

    def check_result(msg):
        sys.stdout.write(msg + " ")
        data = ps.read(100)
        if len(data): print data
        else: print "TIMEOUT"

    def check_result2(msg, func):
        sys.stdout.write(msg + " ")
        data = ps.read(100)
        if len(data):
            print func(data)
        else:
            print "TIMEOUT"
            return None



    if args.baudrate:
        ps.write(t([0xaf, 0xfa, op(0x2), baud_choices.index(args.baudrate)]))
        check_result("Set baudrate to %d" % args.baudrate)

    if args.get_baud:
        ps.write(t([0xaf, 0xfa, op(0x3)]))
        f = lambda data: data[0:2] + " Baudrate:" + str(baud_choices[ord(data[2])])
        check_result2("Get baudrate - ", f)

    if args.rf_chn and args.rf_speed:
        if args.rf_chn < 0 or args.rf_chn > 125:
            print_usage(parser, "rf_chn need be 0 - 125")

        ps.write(t([0xaf, 0xfa, op(0x0), args.rf_chn, rf_speed_choices.index(args.rf_speed)]))
        check_result("Set RF channel to %d, RF speed to %s" % (args.rf_chn, args.rf_speed))
    elif args.rf_chn or args.rf_speed:
        print_usage(parser, "rf_chn and rf_speed both need specified")

    if args.get_rf:
        ps.write(t([0xaf, 0xfa, op(0x1)]))
        f = lambda d: d[0:2] + " chn:%d speed:%s" % (ord(d[2]), rf_speed_choices[ord(d[3])])
        check_result2("Get RF - ", f)

    if args.local_addr:
        local_addr = s2addr(args.local_addr)
        if len(local_addr) != 5:
            print_usage(parser, "local_addr wrong format")
        ps.write(t([0xaf, 0xfa, op(0x4)] + local_addr))
        check_result("Set local address to %s" % (args.local_addr))

    if args.get_local_addr:
        ps.write(t([0xaf, 0xfa, op(0x5)]))
        f = lambda d: d[0:2] + " local_addr:%02x:%02x:%02x:%02x:%02x" % (ord(d[2]), ord(d[3]), ord(d[4]), ord(d[5]), ord(d[6]))
        check_result2("Get local address - ", f)

    if args.remote_addr:
        remote_addr = s2addr(args.remote_addr)
        if len(remote_addr) != 5:
            print_usage(parser, "remote_addr wrong format")

        ps.write(t([0xaf, 0xfa, op(0x6)] + remote_addr))
        check_result("Set remote address to %s" % (args.remote_addr))

    if args.get_remote_addr:
        ps.write(t([0xaf, 0xfa, op(0x7)]))
        f = lambda d: d[0:2] + " remote_addr:%02x:%02x:%02x:%02x:%02x" % (ord(d[2]), ord(d[3]), ord(d[4]), ord(d[5]), ord(d[6]))
        check_result2("Get remote address - ", f)


    if args.set_pwm_duty_cycle:
        idx = args.set_pwm_duty_cycle[0]
        duty_cycle = args.set_pwm_duty_cycle[1]
        if idx >= 4 or duty_cycle < 0 or duty_cycle > 100:
            print_usage(parser, "PWM idx or duty_cycle error")

        ps.write(t([0xaf, 0xfa, op(0x8), idx, duty_cycle]))
        check_result("Set PWM%d duty cycle to %d" % (idx, duty_cycle))

    if args.get_pwm_duty_cycle:
        idx = args.get_pwm_duty_cycle[0]
        if idx >= 4:
            print_usage(parser, "PWM idx error")

        ps.write(t([0xaf, 0xfa, op(0x9), idx]))
        f = lambda d: d[0:2] + " Duty cycle: %d" % ord(d[2])
        check_result2("Get PWM%d duty cycle " % (idx), f)

    if args.set_gpio_state:
        idx = args.set_gpio_state[0]
        if args.set_gpio_state[1]:
            state = 1
        else:
            state = 0
        if idx >= 4:
            print_usage(parser, "GPIO idx error")

        ps.write(t([0xaf, 0xfa, op(0xa), idx, state]))
        check_result("Set GPIO%d  to %d" % (idx, state))

    if args.get_gpio_state:
        idx = args.get_gpio_state[0]
        if idx >= 4:
            print_usage(parser, "GPIO idx error")

        ps.write(t([0xaf, 0xfa, op(0xb), idx]))
        f = lambda d: d[0:2] + " State: %d" % ord(d[2])
        check_result2("Get GPIO%d state " % (idx), f)

    if args.set_pin_mode:
        idx = args.set_pin_mode[0]
        if args.set_pin_mode[1]:
            state = 1
        else:
            state = 0
        if idx >= 4:
            print_usage(parser, "PIN idx error")

        ps.write(t([0xaf, 0xfa, op(0x10), idx, state]))
        check_result("Set PIN%d  to %d" % (idx, state))

    if args.get_pin_mode:
        idx = args.get_pin_mode[0]
        if idx >= 4:
            print_usage(parser, "PIN idx error")

        ps.write(t([0xaf, 0xfa, op(0xb), idx]))
        f = lambda d: d[0:2] + " State: %d" % ord(d[2])
        check_result2("Get PIN%d state " % (idx), f)
