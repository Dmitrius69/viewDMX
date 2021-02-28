#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <Qt>
#include <QTextEdit>
#include <QTime>

//#include <QtGui/QtTextEdit.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label->setText("1");
    ui->label_2->setText("2");
    ui->label_3->setText("3");
    ui->label_4->setText("4");
    ui->label_5->setText("5");
    ui->label_6->setText("6");
    ui->label_7->setText("7");
    ui->label_8->setText("8");
    

    for (int i=0; i<50; i++) Cnt[i]=0;
    QTime midnight(0,0,0); //создаем счетчик времени
    qsrand(midnight.secsTo(QTime::currentTime())); //инициализируем нас счетчик случайных чисел
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()),this,SLOT(slotTimerAlarm()));
    tstart = false;
    scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);


}

MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::on_pushButton_pressed()
{
    if (!tstart)
       { timer->start(300); tstart = true; }
    else
       { timer->stop(); tstart = false; }

}

void MainWindow::on_pushButton_2_pressed()
{
    quick_exit(0);
}

void MainWindow::drawBars(int idx, int bHeight)
{


    int x,y;
    int gW, gH;

    gW = ui->graphicsView->width();
    gH = ui->graphicsView->height();

    QString sDeb = QString::number(gW);
    ui->debug1->setText( sDeb);
    sDeb = QString::number(gH);
    ui->debug2->setText( sDeb);


    int x0 = 0; //будем смещать x c помощью глобальной переменной
    int y0 = 300; //высота наших графиков будет (graphicsView->height ) pix


    QRectF rF(0, 0, gW, gH);
    scene->setSceneRect(0, 0, gW, gH); //устанавливаем размер нашей сцены, он должен совпадать с размером graphicsView
    QPolygon backP(QRect(0,0,gW,gH), false) ;

    QPen mPen(Qt::green);
    QBrush mBrush(Qt::green);
    QPen wPen(Qt::blue);
    QBrush wBrush(Qt::blue);

    //scene->addPolygon(*backP, mPen, mBrush);

    x = x0+idx*deltaBar;
    y = y0 - bHeight;
    QBrush *blueBrush = new QBrush(Qt::blue);
    scene->setBackgroundBrush(*blueBrush);

    if (gX == 0)
    {
        myPoints[idx].gX1= x; myPoints[idx].gY1 = y; myPoints[idx].gW1 = 10; myPoints[idx].gH1 = bHeight;

    }
    else
    {
        QPolygon bar(QRect(myPoints[idx].gX1,myPoints[idx].gY1,myPoints[idx].gW1,myPoints[idx].gH1), false);
        scene->addPolygon(bar, wPen, wBrush);

    }

    //scene->addLine(x, y, x+10, y, mPen);
    //scene->addLine(x+10, y, x+10, y+bHeight, mPen);
    //scene->addLine(x+10, y+bHeight, x, y + bHeight, mPen );
    //scene->addLine(x, y+ bHeight, x, y, mPen);
    myPoints[idx].gX1= x; myPoints[idx].gY1 = y; myPoints[idx].gW1 = widthBar; myPoints[idx].gH1 = bHeight;
    QPolygon bar(QRect(x,y,widthBar,bHeight), false);
    scene->addPolygon(bar, mPen, mBrush);
    if (idx == 7) gX = 1; //первый запуск прошел


}


void MainWindow::drawGraph(int Data[])
{

    for (int i=0;i<8;i++) drawBars(i, Data[i]);
}

void MainWindow::getRandData()
{
   Cnt[0]  = qrand()%255;
   Cnt[1]  = qrand()%255;
   Cnt[2]  = qrand()%255;
   Cnt[3]  = qrand()%255;
   Cnt[4]  = qrand()%255;
   Cnt[5]  = qrand()%255;
   Cnt[6]  = qrand()%255;
   Cnt[7]  = qrand()%255;
   Cnt[8]  = qrand()%255; //лишний! можно не учитываить

}

void MainWindow::slotTimerAlarm()
{

    getRandData();
    ui->lcdNumber->display(Cnt[0]);


    ui->lcdNumber_2->display(Cnt[1]);

    ui->lcdNumber_3->display(Cnt[2]);

    ui->lcdNumber_4->display(Cnt[3]);

    ui->lcdNumber_5->display(Cnt[4]);

    ui->lcdNumber_6->display(Cnt[5]);

    ui->lcdNumber_7->display(Cnt[6]);

    ui->lcdNumber_8->display(Cnt[7]);
    drawGraph(Cnt);

}
