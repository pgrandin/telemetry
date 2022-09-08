#include <QDebug>

#include <QObject>
#include <QSerialPort>
#include <QQmlListProperty>

#include "protocol.h"
// #include <qqml.h>

class Lap: public QObject
{
    Q_OBJECT;
    Q_PROPERTY(int lapnumber READ lapnumber)
    Q_PROPERTY(QString laptime READ laptime)

public:

    Lap(int lapnumber, QString laptime);
    int lapnumber() const { return m_lapnumber; }
    QString laptime() const { return m_laptime; }

private:
    int m_lapnumber;
    QString m_laptime;

};

class Telemetry : public QObject
{
    Q_OBJECT;
    Q_PROPERTY(int rssi READ rssi NOTIFY rssiChanged);
    Q_PROPERTY(int snr READ snr NOTIFY snrChanged);
    Q_PROPERTY(float latitude READ latitude NOTIFY latitudeChanged);
    Q_PROPERTY(float longitude READ longitude NOTIFY longitudeChanged);

    Q_PROPERTY(QList<QObject*> laps READ getLaps NOTIFY lapsChanged)

public:
    explicit Telemetry(QObject *parent = 0);
    ~Telemetry();
    int rssi() const;
    int snr() const;
    float latitude();
    float longitude();
//     QQmlListProperty<Lap> laps();
    QList<QObject*> getLaps () const { 
        qDebug() << "getLaps has " << m_laps.count() << " laps";
        return m_laps; 
    }



public slots:
    // void gpsUpdated(const float lat, const float lon, const QString ele, const QString time);

private slots:
    void readyToRead();

private:
    QSerialPort *m_serialport;
    void openSerialPort();
    void writeData(const QByteArray &data);
    void sendString(const QString &str);
    void log_as_gpx(message_t msg);

    bool m_isReady = false;
    QByteArray m_serial_incoming;

    int m_rssi = 0;
    int m_snr = 0;
    float m_latitude;
    float m_longitude;

    QString m_gpx_file_name = "";
    QList<QObject*> m_laps;


signals:
    void rssiChanged();
    void snrChanged();
    void latitudeChanged();
    void longitudeChanged();
    void lapsChanged ();

};

