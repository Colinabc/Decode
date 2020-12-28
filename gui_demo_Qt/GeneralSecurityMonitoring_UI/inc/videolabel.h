#ifndef VIDEOLABEL_H
#define VIDEOLABEL_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include<QMap>
#include<QPoint>
#include<QPainter>
#include<QPen>
#include<QMouseEvent>
#include<QMessageBox>
#include "opencv2/opencv.hpp"
#include <unistd.h>

class VideoLabel : public QLabel
{
    Q_OBJECT

public:
    explicit VideoLabel(QWidget* parent = Q_NULLPTR);
    ~VideoLabel();

    QVector< QMap<QString, QPoint> >getPointsMap() const;

    bool setEnablePointsGet();
    bool setDisablePointGet();
    void clearROI();

signals:
     void getPointMapSignal(QVector< QMap<QString,QPoint> >);
public slots:
    void showImage(QImage image);
    void showText(QString str);

    void mousePressEvent(QMouseEvent* ev);

private:
   QVector< QMap<QString,QPoint> > pointsMap;
   bool pointsGetFlag;
   std::vector<cv::Point>contour;

};



#endif // VIDEOLABEL_H
