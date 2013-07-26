#ifndef LOCALADJUSTMENTSELECTION_H
#define LOCALADJUSTMENTSELECTION_H


//Qt includes

#include <QObject>
#include <QString>
#include <QImage>
#include <QPoint>

class LocalAdjustments : public QObject
{
   Q_OBJECT

public:
   explicit LocalAdjustments(QObject* const parent);
   ~LocalAdjustments();

   void setImagePath(const QString& path);

   QPoint centerSelection(int outerRadius, QPoint origCenter);
   //Returns a QPoint of the center of the selection.

   QImage getSelection(int innerRadius, int outerRadius, QPoint origCenter);
   //Returns the selection in RGBA32 format
   //QImage source is the image from which the selection is to be taken
   //innerRadius and OuterRadius are integers with radius of the selection
   //origCenter determines the point of the center of the circle

   QImage getSelection(int outerRadius, QPoint origCenter);
   //This is an overloaded function
   //Returns the selection in RGBA32 format
   //QImage source is the image from which the selection is to be taken
   //OuterRadius are integers with radius of the selection
   //InnerRadius is considered to be 70% of the outerRadius
   //origCenter determines the point of the center of the circle

   QImage applySelection(QImage selection, QPoint origCenter, QPoint selcCenter);

   QImage applySelection(QString path, QPoint origCenter, QPoint selcCenter);
   //overloaded function, provide path to layer, instead of layer QImage

Q_SIGNALS:

   void signalProgress(int);

private:

   void postProgress(int, const QString&);

   QImage getcolorSelection(QImage selection, QPoint selectionCenter);
   //returns the circular selection with color
   //QImage source is the original image from which the selection is to be made
   //int outerRadius is the human input value of Outer Radius (value >=0)
   //QPoint origCenter is the center in the source Image.

   QImage getSoftSelection(QImage source, int innerRadius, int outerRadius, QPoint origCenter);
   //returns the blurred/soft edged selection
   //QImage selection is the hard selection
   //int innerRadius determines the radius inside which alpha values are 255
   //int outerRadius determines the radius beyond which alpha values are 0
   //QPoint selectionCenter determines the center of the selection with respect to the original Image

   QImage createLayer(QImage selection, QPoint origCenter, QPoint selectionCenter);

   void srgb2lch(float fimg[][4], int size);

   void srgb2lab(float fimg[][4], int size);

   void srgb2xyz(float fimg[][4], int size);

   void xyz2srgb(float fimg[][4], int size);

   void lab2srgb(float fimg[][4], int size);

   void colorDifference(float fimg[][4], float reference[4], float* difference, int size);

private:

   class Private;
   Private* const d;

};

#endif // LOCALADJUSTMENTSELECTION_H
