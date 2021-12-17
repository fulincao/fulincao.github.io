

import cantools
import json

def test_ars():

    dbc = cantools.database.load_file("/home/cao/work-git/cve/cve/dbc/ARS408.dbc", strict=False)

    rf = open("./test_can_parser_data/ars.txt", "r")
    ids = [m.frame_id for m in dbc.messages]

    for line in rf:
        cols = line.split()
        can_id = int(cols[3], 16)
        data = b''.join([int(x, 16).to_bytes(1, 'little') for x in cols[4:]])

        if can_id in ids:
            j = dbc.decode_message(can_id, data, decode_choices=False)
            print(json.dumps(j))


def test_x1j():

    dbc = cantools.database.load_file("/home/cao/work-git/cve/cve/dbc/X1_AEB_20200812.dbc", strict=False)

    rf = open("./test_can_parser_data/x1j.txt", "r")

    ids = [m.frame_id for m in dbc.messages]

    for line in rf:
        cols = line.split()
        can_id = int(cols[3], 16)
        # print(cols)
        data = b''.join([int(x, 16).to_bytes(1, 'little') for x in cols[4:]])
        
        if can_id in ids:
            j = dbc.decode_message(can_id, data, decode_choices=False)
            print(json.dumps(j))


def compare_result():
    
    can_parser = []
    rf = open("./can_parser.txt", "r")
    for line in rf:
        line = line.strip()
        try:
            if line == "null":
                continue
            item = json.loads(line)
            can_parser.append(item)

        except:
            pass
    
    rf.close()

    can_tools = []
    rf = open("./can_tools.txt", "r")
    for line in rf:
        line = line.strip()
        try:
            item = json.loads(line)
            can_tools.append(item)

        except:
            pass
    
    err_parser = 0        
    for i, item in enumerate(can_tools):
        o_item = can_parser[i]
        flag = 1
        for key in item:
            if key not in o_item:
                flag = 0
                continue
            
            if abs(o_item[key] - item[key]) > 1e5:
                flag = 0
        if not flag:
            err_parser += 1
            print(o_item)
            print(item)
            print("--------")

    print("error count: {}, error rate: {}".format(err_parser, err_parser/len(can_tools)))

if __name__ == "__main__":
    # test_ars()
    # test_x1j()
    compare_result()