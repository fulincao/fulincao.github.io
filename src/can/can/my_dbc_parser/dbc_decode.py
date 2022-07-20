import re
import json
import struct


class DBC:
    def __init__(self, dbc_file):
        self.frame_id = [1, 2, 3]
        self.__dbc = {}
        self.add_dbc_file(dbc_file)

    def decode_message(self, id, buf):

        if id not in self.__dbc:
            return None
        r = {key: None for key in self.__dbc[id]['keys']}
        fmt = '>' + 'B'*len(buf)
        d = struct.unpack(fmt, buf)
        bits = ['0' * (8-len(bin(x)[2:])) + bin(x)[2:] for x in d]

        for key in self.__dbc[id]['keys']:
            val_bits = ''
            d = self.__dbc[id]['keys'][key]
            res = []
            if d['byte_order'] == "little_endian":
                lay = ''.join([x[::-1] for x in bits])
                st, leg = d['start'], d['length']
                t = st
                while t < st + leg:
                    nt = (t//8 + 1)*8
                    if nt < st + leg:
                        res.append(lay[t:nt][::-1])
                        t = nt
                    else:
                        res.append(lay[t:st+leg][::-1])
                        t = st + leg
                res.reverse()
                val_bits = ''.join(res)
            else:
                lay = ''.join(bits)
                st, leg = d['start'], d['length']
                t = st
                while t < st + leg:
                    nt = (t // 8 + 1) * 8
                    if nt < st + leg:
                        res.append(lay[t:nt][::-1])
                        t = nt
                    else:
                        res.append(lay[t:st + leg][::-1])
                        t = st + leg
                val_bits = ''.join(res)

            if d['singed'] and val_bits[0] == 1:
                val = (~int(val_bits, 2)) + 1
                val = -1 * val
            else:
                val = int(val_bits, 2)
            val = min(max(val * d['scale'] + d['offest'], d['min_val']), d['max_val'])
            r[key] = val

        return r

    def add_dbc_file(self, dbc_file):
        with open(dbc_file, "r") as rf:
            last_bo = -1
            for line in rf:
                line = line.strip()
                if line.startswith("BO_"):
                    res = re.findall(r"BO_\s+(\d+)\s+(\w+):", line)
                    if not res:
                        continue
                    can_id, desc = int(res[0][0]), res[0][1]

                    if can_id in self.__dbc:
                        print(can_id, "can id conflict")
                        rf.close()
                        return
                    self.__dbc[can_id] = {}
                    self.__dbc[can_id]['desc'] = desc
                    self.__dbc[can_id]['keys'] = {}
                    last_bo = can_id

                elif line.startswith("SG_"):
                    if last_bo == -1:
                        continue
                    res = re.findall(r'SG_\s+(\w+)\s+:\s+(\d+)\|(\d+)@(\d+)(.)\s+\((.+?),(.+?)\)\s+\[(.*?)\|(.*?)\]\s+"(.*?)"', line)
                    if not res:
                        continue
                    data = res[0]

                    if data[0] in self.__dbc[last_bo]['keys']:
                        print(last_bo, data[0], "have same name in bo")
                        # return
                    d = {}
                    d['start'] = int(data[1])
                    d['length'] = int(data[2])
                    d['byte_order'] = "little_endian" if data[3] == "1" else "big_endian"
                    d['singed'] = False if data[4] == "+" else True
                    d['scale'] = float(data[5])
                    d['offest'] = float(data[6])
                    d['min_val'] = float(data[7])
                    d['max_val'] = float(data[8])
                    d['type'] = data[9]
                    self.__dbc[last_bo]['keys'][data[0]] = d

                elif line.startswith("CM_ SG_"):
                    res = re.findall(r'CM_ SG_\s+(\d+)\s+(\w+)\s+"(.*?)"', line)
                    if not res:
                        continue
                    can_id, key, comment = int(res[0][0]), res[0][1], res[0][2]
                    if can_id in self.__dbc and key in self.__dbc[can_id]['keys']:
                        self.__dbc[can_id]['keys'][key]['comment'] = comment

    def __repr__(self):
        return json.dumps(self.__dbc, indent=4)

    def __contains__(self, item):
        if item in self.__dbc:
            return True
        return False


if __name__ == "__main__":
    import cantools
    dbx = cantools.database.load_file("Car_sensor_V1.3.dbc", strict=False)
    dbx.add_dbc_file('Lanes_V1.8.dbc')
    dbx.add_dbc_file('Objects_V1.4.dbc')
    # dbx.add_dbc_file('Objects_V1.4.dbc')


    dbc = DBC("./Objects_V1.4.dbc")
    dbc.add_dbc_file('Lanes_V1.8.dbc')
    dbc.add_dbc_file('Car_sensor_V1.3.dbc')
    # dbc.add_dbc_file('Objects_V1.4.dbc')

    import time
    st = time.time()
    with open('/home/cao/桌面/eyeq4_data/20190828175011/log.txt', 'r') as rf:
        for line in rf:
            cols = line.strip().split()
            if 'CAN' in cols[2]:
                can_id = int(cols[3], 16)
                data = b''.join([int(x, 16).to_bytes(1, 'little') for x in cols[4:]])
                # print(line, can_id, data)
                d1 = dbc.decode_message(can_id, data)

                ids = [m.frame_id for m in dbx.messages]
                if can_id not in ids:
                    d2 = None
                else:
                    d2 = dbx.decode_message(can_id, data)
                if d1 != d2:
                    print(d1)
                    print(d2)
                    print('--------------')

    print(time.time() - st)

