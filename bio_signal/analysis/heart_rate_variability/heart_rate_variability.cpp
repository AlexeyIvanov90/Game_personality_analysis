#include <cmath>
#include  <numeric>

#include "heart_rate_variability.h"

#define BIN_WIDTH_MS 50.0
#define HIST_MIN 400.0
#define HIST_MAX 1300.0


HeartRateVariability::HeartRateVariability(){}

HeartRateVariability::~HeartRateVariability(){}

void HeartRateVariability::setData(const QVector<double>& data){
    data_.clear();
    data_.append(data);
}

void HeartRateVariability::calculateStatistic(){   
    int n = data_.size();
    if (n < 2)
        return;

    double sum = std::accumulate(data_.begin(), data_.end(), 0.0);

    result_.HR = 60.0 * 1000.0 * (double(n) / sum);
    result_.M  = sum / n;

    // SDNN
    double tmpSDNN = 0.0;
    for (double x : data_)
        tmpSDNN += (x - result_.M) * (x - result_.M);

    result_.SDNN = std::sqrt(tmpSDNN / (n - 1));
    result_.CV   = result_.SDNN / result_.M * 100.0;

    // RMSSD
    double tmpRMSSD = 0.0;
    for (int i = 0; i < n - 1; ++i)
    {
        double diff = data_[i + 1] - data_[i];
        tmpRMSSD += diff * diff;
    }
    result_.RMSSD = std::sqrt(tmpRMSSD / (n - 1));

    // Min / Max
    auto minmax = std::minmax_element(data_.begin(), data_.end());
    result_.Mn = *minmax.first;
    result_.Mx = *minmax.second;
    result_.MxdMn = result_.Mx - result_.Mn;

    // ===== ГИСТОГРАММА ПО МЕТОДИЧКЕ =====
    int bins = int((HIST_MAX - HIST_MIN) / BIN_WIDTH_MS);
    QVector<int> hist(bins, 0);

    for (double x : data_)
    {
        if (x < HIST_MIN || x >= HIST_MAX)
            continue;

        int idx = int((x - HIST_MIN) / BIN_WIDTH_MS);
        if (idx >= 0 && idx < bins)
            hist[idx]++;
    }

    // Поиск модального интервала
    int maxCount = 0;
    int maxIdx = 0;

    for (int i = 0; i < bins; ++i)
    {
        if (hist[i] > maxCount)
        {
            maxCount = hist[i];
            maxIdx = i;
        }
    }

    // Mo — центр интервала
    result_.Mo = HIST_MIN + (maxIdx + 0.5) * BIN_WIDTH_MS;

    // AMo — в процентах
    result_.AMo = (double(maxCount) / n) * 100.0;

    // SI (Mo и MxDMn в секундах!)
    if (result_.Mo > 0 && result_.MxdMn > 0)
    {
        double Mo_s = result_.Mo / 1000.0;
        double MxDMn_s = result_.MxdMn / 1000.0;

        result_.SI = result_.AMo / (2.0 * Mo_s * MxDMn_s);
    }
    else
    {
        result_.SI = 0.0;
    }
}

void HeartRateVariability::setAutocorrelationMaxLag(int maxLag){
    maxLagAutocorrelation_=std::min(128, data_.size() - 1);
}

QVector<double> HeartRateVariability::getVectorAutocorrelation(){
    calculateAutocorrelation();
    return autocorrelation_;
}

void HeartRateVariability::calculateAutocorrelation()
{
    int n = data_.size();
    if (n < 3)
        return;

    autocorrelation_ = QVector<double>(maxLagAutocorrelation_ + 1, 0.0);

    for (int k = 0; k <= maxLagAutocorrelation_; ++k)
    {
        int m = n - k;

        double sum_xy = 0.0;
        double sum_x  = 0.0;
        double sum_y  = 0.0;
        double sum_x2 = 0.0;
        double sum_y2 = 0.0;

        for (int i = 0; i < m; ++i)
        {
            double x = data_[i];
            double y = data_[i + k];

            sum_xy += x * y;
            sum_x  += x;
            sum_y  += y;
            sum_x2 += x * x;
            sum_y2 += y * y;
        }

        double numerator = m * sum_xy - sum_x * sum_y;
        double denominator =
            std::sqrt((m * sum_x2 - sum_x * sum_x) *
                      (m * sum_y2 - sum_y * sum_y));

        if (denominator > 1e-12)
            autocorrelation_[k] = numerator / denominator;
        else
            autocorrelation_[k] = 0.0;
    }

    // ===== CC1 =====
    result_.CC1 = (autocorrelation_.size() > 1)
                      ? autocorrelation_[1]
                      : 0.0;

    // ===== CC0 =====
    int CC0_lag = maxLagAutocorrelation_;

    for (int i = 1; i <= maxLagAutocorrelation_; ++i)
    {
        if (autocorrelation_[i] < 0.0)
        {
            CC0_lag = i;
            break;
        }
    }

    result_.CC0 = CC0_lag;
}


//Спектральный анализ

void HeartRateVariability::setSpectralAnalysisLag_(double lag){
    spectralAnalysisLag_=lag;

}


QVector<double> HeartRateVariability::interpolateData()
{
    qDebug() << "\n=== НАЧАЛО ИНТЕРПОЛЯЦИИ (Cubic Spline) ===";
    qDebug() << "dt =" << spectralAnalysisLag_ << "с";
    qDebug() << "Размер data_ =" << data_.size();

    if (data_.isEmpty()) return QVector<double>();
    if (data_.size() < 3) return data_; // для сплайна нужно >= 3 точки

    // ------------------------------------------------------------------
    // 1. Формируем временные метки (x)
    // ------------------------------------------------------------------
    QVector<double> x;
    x.reserve(data_.size() + 1);

    double current_time = 0.0;
    x.push_back(current_time);

    for (int i = 0; i < data_.size(); ++i) {
        current_time += data_[i] / 1000.0;
        x.push_back(current_time);
    }

    QVector<double> y = data_;   // значения

    int n = y.size();

    // ------------------------------------------------------------------
    // 2. Вычисление коэффициентов натурального кубического сплайна
    // ------------------------------------------------------------------

    QVector<double> h(n - 1);
    for (int i = 0; i < n - 1; ++i)
        h[i] = x[i + 1] - x[i];

    QVector<double> alpha(n - 1);
    for (int i = 1; i < n - 1; ++i)
        alpha[i] = (3.0 / h[i]) * (y[i + 1] - y[i])
                   - (3.0 / h[i - 1]) * (y[i] - y[i - 1]);

    QVector<double> l(n), mu(n), z(n);
    l[0] = 1.0;
    mu[0] = 0.0;
    z[0] = 0.0;

    for (int i = 1; i < n - 1; ++i) {
        l[i] = 2.0 * (x[i + 1] - x[i - 1]) - h[i - 1] * mu[i - 1];
        mu[i] = h[i] / l[i];
        z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
    }

    l[n - 1] = 1.0;
    z[n - 1] = 0.0;

    QVector<double> c(n), b(n - 1), d(n - 1);

    for (int j = n - 2; j >= 0; --j) {
        c[j] = z[j] - mu[j] * c[j + 1];
        b[j] = (y[j + 1] - y[j]) / h[j]
               - h[j] * (c[j + 1] + 2.0 * c[j]) / 3.0;
        d[j] = (c[j + 1] - c[j]) / (3.0 * h[j]);
    }

    // ------------------------------------------------------------------
    // 3. Формируем равномерную временную сетку
    // ------------------------------------------------------------------

    double T_total = x.last();
    int N_interpolated =
        static_cast<int>(std::ceil(T_total / spectralAnalysisLag_)) + 1;

    QVector<double> t_uniform;
    t_uniform.reserve(N_interpolated);

    for (int i = 0; i < N_interpolated; ++i) {
        double t = i * spectralAnalysisLag_;
        if (t > T_total + 1e-9)
            break;
        t_uniform.push_back(t);
    }

    // ------------------------------------------------------------------
    // 4. Вычисление значений сплайна
    // ------------------------------------------------------------------

    QVector<double> result;
    result.reserve(t_uniform.size());

    int interval = 0;

    for (double t : t_uniform) {

        while (interval < n - 2 && t > x[interval + 1])
            interval++;

        double dx = t - x[interval];

        double value = y[interval]
                       + b[interval] * dx
                       + c[interval] * dx * dx
                       + d[interval] * dx * dx * dx;

        result.push_back(value);
    }

    qDebug() << "Интерполяция завершена, точек:" << result.size();
    qDebug() << "Первые 5:" << result.mid(0, 5);
    qDebug() << "Последние 5:" << result.mid(result.size() - 5, 5);

    return result;
}

// ============================================================================
// Шаг II: Применение сглаживающего окна (окно Ханна)
// ============================================================================

QVector<double> HeartRateVariability::applyWindow(const QVector<double>& x)
{
    int N = x.size();
    QVector<double> windowed(N);

    // Окно Ханна (Hann window)
    for (int i = 0; i < N; ++i) {
        double w = 0.5 * (1 - cos(2 * M_PI * i / (N - 1)));
        windowed[i] = x[i] * w;
    }

    qDebug() << "Применено окно Ханна";
    windowed_ = windowed;
    return windowed;
}

// ============================================================================
// Шаг III: Быстрое преобразование Фурье (рекурсивная реализация)
// ============================================================================

void HeartRateVariability::computeFFT(QVector<std::complex<double>> &a)
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

void HeartRateVariability::performFFT(const QVector<double>& signal)
{
    int N = signal.size();

    // Подготавливаем комплексный массив
    QVector<std::complex<double>> complex_signal(N);
    for (int i = 0; i < N; ++i) {
        complex_signal[i] = std::complex<double>(signal[i], 0.0);
    }

    // Вычисляем БПФ
    computeFFT(complex_signal);

    fft_ = complex_signal;

    qDebug() << "БПФ выполнено, размер:" << N;
}

// ============================================================================
// Шаг IV: Расчет спектральной плотности мощности
// ============================================================================

void HeartRateVariability::calculatePowerSpectrum()
{
    int N = fft_.size();
    power_.resize(N);

    // Двусторонний спектр мощности P(k) = |Xk|^2 / N^2
    for (int i = 0; i < N; ++i) {
        double mag = std::abs(fft_[i]);
        power_[i] = (mag * mag) / (N * N);
    }

    // Переход к одностороннему спектру (первые N/2 + 1 точек)
    int one_sided_size = N/2 + 1;
    powerSpectrum_.resize(one_sided_size);
    frequencies_.resize(one_sided_size);

    // Частота дискретизации
    double fs = 1.0 / spectralAnalysisLag_;
    double df = fs / N;      // Шаг по частоте


    for (int i = 0; i < one_sided_size; ++i) {
        frequencies_[i] = i * df;

        if (i == 0 || i == N/2) {
            // Постоянная составляющая и Найквист
            powerSpectrum_[i] = power_[i];
        } else {
            // Удваиваем мощность для одностороннего спектра
            powerSpectrum_[i] = 2.0 * power_[i];
        }
    }

    qDebug() << "Спектр мощности рассчитан";
    qDebug() << "Диапазон частот: 0 -" << fs/2 << "Гц";
    qDebug() << "Разрешение по частоте:" << df << "Гц";
}

// ============================================================================
// Шаг V-VII: Расчет показателей в частотных диапазонах
// ============================================================================

void HeartRateVariability::calculateFrequencyBands()
{
    // Диапазоны частот (согласно таблице 1)
    const double HF_MIN = 0.15;
    const double HF_MAX = 0.4;
    const double LF_MIN = 0.04;
    const double LF_MAX = 0.15;
    const double VLF_MIN = 0.015;
    const double VLF_MAX = 0.04;
    const double ULF_MAX = 0.015; // все что ниже 0.015

    int N = powerSpectrum_.size();

    // Векторы для хранения значений в каждом диапазоне
    QVector<double> hf_values, lf_values, vlf_values, ulf_values;
    QVector<double> hf_freqs, lf_freqs, vlf_freqs;

    double df = frequencies_[1] - frequencies_[0];

    // Проход по всем частотам
    for (int i = 0; i < N; ++i) {
        double f = frequencies_[i];
        double p = powerSpectrum_[i];

        // Пропускаем очень низкие частоты (шум)
        if (f < 0.001) continue;

        if (f >= HF_MIN && f <= HF_MAX) {
            hf_values.append(p);
            hf_freqs.append(f);
            result_.HF += p * df;

            if (p > result_.HF_max) {
                result_.HF_max = p;
                result_.HF_dominant_freq = f;
            }
            if (result_.HF_min == 0 || p < result_.HF_min) result_.HF_min = p;
        }
        else if (f >= LF_MIN && f < LF_MAX) {
            lf_values.append(p);
            lf_freqs.append(f);
            result_.LF += p * df;

            if (p > result_.LF_max) {
                result_.LF_max = p;
                result_.LF_dominant_freq = f;
            }
            if (result_.LF_min == 0 || p < result_.LF_min) result_.LF_min = p;
        }
        else if (f >= VLF_MIN && f < VLF_MAX) {
            vlf_values.append(p);
            vlf_freqs.append(f);
            result_.VLF += p * df;

            if (p > result_.VLF_max) {
                result_.VLF_max = p;
                result_.VLF_dominant_freq = f;
            }
            if (result_.VLF_min == 0 || p < result_.VLF_min) result_.VLF_min = p;
        }
        else if (f < ULF_MAX) {
            ulf_values.append(p);
            result_.ULF += p * df;

            if (p > result_.ULF_max) result_.ULF_max = p;
            if (result_.ULF_min == 0 || p < result_.ULF_min) result_.ULF_min = p;
        }
    }

    // Вычисляем средние значения
    if (!hf_values.isEmpty()) {
        result_.HF_av = std::accumulate(hf_values.begin(), hf_values.end(), 0.0) / hf_values.size();
    }
    if (!lf_values.isEmpty()) {
        result_.LF_av = std::accumulate(lf_values.begin(), lf_values.end(), 0.0) / lf_values.size();
    }
    if (!vlf_values.isEmpty()) {
        result_.VLF_av = std::accumulate(vlf_values.begin(), vlf_values.end(), 0.0) / vlf_values.size();
    }
    if (!ulf_values.isEmpty()) {
        result_.ULF_av = std::accumulate(ulf_values.begin(), ulf_values.end(), 0.0) / ulf_values.size();
    }

    // Суммарная мощность
    result_.TP = result_.HF + result_.LF + result_.VLF + result_.ULF;

    // Проценты
    if (result_.TP > 0) {
        result_.HF_percent = (result_.HF / result_.TP) * 100.0;
        result_.LF_percent = (result_.LF / result_.TP) * 100.0;
        result_.VLF_percent = (result_.VLF / result_.TP) * 100.0;
        result_.ULF_percent = (result_.ULF / result_.TP) * 100.0;
    }

    // Доминирующие периоды
    if (result_.HF_dominant_freq > 0) {
        result_.HF_dominant_period = 1.0 / result_.HF_dominant_freq;
    }
    if (result_.LF_dominant_freq > 0) {
        result_.LF_dominant_period = 1.0 / result_.LF_dominant_freq;
    }
    if (result_.VLF_dominant_freq > 0) {
        result_.VLF_dominant_period = 1.0 / result_.VLF_dominant_freq;
    }

    // Индексы (формулы из методички)
    if (result_.HF > 0) {
        result_.IC = (result_.VLF + result_.LF) / result_.HF;
        result_.IVV = result_.LF / result_.HF;
    }
    if (result_.VLF > 0) {
        result_.IAP = result_.LF / result_.VLF;
    }

    qDebug() << "Результаты спектрального анализа:";
    qDebug() << "  HF:" << result_.HF << "мс² (" << result_.HF_percent << "%)";
    qDebug() << "  LF:" << result_.LF << "мс² (" << result_.LF_percent << "%)";
    qDebug() << "  VLF:" << result_.VLF << "мс² (" << result_.VLF_percent << "%)";
    qDebug() << "  ULF:" << result_.ULF << "мс² (" << result_.ULF_percent << "%)";
    qDebug() << "  TP:" << result_.TP << "мс²";
    qDebug() << "  IC:" << result_.IC;
    qDebug() << "  IVV:" << result_.IVV;
    qDebug() << "  IAP:" << result_.IAP;
}

// ============================================================================
// Основной метод: выполняет все шаги анализа
// ============================================================================

void HeartRateVariability::performSpectralAnalysis()
{
    if (data_.isEmpty()) {
        qDebug() << "Ошибка: нет данных";
        return;
    }

    qDebug() << "\n=== НАЧАЛО СПЕКТРАЛЬНОГО АНАЛИЗА ===";

    // Шаг I: Интерполяция
    QVector<double> interpolated = interpolateData();

    // Удаляем среднее
    double mean = std::accumulate(interpolated.begin(),
                                  interpolated.end(), 0.0)
                  / interpolated.size();

    for (double &v : interpolated)
        v -= mean;

    // Шаг II: Применение окна
    QVector<double> windowed = applyWindow(interpolated);

    double mean2 = std::accumulate(windowed.begin(),
                                   windowed.end(), 0.0)
                   / windowed.size();

    for (double &v : windowed)
        v -= mean2;

    // Шаг III: БПФ
    performFFT(windowed);

    // Шаг IV: Расчет спектра мощности
    calculatePowerSpectrum();

    // Шаги V-VII: Расчет показателей в диапазонах
    calculateFrequencyBands();

    qDebug() << "=== СПЕКТРАЛЬНЫЙ АНАЛИЗ ЗАВЕРШЕН ===\n";
}

QVector<double> HeartRateVariability::getFrequencies(){
    return frequencies_;
}

QVector<double> HeartRateVariability::getPowerSpectrum(){
    return powerSpectrum_;
}

const  resultHeartRateVariability HeartRateVariability::calculate(){
    result_ = resultHeartRateVariability();
    calculateStatistic();
    calculateAutocorrelation();
    performSpectralAnalysis();
    return result_;
}
