#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTime>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int Cnt[50];
    int gX = 0;
    struct mPoint { int gX1 ;
              int gY1 ;
              int gW1 ;
              int gH1 ;
            }  ;
    mPoint myPoints[50];
    const int widthBar = 80;
    const int deltaBar = 83;


private slots:


    void on_pushButton_pressed();

    void on_pushButton_2_pressed();

    void slotTimerAlarm();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    bool tstart;
    QGraphicsScene      *scene;     // Объявляем сцену для отрисовки

    void drawBars(int idx, int bHeight);
    void getRandData();
    void drawGraph(int Data[]);

};

#endif // MAINWINDOW_H


