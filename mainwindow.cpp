#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    QImage img;
    img.load(ui->lineEdit->text());
    QPoint center;
    center.setX(ui->lineEdit_2->text().toInt());
    center.setY(ui->lineEdit_3->text().toInt());
    //-----Task 1 --- Finding size of mask ------------------------------------
    int leftlimit = center.x() - outerRadius;
    int rightlimit = center.x() + outerRadius -1;
    int toplimit = center.y() - outerRadius;
    int bottomlimit = center.y() + outerRadius -1;
    //-----Check if borders exceed image boundaries
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

    //---- Preparing QRect for the crop ---------------------------------------

    QRect crop;
    QPoint p1;
    p1.setX(leftlimit);
    p1.setY(toplimit);
    QPoint p2;
    p2.setX(rightlimit);
    p2.setY(bottomlimit);
    crop.setTopLeft(p1);
    crop.setBottomRight(p2);

    //---- Selection Coordinates-----------------------------------------------

    int sCenterX=center.x()-leftlimit;
    int sCenterY=center.y()-toplimit;
    ui->label_9->setText(QString::number(sCenterX));
    ui->label_10->setText(QString::number(sCenterY));

    //---- Making the QImage Mask ---------------------------------------------
    QImage mask;
    QSize size;
    size.setHeight(bottomlimit-toplimit+1);
    size.setWidth(rightlimit - leftlimit + 1);
    qDebug() << "Rect : "<<crop;
    mask = QImage(size,QImage::Format_ARGB32);
    qDebug() << "The mask says that i am format of : " << mask.format();
    mask = img.copy(crop);
    mask = mask.convertToFormat(QImage::Format_ARGB32);
    qDebug() << "After copy that i am format of : " << mask.format();
    mask.save("maskedImage.jpg",0,-1);

    //-----Task 2 -- Making the crop Circular ---------------------------------

    //To make the crop circular, we first make Alpha = 0, for all points out of
    //the boundary. For the rest, we work on specific values of alpha.
    int centerx=sCenterX;
    int centery=sCenterY;
    int r,g,b;
    int i,j;
    float a;
    QColor c;
    float distance;
    float diffRadius = outerRadius - innerRadius;
    //outerRadius=outerRadius*outerRadius;
    //innerRadius=innerRadius*innerRadius;
    qDebug() << "outerRadius = "<< outerRadius;
    for (i=0;i<mask.height(); i++)
    {
        for (j=0;j<mask.width(); j++)
        {
            c=QColor::fromRgba(mask.pixel(j,i));
            r=c.red();
            g=c.green();
            b=c.blue();
            a=c.alpha();
            //if distance > outer radius, make alpha = zero
            distance=(i-centery)*(i-centery)+(j-centerx)*(j-centerx);
            distance=qSqrt(distance);
            if (distance<outerRadius)
            {
                if (distance<=innerRadius)
                {
                    mask.setPixel(j,i,QColor(r,g,b,255).rgba());
                }
                else
                {
                    a=(diffRadius-(distance-innerRadius))/diffRadius;
                    a=a*255;
                    //**** UNCOMMENT FOR CHECK (next line) ***
//                    qDebug() << "Alpha Value = " << a;
                    mask.setPixel(j,i,QColor(r,g,b,((int)(qFloor(a)))).rgba());
                }
//                a=(diffRadius-(distance-innerRadius))/diffRadius;
//                a=a*255;
//                qDebug() << "Alpha Value = " << a;
            }
            else
            {
                mask.setPixel(j,i,QColor(r,g,b,0).rgba());
            }
            //qDebug() << "Values: ( "<< i <<", "<< j <<") Alpha " << a << "Distance " << distance ;
            //qDebug() << "Color : " << QColor::fromRgba(mask.pixel(j,i));
        }
    }
    bool check = mask.save("Circular.png");
    Q_ASSERT(check);
    //----------Color Difference
    //-----Convert selection to float
    int sz = mask.height() * mask.width();
    float fimg[sz][4];
    int x=0;
    x=0;
    for (i=0;i<mask.height(); i++)
    {
        for(j=0;j<mask.width();j++)
        {
            c=QColor::fromRgba(mask.pixel(j,i));
            fimg[x][0]=c.redF();
            fimg[x][1]=c.blueF();
            fimg[x][2]=c.greenF();
            fimg[x][3]=c.alphaF();
            x++;
        }
    }
    qDebug() << "x = " << x << " size = "<< sz;
    QFile file("/home/sayantan/WORK/samples/srgb.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    for(j=0; j<sz; j++)
    {
        out << fimg[j][0] << "\t" << fimg[j][1] << "\t" << fimg[j][2] << "\t" << fimg[j][3]<<"\n";
    }
    file.close();
    qDebug() << "That was the end of the test!";
    //-----We convert the whole image to CIELAB
//    return;
    srgb2lab(fimg,sz);
    qDebug() << "After conversion:";
    QFile file2("/home/sayantan/WORK/samples/lab.txt");
    file2.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream outlch(&file2);
    for(j=0; j<sz; j++)
    {
        outlch << fimg[j][0] << "\t" << fimg[j][1] << "\t" << fimg[j][2] << "\t" << fimg[j][3]<<"\n";
    }
    file2.close();
    qDebug() << "That was the converted data!";
    //-----Consider the pixel color to be compared with
    //Here we consider the central pixel, i.e. center, we take the color of those pixels
    float reference[4];
    c=QColor::fromRgba(img.pixel(center));
    float difference[sz];
    float ref[1][4];
    ref[0][0]=c.redF();
    ref[0][1]=c.greenF();
    ref[0][2]=c.blueF();
    ref[0][3]=c.alphaF();
    qDebug() << "ref srgb";
    qDebug() << ref[0][0] << "\t" << ref[0][1] << "\t" << ref[0][2] << "\t" << ref[0][3];
    srgb2lab(ref,1);
    qDebug() << "ref lab";
    qDebug() << ref[0][0] << "\t" << ref[0][1] << "\t" << ref[0][2] << "\t" << ref[0][3];
    reference[0]=ref[0][0];
    reference[1]=ref[0][1];
    reference[2]=ref[0][2];
    reference[3]=ref[0][3];
    qDebug() << reference[0] << "\t" << reference[1] << "\t" << reference[2];
    colorDifference(fimg,reference,difference,sz);
    QFile file3("/home/sayantan/WORK/samples/difference.txt");
    file3.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream outdiff(&file3);
    for(j=0; j<sz; j++)
    {
        outdiff << difference[j] << "\n";
        //qDebug() << difference[j];
    }
    file3.close();
    qDebug() << "Differences noted!";
    lab2srgb(fimg,sz);
    x=0;
    for (i=0;i<mask.height(); i++)
    {
        for(j=0;j<mask.width();j++)
        {
            c=QColor::fromRgba(mask.pixel(j,i));
            if (difference[x]<0.2)
            {
                mask.setPixel(j,i,QColor(((int)(fimg[x][0]*255)),((int)(fimg[x][1]*255)),((int)(fimg[x][2]*255)),(fimg[x][3]*255*(0.2-difference[x])*5)).rgba());
            }
            else
            {
                mask.setPixel(j,i,QColor(((int)(fimg[x][0]*255)),((int)(fimg[x][1]*255)),((int)(fimg[x][2]*255)),0).rgba());
            }
            x++;
        }
    }
    bool istrue=mask.save("Colored Circular.png");
    qDebug() << "Check for good save : " << istrue;
    return;
}

void MainWindow::srgb2lch(float fimg[][4], int size)
{
    float c, h;
    int i;
    srgb2lab(fimg,size);

    QFile file("/home/sayantan/WORK/samples/lab.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    for (i=0; i<size; i++)
    {
        out << fimg[i][0] << "\t" << fimg[i][1] << "\t" << fimg[i][2] << "\t" << fimg[i][3]<<"\n";
    }
    file.close();
    for(int i=0; i<size; i++)
    {
        c=qSqrt((fimg[i][1]*fimg[i][1]) + (fimg[i][2]*fimg[i][2])); //chroma
        h=qAtan(fimg[i][2]/fimg[i][1]);                             //hue
        fimg[i][1]=c;
        fimg[i][2]=h;
    }
}

void MainWindow::srgb2xyz(float fimg[][4], int size)
{
    // fimg in [0:1], sRGB
    float x, y, z;

    for (int i = 0; i < size; ++i)
    {
        // scaling and gamma correction (approximate)
        fimg[i][0] = pow(fimg[i][0], (float)2.2);
        fimg[i][1] = pow(fimg[i][1], (float)2.2);
        fimg[i][2] = pow(fimg[i][2], (float)2.2);

        // matrix RGB -> XYZ, with D65 reference white (www.brucelindbloom.com)
        x = 0.412424  * fimg[i][0] + 0.357579 * fimg[i][1] + 0.180464  * fimg[i][2];
        y = 0.212656  * fimg[i][0] + 0.715158 * fimg[i][1] + 0.0721856 * fimg[i][2];
        z = 0.0193324 * fimg[i][0] + 0.119193 * fimg[i][1] + 0.950444  * fimg[i][2];

//      x = 0.412424 * fimg[i][0] + 0.212656  * fimg[i][1] + 0.0193324 * fimg[i][2];
//      y = 0.357579 * fimg[i][0] + 0.715158  * fimg[i][1] + 0.119193  * fimg[i][2];
//      z = 0.180464 * fimg[i][0] + 0.0721856 * fimg[i][1] + 0.950444  * fimg[i][2];

        fimg[i][0] = x;
        fimg[i][1] = y;
        fimg[i][2] = z;
    }
}

void MainWindow::srgb2lab(float fimg[][4], int size)
{
    float l, a, b;

    srgb2xyz(fimg, size);

    for (int i = 0; i < size; ++i)
    {
        // reference white
        fimg[i][0] /= 0.95047F;

        //fimg[i][1] /= 1.00000;          // (just for completeness)

        fimg[i][2] /= 1.08883F;

        // scale
        if (fimg[i][0] > 216.0 / 24389.0)
        {
            fimg[i][0] = pow(fimg[i][0], (float)(1.0 / 3.0));
        }
        else
        {
            fimg[i][0] = (24389.0 * fimg[i][0] / 27.0 + 16.0) / 116.0;
        }

        if (fimg[i][1] > 216.0 / 24389.0)
        {
            fimg[i][1] = pow(fimg[i][1], (float)(1.0 / 3.0));
        }
        else
        {
            fimg[i][1] = (24389 * fimg[i][1] / 27.0 + 16.0) / 116.0;
        }

        if (fimg[i][2] > 216.0 / 24389.0)
        {
            fimg[i][2] = (float)pow(fimg[i][2], (float)(1.0 / 3.0));
        }
        else
        {
            fimg[i][2] = (24389.0 * fimg[i][2] / 27.0 + 16.0) / 116.0;
        }

        l          = 116 * fimg[i][1]  - 16;
        a          = 500 * (fimg[i][0] - fimg[i][1]);
        b          = 200 * (fimg[i][1] - fimg[i][2]);
        fimg[i][0] = l / 116.0; // + 16 * 27 / 24389.0;
        fimg[i][1] = a / 500.0 / 2.0 + 0.5;
        fimg[i][2] = b / 200.0 / 2.2 + 0.5;

        if (fimg[i][0] < 0)
        {
            fimg[i][0] = 0;
        }
    }
}

void MainWindow::xyz2srgb(float fimg[][4], int size)
{
    float r, g, b;

    for (int i = 0; i < size; ++i)
    {
        // matrix RGB -> XYZ, with D65 reference white (www.brucelindbloom.com)
        r = 3.24071   * fimg[i][0] - 1.53726  * fimg[i][1] - 0.498571  * fimg[i][2];
        g = -0.969258 * fimg[i][0] + 1.87599  * fimg[i][1] + 0.0415557 * fimg[i][2];
        b = 0.0556352 * fimg[i][0] - 0.203996 * fimg[i][1] + 1.05707   * fimg[i][2];


//      r =  3.24071  * fimg[i][0] - 0.969258  * fimg[i][1]
//           + 0.0556352 * fimg[i][2];
//      g = -1.53726  * fimg[i][0] + 1.87599   * fimg[i][1]
//           - 0.203996  * fimg[i][2];
//      b = -0.498571 * fimg[i][0] + 0.0415557 * fimg[i][1]
//           + 1.05707   * fimg[i][2];

        // scaling and gamma correction (approximate)
        r = r < 0 ? 0 : pow(r, (float)(1.0 / 2.2));
        g = g < 0 ? 0 : pow(g, (float)(1.0 / 2.2));
        b = b < 0 ? 0 : pow(b, (float)(1.0 / 2.2));

        fimg[i][0] = r;
        fimg[i][1] = g;
        fimg[i][2] = b;
    }
}

void MainWindow::lab2srgb(float fimg[][4], int size)
{
    float x, y, z;

    for (int i = 0; i < size; ++i)
    {
        // convert back to normal LAB
        fimg[i][0] = (fimg[i][0] - 0 * 16 * 27 / 24389.0) * 116;
        fimg[i][1] = (fimg[i][1] - 0.5) * 500 * 2;
        fimg[i][2] = (fimg[i][2] - 0.5) * 200 * 2.2;

        // matrix
        y = (fimg[i][0] + 16) / 116;
        z = y - fimg[i][2] / 200.0;
        x = fimg[i][1] / 500.0 + y;

        // scale
        if (x * x * x > 216 / 24389.0)
        {
            x = x * x * x;
        }
        else
        {
            x = (116 * x - 16) * 27 / 24389.0;
        }

        if (fimg[i][0] > 216 / 27.0)
        {
            y = y * y * y;
        }
        else
        {
             //y = fimg[i][0] * 27 / 24389.0;
            y = (116 * y - 16) * 27 / 24389.0;
        }

        if (z * z * z > 216 / 24389.0)
        {
            z = z * z * z;
        }
        else
        {
            z = (116 * z - 16) * 27 / 24389.0;
        }

        // white reference
        fimg[i][0] = x * 0.95047;
        fimg[i][1] = y;
        fimg[i][2] = z * 1.08883;
    }

    xyz2srgb(fimg, size);
}

void MainWindow::colorDifference(float fimg[][4], float reference[4], float* difference, int size)
{
    //This code will be on the function of CIE94
    //We will test it on the basis that 
    //wiki link: http://en.wikipedia.org/wiki/Color_difference#CIE94
    //stackoverflow link: http://stackoverflow.com/questions/6630599/are-there-known-implementations-of-the-ciede2000-or-cie94-delta-e-color-differen
    
    //fimg contains all the pixels, so for each of the pixel, we consider that the color of the pixel is Lab and not Lch, we will calculate the rest of the data from this Lab. :) If so required, we would also convert from Lab to Lch
    float dl       = 0;   //delta L (Lightness)
    float cab       = 0;
    float c1       = 0;   //chroma 1
    float c2       = 0;   //chroma 2
    float hab      = 0;  //Hypotenuse
    float dab      = 0;  //delta Lab
    float deltaE   = 0;
    const float kl = 1;
    const float k1 = 0.045;
    const float k2 = 0.015;
    float sl       = 1;
    float sc       = 1;
    float sh       = 1;
    int i;
    for(i=0; i<size; i++)
    {
        dl = reference[0] - fimg[i][0];
        c2 = qSqrt(fimg[i][1]*fimg[i][1] + fimg[i][2]*fimg[i][2]);
        c1 = qSqrt(reference[1]*reference[1]+reference[2]*reference[2]);
        cab = c1 - c2;
        //dab = qSqrt(qPow((reference[0] - fimg[i][0]),2) + qPow((reference[1] - fimg[i][1]),2) + qPow((reference[2] - fimg[i][2]),2));
        hab = qSqrt(qPow((reference[1] - fimg[i][1]),2) + qPow((reference[2] - fimg[i][2]),2) - qPow(cab,2));
        sc = 1 +k1*c1;
        sh = 1 +k2*c1;
        deltaE = qSqrt(qPow( ( dl / (kl*sl ) ), 2 ) + qPow((cab / sc), 2) + qPow((hab / sh), 2));
        difference[i]=deltaE;
    }
}

void MainWindow::on_pushButton_6_clicked()
{
    int x,i; //loop variables for x axis
    int y,j; //loop variables for y axis

    QImage img;
    img.load(ui->lineEdit->text());
//    QPainter paint(this);
//    QRectF target(0,0,img.height(),img.width());
//    paint.drawImage(target, img, target);
//    top.load("Colored Circular.png");
//    paint.drawImage(target,top,load);

    //---- Making the new layer as the same size of the old image -------------

    QImage layer;
    layer=QImage(img.size(),QImage::Format_ARGB32);
    //layer is not initialized, contains garbage data
    QImage selection;
    selection.load("Colored Circular.png");
//    return;

    //Copy the selection to the layer, in the correct position.
    //Position is determined by the Main Center Coordinates
    //and the position of the center of the selection

    //Initialize variables
    int origCenterX=ui->lineEdit_2->text().toInt();
    int origCenterY=ui->lineEdit_3->text().toInt();
    int selectionCenterX=ui->label_9->text().toInt();
    int selectionCenterY=ui->label_10->text().toInt();
    int outerRadius=ui->lineEdit_5->text().toInt();
//    int leftlimit = origCenterX - outerRadius;
//    int rightlimit = origCenterX + outerRadius -1;
//    int toplimit = origCenterY - outerRadius;
//    int bottomlimit = origCenterY + outerRadius -1;
    int leftlimit;
    int rightlimit;
    int toplimit;
    int bottomlimit;
    //-----Check if borders exceed image boundaries
//    if (leftlimit < 0 )
//    {
//        leftlimit = 0;
//    }
//    if (rightlimit >= img.width())
//    {
//        rightlimit = img.width()-1;
//    }
//    if (toplimit < 0 )
//    {
//        toplimit = 0;
//    }
//    if ( bottomlimit >= img.height())
//    {
//        bottomlimit = img.height() -1;
//    }

    //we do not need check border, but we can directly write down
    leftlimit = origCenterX - selectionCenterX;
    toplimit  = origCenterY - selectionCenterY;
    rightlimit = leftlimit + selection.width();
    bottomlimit = toplimit + selection.height();

    //Initializing QImage layer with transparent fill

    for(i=0;i<layer.width();i++)
    {
        for (j=0;j<layer.height();j++)
        {
            layer.setPixel(i,j,qRgba(0,0,0,0));
        }
    }

    //test layer
    layer.save("humans rule.png");

    //check parameters
    qDebug() << "left limit  = " << leftlimit;
    qDebug() << "right limit = " << rightlimit;
    qDebug() << "top limit   = " << toplimit;
    qDebug() << "bottom limit = " << bottomlimit;

//    return;
    //The origCenter and selectionCenter should be on the same place on Image layer
    for(x=leftlimit, i=0 ; x < rightlimit ; x++, i++ )
    {
        for (y=toplimit, j=0 ; y < bottomlimit ; y++, j++)
        {
            layer.setPixel(x,y,selection.pixel(i,j));
        }
    }

    //Check

    qDebug() << "Made the layer, saving layer to file, for check";
    layer.save("layer.png");

    //---- Making a pixmap from the initial image -----------------------------
    QPixmap base;
    base=QPixmap::fromImage(img);
    QPainter painter(&img);
    QPixmap pixlayer;
    pixlayer=QPixmap::fromImage(layer);
    QPointF point;
    point.setX(0);
    point.setY(0);
    painter.drawImage(point,layer);
//    painter.drawImage(&point,&layer,&layer.rect(), Qt::ImageConversionFlags flags = Qt::AutoColor);
//    painter.drawImage(0,0,&layer,0,0,-1,-1,Qt::ImageConversionFlag=AutoColor);
    img.save("finalresult.png");
//    painter.drawPixmap(0,0,img.width(),img.height(),&pixlayer);



    //---- Painting new pixmap over the old pixmap-----------------------------

    //---- Converting Pixmap back into QImage ---------------------------------

    //---- Save the QImage ----------------------------------------------------
}
