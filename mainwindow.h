#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QTimer>

#include <array>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_pushButton_pressed();
    void on_pushButton_2_pressed();
    void slotTimerAlarm();

private:
    static constexpr int channelCount = 8;

    Ui::MainWindow *ui;
    QTimer *timer;
    bool tstart;
    QGraphicsScene *scene;
    std::array<int, channelCount> channelValues;
    std::array<QGraphicsRectItem *, channelCount> bars;

    void initializeBars();
    void getRandomData();
    void drawGraph();
};

#endif // MAINWINDOW_H
