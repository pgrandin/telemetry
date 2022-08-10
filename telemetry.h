#include <QObject>
#include <QSerialPort>


#include "protocol.h"
// #include <qqml.h>


class Telemetry : public QObject
{
    Q_OBJECT;
    Q_PROPERTY(int rssi READ rssi NOTIFY rssiChanged);
    Q_PROPERTY(int snr READ snr NOTIFY snrChanged);
    Q_PROPERTY(float latitude READ latitude NOTIFY latitudeChanged);
    Q_PROPERTY(float longitude READ longitude NOTIFY longitudeChanged);

   // QML_ELEMENT

public:
    explicit Telemetry(QObject *parent = 0);
    ~Telemetry();
    int rssi() const;
    int snr() const;
    float latitude();
    float longitude();


public slots:
    // void gpsUpdated(const float lat, const float lon, const QString ele, const QString time);

private slots:
    void readyToRead();

private:
    QSerialPort *m_serialport;
    void openSerialPort();
    void writeData(const QByteArray &data);
    void log_as_gpx(message_t msg);

    bool m_isReady = false;
    QByteArray m_serial_incoming;

    int m_rssi = 0;
    int m_snr = 0;
    float m_latitude;
    float m_longitude;

    QString m_gpx_file_name = "";

signals:
    void rssiChanged();
    void snrChanged();
    void latitudeChanged();
    void longitudeChanged();
};

