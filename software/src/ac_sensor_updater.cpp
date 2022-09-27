#include <cmath>
#include <QsLog.h>
#include <QTimer>
#include "ac_sensor.h"
#include "ac_sensor_settings.h"
#include "ac_sensor_updater.h"
#include "data_processor.h"
#include "modbus_rtu.h"
#include "ac_sensor_phase.h"

static const int MeasurementSystemP1 = 3; // single phase (1P)
static const int MeasurementSystemP2 = 2; // 2 phase (2P)
static const int MeasurementSystemP3 = 0; // 3 phase (3Pn)

// measurement mode used for ET1xx/ET3xx meters. Mode B means that feed back to grid yields
// negative power readings. EM540 supports mode C, which should do total energy accounting.
static const int MeasurementModeB = 1;
static const int MeasurementModeC = 2;

static const int ApplicationH = 7; // show negative power (EM24)
static const int MaxAcquisitionIndex = 16;
static const int MaxRegCount = 5;
static const int MaxTimeoutCount = 5;
static const int MaxErrorCount = 20;

static const int NoError = 0;
static const int ErrorFronSelectorLocked = 1;

static const int FrontSelectorWaitInterval = 5 * 1000; // 5 seconds in ms
static const int ReconnectInterval = 15 * 1000;  // 15 seconds in ms
static const int ZigbeeReconnectInterval = 30 * 1000;  // 30 seconds in ms
static const int UpdateSettingsInterval = 10 * 60 * 1000; // 10 minutes in ms

enum ParameterType {
	None,
	Dummy,
	Power,
	Voltage,
	Current,
	PositiveEnergy,
	NegativeEnergy
};

struct RegisterCommand {
	int regOffset;
	ParameterType action;
	Phase phase;
};

struct CompositeCommand {
	int reg;
	int interval;
	RegisterCommand actions[MaxRegCount];
};

#define CMDCOUNT(x) sizeof(x) / sizeof(x[0])

static const CompositeCommand Em24Commands[] = {
	{ 0x0028, 0, { { 0, Power, MultiPhase } } },
	{ 0x0012, 0, { { 0, Power, PhaseL1 }, { 2, Power, PhaseL2 }, { 4, Power, PhaseL3 } } },
	{ 0x0024, 2, { { 0, Voltage, MultiPhase } } },
	{ 0x0000, 4, { { 0, Voltage, PhaseL1 }, { 2, Voltage, PhaseL2 }, { 4, Voltage, PhaseL3 } } },
	{ 0x000C, 8, { { 0, Current, PhaseL1 }, { 2, Current, PhaseL2 }, { 4, Current, PhaseL3 } } },
	{ 0x003E, 10, { { 0, PositiveEnergy, MultiPhase } } },
	{ 0x0046, 12, { { 0, PositiveEnergy, PhaseL1 }, { 2, PositiveEnergy, PhaseL2 }, { 4, PositiveEnergy, PhaseL3 } } },
	{ 0x005C, 14, { { 0, NegativeEnergy, MultiPhase } } }
};

/// We use dummy commands here to vary the number of requested registers. This
/// way we avoid problems if a response to a modbus request arrives too late.
/// This may happen when sending modbus packages over zigbee.
/// If that happens the packages will be interpreted incorrect (eg. voltage
/// values as power). By varying the number of registers it easier to detect
/// late packets.
static const CompositeCommand Em24CommandsP1[] = {
	{ 0x0028, 0, { { 0, Power, MultiPhase } } },
	{ 0x0024, 2, { { 0, Voltage, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x000C, 8, { { 0, Current, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x003E, 10, { { 0, PositiveEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x005C, 14, { { 0, NegativeEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } }
};

static const CompositeCommand Em24CommandsP1PV[] = {
	{ 0x0012, 0, { { 0, Power, PhaseL1 } } },
	{ 0x0014, 2, { { 0, Power, PhaseL2 }, { 1, Dummy, MultiPhase } } },
	{ 0x0000, 4, { { 0, Voltage, PhaseL1 }, { 2, Voltage, PhaseL2 } } },
	{ 0x000C, 6, { { 0, Current, PhaseL1 }, { 2, Current, PhaseL2 } } },
	{ 0x0046, 8, { { 0, PositiveEnergy, PhaseL1 }, { 2, PositiveEnergy, PhaseL2 } } },
	// Note that NegativeEnergy will give us the energy of all phases. Right now
	// we assume that in case of a shared system L1 is a grid meter and L2 a
	// PV inverter (which always has ReverseEnergy=0 because power and current
	// are always positive).
	{ 0x005C, 10, { { 0, NegativeEnergy, PhaseL1 }, { 1, Dummy, MultiPhase } } }
};

static const CompositeCommand Em112Commands[] = {
	{ 0x0004, 0, { { 0, Power, MultiPhase } } },
	{ 0x0000, 4, { { 0, Voltage, MultiPhase }, { 2, Current, MultiPhase } } },
	{ 0x0010, 8, { { 0, PositiveEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x0020, 12, { { 0, NegativeEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } }
};

static const CompositeCommand Et340Commands[] = {
	{ 0x0028, 0, { { 0, Power, MultiPhase } } },
	{ 0x0012, 0, { { 0, Power, PhaseL1 }, { 2, Power, PhaseL2 }, { 4, Power, PhaseL3 } } },
	{ 0x0024, 1, { { 0, Voltage, MultiPhase }, { 2, Dummy, MultiPhase } } },
	{ 0x0000, 2, { { 0, Voltage, PhaseL1 }, { 2, Voltage, PhaseL2 }, { 4, Voltage, PhaseL3 }, {6, Dummy, MultiPhase } } },
	{ 0x000C, 4, { { 0, Current, PhaseL1 }, { 2, Current, PhaseL2 }, { 4, Current, PhaseL3 }, {6, Dummy, MultiPhase } } },
	{ 0x0034, 6, { { 0, PositiveEnergy, MultiPhase }, { 2, Dummy, MultiPhase } } },
	{ 0x0040, 8, { { 0, PositiveEnergy, PhaseL1 }, { 2, PositiveEnergy, PhaseL2 }, { 4, PositiveEnergy, PhaseL3 }, {6, Dummy, MultiPhase } } },
	{ 0x004E, 10, { { 0, NegativeEnergy, MultiPhase }, { 2, Dummy, MultiPhase } } },
	{ 0x0060, 12, { { 0, NegativeEnergy, PhaseL1 }, { 2, NegativeEnergy, PhaseL2 }, { 4, NegativeEnergy, PhaseL3 }, {6, Dummy, MultiPhase } } },
};

static const CompositeCommand Et340P1Commands[] = {
	{ 0x0012, 0, { { 0, Power, MultiPhase } } },
	{ 0x0000, 1, { { 0, Voltage, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x000C, 3, { { 0, Current, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x0040, 5, { { 0, PositiveEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x0060, 7, { { 0, NegativeEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } },
};

static const CompositeCommand Et340CommandsP1PV[] = {
	{ 0x0012, 0, { { 0, Power, PhaseL1 } } },
	{ 0x0014, 2, { { 0, Power, PhaseL2 }, { 1, Dummy, MultiPhase } } },
	{ 0x0000, 4, { { 0, Voltage, PhaseL1 }, { 2, Voltage, PhaseL2 } } },
	{ 0x000C, 6, { { 0, Current, PhaseL1 }, { 2, Current, PhaseL2 } } },
	{ 0x0040, 8, { { 0, PositiveEnergy, PhaseL1 }, { 2, PositiveEnergy, PhaseL2 } } },
	{ 0x0060, 10, { { 0, NegativeEnergy, PhaseL1 }, { 2, NegativeEnergy, PhaseL2 } } }
};

static const CompositeCommand Em300Commands[] = {
	{ 0x0028, 0, { { 0, Power, MultiPhase } } },
	{ 0x0012, 0, { { 0, Power, PhaseL1 }, { 2, Power, PhaseL2 }, { 4, Power, PhaseL3 } } },
	{ 0x0024, 1, { { 0, Voltage, MultiPhase }, { 2, Dummy, MultiPhase } } },
	{ 0x0000, 2, { { 0, Voltage, PhaseL1 }, { 2, Voltage, PhaseL2 }, { 4, Voltage, PhaseL3 }, {6, Dummy, MultiPhase } } },
	{ 0x000C, 4, { { 0, Current, PhaseL1 }, { 2, Current, PhaseL2 }, { 4, Current, PhaseL3 }, {6, Dummy, MultiPhase } } },
	{ 0x0034, 6, { { 0, PositiveEnergy, MultiPhase }, { 2, Dummy, MultiPhase } } },
	{ 0x0040, 8, { { 0, PositiveEnergy, PhaseL1 }, { 2, PositiveEnergy, PhaseL2 }, { 4, PositiveEnergy, PhaseL3 }, {6, Dummy, MultiPhase } } },
	{ 0x004E, 12, { { 0, NegativeEnergy, MultiPhase }, { 2, Dummy, MultiPhase } } },
};

static const CompositeCommand Em300P1Commands[] = {
	{ 0x0012, 0,  { { 0, Power, MultiPhase } } },
	{ 0x0000, 4,  { { 0, Voltage, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x000C, 8,  { { 0, Current, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x0040, 12, { { 0, PositiveEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } },
};

static const CompositeCommand Em300CommandsP1PV[] = {
	{ 0x0012, 0,  { { 0, Power, PhaseL1 } } },
	{ 0x0014, 4,  { { 0, Power, PhaseL2 }, { 1, Dummy, MultiPhase } } },
	{ 0x0000, 8,  { { 0, Voltage, PhaseL1 }, { 2, Voltage, PhaseL2 } } },
	{ 0x000C, 12, { { 0, Current, PhaseL1 }, { 2, Current, PhaseL2 } } },
	{ 0x0040, 15, { { 0, PositiveEnergy, PhaseL1 }, { 2, PositiveEnergy, PhaseL2 } } },
};

static const CompositeCommand Em540Commands[] = {
	{ 0x0028, 0, { { 0, Power, MultiPhase } } },
	{ 0x0012, 0, { { 0, Power, PhaseL1 }, { 2, Power, PhaseL2 }, { 4, Power, PhaseL3 } } },
	{ 0x0024, 1, { { 0, Voltage, MultiPhase }, { 2, Dummy, MultiPhase } } },
	{ 0x0000, 2, { { 0, Voltage, PhaseL1 }, { 2, Voltage, PhaseL2 }, { 4, Voltage, PhaseL3 }, {6, Dummy, MultiPhase } } },
	{ 0x000C, 4, { { 0, Current, PhaseL1 }, { 2, Current, PhaseL2 }, { 4, Current, PhaseL3 }, {6, Dummy, MultiPhase } } },
	{ 0x0034, 6, { { 0, PositiveEnergy, MultiPhase }, { 2, Dummy, MultiPhase } } },
	{ 0x0040, 8, { { 0, PositiveEnergy, PhaseL1 }, { 2, PositiveEnergy, PhaseL2 }, { 4, PositiveEnergy, PhaseL3 }, {6, Dummy, MultiPhase } } },
	{ 0x004E, 10, { { 0, NegativeEnergy, MultiPhase }, { 2, Dummy, MultiPhase } } },
};

static const CompositeCommand Em540P1Commands[] = {
	{ 0x0012, 0, { { 0, Power, MultiPhase } } },
	{ 0x0000, 1, { { 0, Voltage, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x000C, 3, { { 0, Current, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x0040, 5, { { 0, PositiveEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x004E, 7, { { 0, NegativeEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } },
};

static const CompositeCommand Em540CommandsP1PV[] = {
	{ 0x0012, 0, { { 0, Power, PhaseL1 } } },
	{ 0x0014, 2, { { 0, Power, PhaseL2 }, { 1, Dummy, MultiPhase } } },
	{ 0x0000, 4, { { 0, Voltage, PhaseL1 }, { 2, Voltage, PhaseL2 } } },
	{ 0x000C, 6, { { 0, Current, PhaseL1 }, { 2, Current, PhaseL2 } } },
	{ 0x0040, 8, { { 0, PositiveEnergy, PhaseL1 }, { 2, PositiveEnergy, PhaseL2 } } },
	// As with the EM24, there is no individual negative counters for exported
	// energy. We assume that in a shared system, L1 is a grid meter and L2
	// is a PV-inverter, so on L2 there is never any imported power, therefore
	// all negative energy can be assumed to be on L1.
	{ 0x004E, 10, { { 0, NegativeEnergy, PhaseL1 }, { 2, NegativeEnergy, PhaseL2 } } }
};

// Even though this meter is supposedly the same as an EM24, it is still
// too much of an EM300, and single-phase needs a special command-set.
static const CompositeCommand Em300S27P1Commands[] = {
	{ 0x0028, 0, { { 0, Power, MultiPhase } } },
	{ 0x0000, 2, { { 0, Voltage, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x000C, 8, { { 0, Current, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x003E, 10, { { 0, PositiveEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } },
	{ 0x005C, 14, { { 0, NegativeEnergy, MultiPhase }, { 1, Dummy, MultiPhase } } }
};

int getMaxOffset(const CompositeCommand &cmd) {
	int maxOffset = 0;
	for (int i=0; i<MaxRegCount; ++i) {
		const RegisterCommand &ra = cmd.actions[i];
		if (ra.action == None)
			break;
		maxOffset = qMax(maxOffset, ra.regOffset);
	}
	return maxOffset;
}

AcSensorUpdater::AcSensorUpdater(AcSensor *acSensor, AcSensor *acPvSensor, ModbusRtu *modbus,
								 bool isZigbee, QObject *parent):
	QObject(parent),
	mDataProcessor(0),
	mPvDataProcessor(0),
	mAcSensor(acSensor),
	mAcPvSensor(acPvSensor),
	mSettings(0),
	mModbus(0),
	mAcquisitionTimer(new QTimer(this)),
	mSettingsUpdateTimer(new QTimer(this)),
	mTimeoutCount(0),
	mErrorCount(0),
	mMeasuringSystem(0),
	mDesiredMeasuringSystem(0),
	mIsZigbee(isZigbee),
	mSetupRequested(false),
	mApplication(0),
	mState(DeviceId),
	mCommands(0),
	mCommandCount(0),
	mCommandIndex(0),
	mAcquisitionIndex(0),
	mSetCurrentSign(true)
{
	Q_ASSERT(acSensor != 0);
	Q_ASSERT(acPvSensor != 0);
	Q_ASSERT(acSensor->slaveAddress() == acPvSensor->slaveAddress());
	mModbus = modbus;
	connect(mModbus, SIGNAL(readCompleted(int, quint8, const QList<quint16> &)),
			this, SLOT(onReadCompleted(int, quint8, QList<quint16>)));
	connect(mModbus, SIGNAL(writeCompleted(int, quint8, quint16, quint16)),
			this, SLOT(onWriteCompleted(int, quint8, quint16, quint16)));
	connect(mModbus, SIGNAL(errorReceived(int, quint8, int)),
			this, SLOT(onErrorReceived(int, quint8, int)));
	connect(mAcquisitionTimer, SIGNAL(timeout()),
			this, SLOT(onWaitFinished()));
	connect(mSettingsUpdateTimer, SIGNAL(timeout()),
			this, SLOT(onUpdateSettings()));
	mSettingsUpdateTimer->setInterval(UpdateSettingsInterval);
	mSettingsUpdateTimer->start();
	mAcquisitionTimer->setSingleShot(true);
	mStopwatch.start();
	startNextAction();
}

AcSensor *AcSensorUpdater::acSensor()
{
	return mAcSensor;
}

AcSensor *AcSensorUpdater::pvSensor()
{
	return mAcPvSensor;
}

AcSensorSettings *AcSensorUpdater::settings()
{
	return mSettings;
}

void AcSensorUpdater::startMeasurements()
{
	if (mSettings == 0 || mState != WaitForStart) {
		QLOG_ERROR() << "Cannot start measurements before device has been detected";
		return;
	}
	mAcquisitionIndex = 0;
	mCommandIndex = 0;
	switch (mAcSensor->protocolType()) {
	case AcSensor::Em24Protocol:
		mState = CheckSetup;
		break;
	case AcSensor::Et112Protocol:
	case AcSensor::Em540Protocol:
	case AcSensor::Em300S27Protocol:
	case AcSensor::Em300Protocol:
		// Fall through
	case AcSensor::Et340Protocol:
		mState = CheckMeasurementMode;
		break;
	case AcSensor::Unknown:
		Q_ASSERT(false);
		break;
	}
	startNextAction();
}

void AcSensorUpdater::onErrorReceived(int errorType, quint8 addr, int exception)
{
	if (addr != mAcSensor->slaveAddress())
		return;

	/* An error on fetching the per-phase registers for an EM-300 is "normal"
	   for some meters. Switch protocol and continue with next stage. */
	if (mState == PhaseReverseEnergy) {
		QLOG_INFO() << "Meter has no per-phase reverse registers. Assuming older EM-300 meter.";
		mAcSensor->setProtocolType(AcSensor::Em300Protocol);
		mState = Serial;
	}

	QLOG_DEBUG() << "ModBus Error:" << errorType << exception
				 << "State:" << mState << "Slave Address" << addr
				 << "Acq State:" << mAcquisitionIndex
				 << "Timeout count:" << mTimeoutCount
				 << "Error count:" << mErrorCount;
	/* Deliberately treat all errors the same. Possible errors are Timeout,
	 * Exception, Unsupported, CrcError. If we get any of these 5 times in a
	 * row we should bail. */
	if ((mTimeoutCount >= MaxTimeoutCount) || (mErrorCount >= MaxErrorCount)) {
		if (!mAcSensor->serial().isEmpty()) {
			QLOG_ERROR() << "Lost connection to energy meter"
						 << mAcSensor->serial() << '@'
						 << mAcSensor->portName() << ':'
						 << mAcSensor->slaveAddress();
		}
		disconnectSensor();
	} else if (errorType == ModbusRtu::Timeout) {
		++mTimeoutCount;
	} else {
		++mErrorCount;
	}
	startNextAction();
}

void AcSensorUpdater::onReadCompleted(int function, quint8 addr, const QList<quint16> &registers)
{
	if (addr != mAcSensor->slaveAddress())
		return;
	Q_UNUSED(function)
	switch (mState) {
	case DeviceId:
		QLOG_INFO() << "Device ID:" << registers[0];
		mAcSensor->setDeviceType(registers[0]);
		mAcPvSensor->setDeviceType(registers[0]);
		switch (mAcSensor->protocolType()) {
		case AcSensor::Em24Protocol:
			mSetCurrentSign = true;
			mState = VersionCode;
			break;
		case AcSensor::Em300Protocol:
		case AcSensor::Et340Protocol:
			mSetCurrentSign = false;
			mState = PhaseReverseEnergy; // Check phase reverse energy support
			break;
		case AcSensor::Et112Protocol:
		case AcSensor::Em300S27Protocol:
			mSetCurrentSign = false;
			mState = Serial;
			break;
		case AcSensor::Em540Protocol:
			mSetCurrentSign = true;
			mState = Serial;
			break;
		case AcSensor::Unknown:
			QLOG_WARN() << "Unknown device ID, disconnecting";
			disconnectSensor();
			return;
		}
		break;
	case VersionCode:
		mAcSensor->setDeviceSubType(registers[0]);
		mAcPvSensor->setDeviceSubType(registers[0]);
		mState = Serial;
		break;
	case Serial:
	{
		Q_ASSERT(registers.size() == 7);
		QString serial;
		foreach (quint16 r, registers) {
			serial.append(r >> 8);
			serial.append(r & 0xFF);
		}
		// Some grid meters (ET112, ET340) add zero values in the MSB's of the
		// registers. Others (EM24) add zero padding at the end.
		serial.remove(QChar(0));
		// Sometimes the serial reported contains this first character of the
		// serial number only. If that happens we simulate a timeout to the
		// detection process will be reset or aborted.
		if (serial.size() < 2) {
			QLOG_WARN() << "Incorrect serial reported:" << serial;
			onErrorReceived(ModbusRtu::Timeout, mAcPvSensor->slaveAddress(), 0);
			return;
		}
		mAcSensor->setSerial(serial);
		mAcPvSensor->setSerial(serial);
		mState = FirmwareVersion;
		break;
	}
	case FirmwareVersion:
		mAcSensor->setFirmwareVersion(registers[0]);
		mAcPvSensor->setFirmwareVersion(registers[0]);

		// For meters that support it, read the phase sequence
		if ((mAcSensor->protocolType() == AcSensor::Em24Protocol) ||
				(mAcSensor->protocolType() == AcSensor::Et340Protocol) ||
				(mAcSensor->protocolType() == AcSensor::Em300Protocol) ||
				(mAcSensor->protocolType() == AcSensor::Em300S27Protocol) ||
				(mAcSensor->protocolType() == AcSensor::Em540Protocol)) {
			mState = PhaseSequence;
		} else {
			mState = WaitForStart;
		}
		break;
	case PhaseSequence:
		{
			int seq = (registers[0] == 0 ? PhaseSequenceOk : PhaseSequenceNotOk);
			mAcSensor->setPhaseSequence(seq);
			mState = WaitForStart;
		}
		break;
	case PhaseReverseEnergy:
		mState = Serial;
		break;
	case CheckSetup:
		Q_ASSERT(registers.size() == 2);
		mApplication = registers[0];
		mDesiredMeasuringSystem =
			mSettings->isMultiPhase() || mSettings->piggyEnabled() ?
			MeasurementSystemP3 : MeasurementSystemP1;
		mMeasuringSystem = registers[1];
		mState = mApplication == ApplicationH &&
				 mMeasuringSystem == mDesiredMeasuringSystem ?
					 Acquisition : CheckFrontSelector;
		break;
	case CheckFrontSelector:
		if (registers[0] == 3) {
			// There are 2 reasons to change settings on the device:
			// * Change measuring system (single phase, multi phase, ...).
			//   EM24: Setting the device in multi phase mode while it is wired
			//   for single phase (ie. port 1 & 4 connected) will distort
			//   measurements.
			// * Change application: only application H will show power
			//   flowing back to the grid as negative values.
			QLOG_ERROR() << "Energy meter Application incorrect";
			mState = WaitFrontSelector;
			mAcSensor->setErrorCode(ErrorFronSelectorLocked);
			mAcPvSensor->setErrorCode(ErrorFronSelectorLocked);
		} else {
			if (mApplication != RegApplication) {
				mState = SetApplication;
			} else if (mMeasuringSystem != mDesiredMeasuringSystem) {
				mState = SetMeasuringSystem;
			} else {
				mState = Acquisition;
			}
			mAcSensor->setErrorCode(NoError);
			mAcPvSensor->setErrorCode(NoError);
		}
		break;
	case CheckMeasurementMode:
		{
			Q_ASSERT(registers.size() == 1);
			int desiredMode = mAcSensor->protocolType() == AcSensor::Em540Protocol ?
				MeasurementModeC : MeasurementModeB;
			if (registers[0] == desiredMode) {
				switch (mAcSensor->protocolType()) {
				case AcSensor::Et340Protocol:
				case AcSensor::Em300Protocol:
				case AcSensor::Em540Protocol:
				case AcSensor::Em300S27Protocol:
					mState = CheckMeasurementSystem;
					break;
				default:
					mState = Acquisition;
				}
			} else {
				mState = SetMeasurementMode;
			}
		}
		break;
	case CheckMeasurementSystem:
		Q_ASSERT(registers.size() == 1);
		Q_ASSERT((mAcSensor->protocolType() == AcSensor::Et340Protocol) ||
			(mAcSensor->protocolType() == AcSensor::Em300Protocol) ||
			(mAcSensor->protocolType() == AcSensor::Em300S27Protocol) ||
			(mAcSensor->protocolType() == AcSensor::Em540Protocol));
		// Caution: EM3xx meters do not support MeasurementSystemP1
		// Changing the measurement system also resets the kWh counters.
		mDesiredMeasuringSystem = mSettings->isMultiPhase() || !mSettings->piggyEnabled() ?
			MeasurementSystemP3 : MeasurementSystemP2;
		mState = mDesiredMeasuringSystem == registers[0] ? Acquisition : SetMeasuringSystem;
		break;
	case Acquisition:
		processAcquisitionData(registers);
		++mCommandIndex;
		break;
	case Wait:
		mState = Acquisition;
		break;
	default:
		QLOG_ERROR() << "Unknown updater state" << mState;
		mState = mSettings == 0 ? DeviceId : Acquisition;
		break;
	}
	mTimeoutCount = 0;
	mErrorCount = 0;
	startNextAction();
}

void AcSensorUpdater::onWriteCompleted(int function, quint8 addr,
										  quint16 address, quint16 value)
{
	if (addr != mAcSensor->slaveAddress())
		return;
	Q_UNUSED(function)
	Q_UNUSED(address)
	Q_UNUSED(value)
	switch (mState) {
	case SetApplication:
		Q_ASSERT(function == ModbusRtu::WriteSingleRegister);
		Q_ASSERT(address == RegApplication);
		Q_ASSERT(value == ApplicationH);
		mState = mMeasuringSystem == mDesiredMeasuringSystem ?
			Acquisition : SetMeasuringSystem;
		break;
	case SetMeasuringSystem:
		Q_ASSERT(function == ModbusRtu::WriteSingleRegister);
		Q_ASSERT(address == RegMeasurementSystem || address == RegEm300MeasurementSystem);
		Q_ASSERT(value == mDesiredMeasuringSystem);
		mState = Acquisition;
		break;
	case SetMeasurementMode:
		{
			switch (mAcSensor->protocolType()) {
			case AcSensor::Et340Protocol:
			case AcSensor::Em300Protocol:
			case AcSensor::Em540Protocol:
			case AcSensor::Em300S27Protocol:
				mState = CheckMeasurementSystem;
				break;
			default:
				mState = Acquisition;
			}
		}
		break;
	case SetAddress:
		QLOG_WARN() << "Slave Address Changed";
		mState = Serial;
		break;
	default:
		mState = mAcSensor->protocolType() == AcSensor::Em24Protocol ?
			CheckSetup :
			Acquisition;
		break;
	}
	mTimeoutCount = 0;
	mErrorCount = 0;
	startNextAction();
}

void AcSensorUpdater::onWaitFinished()
{
	switch (mState) {
	case Wait:
		mStopwatch.restart();
		mState = Acquisition;
		break;
	case WaitFrontSelector:
		mState = CheckSetup;
		break;
	case WaitOnConnectionLost:
		mState = DeviceId;
		break;
	default:
		mState = mAcSensor->protocolType() == AcSensor::Em24Protocol ?
			CheckSetup :
			Acquisition;
		break;
	}
	startNextAction();
}

void AcSensorUpdater::onUpdateSettings()
{
	if (mDataProcessor != 0)
		mDataProcessor->updateEnergySettings();
	if (mPvDataProcessor != 0)
		mPvDataProcessor->updateEnergySettings();
}

void AcSensorUpdater::onIsMultiPhaseChanged()
{
	mSetupRequested = true;
}

void AcSensorUpdater::onL2ServiceTypeChanged()
{
	mSetupRequested = true;
}

void AcSensorUpdater::startNextAction()
{
	if (mSetupRequested) {
		mSetupRequested = false;
		mAcSensor->resetValues();
		mAcPvSensor->resetValues();
		switch (mAcSensor->protocolType()) {
		case AcSensor::Em24Protocol:
			mState = CheckSetup;
			break;
		case AcSensor::Et340Protocol:
		case AcSensor::Em300Protocol:
		case AcSensor::Em300S27Protocol:
			mState = CheckMeasurementMode;
			break;
		default:
			break;
		}
	}
	switch (mState) {
	case DeviceId:
		mAcSensor->setConnectionState(Searched);
		readRegisters(RegDeviceId, 1);
		break;
	case VersionCode:
		readRegisters(RegEm24VersionCode, 1);
		break;
	case Serial:
		if (mAcSensor->protocolType() == AcSensor::Em24Protocol) {
			readRegisters(RegEm24Serial, 7);
		} else {
			// Also works for EM3xx and EM5xx
			readRegisters(RegEm112Serial, 7);
		}
		break;
	case FirmwareVersion:
		switch (mAcSensor->protocolType()) {
		case AcSensor::Em24Protocol:
		case AcSensor::Et112Protocol:
			readRegisters(RegAltFirmwareVersion, 1); // EM24 and ET112 has version code at 303h
			break;
		default:
			readRegisters(RegFirmwareVersion, 1); // EM340 and EM540 has firmware version at 302h
		}
		break;
	case PhaseSequence:
		switch(mAcSensor->protocolType()) {
		case AcSensor::Em24Protocol:
			readRegisters(RegEm24PhaseSequence, 1);
			break;
		case AcSensor::Em540Protocol:
			readRegisters(RegEm540PhaseSequence, 1);
			break;
		default:
			readRegisters(RegEm300PhaseSequence, 1);
		}
		break;
	case PhaseReverseEnergy: // This state is only reached for EM-300 meters
		readRegisters(0x0060, 2); // Location of L1 reverse energy counter
		break;
	case CheckSetup:
		readRegisters(RegApplication, 2);
		break;
	case CheckFrontSelector:
		readRegisters(RegEm24FrontSelector, 1);
		break;
	case WaitFrontSelector:
		mAcquisitionTimer->setInterval(FrontSelectorWaitInterval);
		mAcquisitionTimer->start();
		break;
	case SetApplication:
		QLOG_INFO() << "Change application to application H";
		writeRegister(RegApplication, ApplicationH);
		break;
	case SetMeasuringSystem:
		QLOG_INFO() << "Change measuring system to:" << mDesiredMeasuringSystem;
		writeRegister(
			mAcSensor->protocolType() == AcSensor::Em24Protocol ?
				RegMeasurementSystem :
				RegEm300MeasurementSystem,
			mDesiredMeasuringSystem);
		break;
	case CheckMeasurementMode:
		readRegisters(RegEm112MeasurementMode, 1); // Same for EM3xx, EM1xx, EM540
		break;
	case CheckMeasurementSystem:
		readRegisters(
			mAcSensor->protocolType() == AcSensor::Em24Protocol ?
				RegMeasurementSystem :
				RegEm300MeasurementSystem, // Also works for EM540
			1);
		break;
	case SetMeasurementMode:
		switch (mAcSensor->protocolType()) {
		case AcSensor::Em540Protocol:
			QLOG_INFO() << "Set EM540 measurement mode to C";
			writeRegister(RegEm112MeasurementMode, MeasurementModeC);
			break;
		default:
			// This will cause the EM1xx/EM3xx range to report negative power values when
			// sending power to the grid.
			QLOG_INFO() << "Set EM1xx/EM3xx measurement mode to B";
			writeRegister(RegEm112MeasurementMode, MeasurementModeB);
		}
		break;
	case WaitForStart:
		Q_ASSERT(mSettings == 0);
		mSettings = new AcSensorSettings(mAcSensor->supportMultiphase(),
											mAcSensor->serial(),
											mAcSensor);
		mDataProcessor = new DataProcessor(mAcSensor, mSettings, this);
		mPvDataProcessor = new DataProcessor(mAcPvSensor, mSettings, this);
		connect(mSettings, SIGNAL(isMultiPhaseChanged()),
				this, SLOT(onIsMultiPhaseChanged()));
		connect(mSettings, SIGNAL(l2ClassAndVrmInstanceChanged()),
				this, SLOT(onL2ServiceTypeChanged()));
		mAcSensor->setConnectionState(Detected);
		break;
	case Acquisition:
		switch (mAcSensor->protocolType()) {
		case AcSensor::Em24Protocol:
			if (mSettings->isMultiPhase()) {
				mCommands = Em24Commands;
				mCommandCount = CMDCOUNT(Em24Commands);
			} else if (mSettings->piggyEnabled()) {
				mCommands = Em24CommandsP1PV;
				mCommandCount = CMDCOUNT(Em24CommandsP1PV);
			} else {
				mCommands = Em24CommandsP1;
				mCommandCount = CMDCOUNT(Em24CommandsP1);
			}
			break;
		case AcSensor::Et112Protocol:
			mCommands = Em112Commands;
			mCommandCount = CMDCOUNT(Em112Commands);
			break;
		case AcSensor::Et340Protocol:
			if (mSettings->isMultiPhase()) {
				mCommands = Et340Commands;
				mCommandCount = CMDCOUNT(Et340Commands);
			} else if (mSettings->piggyEnabled()) {
				mCommands = Et340CommandsP1PV;
				mCommandCount = CMDCOUNT(Et340CommandsP1PV);
			} else {
				mCommands = Et340P1Commands;
				mCommandCount = CMDCOUNT(Et340P1Commands);
			}
			break;
		case AcSensor::Em300Protocol:
			if (mSettings->isMultiPhase()) {
				mCommands = Em300Commands;
				mCommandCount = CMDCOUNT(Em300Commands);
			} else if (mSettings->piggyEnabled()) {
				mCommands = Em300CommandsP1PV;
				mCommandCount = CMDCOUNT(Em300CommandsP1PV);
			} else {
				mCommands = Em300P1Commands;
				mCommandCount = CMDCOUNT(Em300P1Commands);
			}
			break;
		case AcSensor::Em540Protocol:
			if (mSettings->isMultiPhase()) {
				mCommands = Em540Commands;
				mCommandCount = CMDCOUNT(Em540Commands);
			} else if (mSettings->piggyEnabled()) {
				mCommands = Em540CommandsP1PV;
				mCommandCount = CMDCOUNT(Em540CommandsP1PV);
			} else {
				mCommands = Em540P1Commands;
				mCommandCount = CMDCOUNT(Em540P1Commands);
			}
			break;
		case AcSensor::Em300S27Protocol:
			// Same registers as the EM24... mostly.
			if (mSettings->isMultiPhase()) {
				mCommands = Em24Commands;
				mCommandCount = CMDCOUNT(Em24Commands);
			} else if (mSettings->piggyEnabled()) {
				mCommands = Em24CommandsP1PV;
				mCommandCount = CMDCOUNT(Em24CommandsP1PV);
			} else {
				mCommands = Em300S27P1Commands;
				mCommandCount = CMDCOUNT(Em24CommandsP1);
			}
			break;
		case AcSensor::Unknown:
			Q_ASSERT(false);
			break;
		}
		startNextAcquisition();
		break;
	case Wait:
	{
		int sleep = mStopwatch.elapsed();
		sleep = 250 - sleep;
		if (sleep > 50) {
			mAcquisitionTimer->setInterval(sleep);
			mAcquisitionTimer->start();
		} else {
			onWaitFinished();
		}
		break;
	}
	case WaitOnConnectionLost:
		mAcquisitionTimer->setInterval(mIsZigbee ? ZigbeeReconnectInterval : ReconnectInterval);
		mAcquisitionTimer->start();
		break;
	case SetAddress:
		QLOG_INFO() << "Set modbuss address to 2";
		writeRegister(0x2000, 2);
		break;
	default:
		QLOG_ERROR() << "Invalid state while starting new action" << mState;
		mState = Acquisition;
		startNextAction();
		break;
	}
}

void AcSensorUpdater::startNextAcquisition()
{
	const CompositeCommand *cmd = 0;
	for (;;) {
		if (mCommandIndex < mCommandCount)
			cmd = &mCommands[mCommandIndex];
		if (cmd !=0 && (cmd->interval == 0 || mAcquisitionIndex == cmd->interval)) {
			break;
		} else {
			++mCommandIndex;
			if (mCommandIndex >= mCommandCount) {
				mState = Wait;
				mCommandIndex = 0;
				++mAcquisitionIndex;
				if (mAcquisitionIndex == MaxAcquisitionIndex) {
					mAcquisitionIndex = 0;
					mAcSensor->setConnectionState(Connected);
					mAcPvSensor->setConnectionState(Connected);
				}
				mAcSensor->flushValues(); // Flush values out on dbus
				startNextAction();
				return;
			}
		}
	}
	int maxOffset = getMaxOffset(*cmd);
	readRegisters(cmd->reg, maxOffset + 2);
}

void AcSensorUpdater::disconnectSensor()
{
	mState = WaitOnConnectionLost;
	delete mSettings;
	mSettings = 0;
	delete mDataProcessor;
	mDataProcessor = 0;
	delete mPvDataProcessor;
	mPvDataProcessor = 0;
	mTimeoutCount = MaxTimeoutCount;
	mErrorCount = MaxErrorCount;
	mAcSensor->setSerial(QString());
	mAcSensor->resetValues();
	mAcSensor->setConnectionState(Disconnected);
	mAcPvSensor->setSerial(QString());
	mAcPvSensor->resetValues();
	mAcPvSensor->setConnectionState(Disconnected);
}

void AcSensorUpdater::readRegisters(quint16 startReg, quint16 count)
{
	mModbus->readRegisters(ModbusRtu::ReadHoldingRegisters,
						   mAcSensor->slaveAddress(), startReg, count);
}

void AcSensorUpdater::writeRegister(quint16 reg, quint16 value)
{
	mModbus->writeRegister(ModbusRtu::WriteSingleRegister,
						   mAcSensor->slaveAddress(), reg, value);
}

void AcSensorUpdater::processAcquisitionData(const QList<quint16> &registers)
{
	const CompositeCommand &cmd = mCommands[mCommandIndex];
	int regCount = getMaxOffset(cmd) + 2;
	if (regCount != registers.size()) {
		QLOG_WARN() << "Incorrect number of registers received"
					<< regCount << registers.size() << mCommandIndex;
		return;
	}
	for (int i=0; i<=regCount; ++i) {
		RegisterCommand ra = cmd.actions[i];
		if (ra.action == None)
			break;
		DataProcessor *dest = mDataProcessor;
		if (mSettings->piggyEnabled()) {
			if (ra.phase == PhaseL2)
				dest = mPvDataProcessor;
			ra.phase = MultiPhase;
		}
		if (mSettings->isMultiPhase() || ra.phase == MultiPhase) {
			bool setPhaseL1 = ra.phase == MultiPhase && !mSettings->isMultiPhase();
			double v = 0;
			switch (ra.action) {
			case Power:
				v = getDouble(registers, ra.regOffset, 0.1);
				dest->setPower(ra.phase, v);
				if (setPhaseL1)
					dest->setPower(PhaseL1, v);
				break;
			case Voltage:
				v = getDouble(registers, ra.regOffset, 0.1);
				dest->setVoltage(ra.phase, v);
				if (setPhaseL1)
					dest->setVoltage(PhaseL1, v);
				break;
			case Current:
				v = getDouble(registers, ra.regOffset, 1e-3);
				if (mSetCurrentSign &&
					dest == mDataProcessor &&
					mAcSensor->getPhase(ra.phase)->power() < 0) {
					v = -v;
				}
				dest->setCurrent(ra.phase, v);
				if (setPhaseL1)
					dest->setCurrent(PhaseL1, v);
				if (mSettings->isMultiPhase() && ra.phase == PhaseL3) {
					dest->setCurrent(MultiPhase,
						mAcSensor->l1()->current() +
						mAcSensor->l2()->current() +
						mAcSensor->l3()->current());
				}
				break;
			case PositiveEnergy:
				v = getDouble(registers, ra.regOffset, 0.1);
				dest->setPositiveEnergy(ra.phase, v);
				if (setPhaseL1)
					dest->setPositiveEnergy(PhaseL1, v);
				break;
			case NegativeEnergy:
				// ET112 seems to return negative values for kWh(-), unlike the
				// other meters.
				v = qAbs(getDouble(registers, ra.regOffset, 0.1));

				// These meters dont' have per-phase reverse counters
				if ((mAcSensor->protocolType() == AcSensor::Em24Protocol ||
					mAcSensor->protocolType() == AcSensor::Em540Protocol ||
					mAcSensor->protocolType() == AcSensor::Em300S27Protocol ||
					mAcSensor->protocolType() == AcSensor::Em300Protocol) &&
					mSettings->isMultiPhase()) {
					dest->setNegativeEnergy(v);
				} else {
					dest->setNegativeEnergy(ra.phase, v);
					if (setPhaseL1)
						dest->setNegativeEnergy(PhaseL1, v);
				}
				break;
			default:
				break;
			}
		}
	}
}

double AcSensorUpdater::getDouble(const QList<quint16> &registers,
									 int offset, double factor)
{
	double value = 0;
	value = (int32_t)(registers[offset] | registers[offset + 1] << 16);
	if (value == 0x7FFFFFFF) return qQNaN();
	return value * factor;
}
