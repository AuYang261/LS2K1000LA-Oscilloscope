#include "mainwindow.h"

#include "ui_mainwindow.h"

void MainWindow::spi_get_data() {
    uint16_t buf[512];
    struct timeval dwStart, dwEnd;
    Slide_Window &sw = Slide_Window::get_instance();
    uint64_t cnt = 0;
    gettimeofday(&dwStart, NULL);
    while (1) {
        spi_read(buf, sizeof(buf) / sizeof(buf[0]));
        for (size_t i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
            // std::cout << "#" << i << " 0x" << std::hex << buf[i] <<
            // std::endl;
            mtx.lock();
            sw.add_data(buf[i]);
            mtx.unlock();
        }
        cnt += sizeof(buf) / sizeof(buf[0]);
        gettimeofday(&dwEnd, NULL);
        int64_t us = 1000 * 1000 * (dwEnd.tv_sec - dwStart.tv_sec) +
                     (dwEnd.tv_usec - dwStart.tv_usec);
        printf(
            "%ld samples in %dms, %ld per second, "
            "error_cnt=%ld, error_rate=%ld/10000\r",
            sw.size(), us / 1000, 1000 * 1000 * sw.size() / (us == 0 ? 1 : us),
            sw.get_error_cnt(), sw.get_error_cnt() * 10000 / sw.size());
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    qDebug() << "MainWindow init" << endl;
    ui->setupUi(this);
    spi_init();
    Slide_Window::get_instance().init();

    customPlot = ui->customPlot;

    spi_thread = std::thread(&MainWindow::spi_get_data, this);

    QPlot_init(customPlot);
}

MainWindow::~MainWindow() { delete ui; }

// 绘图图表初始化
void MainWindow::QPlot_init(QCustomPlot *customPlot) {
    // 图表添加两条曲线
    pGraph1_1 = customPlot->addGraph();

    // 设置曲线颜色
    pGraph1_1->setPen(QPen(Qt::red));

    // 设置坐标轴名称
    set_Xlable();
    customPlot->xAxis->setRange(
        0, horizontal_div * scan_speed_ns_per_div / horizontal_scale());
    ui->horizontalSlider->setValue(100);
    ui->horizontalSlider->setPageStep(1);
    ui->horizontalSlider->setMaximum(100);
    // 0<=value<=100
    connect(ui->horizontalSlider, &QSlider::sliderMoved, [this](int value) {
        // 100<=scan_speed_ns_per_div<=20*1000*1000
        scan_speed_ns_per_div = 100 * pow(10, value * (log10(2) + 5) / 100);
        set_Xlable();
        ui->customPlot->xAxis->setRange(
            0, horizontal_div * scan_speed_ns_per_div / horizontal_scale());
    });

    customPlot->yAxis->setLabel("Voltage(V)");
    customPlot->yAxis->setRange(0, vertical_div * vertical_mV_per_div / 1000);
    ui->verticalSlider->setValue(100);
    ui->verticalSlider->setPageStep(1);
    ui->verticalSlider->setMaximum(100);
    // 0<=value<=100
    connect(ui->verticalSlider, &QSlider::sliderMoved, [this](int value) {
        // 50<=vertical_mV_per_div<=500
        vertical_mV_per_div = 50 + 4.5 * value;
        ui->customPlot->yAxis->setRange(
            0, vertical_div * vertical_mV_per_div / 1000);
    });

    // 显示图表的图例
    customPlot->legend->setVisible(true);
    // 添加曲线名称
    pGraph1_1->setName("wave1");

    // 允许用户用鼠标拖动轴范围，用鼠标滚轮缩放，点击选择图形:
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                                QCP::iSelectPlottables);

    // 创建定时器，用于定时生成曲线坐标点数据
    QTimer *timer = new QTimer(this);
    timer->start(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(TimeData_Update()));
}

// 定时器溢出处理槽函数。用来生成曲线的坐标数据。
void MainWindow::TimeData_Update(void) {
    Slide_Window &sw = Slide_Window::get_instance();
    if (sw.size() < 2) {
        return;
    }

    QVector<double> x, y;
    double time_ns = horizontal_div * scan_speed_ns_per_div;
    mtx.lock();
    auto trigger = sw.get_seq().rbegin();
    // 寻找触发点
    for (; trigger + 1 != sw.get_seq().rend() &&
           trigger - sw.get_seq().rbegin() <
               trigger_timeout_ns * RT_sampling_rate / 1000 / 1000 / 1000;
         trigger++) {
        if (DAC(trigger->data) > trigger_voltage &&
            DAC((trigger + 1)->data) <= trigger_voltage) {
            // 触发
            break;
        }
    }
    for (int cnt = 0;
         cnt < horizontal_div * horizontal_point_per_div &&
         cnt * point_per_sampling() < sw.get_seq().rend() - trigger;
         cnt++) {
        x.push_back(time_ns / horizontal_scale());
        y.push_back(DAC((trigger + cnt * point_per_sampling())->data));
        time_ns -= scan_speed_ns_per_div / horizontal_point_per_div;
    }
    mtx.unlock();
    Show_Plot(customPlot, x, y);
}

// 曲线更新绘图
void MainWindow::Show_Plot(QCustomPlot *customPlot, QVector<double> x,
                           QVector<double> y) {
    static double cnt;

    cnt++;
    // 给曲线添加数据
    //    pGraph1_1->addData(cnt, num);
    pGraph1_1->setData(x, y);

    // 更新绘图，这种方式在高填充下太浪费资源。有另一种方式rpQueuedReplot，可避免重复绘图。
    // 最好的方法还是将数据填充、和更新绘图分隔开。将更新绘图单独用定时器更新。例程数据量较少没用单独定时器更新，实际工程中建议大家加上。
    // customPlot->replot();
    customPlot->replot(QCustomPlot::rpQueuedReplot);

    static QTime time(QTime::currentTime());
    double key = time.elapsed() / 1000.0;  // 开始到现在的时间，单位秒
    ////计算帧数
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key - lastFpsKey > 1)  // 每1秒求一次平均值
    {
        // 状态栏显示帧数和数据总数
        ui->statusbar->showMessage(
            QString("%1 FPS, Total Data points: %2")
                .arg(frameCount / (key - lastFpsKey), 0, 'f', 0)
                .arg(customPlot->graph(0)->data()->size()),
            0);
        lastFpsKey = key;
        frameCount = 0;
    }
}
