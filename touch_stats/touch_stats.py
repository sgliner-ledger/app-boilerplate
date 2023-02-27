from ledgercomm import Transport
import random
import statistics
import csv
from itertools import zip_longest
import argparse


SCREEN_WIDTH  = 400
SCREEN_HEIGHT = 672

EXCLUDE_BORDER = 40

SQUARE_WIDTH = 24

class TouchData():
    state = 0
    x = 0
    y = 0
    w = 0
    h = 0
    timestamp = 0

    def __init__(self, state, x, y, w, h, timestamp):
        self.state = state
        self.x = x
        self.y = y
        self.w = w
        self.h = h
        self.timestamp = timestamp

    def print(self):
        print(f"state:{self.state} x:{self.x} y:{self.y} w:{self.w} h:{self.h} ticks:{self.timestamp}")

def _clear_touch_buffer(trs):
    trs.send(cla=0xe0, ins=0x08, p1=0x00, p2=0x00, cdata=b'')
    sw, response = trs.recv()
    return sw, response

def _read_touch_buffer(trs):
    trs.send(cla=0xe0, ins=0x07, p1=0x00, p2=0x00, cdata=b'')
    sw, response = trs.recv()
    return sw, response

def _parse_touch_data(data):
    touch_data_list = []
    for i in range(0,10):
        x = data[9*i] | data[9*i+1] << 8
        y = data[9*i+2] | data[9*i+3] << 8
        state = data[9*i+4]
        w = data[9*i+5]
        h = data[9*i+6]
        ticks = data[9*i+7] | data[9*i+8] << 8
        touch_data = TouchData(state, x, y , w, h, ticks)
        if state != 0xFF:
            touch_data_list.append(touch_data)
    return touch_data_list

def _wait_new_touch_data(trs):
    sw = bytes([0])
    response = bytes([0])
    while True:
        sw, response = _read_touch_buffer(trs)
        data = _parse_touch_data(response)
        if len(data):
            if data[0].state == 2:
                break
    return sw, response

def _draw_rect(trs, x, y, w, h):
    apdu_data = bytearray()
    apdu_data += x.to_bytes(2, 'big')
    apdu_data += y.to_bytes(2, 'big')
    apdu_data += w.to_bytes(2, 'big')
    apdu_data += h.to_bytes(2, 'big')
    sw = 0
    while sw != 0x9000:
        trs.send(cla=0xe0, ins=0x0a, p1=0x00, p2=0x00, cdata=apdu_data)
        sw, response = trs.recv()
    return sw, response

def _draw_square_and_wait_touch(trs, y_min_max=None):
    _clear_touch_buffer(trs)
    x = random.randint(EXCLUDE_BORDER, SCREEN_WIDTH-SQUARE_WIDTH-EXCLUDE_BORDER)
    if y_min_max:
        y = random.randint(EXCLUDE_BORDER+y_min_max[0], y_min_max[1]-SQUARE_WIDTH-EXCLUDE_BORDER)
    else:
        y = random.randint(EXCLUDE_BORDER, SCREEN_HEIGHT-SQUARE_WIDTH-EXCLUDE_BORDER)
    y = y-y%4
    x_center = int(x + SQUARE_WIDTH/2)
    y_center = int(y + SQUARE_WIDTH/2)
    _draw_rect(trs, x, y, SQUARE_WIDTH, SQUARE_WIDTH)
    sw, response = _wait_new_touch_data(transport)
    _clear_touch_buffer(trs)
    return x_center, y_center, _parse_touch_data(response)[0]

def display_info(trs, step=0):
    trs.send(cla=0xe0, ins=0x0B, p1=step, p2=0x00, cdata=b'')
    sw, response = trs.recv()
    return sw, response

def run_test(transport, step=0, nb_points=30, type="hand"):

    x_target_list = []
    y_target_list = []
    x_touch_list = []
    y_touch_list = []
    delta_x_list = []
    delta_y_list = []
    type_list = []

    display_info(transport, step=step)

    for i in range(0, nb_points):
        x_target, y_target, touch_data = _draw_square_and_wait_touch(transport)
        x_touch = touch_data.x
        y_touch = touch_data.y
        x_target_list.append(x_target)
        y_target_list.append(y_target)
        x_touch_list.append(x_touch)
        y_touch_list.append(y_touch)
        delta_x = x_touch - x_target
        delta_y = y_touch - y_target
        delta_x_list.append(delta_x)
        delta_y_list.append(delta_y)
        type_list.append(type)
        print(f"Point {i+1}:\n - x_target {x_target} x_touch {x_touch} delta {delta_x}\n - y_target {y_target} y_touch {y_touch} delta {delta_y}\n")
    print(f"delta_x_mean {statistics.mean(delta_x_list):.2f} delta_x_dev {statistics.stdev(delta_x_list):.2f}")
    print(f"delta_y_mean {statistics.mean(delta_y_list):.2f} delta_x_dev {statistics.stdev(delta_y_list):.2f}")
    # delta_x_list.append(statistics.mean(delta_x_list))
    # delta_y_list.append(statistics.mean(delta_y_list))
    return [x_target_list, x_touch_list, delta_x_list, y_target_list, y_touch_list, delta_y_list, type_list]

def write_list_to_csw_row(filename, mode, names, lists):
    export_data = zip_longest(*lists, fillvalue = '')
    with open(filename, mode, encoding="ISO-8859-1", newline='') as myfile:
        wr = csv.writer(myfile)
        wr.writerow(names)
        wr.writerows(export_data)

def quit_app(trs):
    trs.send(cla=0xe0, ins=0xFF, p1=0x00, p2=0x00, cdata=b'')

def auto_int(x):
    return int(x, 0)

def get_argparser():
    parser = argparse.ArgumentParser(description="Run STAX touch Stats test")
    parser.add_argument("--fileName", help="Test output filename", required=True)
    parser.add_argument("--nbPoints", help="Number of points of each test", type=auto_int)
    return parser

if __name__ == "__main__":

    nb_points = 30
    args = get_argparser().parse_args()

    csv_filename = f"{args.fileName}.csv"
    if args.nbPoints:
        nb_points = args.nbPoints

    transport = Transport(interface='hid', debug=True)
    test_lists = run_test(transport, 0, nb_points, 'hand')
    table_test_lists = run_test(transport, 4, nb_points, 'table')

    row_names = [
        "x_target",
        "x_touch",
        "delta_x",
        "y_target",
        "y_touch",
        "delta_y",
        "test_type"
        ]
    idx = 0
    for y in table_test_lists:
        test_lists[idx].extend(y)
        idx = idx+1
    write_list_to_csw_row(csv_filename, 'w+', row_names, test_lists)
    quit_app(transport)

