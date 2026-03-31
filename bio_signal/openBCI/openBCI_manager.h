#ifndef OPENBCI_MANAGER_H
#define OPENBCI_MANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>
#include <QByteArray>

class QSerialPort;

struct openBCISetting{
    QString comport;
    uint ECG;
    uint EEG1;
    uint EEG2;
};

class OpenBCIManager final : public QObject
{
    Q_OBJECT
public:
    static OpenBCIManager& instance();

    OpenBCIManager(const OpenBCIManager&) = delete;
    OpenBCIManager& operator=(const OpenBCIManager&) = delete;

    void setPortName(const QString& portName);
    QString portName() const;

    void start();
    void stop();

    bool isRunning() const;

    // Temporary API until real OpenBCI integration is added.
    // Games can request a sliding window for analysis/logging.
    QVector<double> getLatestEcgWindow(int sampleCount) const;
    // channel < 0: использовать канал ЭЭГ1 из настроек (openBCISetting::EEG1)
    QVector<double> getLatestEegWindow(int sampleCount, int channel = -1) const;

    // Feed samples from transport layer (serial/LSL/UDP).
    void pushEcgSample(double v);
    void pushEegSample(int channel, double v);

    openBCISetting getSetting() const;
    void setSetting(openBCISetting setting);

signals:
    void runningChanged(bool running);
    void statusMessage(const QString& msg);

private slots:
    void onReadyRead();

private:
    explicit OpenBCIManager(QObject* parent = nullptr);

    static QVector<double> tailWindow(const QVector<double>& src, int sampleCount);
    void handleRxBytes(const QByteArray& bytes);
    void parseCytonPackets();
    void trimBuffersIfNeeded();

    mutable QMutex mutex_;
    bool running_ = false;

    QString portName_ = "COM3";
    QByteArray rxBuffer_;

    QSerialPort* serial_ = nullptr;

    QVector<double> ecg_;
    QVector<QVector<double>> eegByChannel_;
    openBCISetting setting_;
};

#endif // OPENBCI_MANAGER_H
