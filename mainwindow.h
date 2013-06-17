#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QGraphicsScene* scene = new QGraphicsScene();
    QGraphicsPixmapItem item;
//    QColor sRGBtoLch(QColor);

    void srgb2lch(float fimg[][4], int size);

    void srgb2lab(float fimg[][4], int size);

    void srgb2xyz(float fimg[][4], int size);

    void xyz2srgb(float fimg[][4], int size);

    void lab2srgb(float fimg[][4], int size);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

    void checkLineEdits();

    void checkLineRadius();

    void on_pushButton_3_clicked();

    void on_pushButton_5_clicked();

private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
