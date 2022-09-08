#include "telemetry.h"

#include <QObject>
#include <QDebug>
#include <QSerialPort>
#include <QDir>
#include <QDateTime>

Lap::Lap(int lapnumber, QString laptime)
    : m_lapnumber(lapnumber)
    , m_laptime(laptime)
{
}

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

/*
QQmlListProperty<Lap> Telemetry::messages()
{
    return QQmlListProperty<Lap>(this, 0, &Telemetry::append_message);
}

void Telemetry::append_message(QQmlListProperty<Lap> *list, Lap *msg)
{
    Telemetry *msgBoard = qobject_cast<Telemetry *>(list->object);
    if (msg)
        msgBoard->m_messages.append(msg);
}

*/

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

void Telemetry::sendString(const QString &str)
{
    QByteArray data = str.toUtf8();
    writeData(data);
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
    while (m_serial_incoming.contains('\n'))
    {
        int end = m_serial_incoming.indexOf('\n') + 1;
        QString message = m_serial_incoming;
        qDebug()<< "telemetry response: " << m_serial_incoming;
        message.remove(end, m_serial_incoming.length());

        m_serial_incoming.remove(0, end);
        qDebug()<< "Processed Message" << message;

        if (message == "ACK\n") {
            m_isReady = true;
            // pushFromQueue();
        }

        if (message.startsWith("RADIO")) {
            QStringList parts = message.split(',');
            m_rssi = parts[1].toFloat();
            m_snr = parts[2].toFloat();
            emit rssiChanged();
            emit snrChanged();
        }

        if (message.startsWith("GPS")) {
            QStringList parts = message.split(',');
            m_latitude = parts[1].toFloat();
            m_longitude = parts[2].toFloat();
            emit latitudeChanged();
            emit longitudeChanged();

            sendString("ACK\n");

            // log_as_gpx(msg);
        }

        if (message.startsWith("WATERTEMP")) {
            QStringList parts = message.split(',');
            m_water_temp = parts[1].toFloat();
            emit waterTempChanged();
            sendString("ACK\n");
        }

        if (message.startsWith("FUELLEVEL")) {
            QStringList parts = message.split(',');
            m_fuel_level = parts[1].toFloat();
            emit fuelLevelChanged();
            sendString("ACK\n");
        }


        if (message.startsWith("LAP")) {
            QStringList parts = message.split(',');
            int lap = parts[1].toInt();
            int64_t time = parts[2].toInt();

            // convert the time into a mm:ss:ms format
            int64_t ms = time % 1000;
            int64_t s = (time / 1000) % 60;
            int64_t m = (time / 1000) / 60;
            QString laptime = QString("%1:%2.%3").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')).arg(ms, 3, 10, QChar('0'));

            m_laps.append(new Lap(lap, laptime));
            emit lapsChanged();


            sendString("ACK," + message);
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
