#include "openBCI_manager.h"

#include <algorithm>
#include <QMutexLocker>
#include <QSerialPort>
#include <QtGlobal>

namespace {

// OpenBCI Cyton default: 115200 baud, 33-byte packets with 0xA0 ... 0xC0 framing
constexpr int kCytonPacketSize = 33;
constexpr quint8 kStartByte = 0xA0;
constexpr quint8 kStopByte  = 0xC0;

// Cyton raw data is 24-bit signed big-endian.
inline qint32 readInt24BE(const quint8* p)
{
    qint32 v = (qint32(p[0]) << 16) | (qint32(p[1]) << 8) | qint32(p[2]);
    if (v & 0x00800000)
        v |= 0xFF000000;
    return v;
}

inline bool isStopByte(quint8 b)
{
    // Cyton uses 0xC0..0xC7 depending on aux mode; accept the whole range.
    return (b & 0xF0) == 0xC0;
}

constexpr int kDefaultSampleRateHz = 250;
constexpr int kMaxSecondsToKeep = 10 * 60; // 10 minutes
constexpr int kMaxSamplesToKeep = kDefaultSampleRateHz * kMaxSecondsToKeep;

} // namespace

OpenBCIManager& OpenBCIManager::instance()
{
    static OpenBCIManager s;
    return s;
}

OpenBCIManager::OpenBCIManager(QObject* parent)
    : QObject(parent)
{
    eegByChannel_.resize(8);
    setting_.comport = QStringLiteral("COM3");
    setting_.ECG = 0;
    setting_.EEG1 = 0;
    setting_.EEG2 = 1;
    portName_ = setting_.comport;
}

void OpenBCIManager::setPortName(const QString& portName)
{
    QMutexLocker lock(&mutex_);
    portName_ = portName;
}

QString OpenBCIManager::portName() const
{
    QMutexLocker lock(&mutex_);
    return portName_;
}

void OpenBCIManager::start()
{
    QString portToOpen;
    {
        QMutexLocker lock(&mutex_);
        if (running_)
            return;
        running_ = true;
        rxBuffer_.clear();
        portToOpen = portName_;
    }

    emit runningChanged(true);
    emit statusMessage(QString("OpenBCI: start (port %1)").arg(portToOpen));

    if (!serial_) {
        serial_ = new QSerialPort(this);
        connect(serial_, &QSerialPort::readyRead, this, &OpenBCIManager::onReadyRead);
    }

    if (serial_->isOpen())
        serial_->close();

    serial_->setPortName(portToOpen);
    serial_->setBaudRate(115200);
    serial_->setDataBits(QSerialPort::Data8);
    serial_->setParity(QSerialPort::NoParity);
    serial_->setStopBits(QSerialPort::OneStop);
    serial_->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial_->open(QIODevice::ReadWrite)) {
        emit statusMessage(QString("OpenBCI: failed to open %1: %2").arg(portToOpen, serial_->errorString()));
        {
            QMutexLocker lock(&mutex_);
            running_ = false;
        }
        emit runningChanged(false);
        return;
    }

    {
        QMutexLocker lock(&mutex_);
        ecg_.clear();
        for (auto& ch : eegByChannel_)
            ch.clear();
        if (eegByChannel_.size() < 8)
            eegByChannel_.resize(8);
    }

    // Start streaming (Cyton): 'b' is commonly used to start binary stream; 's' to stop.
    serial_->write("b");
    serial_->flush();
}

void OpenBCIManager::stop()
{
    {
        QMutexLocker lock(&mutex_);
        if (!running_)
            return;
        running_ = false;
        rxBuffer_.clear();
    }

    if (serial_ && serial_->isOpen()) {
        serial_->write("s");
        serial_->flush();
        serial_->close();
    }
    emit runningChanged(false);
    emit statusMessage("OpenBCI: stop");
}

bool OpenBCIManager::isRunning() const
{
    QMutexLocker lock(&mutex_);
    return running_;
}

QVector<double> OpenBCIManager::getLatestEcgWindow(int sampleCount) const
{
    QMutexLocker lock(&mutex_);
    // ЭКГ берём из канала, выбранного в настройках (setting_.ECG).
    // Так окно всегда соответствует актуальным настройкам и не зависит от отдельного буфера ecg_.
    const int ecgCh = qBound(0, static_cast<int>(setting_.ECG), 7);
    if (ecgCh >= eegByChannel_.size())
        return {};
    return tailWindow(eegByChannel_[ecgCh], sampleCount);
}

QVector<double> OpenBCIManager::getLatestEegWindow(int sampleCount, int channel) const
{
    QMutexLocker lock(&mutex_);
    int ch = channel;
    if (ch < 0)
        ch = qBound(0, static_cast<int>(setting_.EEG1), 7);
    else
        ch = qBound(0, ch, qMax(0, eegByChannel_.size() - 1));
    return tailWindow(eegByChannel_[ch], sampleCount);
}

QVector<double> OpenBCIManager::getLatestChannelSamples(int channel, int sampleCount) const
{
    return getLatestEegWindow(sampleCount, channel);
}

bool OpenBCIManager::hasAtLeastSamples(int sampleCount, int eegChannel) const
{
    if (sampleCount <= 0)
        return true;
    QMutexLocker lock(&mutex_);
    const int ecgCh = qBound(0, static_cast<int>(setting_.ECG), 7);
    if (ecgCh >= eegByChannel_.size() || eegByChannel_[ecgCh].size() < sampleCount)
        return false;
    int ch = eegChannel;
    if (ch < 0)
        ch = qBound(0, static_cast<int>(setting_.EEG1), 7);
    else
        ch = qBound(0, ch, qMax(0, eegByChannel_.size() - 1));
    if (ch >= eegByChannel_.size())
        return false;
    return eegByChannel_[ch].size() >= sampleCount;
}

void OpenBCIManager::pushEcgSample(double v)
{
    QMutexLocker lock(&mutex_);
    ecg_.push_back(v);
    trimBuffersIfNeeded();
}

void OpenBCIManager::pushEegSample(int channel, double v)
{
    QMutexLocker lock(&mutex_);
    if (channel < 0)
        return;
    if (channel >= eegByChannel_.size())
        eegByChannel_.resize(channel + 1);
    eegByChannel_[channel].push_back(v);
    trimBuffersIfNeeded();
}

QVector<double> OpenBCIManager::tailWindow(const QVector<double>& src, int sampleCount)
{
    if (sampleCount <= 0 || src.isEmpty())
        return {};
    if (sampleCount >= src.size())
        return src;
    return src.mid(src.size() - sampleCount, sampleCount);
}

void OpenBCIManager::onReadyRead()
{
    QByteArray bytes;
    {
        QMutexLocker lock(&mutex_);
        if (!serial_ || !serial_->isOpen())
            return;
        bytes = serial_->readAll();
    }
    if (!bytes.isEmpty())
        handleRxBytes(bytes);
}

void OpenBCIManager::handleRxBytes(const QByteArray& bytes)
{
    QMutexLocker lock(&mutex_);
    if (!running_)
        return;
    rxBuffer_.append(bytes);
    parseCytonPackets();
}

void OpenBCIManager::parseCytonPackets()
{
    // Find packets framed as:
    // [0] 0xA0
    // [1] sampleId
    // [2..25] 8 EEG channels x 3 bytes
    // [26..31] aux (6 bytes)
    // [32] 0xC0..0xC7
    while (rxBuffer_.size() >= kCytonPacketSize) {
        int start = rxBuffer_.indexOf(char(kStartByte));
        if (start < 0) {
            rxBuffer_.clear();
            return;
        }

        if (start > 0)
            rxBuffer_.remove(0, start);

        if (rxBuffer_.size() < kCytonPacketSize)
            return;

        const quint8* p = reinterpret_cast<const quint8*>(rxBuffer_.constData());
        if (p[0] != kStartByte) {
            rxBuffer_.remove(0, 1);
            continue;
        }

        const quint8 stop = p[kCytonPacketSize - 1];
        if (!isStopByte(stop)) {
            // Not a valid packet boundary; shift by one and resync.
            rxBuffer_.remove(0, 1);
            continue;
        }

        // Extract EEG channels.
        // Conversion: keep raw integer as double for now (games/analysis can scale later).
        for (int ch = 0; ch < 8; ++ch) {
            const int off = 2 + ch * 3;
            const qint32 raw = readInt24BE(p + off);
            eegByChannel_[ch].push_back(double(raw));
        }

        trimBuffersIfNeeded();

        rxBuffer_.remove(0, kCytonPacketSize);
    }
}

void OpenBCIManager::trimBuffersIfNeeded()
{
    // Keep memory bounded.
    // ecg_ может заполняться через pushEcgSample() из другого транспорта.
    if (ecg_.size() > kMaxSamplesToKeep)
        ecg_.remove(0, ecg_.size() - kMaxSamplesToKeep);

    for (auto& ch : eegByChannel_) {
        if (ch.size() > kMaxSamplesToKeep)
            ch.remove(0, ch.size() - kMaxSamplesToKeep);
    }
}

openBCISetting OpenBCIManager::getSetting() const
{
    QMutexLocker lock(&mutex_);
    return setting_;
}

void OpenBCIManager::setSetting(openBCISetting setting)
{
    bool wasRunning = false;
    {
        QMutexLocker lock(&mutex_);
        wasRunning = running_;
        if (running_ && serial_ && serial_->isOpen()) {
            serial_->write("s");
            serial_->flush();
            serial_->close();
        }
        running_ = false;
        setting_ = setting;
        if (!setting.comport.isEmpty())
            portName_ = setting.comport;
        rxBuffer_.clear();
    }
    if (wasRunning)
        emit runningChanged(false);
}
