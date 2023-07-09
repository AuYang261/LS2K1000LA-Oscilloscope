#include "mainwindow.h"

#include "ui_mainwindow.h"

void MainWindow::spi_get_data() {
    uint16_t buf[1];
    struct timeval dwStart, dwEnd;
    // uint64_t cnt = 0, last_cnt = 0;
    gettimeofday(&dwStart, NULL);
    int64_t last_us = 0, us = 0;
    while (1) {
        spi_read(buf, sizeof(buf) / sizeof(buf[0]));
        for (size_t i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
            buf[i] >>= 1;
            buf[i] &= 0xfff;
            mtx.lock();
            seq.push_back(buf[i]);
            mtx.unlock();
        }
        // cnt += sizeof(buf) / sizeof(buf[0]);
        do {
            gettimeofday(&dwEnd, NULL);
            us = 1000 * 1000 * (dwEnd.tv_sec - dwStart.tv_sec) +
                 (dwEnd.tv_usec - dwStart.tv_usec);
        } while (us < last_us + 1000 * 1000 * sizeof(buf) / sizeof(buf[0]) /
                                    RT_sampling_rate);
        // printf("%ld samples in %dms, %ld per second\r", cnt, us / 1000,
        //        1000 * 1000 * (cnt - last_cnt) /
        //            (us - last_us == 0 ? 1 : us - last_us));
        last_us = us;
        // last_cnt = cnt;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    qDebug() << "MainWindow init" << endl;
    ui->setupUi(this);
    spi_init();
    // Slide_Window::get_instance().init();

    customPlot = ui->customPlot;
    XAxisScaler = ui->XAxisScaler;
    YAxisScaler = ui->YAxisScaler;
    TriggerSlider = ui->TriggerSlider;

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

    // 设置坐标轴
    set_Xlable();
    customPlot->xAxis->setRange(
        0, horizontal_div * scan_speed_ns_per_div / horizontal_scale());
    XAxisScaler->setMaximum(100);
    XAxisScaler->setValue(100);
    XAxisScaler->setPageStep(1);
    // 0<=value<=100
    connect(XAxisScaler, &QSlider::sliderMoved, [this](int value) {
        // 100<=scan_speed_ns_per_div<=20*1000*1000
        scan_speed_ns_per_div = 100 * pow(10, value * (log10(2) + 5) / 100);
        set_Xlable();
        ui->customPlot->xAxis->setRange(
            0, horizontal_div * scan_speed_ns_per_div / horizontal_scale());
    });

    customPlot->yAxis->setLabel("Voltage(V)");
    customPlot->yAxis->setRange(
        0, vertical_div * vertical_mV_per_div / vertical_scale());
    YAxisScaler->setMaximum(100);
    YAxisScaler->setValue(100);
    YAxisScaler->setPageStep(1);
    // 0<=value<=100
    connect(YAxisScaler, &QSlider::sliderMoved, [this](int value) {
        // 2<=vertical_mV_per_div<=500
        vertical_mV_per_div = 2 + 4.98 * value;
        set_Ylable();
        ui->customPlot->yAxis->setRange(
            0, vertical_div * vertical_mV_per_div / vertical_scale());
    });

    TriggerSlider->setValue(0);
    TriggerSlider->setPageStep(trigger_voltage_mV * 100 / max_voltage_mV);
    TriggerSlider->setMaximum(100);
    // 0<=value<=100
    connect(TriggerSlider, &QSlider::sliderMoved, [this](int value) {
        // 0<=trigger_voltage<=max_voltage
        trigger_voltage_mV = max_voltage_mV * value / 100;
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
    if (seq.size() < 2) {
        return;
    }

    QVector<double> x, y;
    double time_ns = 0;
    mtx.lock();
    auto trigger = seq.rbegin();
    // 寻找触发点
    for (; trigger + 1 != seq.rend() &&
           trigger - seq.rbegin() <
               trigger_timeout_ns * RT_sampling_rate / 1000 / 1000 / 1000;
         trigger++) {
        if (DAC(*trigger) > trigger_voltage_mV &&
            DAC(*(trigger + 1)) <= trigger_voltage_mV) {
            // 触发
            break;
        }
    }
    int cnt;
    for (cnt = 0; cnt < horizontal_div * horizontal_point_per_div &&
                  cnt * point_per_sampling() < seq.rend() - trigger;
         cnt++) {
        x.push_back(time_ns / horizontal_scale());
        y.push_back(DAC(*(trigger + cnt * point_per_sampling())) /
                    vertical_scale());
        time_ns += scan_speed_ns_per_div / horizontal_point_per_div;
    }
    seq.assign((trigger + (cnt - 1) * point_per_sampling()).base(), seq.end());
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
