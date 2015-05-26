dbus-cgwacs
===========

Reads energy measuremens from Carlo Gavazzi Wired AC Sensors (hence cgwacs),
and puts in on the D-Bus. At this point only the EM24 model is supported.
Also contains a control loop to minimize power taken from the net when used in
a setup with a PV inverter, multi and battery (Hub4). This control loop will
minimize the power taken from the net. Instead power from a PV inverter will be
used whenever available. If not, the multi will be requested to retrieve the
required power from a battery.

Architecture
============

The application consists of several layers:
* Data acquisition layer:
    - The _ModbusRtu_ class provides a simple implementation of the modbus
      RTU protocol, which supports the functions ReadHoldingRegisters (3),
      ReadInputRegisters (4) and WriteSingleRegister (6) only.
    - _AcSensorUpdater_ connects to an ac sensor, retrieves its identity
      (type & serial) and retrieves measured data. The data retrieved will be
      stored in an _AcSensor_ object.
* Data model
    - _AcSensor_ contains the latest measurements taken from an AC sensor.
    - _Settings_ Persistent settings objects which contains global settings.
    - _AcSensorSettings_ Persistent AC sensor settings.
    - _Multi_ Contains data retrieved from the multi(s) in the setup needed for
      the control loop.
* Control Loop:
    - _Control Loop_ adjusts the behaviour of the multi(s) by setting
      com.victronenergy.vebus.tty??/Hub4/AcPowerSetpoint. As input the power
      levels taken from the AC sensor is used. The AC sensor must be
      set up as grid meter in the settings.
* D-Bus layer
    - _AcSensorBridge_ produces the _com.victronenergy.grid.ttyUSB??_ D-Bus
      service. Information is taken from an _AcSensor_, and an
      _AcSensorSettings_ object.
    - _AcSensorSettingsBridge_ consumes a subtree from the local settings.
      Path: /Settings/CGwacs/D[serial]
    - _MultiBridge_ consumes some of the objects in the path
      com.victronenergy.vebus.ttyO1, where we expect data from (one or more)
      multis.
