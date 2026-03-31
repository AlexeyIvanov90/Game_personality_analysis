#ifndef HEART_RATE_VARIABILITY_H
#define HEART_RATE_VARIABILITY_H

#include <QString>
#include <QVector>
#include <QDebug>
#include <complex>

struct resultHeartRateVariability{
    double HR=0; //частота ритма
    double M=0; //среднее значение интервалов
    double SDNN=0; //среднеквадратическое отклонение
    double CV=0; //коэффициент вариации
    double RMSSD=0; //среднеквадратическая разностная характеристика
    double Mo=0; //мода
    double AMo=0; //моды амплитуда
    double Mx=0; //максимальное значений кардиоинтервалов
    double Mn=0; //минимальное значений кардиоинтервалов
    double MxdMn=0; //вариационный размах
    double SI=0; //стресс-индекс
    double CC1=0; //коэффициента корреляции после первого сдвига
    double CC0=0; //число сдвигов до первого нулевого значения коэффициента корреляции

    double HF_min=0;
    double HF=0;
    double HF_max=0;
    double HF_dominant_freq=0;
    double HF_dominant_period=0;
    double HF_av=0;
    double HF_percent=0;

    double LF_min=0;
    double LF=0;
    double LF_max=0;
    double LF_dominant_freq=0;
    double LF_dominant_period=0;
    double LF_av=0;
    double LF_percent=0;

    double VLF_min=0;
    double VLF=0;
    double VLF_max=0;
    double VLF_dominant_freq=0;
    double VLF_dominant_period=0;
    double VLF_av=0;
    double VLF_percent=0;

    double ULF_min=0;
    double ULF=0;
    double ULF_max=0;
    double ULF_dominant_freq=0;
    double ULF_dominant_period=0;
    double ULF_av=0;
    double ULF_percent=0;

    double TP=0; // суммарная мощность
    double IC=0;
    double IVV=0;
    double IAP=0;

    double IRSA=0; //показателя активности регуляторных систем (ПАРС/IRSA)


    int calculateTotalRegulationEffect(){ //Суммарный эффект регуляции
        if(HR < 51)
            return -2;
        else if(HR >= 51 && HR < 60)
            return -1;
        else if(HR >= 80 && HR < 90)
            return 1;
        else if(HR >= 90)
            return 2;
        return 0;
    }

    int calculateAutomationFunctions(){ //Функции автоматизма
        if(MxdMn <= 60 && CV <= 2)
            return  2;
        else if((MxdMn > 60 && MxdMn < 150) && (CV > 2.0 && CV <= 4.0))
            return 1;
        else if(MxdMn > 300 && MxdMn <= 500)
            return -1;
        else if(MxdMn > 500)
            return -2;
        return 0;
    }

    int calculateVegetativeHomeostasis(){ //Вегетативный гомеостаз
        if(MxdMn <= 60 && AMo > 80)
            return 2;
        else if((MxdMn > 60 && MxdMn < 150) && (CV >= 51 && CV <= 80))
            return 1;
        else if((MxdMn >= 150 && MxdMn <= 300) && (CV >= 30 && CV < 51))
            return 0;
        else if((MxdMn > 300 && MxdMn <= 500) && (CV >= 20 && CV < 30))
            return -1;
        else if(MxdMn > 500 && CV < 20)
            return -2;
        return 0;
    }

    bool addStabilityOfRegulation(){ //Устойчивость регуляции
        if(CV>15. && SI<15.){
            qDebug() << "Наблюдаемая нестабильность СР связана спереходными процессами";
            return false;
        }
        return true;
    }

    int calculateVasomotorCenter(){ //Вазомоторный (сосудистый) центр
        if (LF_percent >= 55)
            return 2;
        else if (LF_percent >= 40 && LF_percent < 55)
            return 1;
        else if (LF_percent >= 11  && LF_percent < 20)
            return -1;
        else if (LF_percent < 11)
            return -2;
        return 0;
    }

    int calculateSympatheticCardiovascularSubcorticalNerveCenter(){ //Симпатический сердечно-сосудистый подкорковый нервный центр
        if(VLF_percent >= 60)
            return 2;
        else if(VLF_percent >= 45 && VLF_percent < 60)
            return 1;
        else if(VLF_percent >= 16  && VLF_percent < 25)
            return -1;
        else if(VLF_percent < 16)
            return -2 ;
        return 0;
    }

    QString getText(){
        QString text="";

        text += "частота ритма(HR): "  + QString::number(HR) +
                "\nсреднее значение интервалов(M): " +  QString::number(M) +
                "\nсреднеквадратическое отклонение(SDNN): " + QString::number(SDNN) +
                "\nкоэффициент вариации(CV): " + QString::number(CV) +
                "\nсреднеквадратическая разностная характеристика(RMSSD): "+ QString::number(RMSSD) +
                "\nмода(Mo): "+ QString::number(Mo) +
                "\nмоды амплитуда(AMo): " + QString::number(AMo) +
                "\nмаксимальное значений кардиоинтервалов(Mx): "+ QString::number(Mx) +
                "\nминимальное значений кардиоинтервалов(Mn): "+ QString::number(Mn) +
                "\nвариационный размах(MxdMn): "+ QString::number(MxdMn) +
                "\nстресс-индекс(SI): " + QString::number(SI) +
                "\nкоэффициент корреляции после первого сдвига: "  + QString::number(CC1) +
                "\nчисло сдвигов до первого нулевого значения: " + QString::number(CC0) +
                "\nHF_percent: " + QString::number(HF_percent) +
                "\nLF_percent: " + QString::number(LF_percent) +
                "\nVLF_percent: " + QString::number(VLF_percent) +
                "\nULF_percent: " + QString::number(ULF_percent);

        int regulationEffect, automationFunctions, vegetativeHomeostasis, vasomotorCenter, sympatheticCardiovascularSubcorticalNerveCenter;
        regulationEffect=calculateTotalRegulationEffect();
        automationFunctions= calculateAutomationFunctions();
        vegetativeHomeostasis= calculateVegetativeHomeostasis();
        vasomotorCenter= calculateVasomotorCenter();
        sympatheticCardiovascularSubcorticalNerveCenter= calculateSympatheticCardiovascularSubcorticalNerveCenter();

        IRSA=0.;
        IRSA+= regulationEffect;
        IRSA+= automationFunctions;
        IRSA+= vegetativeHomeostasis;
        IRSA+= vasomotorCenter;
        IRSA+= sympatheticCardiovascularSubcorticalNerveCenter;

        text += "\nСуммарный эффект регуляции: " + QString::number(regulationEffect) +
                "\nФункции автоматизма: " + QString::number(automationFunctions) +
                 "\nВегетативный гомеостаз: " + QString::number(vegetativeHomeostasis) +
                 "\nВазомоторный (сосудистый) центр: " + QString::number(vasomotorCenter) +
                 "\nСимпатический сердечно-сосудистый подкорковый нервный центр: " + QString::number(sympatheticCardiovascularSubcorticalNerveCenter);


        text+="\nПАРС= " + QString::number(IRSA);
        if(IRSA<2.){
            text +=+"\nОптимальное напряжение регуляторных систем";
        }else if(IRSA<4.){
            text+="\nУмеренное напряжение регуляторных систем";
        }else if(IRSA<6.){
            text+="\nВыраженное напряжение регуляторных систем";
        }else if(IRSA<7.){
            text+="\nПеренапряжение регуляторных систем";
        }else if(IRSA<8.){
            text+="\nИстощение регуляторных систем";
        }else if(IRSA<10.){
            text+="\nСрыв адаптации";
        }

        qDebug() << text;

        return text;
    }
};

class HeartRateVariability{
public:
    HeartRateVariability();
    ~HeartRateVariability();
    void setData(const QVector<double>& data);

    const resultHeartRateVariability calculate();

    void setAutocorrelationMaxLag(int maxLag);
    QVector<double> getVectorAutocorrelation();

    void setSpectralAnalysisLag_(double lag);
    QVector<double> getFrequencies();
    QVector<double> getPowerSpectrum();
private:
    void calculateStatistic();

    void calculateAutocorrelation();

    void performSpectralAnalysis();
    QVector<double> interpolateData();
    QVector<double> applyWindow(const QVector<double>& x);
    void performFFT(const QVector<double>& signal);
    void computeFFT(QVector<std::complex<double>> &a);

    void calculatePowerSpectrum();
    void calculateFrequencyBands();

    int maxLagAutocorrelation_ = 128;
    QVector<double> autocorrelation_;

    double spectralAnalysisLag_ = 0.25;
    QVector<double> interpolated_;
    QVector<double> windowed_;
    QVector<std::complex<double>> fft_;
    QVector<double> power_;
    QVector<double> frequencies_;
    QVector<double> powerSpectrum_;

    QVector<double> data_;
    resultHeartRateVariability result_;
};

#endif // HEART_RATE_VARIABILITY_H
