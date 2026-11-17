#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QBrush>
#include <QPen>
#include <QResizeEvent>

#include <random>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
    , tstart(false)
    , scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    for (int i = 0; i < channelCount; ++i) {
        ui->gridLayout->itemAtPosition(0, i)->widget()->setProperty("dmxChannel", i + 1);
        channelValues[i] = 0;
        bars[i] = nullptr;
    }

    ui->label->setText("1");
    ui->label_2->setText("2");
    ui->label_3->setText("3");
    ui->label_4->setText("4");
    ui->label_5->setText("5");
    ui->label_6->setText("6");
    ui->label_7->setText("7");
    ui->label_8->setText("8");

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    initializeBars();

    timer->setInterval(300);
    connect(timer, &QTimer::timeout, this, &MainWindow::slotTimerAlarm);
}

MainWindow::~MainWindow()
{
    timer->stop();
    delete ui;
}

void MainWindow::initializeBars()
{
    scene->setBackgroundBrush(QBrush(Qt::blue));

    for (int i = 0; i < channelCount; ++i) {
        bars[i] = scene->addRect(0, 0, 1, 1,
                                 QPen(Qt::darkGreen),
                                 QBrush(Qt::green));
    }

    drawGraph();
}

void MainWindow::on_pushButton_pressed()
{
    tstart = !tstart;

    if (tstart)
        timer->start();
    else
        timer->stop();
}

void MainWindow::on_pushButton_2_pressed()
{
    close();
}

void MainWindow::getRandomData()
{
    static std::random_device randomDevice;
    static std::mt19937 generator(randomDevice());
    std::uniform_int_distribution<int> distribution(0, 255);

    for (int i = 0; i < channelCount; ++i)
        channelValues[i] = distribution(generator);
}

void MainWindow::drawGraph()
{
    const QRectF viewRect = ui->graphicsView->rect();
    const double width = viewRect.width();
    const double height = viewRect.height();

    if (width <= 0.0 || height <= 0.0)
        return;

    const double padding = 10.0;
    const double plotWidth = width - 2.0 * padding;
    const double plotHeight = height - 2.0 * padding;
    const double slotWidth = plotWidth / channelCount;
    const double barWidth = slotWidth * 0.70;

    scene->setSceneRect(0.0, 0.0, width, height);

    ui->debug1->setText(QString::number(static_cast<int>(width)));
    ui->debug2->setText(QString::number(static_cast<int>(height)));

    for (int i = 0; i < channelCount; ++i) {
        const double barHeight = plotHeight * channelValues[i] / 255.0;
        const double x = padding + i * slotWidth + (slotWidth - barWidth) / 2.0;
        const double y = padding + plotHeight - barHeight;

        bars[i]->setRect(x, y, barWidth, barHeight);
    }
}

void MainWindow::slotTimerAlarm()
{
    getRandomData();

    ui->lcdNumber->display(channelValues[0]);
    ui->lcdNumber_2->display(channelValues[1]);
    ui->lcdNumber_3->display(channelValues[2]);
    ui->lcdNumber_4->display(channelValues[3]);
    ui->lcdNumber_5->display(channelValues[4]);
    ui->lcdNumber_6->display(channelValues[5]);
    ui->lcdNumber_7->display(channelValues[6]);
    ui->lcdNumber_8->display(channelValues[7]);

    drawGraph();
}
