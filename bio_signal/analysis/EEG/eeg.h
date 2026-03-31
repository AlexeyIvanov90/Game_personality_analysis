#ifndef EEG_H
#define EEG_H

#include <QVector>
#include <complex>

struct resultEEG{
    double powerAlphaRhythm=0.;
    double powerBetaRhythm=0.;
    double  alphaRhythms_percent=0.;
    double  betaRhythms_percent=0.;
    double ratio=0.;
};

class EEG
{
public:
    EEG();
    ~EEG();
    void setData(const QVector<double>& data);
    resultEEG calculate();
    void computeFFT(QVector<std::complex<double>> &a);

    void clear();
private:
    QVector<double> data_;
};

#endif // EEG_H
