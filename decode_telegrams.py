#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import requests
import socket
import crcmod
import sys
import time

host = "academicstrokes.com"

anmeldung_typ = {
        0: "Vor",
        1: "An",
        2: "Ab",
        3: "Tür zu"
        }

def get_knotenpunkt(nummer):
    kpdict = {
            # AP 3.1.1(.2) DDL003
            102: "LSA Anton-/Leipziger Straße",
            # AP 3.2.1 DDL072
            103: "Anton-/Zur Eisenbahnstraße",
            # Bf Neustadt
            # AP 3.3.1(.2) DDL071
            104: "Schlesischer Platz",
            # AP 3.4.1.1
            107: "Albertplatz",
            108: "Albertplatz West",
            109: "Bautzner/Rothenburger",
            # AP 3.10.1(.2) DDL036
            119: "Straßburger Platz",
            # AP 3.19.1 DDL310
            120: "Lennéstraße/Hauptallee",
            121: "Lennéplatz",
            136: "Könneritzstraße/Jahnstraße",

            200: "Synagoge",
            # AP 3.3.1.1
            # ???
            201: "St. Petersburger Straße/Pulmann-Newa",
            203: "Pirnaischer Platz",
            # AP 3.6.1.1
            204: "Rathenauplatz",
            # AP 3.5.1.1
            205: "Carolaplatz",
            208: "Bodenbacher / Zwinglistraße",
            268: "Albertplatz Süd",

            324: "Fetscherplatz",
            346: "Schillerplatz",
            363: "Blasewitzer /Fetscherstraße",

            421: "Wasaplatz",
            429: "Moränenende/Breitscheidstraße",
            435: "Moränenende/Wilhelm-Liebknecht-Straße",
            459: "Mügelner Straße/Moränenende",

            504: "Reichenbachstraße",
            506: "Fritz-Foerster-Platz",

            604: "Nürnberger /Budapester Str.",
            606: "Chemnitzer /Nöthritzer Str.",
            651: "Nürnberger Platz",

            804: "Tharandter /Kesselsdorfer Straße",
            # AP 3.35.1.2 DDL069
            851: "Löbtauer Str. / Fröbelstraße",

            1101: "Louisenstraße",
            1102: "Bischofsweg",
            1103: "Tannenstraße",
            1104: "Staufenbergallee",
            1151: "S-Bahnhof Bischofsplatz",

            1202: "LSA BTZ <-> Pulsnitzer",
            1203: "Pulsnitzer Straße",
            1205: "Diakonissenkrankenhaus",
            1220: "Nordstraße",

            1301: "Körnerplatz/Grundstr.",
            # AP 3.4.1(.2) DDL184

            1404: "Liststraße",

            1501: "Hansastraße/Eisenbahnstraße",
            1504: "Friedensstraße"
            }

    try:
        return kpdict[nummer]
    except:
        return str(nummer)

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
            return False

        mode = payload[0] >> 4
        r09_type = payload[0] & 0xf

        if mode == 9 and r09_type == 1:
            length = payload[1] & 0xf

            # check length
            _ = payload[3 + length + 2 - 1]

            # check crc
            if crc_at(9) != crc16_func(bytearray(payload[0:9])):
                return False

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
        
                deviation = "{}{}:{}min".format(chr(0x2b+2*ZV), (ZW >> 1), (ZW & 1) * 30)
                # TODO, XXX this is wrong!
                print("Liniennummer: {}, Kursnummer: {}, Zielnummer: {}, t{}".format(LN, KN, ZN, deviation))
                print("ZV: {}, ZW: {}, MP: {} / {}, MP: {}, PR: {}, HA: {}, R: {}, ZL: {}".format(ZV, ZW, hex(MP >> 2), MP & 0x3, hex(MP), PR, HA, R, ZL))
                print("Knotenpunkt: {} / {} / {}".format(get_knotenpunkt(KnotenPunkt), KnotenPunktNummer, anmeldung_typ[MP & 0x3]))

                req_payload = {
                    "time_stamp": int(time.time()),
                    "lat": 51.027107,
                    "lon": 13.723566,
                    "station_id": 100,
                    "line": LN,
                    "course_number": KN,
                    "destination_number": ZN,
                    "zv": ZV,
                    "zw": ZW,
                    "mp": MP,
                    "ha": HA,
                    "ln": LN,
                    "kn": KN,
                    "zn": ZN,
                    "r": R,
                    "zl": ZL,
                    "junction": KnotenPunkt,
                    "junction_number": KnotenPunktNummer
                }

                print("Making the request")
                r = requests.post("http://{}/formatted_telegram".format(host), json=req_payload)
                print("Response:", r);


            else:
                print("Mode: {}, Type: {}, Length: {}".format(mode, r09_type, length))
                print(payload)
                req_payload = {
                    "time_stamp": time.time(),
                    "lat": 51.027107,
                    "lon": 13.723566,
                    "station_id": 100,
                    "raw_data": " ".join(payload)
                }
                r = requests.post("http://{}/raw".format(host), json=req_payload)
                print("Response:", r);

        else:
            p = list(map(hex, payload))
            if mode == 9:
                print("Mode: {}, Type: {}, Length: {}, Payload: {}".format(mode, r09_type, length, p[:length+2]))
            else:
                if length > 4:
                    # discard
                    return False
                print("Mode: {}, Length: {}, Payload: {}".format(mode, length, p[:length+2]))

        return True

    except IndexError:
        pass

    return False

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

print("[*] Started Telegram Decoder")

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

    return_code = decode(payload)
    # try to decode every 1 bit error
    one_bit_correted = False
    if return_code != True:
        for corrected in one_bit_error_list(payload):
            return_code = decode(corrected)
            if return_code == True:
                one_bit_correted = True
                break

    if one_bit_correted:
        print("one bit correction worked!!!")

    if return_code == True:
        print("======")
