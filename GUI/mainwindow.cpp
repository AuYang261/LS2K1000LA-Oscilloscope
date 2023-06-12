#include "mainwindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    // spi_init();
    Slide_Window::get_instance().init();
    buildChart();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::buildChart() {
    /*--------------------------------
     *  构造示波器
     *-------------------------------*/
    ui->customPlot->setBackground(QBrush(QColor("#474848")));  //
    ui->customPlot->setInteractions(
        QCP::iRangeDrag | QCP::iRangeZoom |
        QCP::iSelectPlottables);  // 可拖拽+可滚轮缩放
    ui->customPlot->yAxis->setRange(0, 3.3);
}
