dbus-cgwacs
===========

Reads energy measuremens from Carlo Gavazzi Wired AC Sensors (hence cgwacs),
and puts in on the D-Bus. Supported models: EM24, ET112, ET340, EM530 and EM540.

Note that for the EM530 and EM540, only Sub-family `X` and `PFC` is supported.

| Family     | Sub-family | Part number        | Supported |
|------------|------------|--------------------|-----------|
| EM530-IS   | X          | EM530DINAV53XS1X   | Yes       |
|            | PFA        | EM530DINAV53XS1PFA | No        |
|            | PFB        | EM530DINAV53XS1PFB | No        |
|            | PFC        | EM530DINAV53XS1PFC | Yes       |
| EM540-IS   | X          | EM540DINAV23XS1X   | Yes       |
|            | PFA        | EM540DINAV23XS1PFA | No        |
|            | PFB        | EM540DINAV23XS1PFB | No        |
|            | PFC        | EM540DINAV23XS1PFC | Yes       |

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
* D-Bus layer
    - _AcSensorBridge_ produces the _com.victronenergy.grid.ttyUSB??_ D-Bus
      service. Information is taken from an _AcSensor_, and an
      _AcSensorSettings_ object.
    - _AcSensorSettingsBridge_ consumes a subtree from the local settings.
      Path: /Settings/CGwacs/D[serial]

Finally _AcSensorMediator_ ties everything together.

Error handling
==============

It is important for dbus-cgwacs to terminate if it is not talking to a
Carlo Gavazzi meter, otherwise serial-starter cannot work properly. Two
kinds of errors are possible.

* A timeout:
    - There was no response to a modbus request. The default timeout is 250ms
      (2 seconds for zigbee) and can be overridden with the `--timeout`
      commandline option.
    - dbus-cgwacs will terminate after 5 consecutive requests ends in a timeout.
* A modbus error:
    - These include outright errors such as unsupported registers or CRC errors.
    - In order to be forgiving to intermittent conditions causing CRC errors,
      dbus-cgwacs will terminate after 20 consecutive errors.
