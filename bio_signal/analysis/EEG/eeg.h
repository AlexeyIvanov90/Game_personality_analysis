#ifndef EEG_H
#define EEG_H

#include <QVector>
#include <complex>
#include <QDebug>

struct resultEEG{
    double powerAlphaRhythm=0.;
    double powerBetaRhythm=0.;
    double  alphaRhythms_percent=0.;
    double  betaRhythms_percent=0.;
    double ratio=0.;
    void print(){
        qDebug() << "powerAlphaRhythm: " << powerAlphaRhythm
                 << "\npowerBetaRhythm: " << powerBetaRhythm
                 << "\nalphaRhythms_percent: " << alphaRhythms_percent
                 << "\nbetaRhythms_percent: " << betaRhythms_percent
                 << "\nratio: " << ratio;
    }
};

class EEG
{
public:
    EEG();
    ~EEG();
    void setDataFromSensor(const QVector<double>& data);
    resultEEG calculate();
    void computeFFT(QVector<std::complex<double>> &a);

    void clear();
private:
    QVector<double> data_;
};

#endif // EEG_H
