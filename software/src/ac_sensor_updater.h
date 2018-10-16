#ifndef AC_SENSOR_UPDATER_H
#define AC_SENSOR_UPDATER_H

#include <QElapsedTimer>
#include <QObject>
#include "defines.h"
#include "modbus_rtu.h"

class DataProcessor;
class AcSensor;
class AcSensorSettings;
class AcSensorPhase;
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
	 * If the setup succeeds, the `connectionState` of the `acSensor` will be
	 * set to `Detected`, otherwise to `disconnected` signal.
	 * Once the state is `Detected`, the object will become idle until
	 * `startMeasurement` is called.
	 * @param acSensor. The storage object. All retrieved values will be
	 * stored here. The `portName` property of this object should be set before
	 * calling this contructor.
	 * @param modbus. The modbus connection object. This object may be shared
	 * between multiple `AcSensorUpdater` objects. The `modbus` object will not
	 * be deleted in the destructor.
	 */
	AcSensorUpdater(AcSensor *acSensor, AcSensor *acPvSensor, ModbusRtu *modbus, bool isZigbee,
					QObject *parent = 0);

	/*!
	 * Returns the storage object.
	 * This is the object passed to the constructor.
	 */
	AcSensor *acSensor();

	AcSensor *pvSensor();

	/*!
	 * Returns the settings object.
	 * This object is created by the `AcSensorUpdater` when a device has been
	 * detected. It will be null while the `connectionState` of the `AcSensor`
	 * is `Disconnected` or `Searched`.
	 * The information in the settings will be used for data retrieval and can
	 * be changed while retrieval is active. This may lead to reinitialization
	 * of the updater (and the energy meter itself).
	 */
	AcSensorSettings *settings();

	/*!
	 * Starts actual measurement.
	 * This function should be called if the `connectionState` is `Detected`
	 * or later. It was intended to allow the user of this class to change the
	 * settings before starting measurements.
	 */
	void startMeasurements();

private slots:
	void onErrorReceived(int errorType, quint8 addr, int exception);

	void onReadCompleted(int function, quint8 addr, const QList<quint16> &registers);

	void onWriteCompleted(int function, quint8 addr, quint16 address, quint16 value);

	void onWaitFinished();

	void onUpdateSettings();

	void onIsMultiPhaseChanged();

	void onL2ServiceTypeChanged();

private:
	void startNextAction();

	void startNextAcquisition();

	void disconnectSensor();

	void readRegisters(quint16 startReg, quint16 count);

	void writeRegister(quint16 reg, quint16 value);

	void processAcquisitionData(const QList<quint16> &registers);

	double getDouble(const QList<quint16> &registers, int offset, double factor);

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
		CheckMeasurementSystem,
		SetMeasuringSystem,
		CheckMeasurementMode,
		SetMeasurementMode,
		Acquisition,
		Wait,
		WaitOnConnectionLost,

		SetAddress
	};

	enum Registers {
		RegDeviceId = 0x000B,
		RegEm340MeasurementSystem = 0x1002,
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
	DataProcessor *mPvDataProcessor;
	AcSensor *mAcSensor;
	AcSensor *mAcPvSensor;
	AcSensorSettings *mSettings;
	ModbusRtu *mModbus;
	QTimer *mAcquisitionTimer;
	QTimer *mSettingsUpdateTimer;
	int mTimeoutCount;
	int mMeasuringSystem;
	int mDesiredMeasuringSystem;
	bool mIsZigbee;
	bool mSetupRequested;
	int mApplication;
	QElapsedTimer mStopwatch;
	State mState;
	const CompositeCommand *mCommands;
	int mCommandCount;
	int mCommandIndex;
	int mAcquisitionIndex;
	/// Some grid meters return a negative current on backfeed, others don't.
	/// In case the meter does not, we correct the sing of the current using the
	/// sign of the power.
	bool mSetCurrentSign;
};

#endif // AC_SENSOR_UPDATER_H
