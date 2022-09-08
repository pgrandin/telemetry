import QtQuick 2.0
import QtPositioning 5.15
import QtLocation 5.15

import io.qt.examples.telemetry 1.0

Item {

    Connections {
        target: telemetry
        onLatitudeChanged: {
            gpsColorAnimation.restart()
        }
    }

    Plugin {
        id: osm
        name: "osm" // "mapboxgl", "esri", ...
        // specify plugin parameters if necessary
        // PluginParameter {
        //     name:
        //     value:
        // }
    }

    Plugin {
        id: mapboxgl
        name: "mapboxgl" 
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapboxgl
        center: QtPositioning.coordinate(telemetry.latitude, telemetry.longitude) // Oslo
        activeMapType: map.supportedMapTypes[1]
        zoomLevel: 14

        // Draw a small red circle for current Vehicle Location
        MapQuickItem {
            id: marker
            anchorPoint.x: 10
            anchorPoint.y: 10
            width: 15
            coordinate: QtPositioning.coordinate(telemetry.latitude,
                                                    telemetry.longitude)
            sourceItem: Rectangle {
                id: gps_position
                width: 20
                height: width
                radius: width * 0.5
                color: "red"
            }
        }        
    }

    ColorAnimation {
        id: gpsColorAnimation
        target: gps_position
        property: "color"
        duration: 5000
        to: "#ed0e0e"
        from: "#1e09da"
    }    

}
