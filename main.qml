import QtQuick 2.15
import QtQuick.Window 2.15

import io.qt.examples.telemetry 1.0

import QtQuick.Extras 1.4

Window {
    id: mainWindow
    width: 393
    height: 680
    visible: true
    title: qsTr("Base Station")


    Connections {
        target: telemetry
        onLatitudeChanged: {
            console.log("latitude: " + telemetry.latitude)
        }
    }

    onWidthChanged: {
        console.log(mainWindow.height + " and " + mainWindow.width);
    }
    onHeightChanged: {
        console.log(mainWindow.height + " and " + mainWindow.width);
    }

    Text {
        id: text1
        x: 10
        y: 10
        text: telemetry.rssi        
        font.pixelSize: 32
    }

    Text {
        id: text2
        x: 10
        y: 60
        text: telemetry.snr
        font.pixelSize: 32
    }


    Gauge {
        x: 80
        y: 10
        width: 300
        height: 40
        minimumValue: -120
        value: telemetry.rssi        
        maximumValue: 0
        orientation: Qt.Horizontal
        tickmarkStepSize: 30
    }

    Gauge {
        x: 80
        y: 60
        width: 300
        height: 40
        minimumValue: 0
        value: telemetry.snr        
        maximumValue: 60
        orientation: Qt.Horizontal
        tickmarkStepSize: 10
    }    

    Rectangle {
        id: rectangle
        width: parent.width
        height: 300
        color: "#cecece"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0

        Loader {
            id: loader
            anchors.fill: parent
            source: "qrc:/map.qml"

        }



    }

}
