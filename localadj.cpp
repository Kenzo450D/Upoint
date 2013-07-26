//local includes

#include "localadj.h"

// C++ includes

#include <cmath>
#include <cfloat>

// Qt includes.

#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QPoint>
#include <QRect>
#include <QImage>
#include <QColor>
#include <QPainter>
#include <qmath.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPixmap>


class LocalAdjustments::Private
{
public:

    QString path;
    QString output;
};

LocalAdjustments::LocalAdjustments(QObject* const parent)
    : QObject(parent), d(new Private)
    {
    }

LocalAdjustments::~LocalAdjustments()
{
    delete d;
}

void LocalAdjustments::setImagePath(const QString& path)
{
    d->path=path;
}

//QString LocalAdjustments::output() const
//{
//    return d->output;
//}


void LocalAdjustments::postProgress(int p, const QString& t)
{
    qDebug() << "\n" << p << "% - " << t << "\n";
    emit signalProgress(p);
    qApp->processEvents();
}

QPoint LocalAdjustments::centerSelection(int outerRadius, QPoint origCenter)
{
    QImage img;
    img.load(d->path);
    if (outerRadius == 0)
    {
        return origCenter;
    }
    int leftlimit   = origCenter.x() - outerRadius;
    int rightlimit  = origCenter.x() + outerRadius - 1;
    int toplimit    = origCenter.y() - outerRadius;
    int bottomlimit = origCenter.y() + outerRadius - 1;
    QPoint sCenter;

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

    sCenter.setX(origCenter.x()-leftlimit);
    sCenter.setY(origCenter.y()-toplimit);
    qDebug() << "scenter X = " << sCenter.x();
    qDebug() << "scenter Y = " << sCenter.y();
    return sCenter;
}


QImage LocalAdjustments::getSelection(int innerRadius, int outerRadius, QPoint origCenter)
{
    QImage img;
    img.load(d->path);
    QImage selection;
//    selection=getbasicSelection(img,outerRadius,origCenter);
    selection=getSoftSelection(img,innerRadius,outerRadius,origCenter);
    selection=getcolorSelection(selection,centerSelection(outerRadius,origCenter));
    return selection;
}

QImage LocalAdjustments::getSelection(int outerRadius, QPoint origCenter)
{
    int innerRadius= (0.7 * outerRadius);
    return(getSelection(innerRadius, outerRadius, origCenter));
}

QImage LocalAdjustments::getSoftSelection(QImage source, int innerRadius, int outerRadius, QPoint origCenter)
{
    int leftlimit   = origCenter.x() - outerRadius;
    int rightlimit  = origCenter.x() + outerRadius - 1;
    int toplimit    = origCenter.y() - outerRadius;
    int bottomlimit = origCenter.y() + outerRadius - 1;
    QRect crop;
    QSize size;

    //---- Fixing out of bounds for the boudaries -----------------------------

    if (leftlimit < 0 )
    {
        leftlimit = 0;
    }
    if (rightlimit >= source.width())
    {
        rightlimit = source.width()-1;
    }
    if (toplimit < 0 )
    {
        toplimit = 0;
    }
    if ( bottomlimit >= source.height())
    {
        bottomlimit = source.height() -1;
    }

    //---- Setting up Crop ----------------------------------------------------

    QPoint p1;
    p1.setX(leftlimit);
    p1.setY(toplimit);
    QPoint p2;
    p2.setX(rightlimit);
    p2.setY(bottomlimit);
    crop.setTopLeft(p1);
    crop.setBottomRight(p2);

    //---- Fixing the size of the QSize size -----------------------------------

    size.setHeight(bottomlimit-toplimit+1);
    size.setWidth(rightlimit - leftlimit + 1);
    qDebug() << "Rect : "<<crop;

    //---- Copying the crop into mask -------------------------------------------

    QImage mask = QImage(size,QImage::Format_ARGB32);
    mask = source.copy(crop);
    mask = mask.convertToFormat(QImage::Format_ARGB32);

    //---- Saving the mask for check --------------------------------------------

    mask.save("localadj maskedimage.jpg");

    QPoint selectionCenter=centerSelection(outerRadius,origCenter);

    //---- Make the circular selection ------------------------------------------

    int centerx=selectionCenter.x();
    int centery=selectionCenter.y();
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
            }
            else
            {
                mask.setPixel(j,i,QColor(r,g,b,0).rgba());
            }
        }
    }
    bool check = mask.save("localadj Circular.png");
    Q_ASSERT(check);
    return mask;
}

QImage LocalAdjustments::getcolorSelection(QImage selection, QPoint selectionCenter)
{
    //here we consider the color of the point of the selectionCenter and we proceed to make
    //the color baised selection.
    int sz = selection.height() * selection.width();
    float fimg[sz][4];
    QColor c;
    int x = 0;
    int i = 0;
    int j = 0;
    for (i=0;i<selection.height(); i++)
    {
        for(j=0;j<selection.width();j++)
        {
            c=QColor::fromRgba(selection.pixel(j,i));
            fimg[x][0]=c.redF();
            fimg[x][1]=c.blueF();
            fimg[x][2]=c.greenF();
            fimg[x][3]=c.alphaF();
            x++;
        }
    }
    qDebug() << "LocalAdjustments : x = " << x << " size = "<< sz;
    //-----We convert the whole image to CIELAB
    srgb2lab(fimg,sz);
    qDebug() << "After conversion:";
    //-----Consider the pixel color to be compared with
    //Here we consider the central pixel, i.e. center, we take the color of those pixels
    float reference[4];
    c=QColor::fromRgba(selection.pixel(selectionCenter));
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
//    QFile file3("/home/sayantan/WORK/samples/difference.txt");
//    file3.open(QIODevice::WriteOnly | QIODevice::Text);
//    QTextStream outdiff(&file3);
//    for(j=0; j<sz; j++)
//    {
//        outdiff << difference[j] << "\n";
//        //qDebug() << difference[j];
//    }
//    file3.close();
//    qDebug() << "Differences noted!";
    lab2srgb(fimg,sz);
    x=0;
    for (i=0;i<selection.height(); i++)
    {
        for(j=0;j<selection.width();j++)
        {
            c=QColor::fromRgba(selection.pixel(j,i));
            if (difference[x]<0.2)
            {
                selection.setPixel(j,i,QColor(((int)(fimg[x][0]*255)),((int)(fimg[x][1]*255)),((int)(fimg[x][2]*255)),(fimg[x][3]*255*(0.2-difference[x])*5)).rgba());
            }
            else
            {
                selection.setPixel(j,i,QColor(((int)(fimg[x][0]*255)),((int)(fimg[x][1]*255)),((int)(fimg[x][2]*255)),0).rgba());
            }
            x++;
        }
    }
    bool istrue=selection.save("Localadj Colored Circular.png");
    qDebug() << "Check for good save : " << istrue;
    return selection;
}

QImage LocalAdjustments::createLayer(QImage selection, QPoint origCenter, QPoint selectionCenter)
{
    QImage img;
    img.load(d->path);
    QImage layer;
    layer=QImage(img.size(),QImage::Format_ARGB32);
    int i;
    int j;
    int x;
    int y;
    //initialize layer
    for( x=0; x<layer.width(); x++)
    {
        for ( y=0; y<layer.height(); y++)
        {
            layer.setPixel(x,y,qRgba(0,0,0,0));
        }
    }
    int leftlimit;
    int rightlimit;
    int toplimit;
    int bottomlimit;
    leftlimit = origCenter.x() - selectionCenter.x();
    toplimit  = origCenter.y() - selectionCenter.y();
    rightlimit = leftlimit + selection.width();
    bottomlimit = toplimit + selection.height();

    for(x=leftlimit, i=0 ; x < rightlimit ; x++, i++ )
    {
        for (y=toplimit, j=0 ; y < bottomlimit ; y++, j++)
        {
            layer.setPixel(x,y,selection.pixel(i,j));
        }
    }
    return layer;
}

void LocalAdjustments::srgb2lch(float fimg[][4], int size)
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

void LocalAdjustments::srgb2xyz(float fimg[][4], int size)
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

void LocalAdjustments::srgb2lab(float fimg[][4], int size)
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

void LocalAdjustments::xyz2srgb(float fimg[][4], int size)
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

void LocalAdjustments::lab2srgb(float fimg[][4], int size)
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

void LocalAdjustments::colorDifference(float fimg[][4], float reference[4], float* difference, int size)
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


QImage LocalAdjustments::applySelection(QImage selection, QPoint origCenter, QPoint selcCenter)
{
    //now we create a layer from the selection, and paste the layer on the image
    QImage layer=createLayer(selection, origCenter, selcCenter);
    QImage img;
    img.load(d->path);
    QPainter painter(&img);
    QPointF point;
    point.setX(0);
    point.setY(0);
    painter.drawImage(point,layer);
    return img;
}

QImage LocalAdjustments::applySelection(QString path, QPoint origCenter, QPoint selcCenter) {
    //we load the image, and send it to the QImage applySelection(QImage layer, QPoint origCenter, QPoint selcCenter)
    QImage img;
    img.load(path);
    return(applySelection(img,origCenter,selcCenter));
}
