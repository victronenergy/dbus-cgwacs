#ifndef AC_SENSOR_UPDATER_H
#define AC_SENSOR_UPDATER_H

#include <QElapsedTimer>
#include <QObject>
#include "defines.h"
#include "modbus_rtu.h"

class DataProcessor;
class AcSensor;
class AcSensorSettings;
class PowerInfo;
struct CompositeCommand;

/*!
 * Retrieves data from a Carlo Gavazzi energy meter.
 * This class will setup a connection (modbus RTU) to an energy meter and
 * retrieve data from the device. The value will be stored in an `AcSensor`
 * object.
 *
 * This class is implemented as a state engine. The diagram below shows the
 * progress through the states.
 * @dotfile ac_sensor_updater_states.dot
 */
class AcSensorUpdater : public QObject
{
	Q_OBJECT
public:
	/*!
	 * Creates an instance of `AcSensorUpdater`, and starts the setup
	 * process.
	 * If the setup succeeds, the `deviceFound` signal will be emitted,
	 * otherwise the `connectLost` signal. Once the `deviceFound` signal is
	 * emitted, the object will become idle until `startMeasurement` is called.
	 * @param acSensor. The storage object. All retrieved values will be
	 * stored here. The `portName` property of this object should be set before
	 * calling this contructor.
	 */
	AcSensorUpdater(AcSensor *acSensor, ModbusRtu *modbus, QObject *parent = 0);

	/*!
	 * Returns the storage object.
	 * This is the object passed to the constructor.
	 */
	AcSensor *acSensor();

	/*!
	 * Returns the settings object.
	 * This object is created by the `AcSensorUpdater` when a device has been
	 * found. It will be null before the `deviceFound` signal has been fired.
	 * The information in the settings will be used for data retrieval and can
	 * be changed while retrieval is active. This may lead to reinitialization
	 * of the updater (and the energy meter itself).
	 */
	AcSensorSettings *settings();

	/*!
	 * Starts actual measurement.
	 * This function should be called in while handling the `deviceFound`
	 * signal, or later. It was intended to allow the user of this class to
	 * change the settings before starting measurements.
	 */
	void startMeasurements();

signals:
	/*!
	 * Fired if an energy meter is found.
	 * You can call `startMeasurement` now to start data acquisition.
	 */
	void deviceFound();

	/*!
	 * Fired when all relevant measurements have been retrieved from the
	 * energy meter for this first time.
	 * This may be a good time to publish to data on the D-Bus etc.
	 */
	void deviceInitialized();

	/*!
	 * Fired when communication with the energy meter timed out for several
	 * seconds.
	 * The signals will when no communication was established at all.
	 */
	void connectionLost();

private slots:
	void onErrorReceived(int errorType, quint8 addr, int exception);

	void onReadCompleted(int function, quint8 addr, const QList<quint16> &registers);

	void onWriteCompleted(int function, quint8 addr, quint16 address, quint16 value);

	void onWaitFinished();

	void onUpdateSettings();

	void onIsMultiPhaseChanged();

private:
	void startNextAction();

	void startNextAcquisition();

	void readRegisters(quint16 startReg, quint16 count);

	void writeRegister(quint16 reg, quint16 value);

	void processAcquisitionData(const QList<quint16> &registers);

	void setInitialized();

	double getDouble(const QList<quint16> &registers, int offset, int size,
					 double factor);

	enum State {
		DeviceId,
		VersionCode,
		Serial,
		FirmwareVersion,
		WaitForStart,
		CheckSetup,
		CheckFrontSelector,
		WaitFrontSelector,
		SetApplication,
		SetMeasuringSystem,
		CheckMeasurementMode,
		SetMeasurementMode,
		Acquisition,
		Wait,

		SetAddress
	};

	enum Registers {
		RegDeviceId = 0x000B,
		RegApplication = 0x1101,
		RegMeasurementSystem = 0x1102,
		RegEm112MeasurementMode = 0x1103,
		RegEm24Serial = 0x1300,
		RegEm24VersionCode = 0x0302,
		RegFirmwareVersion = 0x0303,
		RegEm24FrontSelector = 0x0304,
		RegEm112Serial = 0x5000,
	};

	DataProcessor *mDataProcessor;
	AcSensor *mAcSensor;
	AcSensorSettings *mSettings;
	ModbusRtu *mModbus;
	QTimer *mAcquisitionTimer;
	QTimer *mSettingsUpdateTimer;
	int mTimeoutCount;
	bool mInitialized;
	int mMeasuringSystem;
	int mDesiredMeasuringSystem;
	bool mSetupRequested;
	int mApplication;
	QElapsedTimer mStopwatch;
	State mState;
	const CompositeCommand *mCommands;
	int mCommandCount;
	int mCommandIndex;
	int mAcquisitionIndex;
};

#endif // AC_SENSOR_UPDATER_H
