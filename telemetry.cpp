#include "telemetry.h"

#include <QObject>
#include <QDebug>
#include <QSerialPort>
#include <QDir>
#include <QDateTime>

Telemetry::Telemetry(QObject *parent)
    : QObject(parent){
    m_serialport = new QSerialPort(this);
    openSerialPort();
    qDebug() << "Telemetry::Telemetry()";
}

// Destructor
Telemetry::~Telemetry(){
    qDebug() << "Telemetry::~Telemetry() closing gpx file";

    // Open the log file for appending
    QFile file(m_gpx_file_name);
    if (!file.open(QIODevice::Append)){
        qDebug() << "log_as_gpx()" << "Could not open file for appending";
        return;
    }


    // Add gpx closing tags
    QTextStream out(&file);
    out << "\n</trkseg>\n</trk>\n</gpx>\n";
}

int Telemetry::rssi() const{
    return m_rssi;
}

int Telemetry::snr() const{
    return m_snr;
}

float Telemetry::latitude()
{
    return m_latitude;
}

float Telemetry::longitude()
{
    return m_longitude;
}

void Telemetry::openSerialPort()
{
    m_serialport->setPortName("ttyUSB0");
    m_serialport->setBaudRate(QSerialPort::Baud115200);
    m_serialport->setDataBits(QSerialPort::Data8);
    m_serialport->setParity(QSerialPort::NoParity);
    m_serialport->setStopBits(QSerialPort::OneStop);
    m_serialport->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialport->open(QIODevice::ReadWrite))
    {     
        qDebug() << "Serial port opened";
        connect(this->m_serialport, SIGNAL(readyRead()), this, SLOT(readyToRead()));
    }
    else
    {     
        qDebug() << "Serial port failed to open";
    }
}     

void Telemetry::writeData(const QByteArray &data)
{
    m_isReady = false;
    m_serialport->write(data);
    qDebug() << "Telemetry::writeData()" << data;
}

void Telemetry::readyToRead()
{
    QByteArray rawData = m_serialport->readAll();          // read data from serial port
    // qDebug()<< "chunk " << rawData;
    m_serial_incoming.append(rawData);

    qDebug() << "Telemetry::readyToRead()" << m_serial_incoming;

    // fixme : this causes issues when there is no gps fix
    while (m_serial_incoming.contains('\0'))
    {
        int end = m_serial_incoming.indexOf('\0') + 1;
        QByteArray message = m_serial_incoming;
        qDebug()<< "telemetry response: " << m_serial_incoming;
        message.remove(end, m_serial_incoming.length());
        // qDebug()<< "Processed Message" << message;
        m_serial_incoming.remove(0, end);
        // qDebug()<< "line new" << line;
        // ProcessMessage(message);

        if (message.startsWith("Message: ACK,")){
            m_isReady = true;
        } else {

            // "Received packet '00ff99721e42de71d1c2'\r\n421"
            // extract the packet between "'" from the message
            QByteArray packet = message.mid(message.indexOf("'") + 1, message.lastIndexOf("'") - message.indexOf("'") - 1);

            qDebug() << "packet: " << packet;
            message_t msg;

            // copy the packet to the message struct
            memcpy(&msg, packet.data(), sizeof(msg));
            qDebug() << "msg.rssi: " << msg.rssi;
            qDebug() << "msg.snr: " << msg.snr;
            qDebug("msg.lat %.6f", msg.latitude);
            qDebug("msg.lon %.6f", msg.longitude);

            qDebug() << "rssi from raw data" << packet.mid(0, 1);
            qDebug() << "snr from raw data" << packet.mid(1, 1);

            m_rssi = msg.rssi;
            emit rssiChanged();

            m_snr = msg.snr;
            emit snrChanged();

            if (msg.latitude < -90 || msg.latitude > 90){
                qDebug() << "Invalid latitude";
                return;
            }

            if (msg.longitude < -180 || msg.longitude > 180){
                qDebug() << "Invalid longitude";
                return;
            }

            m_latitude = msg.latitude;
            emit latitudeChanged();
            m_longitude = msg.longitude;
            emit longitudeChanged();

            qDebug() << "Telemetry::readyToRead() coords" << m_latitude << m_longitude;            

            log_as_gpx(msg);
        }
    }
}

void Telemetry::log_as_gpx(message_t msg){
    // Check if we already have a log file name for this session
    if (m_gpx_file_name.isEmpty()){
        m_gpx_file_name = QString("%1/%2.gpx").arg(QDir::currentPath()).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
        qDebug() << "log_as_gpx()" << m_gpx_file_name;

        QFile file(m_gpx_file_name);
        if (!file.open(QIODevice::Append)){
            qDebug() << "log_as_gpx()" << "Could not open file for appending";
            return;
        }

        QTextStream out(&file);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        out << "<gpx creator=\"StravaGPX\" "
            " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
            " xsi:schemaLocation=\"http://www.garmin.com/xmlschemas/GpxExtensions/v3 "
            "    http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd "
            "    http://www.garmin.com/xmlschemas/TrackPointExtension/v1 "
            "    http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd\" "
            " version=\"1.1\" "
            " xmlns=\"http://www.topografix.com/GPX/1/1\" "
            " xmlns:gpxtpx=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\" "
            " xmlns:gpxx=\"http://www.garmin.com/xmlschemas/GpxExtensions/v3\">\n";
        out << "<trk>\n";
        out << "<trkseg>\n";

    }


    // Open the log file for appending
    QFile file(m_gpx_file_name);
    if (!file.open(QIODevice::Append)){
        qDebug() << "log_as_gpx()" << "Could not open file for appending";
        return;
    }

    // Write the gpx point
    QTextStream out(&file);
    out << qSetRealNumberPrecision(16);
    out << "<trkpt lat=\"" << msg.latitude << "\" lon=\"" << msg.longitude << "\">\n";
    out << "<time>" << QDateTime::currentDateTime().toString(Qt::ISODate) << "</time>\n";
    out << "<extensions>\n";
    out << "<gpxtpx:TrackPointExtension>\n";
    out << "<gpxtpx:rssi>" << msg.rssi << "</gpxtpx:rssi>\n";
    out << "<gpxtpx:snr>" << msg.snr << "</gpxtpx:snr>\n";
    out << "</gpxtpx:TrackPointExtension>\n";
    out << "</extensions>\n";
    out << "</trkpt>\n";


}
