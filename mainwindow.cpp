#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "localadj.h"

#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPixmap>
#include <qmath.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);
    ui->pushButton_5->setEnabled(false);
    //ui->pushButton_6->setEnabled(false);
    ui->lineEdit_2->setEnabled(false);
    ui->lineEdit_3->setEnabled(false);
    ui->lineEdit_4->setEnabled(false);
    ui->lineEdit_5->setEnabled(false);
    ui->graphicsView->show();
    this->setFixedSize(1024,583);
    //----To Enable the "Generate Crop Button"-----------------------------------------
    connect(ui->lineEdit_2,SIGNAL(textChanged(QString)), this, SLOT(checkLineEdits()));
    connect(ui->lineEdit_3,SIGNAL(textChanged(QString)), this, SLOT(checkLineEdits()));
    connect(ui->lineEdit_4,SIGNAL(textChanged(QString)), this, SLOT(checkLineEdits()));
    connect(ui->lineEdit_5,SIGNAL(textChanged(QString)), this, SLOT(checkLineEdits()));

    //----To Enable the "Select Radius Button"-----------------------------------------
    connect(ui->lineEdit_2,SIGNAL(textChanged(QString)), this, SLOT(checkLineRadius()));
    connect(ui->lineEdit_3,SIGNAL(textChanged(QString)), this, SLOT(checkLineRadius()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("*.*"));

    if (fileName == "")
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not open File"));
        return;
    }

    ui->lineEdit->setText(fileName);

    QImage img;
    img.load(ui->lineEdit->text());

    int width   = img.width();
    int height  = img.height();
    int depth   = img.depth();
    int channel = img.dotsPerMeterY();
    QString info;
    info.append("Width: ");
    info.append(QString::number(width));
    info.append("\nHeight: ");
    info.append(QString::number(height));
    info.append("\nChannels: ");
    info.append(QString::number(channel));
    info.append("\nDots Per Meter Y: ");
    info.append(QString::number(depth));
    ui->textEdit->setText(info);
    qDebug() << info;
    this->scene = new QGraphicsScene;
    this->item.setPixmap(QPixmap::fromImage(img));
    this->scene->addItem(&(this->item));
    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();
    ui->pushButton_4->setEnabled(true);
}

void MainWindow::on_pushButton_4_clicked()
{
    ui->lineEdit_2->setEnabled(true);
    ui->lineEdit_3->setEnabled(true);
}

void MainWindow::checkLineEdits()
{
//    bool ok = !ui->lineEdit_2->text().isEmpty()
//            && !ui->lineEdit_3->text().isEmpty()
//            && !ui->lineEdit_4->text().isEmpty()
//            && !ui->lineEdit_5->text().isEmpty();
    ui->pushButton_5->setEnabled(true);
    ui->pushButton_6->setEnabled(true);
}

void MainWindow::checkLineRadius()
{
    bool ok = !ui->lineEdit_2->text().isEmpty()
            && !ui->lineEdit_3->text().isEmpty();
    ui->pushButton_3->setEnabled(true);
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->lineEdit_4->setEnabled(true);
    ui->lineEdit_5->setEnabled(true);
}

void MainWindow::on_pushButton_5_clicked() // Generate Crop
{
    int innerRadius = ui->lineEdit_4->text().toInt();
    int outerRadius = ui->lineEdit_5->text().toInt();
    QPoint center;
    center.setX(ui->lineEdit_2->text().toInt());
    center.setY(ui->lineEdit_3->text().toInt());


    //----For using class and object -----------------------------------------

    LocalAdjustments ladj(this);
    connect(&ladj, SIGNAL(signalProgress(int)),
                this, SLOT(slotProgress(int)));
    ladj.setImagePath(ui->lineEdit->text());
    QPoint checkPoint=ladj.centerSelection(outerRadius, center);
    qDebug() << "checkpoint is : " << checkPoint;
    QImage localadjselection = ladj.getSelection(innerRadius,outerRadius,center);
    localadjselection.save("kenzo test selection.png");
    return;
}

void MainWindow::on_pushButton_6_clicked()
{
    int x,i; //loop variables for x axis
    int y,j; //loop variables for y axis

    QImage img;
    img.load(ui->lineEdit->text());
    QImage myselection;
    myselection.load("kenzo test selection.png");
    QPoint center;
    center.setX(ui->lineEdit_2->text().toInt());
    center.setY(ui->lineEdit_3->text().toInt());
    int outerRadius=ui->lineEdit_5->text().toInt();

    //----For using class and object -----------------------------------------

    LocalAdjustments ladj(this);
    connect(&ladj, SIGNAL(signalProgress(int)),
                this, SLOT(slotProgress(int)));
    ladj.setImagePath(ui->lineEdit->text());
    QPoint selcCenter=ladj.centerSelection(outerRadius, center);
    QImage finalImage=ladj.applySelection(myselection, center , selcCenter);
    finalImage.save("kenzo final output.png");
    return;
}
