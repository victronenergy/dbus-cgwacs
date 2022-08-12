dbus-cgwacs
===========

Reads energy measuremens from Carlo Gavazzi Wired AC Sensors (hence cgwacs),
and puts in on the D-Bus. Supported models: EM24, ET112, ET340, EM530, EM540
and selected EM-300 models.

### EM530/540 support
Note that for the EM530 and EM540, only Sub-family `X` and `PFC` is supported.

| Family     | Sub-family | Part number        | Measurement |
|------------|------------|--------------------|-------------|
| EM530-IS   | X          | EM530DINAV53XS1X   | CT          |
|            | PFC        | EM530DINAV53XS1PFC | CT          |
| EM540-IS   | X          | EM540DINAV23XS1X   | Internal    |
|            | PFC        | EM540DINAV23XS1PFC | Internal    |

### EM300 support
For the EM-300 series of meters, only the series-27 meters are supported,
and only `X` and `PFB` models are supported.  The series-27 is a meter that
simulates (in part) the registers of an EM24.  The following part numbers are
supported.  Note the 27 series appended at the end of the part number.

| Family     | Sub-family | Part number          | Measurement |
|------------|------------|----------------------|-------------|
| EM330      | X          | EM330DINAV53HS1X27   | CT          |
|            | PFB        | EM330DINAV53HS1PFB27 | CT          |
| EM340      | X          | EM340DINAV23XS1X27   | Internal    |
|            | PFB        | EM340DINAV23XS1PFB27 | Internal    |

Energy counters
===============
* With the exception of the ET340, all 3-phase meters lack per-phase
  reverse energy counters. Only a total energy value is available. dbus-cgwacs
  estimates the per-phase reverse energy by using the proportion of negative
  reverse power of each phase.

* The total energy counters on the EM300 series (the ET340 and the
  EM-300-series-27) is simply the sum of the individual phase counters.
  It does not aggregate the total across all the phases, therefore resulting
  in a different outcome. Where energy is simultaneously imported and exported
  on different phases, the EM24 and EM530/540 will correctly show the total
  energy across all phases, while the other meters will show higher import
  and export numbers that may not correspond with the utility meter.

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
