#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <thread>

// #include "../slide_window.h"
#include "../spi.h"
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void spi_get_data();

   public slots:
    void TimeData_Update(void);

   private:
    Ui::MainWindow *ui;
    QCPGraph *pGraph1_1;
    // 绘图控件的指针
    QCustomPlot *customPlot;
    std::thread spi_thread;
    std::mutex mtx;

    const uint max_sample_value = 0xfff;
    const double max_voltage_mV = 3300;
    const int horizontal_div = 10;
    const int vertical_div = 8;
    const int horizontal_point_per_div = 50;
    int vertical_mV_per_div = 500;
    const double RT_sampling_rate = 100 * 1000;                 // 100kHz
    const double Equivalent_sampling_rate = 200 * 1000 * 1000;  // 200MHz
    double scan_speed_ns_per_div = 20 * 1000 * 1000;
    double trigger_voltage_mV = 100;
    const double trigger_timeout_ns = 50 * 1000 * 1000;  // 50ms
    QVector<double> seq;

    double horizontal_scale() const {
        if (horizontal_div * scan_speed_ns_per_div < 1000) {
            return 1;
        } else if (horizontal_div * scan_speed_ns_per_div < 1000 * 1000) {
            return 1000;
        } else if (horizontal_div * scan_speed_ns_per_div <
                   1000 * 1000 * 1000) {
            return 1000 * 1000;
        } else {
            return 1000 * 1000 * 1000;
        }
    };
    double vertical_scale() const {
        if (vertical_div * vertical_mV_per_div < 1000) {
            return 1;
        } else {
            return 1000;
        }
    };
    void set_Xlable() {
        auto hs = horizontal_scale();
        if (hs <= 1.0) {
            customPlot->xAxis->setLabel("time(ns)");
        } else if (hs <= 1000.0) {
            customPlot->xAxis->setLabel("time(us)");
        } else if (hs <= 1000.0 * 1000) {
            customPlot->xAxis->setLabel("time(ms)");
        } else {
            customPlot->xAxis->setLabel("time(s)");
        }
    }
    void set_Ylable() {
        auto vs = vertical_scale();
        if (vs <= 1.0) {
            customPlot->yAxis->setLabel("Voltage(mV)");
        } else {
            customPlot->yAxis->setLabel("Voltage(V)");
        }
    }
    // 图中每个点对应的采样点数
    inline double point_per_sampling() const {
        return RT_sampling_rate * scan_speed_ns_per_div /
               horizontal_point_per_div / 1000 / 1000 / 1000;
    }
    inline double DAC(uint16_t value) const {
        return max_voltage_mV * value / 0xfff;
    }

    void QPlot_init(QCustomPlot *customPlot);
    void Show_Plot(QCustomPlot *customPlot, QVector<double> x,
                   QVector<double> y);
};
#endif  // MAINWINDOW_H
