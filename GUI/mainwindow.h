#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>

#include "../slide_window.h"
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
    const double max_voltage = 3.3;

    void QPlot_init(QCustomPlot *customPlot);
    void Show_Plot(QCustomPlot *customPlot, QVector<double> x,
                   QVector<double> y);
};
#endif  // MAINWINDOW_H
