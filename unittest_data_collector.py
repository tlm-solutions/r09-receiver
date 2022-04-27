#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import crcmod
import sys
import json

def bitvec_to_bytes(vec):
    if len(vec) != 8:
        return 0

    a = 0
    for i in range(8):
        if vec[i] > 0:
            a += 1 << i

    return a

def decode(payload):
    try:
        # check minimum length 3 data bytes + 2 crc bytes
        _ = payload[3 + 2 - 1]

        crc_at = lambda at: (payload[at+1] << 8 | payload[at]) ^ 0xffff
        crc16_func = crcmod.mkCrcFun(0x16f63, rev=True, initCrc=0x0000, xorOut=0x0000)

        # find packets with unknown length by checking the crc over multiple sizes
        length = 0

        for at in range(4, len(payload) - 1):
            if crc_at(at) == crc16_func(bytearray(payload[0:at])):
                length = at

        # could not find proper length, discard
        if length == 0:
            return None

        mode = payload[0] >> 4
        r09_type = payload[0] & 0xf

        if mode == 9 and r09_type == 1:
            length = payload[1] & 0xf

            # check length
            _ = payload[3 + length + 2 - 1]

            # check crc
            if crc_at(9) != crc16_func(bytearray(payload[0:9])):
                return None

            if length == 6:
                ZV = payload[1] >> 7
                ZW = (payload[1] >> 4) & 0x7
                MP = ((payload[2] >> 4) << 12) | ((payload[2] & 0x0f) << 8) | ((payload[3] >> 4) << 4) | ((payload[3] & 0x0f))
                PR = payload[4] >> 6
                HA = (payload[4] >> 4) & 0x3
                LN = 100 * (payload[4] & 0xf) + 10 * (payload[5] >> 4) +  (payload[5] & 0xf)
                KN = 10 * (payload[6] >> 4) + (payload[6] & 0xf)
                ZN = 100 * (payload[7] >> 4) + 10 * (payload[7] & 0xf) + (payload[8] >> 4)
                R = (payload[8] >> 3) & 0x1
                ZL = payload[8] & 0x7

                KnotenPunkt = (MP >> 2) // 10
                KnotenPunktNummer = (MP >> 2) - 10 * KnotenPunkt
                RequestStatus = MP & 0x3

                # TODO, XXX this is wrong!
                print("Liniennummer: {}, Kursnummer: {}, Zielnummer: {}".format(LN, KN, ZN))
                print("ZV: {}, ZW: {}, MP: {} / {}, MP: {}, PR: {}, HA: {}, R: {}, ZL: {}".format(ZV, ZW, hex(MP >> 2), MP & 0x3, hex(MP), PR, HA, R, ZL))
                print("Knotenpunkt: {} / {} / {}".format(KnotenPunkt, KnotenPunktNummer, RequestStatus))

                req_payload = {
                    "line": LN,
                    "course_number": KN,
                    "destination_number": ZN,
                    "zv": ZV,
                    "zw": ZW,
                    "mp": MP,
                    "pr": PR,
                    "ha": HA,
                    "ln": LN,
                    "kn": KN,
                    "zn": ZN,
                    "r": R,
                    "zl": ZL,
                    "junction": KnotenPunkt,
                    "junction_number": KnotenPunktNummer,
                    "request_status": RequestStatus
                }

                return req_payload
    except IndexError:
        pass

    return None

def one_bit_error_list(payload):
    corrected = []

    for i in range(len(payload)):
        for j in range(8):
            tmp = payload.copy()
            tmp[i] ^= (1 << j)
            corrected.append(tmp)

    return corrected

ip = "127.0.0.1"
port = 40000
sample_size = 100

# Create a UDP socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Bind the socket to the port
server_address = (ip, port)
s.bind(server_address)

print("[*] Started Telegram Unit Test Data Collector")

unittest_data = []

try:
    while True:
        data, address = s.recvfrom(4096)

        i = 0
        bits = []
        while i < len(data):
            v = sum(data[i:i+2])
            if v != 1:
                bits.append(1)
            else:
                bits.append(0)
            i += 2

        # assume a max length of 20 bytes
        # 20 * 8 + 19 bits inbetween
        payload = []
        i = -1
        while i <= 180:
            payload.append(bitvec_to_bytes(data[i+1:i+9]))
            i += 9

        # print("======")
        # print(list(map(hex, payload)))

        json_data = decode(payload)

        if json_data == None:
            continue

        # sava data and json to file
        unittest_data.append({"input": data.decode("utf-8"), "output": json_data})
except KeyboardInterrupt:
    pass

with open('unittest_data.json', 'w') as fp:
    json.dump(unittest_data, fp)
