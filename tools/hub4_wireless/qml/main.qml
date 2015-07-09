import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

ApplicationWindow {
	id: mainWindow
	width: 800
	height: 600
	visible: true

	property variant messages: {
		'CoordinatorNotFound' : 'No Zigbee to USB converter found',
		'CoordinatorOk' : 'Zigbee to USB converter programmed',
		'CoordinatorFailed' : 'Zigbee to USB converter programming failed',
		'RouterNotFound' : 'No Zigbee to RS485 converter found',
		'RouterOk' : 'Zigbee to RS485 converter programmed.<br>Both LEDs should be blinking now.<br>Disconnect the power supply before connecting another zigbee device.',
		'RouterFailed' : 'Zigbee to RS485 converter programming failed',
		'Busy' : 'Working, please wait'
	}

	TabView {
		id: instructions
		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			margins: 10
		}
		height: 500

		Tab {
			title: qsTr("Zigbee to USB")
			ColumnLayout {
				anchors.fill: parent
				anchors.margins: 10
				spacing: 10
				Image {
					Layout.row: 0
					Layout.fillHeight: true
					Layout.fillWidth: true
					source: '../images/ZigbeeToUsb.jpg'
					fillMode: Image.PreserveAspectFit
				}
				Label {
					Layout.row: 1
					Layout.fillWidth: true
					wrapMode: 'WrapAtWordBoundaryOrAnywhere'
					font {
						family: 'arial'
						pointSize: 12
					}
					text:
						'Connect the <i>zigbee to USB converter</i> to the PC.<br>' +
						'If necessary, wait for the windows drivers to be installed (first time on PC only).<br>' +
						'Make sure you have no other zigbee devices powered up.'
				}
				Button {
					Layout.row: 2
					text: 'Program'
					enabled: info !== 'Busy'
					onClicked: model.programCoordinator()
				}
			}
		}

		Tab {
			title: qsTr("Zigbee to RS485")
			ColumnLayout {
				anchors.fill: parent
				anchors.margins: 10
				spacing: 10
				Image {
					Layout.row: 0
					Layout.fillHeight: true
					Layout.fillWidth: true
					source: '../images/ZigbeeToRs485.jpg'
					fillMode: Image.PreserveAspectFit
				}
				Label {
					Layout.row: 1
					Layout.fillWidth: true
					wrapMode: 'WrapAtWordBoundaryOrAnywhere'
					font {
						family: 'arial'
						pointSize: 12
					}
					text:
						'Connect the <i>zigbee to RS485 converter</i> to the FTDI RS485 to USB converter (see image).<br>' +
						'Connect the RS485 to USB converter to the PC.<br>' +
						'If necessary, wait for the windows drivers to be installed (first time on PC only).<br>' +
						'Make sure you have no other zigbee devices powered up.'
				}
				Button {
					Layout.row: 2
					text: 'Program'
					enabled: info !== 'Busy'
					onClicked: model.programRouter()
				}
			}
		}
	}

	Text {
		id: info
		anchors {
			top: instructions.bottom
			left: parent.left
			right: parent.right
			margins: 10
		}
		font {
			family: 'arial'
			pointSize: 12
		}
		wrapMode: 'WrapAtWordBoundaryOrAnywhere'
		text: (model.info !== undefined && model.info !== "") ? messages[model.info] : ""
	}

	Button {
		id: logButton
		text: 'Show log'
		anchors {
			right: parent.right
			top: info.bottom
			margins: 10
		}
		onClicked: logLabel.visible = !logLabel.visible
	}

	TextArea {
		id: logLabel
		anchors {
			left: parent.left
			right: parent.right
			top: logButton.bottom
			bottom: parent.bottom
			margins: 10
		}
		visible: false
		enabled: false
		text: model.log
	}
}
