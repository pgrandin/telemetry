import QtQuick 2.12
import QtQuick.Window 2.12

// import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

// import io.qt.examples.telemetry 1.0

import QtQuick.Extras 1.4



import QtLocation 5.9
import QtPositioning 5.9


Window {
    id: mainWindow
     visibility: "FullScreen"
    width: 1280
    height: 800
    visible: true
    title: qsTr("Base Station")
    
    
    Connections {
        target: telemetry
        onLatitudeChanged: {
            console.log("latitude: " + telemetry.latitude)
        }
        onSnrChanged: {
            snrColorAnimation.restart()
        }
        onRssiChanged: {
            rssiColorAnimation.restart()
        }

    }
    
    onWidthChanged: {
        console.log(mainWindow.height + " and " + mainWindow.width);
    }
    onHeightChanged: {
        console.log(mainWindow.height + " and " + mainWindow.width);
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

            Map {
                id: mapOverlay
                anchors.fill: map
                plugin: Plugin { name: "itemsoverlay" }
                gesture.enabled: false
                center: map.center
                color: 'transparent' // Necessary to make this map transparent
                zoomLevel: map.zoomLevel
                tilt: map.tilt;

                MapPolyline
                {
                    id: trackLine
                    line.width: 10
                    line.color: 'white'
                }

                MapPolyline {
                    id:startline
                    line.width: 6
                    line.color: 'green'
                }
                MapPolyline {
                    id:finishline2
                    line.width: 6
                    line.color: 'red'
                }
                Component.onCompleted:
                {
                    var lines = []
                    for(var i=0;i<geopath.size();i++)
                    {
                        lines[i] = geopath.coordinateAt(i)
                    }
                    trackLine.path = lines
                }
            // Draw a small red circle for current Vehicle Location
            MapQuickItem {
                id: marker
                anchorPoint.x: 10
                anchorPoint.y: 10
                width: 15
                coordinate: QtPositioning.coordinate(Dashboard.gpsLatitude,Dashboard.gpsLongitude)
                sourceItem: Rectangle {
                    id: image
                    width:20
                    height: width
                    radius: width*0.5
                    color: "red"
                }
            }
        }    
    

    Rectangle{
        id :laptimes
        x: 0
        y: 0

        anchors.top: grid1.bottom
        anchors.topMargin: 5
        anchors.left: map.right
        anchors.leftMargin: 5
        anchors.bottom : buttongrid.top
        anchors.bottomMargin: 5
        width: 347
        height: 489
        color: "grey"
        ListModel {
            id: laptimeModel
        }

        Component {
            id: contactDelegate

            Item {
                id:leftcolum
                width: 400; height: 40
                Row {
                Column {
                    Text {
                            text: "LAP " + model.modelData.lapnumber
                            width: 110
                            color: "white"
                            font.pixelSize:  15
                            font.bold: true}
                }
                Column {
                    Text { text: model.modelData.laptime
                        anchors.right: parent.right
                        width: 195
                        color: "white"
                        font.pixelSize:  30
                        font.bold: true}
                }
            }
            }
        }

        ListView {
            id: laptimelistview
            width: 400
            height: 200
            anchors.fill: parent
            model: telemetry.laps
            delegate: contactDelegate
            highlight: Rectangle { color: "#505050"; radius: 5}
            focus: false
            clip: true

            addDisplaced: Transition {
                NumberAnimation { properties: "x,y"; duration: 1000 }
            }
        }
    }

    Rectangle {
        id: radio_info
        width: 400
        height: 100
        color: "#a9a1a1"
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 13
        
        Gauge {
            id: snr_gauge
            x: 0
            width: 300
            height: 40
            anchors.top: rssi_gauge.bottom
            anchors.topMargin: 10
            value: telemetry.snr
            maximumValue: 60
            orientation: Qt.Horizontal
            tickmarkStepSize: 10
            Behavior on value {
                NumberAnimation {
                    duration: 1000
                }
            }

        }
        
        Gauge {
            id: rssi_gauge
            x: 0
            width: 300
            height: 40
            anchors.top: parent.top
            anchors.topMargin: 0
            minimumValue: -120
            value: telemetry.rssi
            maximumValue: 0
            orientation: Qt.Horizontal
            tickmarkStepSize: 30

            Behavior on value {
                NumberAnimation {
                    duration: 1000
                }
            }


        }
        
        Text {
            id: rssi_value
            x: rssi_gauge.width + 8
            anchors.top: rssi_gauge.top
            text: telemetry.rssi        
            font.pixelSize: 32
        }
        
        Text {
            id: snr_value
            x: snr_gauge.width + 8
            anchors.top: snr_gauge.top
            color: "#1c8630"
            text: telemetry.snr
            font.pixelSize: 32
        }

        ColorAnimation {
            id: snrColorAnimation
            target: snr_value
            property: "color"
            duration: 5000
            to: "#ed0e0e"
            from: "#1e09da"
        }

        ColorAnimation {
            id: rssiColorAnimation
            target: rssi_value
            property: "color"
            duration: 5000
            to: "#ed0e0e"
            from: "#1e09da"
        }

    }

    Rectangle {
        id: fuel_area
        anchors.right: parent.right
        y: 156
        width: 464
        height: 109
        color: "#ffffff"

        Text {
            id: fuel
            x: 30
            y: 27
            text: qsTr("Fuel")
            font.pixelSize: 24
            minimumPixelSize: 24
        }

        Gauge {
            id: fuel_gauge
            x: 142
            y: -6
            width: 300
            height: 40
            anchors.top: fuel_area.top
            value: telemetry.fuel_level
            tickmarkStepSize: 10
            maximumValue: 100
            orientation: Qt.Horizontal
            anchors.topMargin: 10
        }

        Text {
            id: text1
            x: 30
            y: 61
            text: qsTr("Estimated gas remaining :")
            font.pixelSize: 18
        }

        Text {
            id: text2
            x: 250
            y: 61
            text: ( telemetry.fuel_level / 100 * 17.2).toFixed(1) + " gal"
            font.pixelSize: 18
        }

        Text {
            id: text3
            x: 30
            y: 82
            text: qsTr("Estimated gas to fill up :")
            font.pixelSize: 18
        }

        Text {
            id: text4
            x: 250
            y: 82
            text: ( 17.2 - ( telemetry.fuel_level / 100 * 17.2)).toFixed(1) + " gal"
            font.pixelSize: 18
        }

    }

    Rectangle {
        id: watertemp_area
        x: 816
        anchors.right: parent.right
        y: 271
        width: 464
        height: 121
        color: "#ffffff"
        Text {
            id: watertemp_C
            x: 0
            y: 22
            text: qsTr("Water Temp (C)")
            font.pixelSize: 24
            minimumPixelSize: 24
        }

        Gauge {
            id: watertemp_gauge_C
            x: 180
            y: 1
            width: 250
            height: 40
            anchors.top: watertemp_area.top
            minimumValue: 70
            value: telemetry.water_temp
            tickmarkStepSize: 10
            maximumValue: 120
            orientation: Qt.Horizontal
            anchors.topMargin: 10
        }

        Text {
            id: watertemp_F
            x: 0
            y: 60
            text: qsTr("Water Temp (F)")
            font.pixelSize: 24
            minimumPixelSize: 24
        }

        Gauge {
            id: watertemp_gauge_F
            x: 180
            y: 60
            width: 250
            height: 40
            minimumValue: 180
            value: (telemetry.water_temp * 9/5 + 32)
            tickmarkStepSize: 10
            maximumValue: 240
            orientation: Qt.Horizontal
            anchors.topMargin: 45
        }
    }
    
}
