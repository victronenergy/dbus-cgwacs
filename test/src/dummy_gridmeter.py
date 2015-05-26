#!/usr/bin/env python

from dbus.mainloop.glib import DBusGMainLoop
import gobject
import argparse
import sys
import os

# our own packages
sys.path.insert(1, os.path.join(os.path.dirname(__file__), '../ext/velib_python'))
from dbusdummyservice import DbusDummyService
from logger import setup_logging

# Argument parsing
parser = argparse.ArgumentParser(
description='dummy dbus service'
)

parser.add_argument("-n", "--name", help="the D-Bus service you want me to claim",
                    type=str, default="com.victronenergy.gridmeter.ttyUSB0")

args = parser.parse_args()

print(__file__ + " is starting up, use -h argument to see optional arguments")
logger = setup_logging(debug=True)

# Have a mainloop, so we can send/receive asynchronous calls to and from dbus
DBusGMainLoop(set_as_default=True)

s = DbusDummyService(
    servicename=args.name,
    deviceinstance=80,
    paths={
        '/Ac/Current': {'initial': 3.147},
        '/Ac/Voltage': {'initial': 221},
        '/Ac/Power': {'initial': 695.7},
        '/Ac/Energy/Forward': {'initial': 12.3, 'update': 0.02},
        '/Ac/Energy/Reverse': {'initial': 43.2},
        
        '/Ac/L1/Current': {'initial': 3.147},
        '/Ac/L1/Voltage': {'initial': 221},
        '/Ac/L1/Power': {'initial': 695.7},
        '/Ac/L1/Energy/Forward': {'initial': 12.3, 'update': 0.02},
        '/Ac/L1/Energy/Reverse': {'initial': 43.2},
        
        '/Ac/L2/Current': {'initial': 0},
        '/Ac/L2/Voltage': {'initial': 0},
        '/Ac/L2/Power': {'initial': 0},
        '/Ac/L2/Energy/Forward': {'initial': 0},
        '/Ac/L2/Energy/Reverse': {'initial': 0},
        
        '/Ac/L3/Current': {'initial': 0},
        '/Ac/L3/Voltage': {'initial': 0},
        '/Ac/L3/Power': {'initial': 0},
        '/Ac/L3/Energy/Forward': {'initial': 0},
        '/Ac/L3/Energy/Reverse': {'initial': 0},
        
        '/CustomName': {'initial': 'EM24-DIN.AV9.3.X.IS.X'},
        '/Serial': {'initial': 'BO24700610016'},
    },
    productname='EM24-DIN.AV9.3.X.IS.X',
    connection='/dev/ttyUSB0')

logger.info('Connected to dbus, and switching over to gobject.MainLoop() (= event based)')
mainloop = gobject.MainLoop()
mainloop.run()
