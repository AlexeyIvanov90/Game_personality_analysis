#include "eeg.h"

EEG::EEG(){
}

EEG::~EEG(){
}

void EEG::setData(const QVector<double>& data){
    data_=data;
}

void EEG::clear(){
    data_.clear();
}


resultEEG EEG::calculate(){
    resultEEG result;

    if (data_.isEmpty())
        return result;

    const double Fs = 250.0; // OpenBCI
    int N = data_.size();

    // =========================
    // 1. Удаление среднего
    // =========================
    double mean = std::accumulate(data_.begin(), data_.end(), 0.0) / N;

    QVector<double> signal = data_;

    for (double &v : signal)
        v -= mean;

    // =========================
    // 2. Дополнение до степени 2
    // =========================
    int pow2 = 1;
    while (pow2 < N) pow2 <<= 1;

    int oldSize = signal.size();
    signal.resize(pow2);
    std::fill(signal.begin() + oldSize, signal.end(), 0.0);

    N = pow2;

    // =========================
    // 3. Окно Ханна
    // =========================
    for (int i = 0; i < N; ++i) {
        double w = 0.5 * (1 - cos(2 * M_PI * i / (N - 1)));
        signal[i] *= w;
    }

    // =========================
    // 4. FFT
    // =========================
    QVector<std::complex<double>> fft(N);
    for (int i = 0; i < N; ++i)
        fft[i] = std::complex<double>(signal[i], 0.0);

    computeFFT(fft);

    // =========================
    // 5. Power Spectrum
    // =========================
    QVector<double> power(N);

    for (int i = 0; i < N; ++i) {
        double mag = std::abs(fft[i]);
        power[i] = (mag * mag) / (N * N);
    }

    int half = N / 2;
    double df = Fs / N;

    double alphaPower = 0.0;
    double betaPower = 0.0;
    double totalPower = 0.0;

    // =========================
    // 6. Интеграция по частотам
    // =========================
    for (int i = 1; i <= half; ++i) {
        double freq = i * df;

        double p = (i == half) ? power[i] : 2.0 * power[i]; // односторонний спектр

        totalPower += p;

        if (freq >= 8.0 && freq <= 13.0)
            alphaPower += p;
        else if (freq > 13.0 && freq <= 30.0)
            betaPower += p;
    }

    // =========================
    // 7. Результаты
    // =========================
    result.powerAlphaRhythm = alphaPower;
    result.powerBetaRhythm  = betaPower;

    if (totalPower > 0.0) {
        result.alphaRhythms_percent = alphaPower / totalPower * 100.0;
        result.betaRhythms_percent  = betaPower / totalPower * 100.0;
    }

    if (betaPower > 1e-12)
        result.ratio = alphaPower / betaPower;

    return result;
}

void EEG::computeFFT(QVector<std::complex<double>> &a)
{
    int n = a.size();
    if (n <= 1) return;

    // Разделяем на четные и нечетные элементы
    QVector<std::complex<double>> even(n/2);
    QVector<std::complex<double>> odd(n/2);

    for (int i = 0; i < n/2; ++i) {
        even[i] = a[i*2];
        odd[i] = a[i*2 + 1];
    }

    // Рекурсивно вычисляем БПФ для половин
    computeFFT(even);
    computeFFT(odd);

    // Объединяем результаты
    for (int i = 0; i < n/2; ++i) {
        std::complex<double> t = std::polar(1.0, -2.0 * M_PI * i / n) * odd[i];
        a[i] = even[i] + t;
        a[i + n/2] = even[i] - t;
    }
}




