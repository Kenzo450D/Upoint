#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);
    ui->pushButton_5->setEnabled(false);
    ui->lineEdit_2->setEnabled(false);
    ui->lineEdit_3->setEnabled(false);
    ui->lineEdit_4->setEnabled(false);
    ui->lineEdit_5->setEnabled(false);
    ui->graphicsView->show();
    this->setFixedSize(760,530);
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
    bool ok = !ui->lineEdit_2->text().isEmpty()
            && !ui->lineEdit_3->text().isEmpty()
            && !ui->lineEdit_4->text().isEmpty()
            && !ui->lineEdit_5->text().isEmpty();
    ui->pushButton_5->setEnabled(ok);
}

void MainWindow::checkLineRadius()
{
    bool ok = !ui->lineEdit_2->text().isEmpty()
            && !ui->lineEdit_3->text().isEmpty();
    ui->pushButton_3->setEnabled(ok);
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
    QImage img;
    img.load(ui->lineEdit->text());
    QPoint center;
    center.setX(ui->lineEdit_2->text().toInt());
    center.setY(ui->lineEdit_3->text().toInt());
    //-----Task 1 --- Finding size of mask ----------------------------
    int leftlimit = center.x() - outerRadius;
    int rightlimit = center.x() + outerRadius -1;
    int toplimit = center.y() - outerRadius;
    int bottomlimit = center.y() + outerRadius -1;
    if (leftlimit < 0 )
    {
        leftlimit = 0;
    }
    if (rightlimit >= img.width())
    {
        rightlimit = img.width()-1;
    }
    if (toplimit < 0 )
    {
        toplimit = 0;
    }
    if ( bottomlimit >= img.height())
    {
        bottomlimit = img.height() -1;
    }
    QRect crop;
    QPoint p1;
    p1.setX(leftlimit);
    p1.setY(toplimit);
    QPoint p2;
    p2.setX(rightlimit);
    p2.setY(bottomlimit);
    crop.setTopLeft(p1);
    crop.setBottomRight(p2);
    QImage mask;
    QSize size;
    size.setHeight(bottomlimit-toplimit+1);
    size.setWidth(rightlimit - leftlimit + 1);
    qDebug() << "Rect : "<<crop;
    mask = QImage(size,QImage::Format_ARGB32);
    mask = img.copy(crop);
    mask.save("maskedImage.jpg",0,-1);
}
