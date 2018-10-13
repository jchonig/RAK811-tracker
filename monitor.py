#!/usr/bin/env python3

from __future__ import print_function

import argparse
import base64
import signal
import sys

import grpc
import ttn

def uplink_callback(msg, client):
    """ Print the received packet """

    # XXX - Just queue the message for the main process

    print("%s app_id %s dev_id %s port %d" % (msg.metadata.time,
                                              msg.app_id,
                                              msg.dev_id,
                                              msg.port))
    if msg.payload_raw:
        data = base64.b64decode(msg.payload_raw)
        print("\t%d: %s" % (len(data), data.hex().upper()))
    for gateway in msg.metadata.gateways:
        print("\tGW %s Channel %d RSSI %ddBm S/N %.2f" % (
            gateway.gtw_id,
            gateway.channel,
            gateway.rssi,
            gateway.snr))

    if 'payload_fields' in msg:
        fields = msg.payload_fields
        if msg.port == 2:
            print("\tGPS lat: %f long: %f alt %dm HDOP: %.1f %s" % (
                fields.lat,
                fields.lon,
                fields.alt,
                fields.hdop,
                "https://www.google.com/maps/@%f,%f,14z" % (fields.lat, fields.lon)))
        elif msg.port == 3:
            print("\tBattery %.3fmV" % (float(fields.bat) / 1000.0))
        elif msg.port == 4:
            print("\tAcceleration: X: %d Y: %d Z: %d" % (fields.aX,
                                                         fields.aY,
                                                         fields.aZ))

def parse_args():
    """ Parse our arguments """

    parser = argparse.ArgumentParser(description="Send pushbullet messages")

    #	Debugging
    group = parser.add_argument_group("Debugging options")
    group.add_argument("-d", "--debug",
                       dest="debug", default=False,
                       action='store_true',
                       help="print debugging messages")
    group.add_argument("-v", "--verbose",
                       dest="verbose", default=False,
                       action='store_true',
                       help="print verbose messages")

    group = parser.add_argument_group("Application Info")
    group.add_argument("--app_id",
                       dest="app_id", required=True,
                       help="Applciation ID from TTN Console")
    group.add_argument("--access_key",
                       dest="access_key", required=True,
                       help="Application Access Key from TTN Console")

    options = parser.parse_args()

    if options.debug:
        options.verbose = options.debug

    return options

def main():
    """ Where the action is """

    options = parse_args()

    if options.debug:
        print("Connecting to Application %s" % options.app_id)

    try:
        handler = ttn.HandlerClient(options.app_id, options.access_key)
    except grpc._channel._Rendezvous as error:
        sys.exit("Failed to connect to %s: %s" % (
            options.app_id,
            error.details()))

    # using mqtt client
    mqtt_client = handler.data()
    mqtt_client.set_uplink_callback(uplink_callback)

    if options.debug:
        print("Connecting to MQTT client")

    mqtt_client.connect()

    if options.debug:
        print("Waiting for messages from %s" % options.app_id)

    # XXX - Wait for packets
    signal.pause()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("")
