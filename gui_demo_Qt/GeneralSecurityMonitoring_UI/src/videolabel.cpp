#include "videolabel.h"

VideoLabel::VideoLabel(QWidget *parent)
{
   this->pointsGetFlag = false;
}

VideoLabel::~VideoLabel()
{

}

void VideoLabel::showImage(QImage image)
{
    QImage tmpImage;
//    if(image.width()*1.0/image.height() * this->height() <= this->width())
//    {
//        tmpImage = image.scaled(image.width()*1.0/image.height() * this->height(), this->height()).copy();
//    }
//    else if(image.height()*1.0/image.width() * this->width() <= this->height())
//    {
//        tmpImage = image.scaled(this->width(), image.height()*1.0/image.width() * this->width()).copy();
//    }
    if(!image.isNull())
    {
      tmpImage = image.scaled(this->width(), this->height());

    if(this->pointsGetFlag)
    {
        if(this->pointsMap.size() > 1)
        {
            QPainter paint(&tmpImage);
            QPen pen(Qt::red,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
            paint.setPen(pen);
            QPoint lastPoint;
            for(int i=0;i<this->pointsMap.size();i++)
            {
                foreach (QString canvasSizeStr, this->pointsMap[i].keys())
                {
                   QStringList canvasSizeStrList = canvasSizeStr.split(",");
                   QString wStr = canvasSizeStrList[0];
                   QString hStr = canvasSizeStrList[1];

                   int oldW = wStr.toInt();
                   int oldH = hStr.toInt();

                   int oldPointX = this->pointsMap[i][canvasSizeStr].x();
                   int oldPointY = this->pointsMap[i][canvasSizeStr].y();

                   int nowW = this->width();
                   int nowH = this->height();

                   int  nowPointX = oldPointX* nowW/oldW;
                   int  nowPointY = oldPointY* nowH/oldH;

                   if(!lastPoint.isNull())
                   {
                       QPen pen(Qt::red,3);
                       paint.setPen(pen);

                       paint.drawLine(lastPoint,QPoint(nowPointX,nowPointY));
                   }

                   lastPoint.setX(nowPointX);
                   lastPoint.setY(nowPointY);

                   QPen pen(Qt::green,6,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
                   paint.setPen(pen);
                   paint.drawPoint(lastPoint);
                }

            }
        }
    }
    else
    {
        if(this->contour.size() == 0)
        {
            QPainter paint(&tmpImage);
            QPen pen(Qt::green,3);
            paint.setPen(pen);
            QPoint lastPoint;
            QPoint firstPoint;
            for(int i=0;i<this->pointsMap.size();i++)
            {

                foreach (QString canvasSizeStr, this->pointsMap[i].keys())
                {
                    QStringList canvasSizeStrList = canvasSizeStr.split(",");
                    QString wStr = canvasSizeStrList[0];
                    QString hStr = canvasSizeStrList[1];

                    int oldW = wStr.toInt();
                    int oldH = hStr.toInt();

                    int oldPointX = this->pointsMap[i][canvasSizeStr].x();
                    int oldPointY = this->pointsMap[i][canvasSizeStr].y();

                    int nowW = this->width();
                    int nowH = this->height();

                    int  nowPointX = oldPointX* nowW/oldW;
                    int  nowPointY = oldPointY* nowH/oldH;

                    std::string str1 = "("+std::to_string(nowPointX)+" , "+std::to_string(nowPointY)+")";
                    std::string str2 = "W: "+std::to_string(nowW)+" H: "+std::to_string(nowH);
                    paint.drawText(nowPointX,nowPointY-10,QString::fromStdString(str1));
                    paint.drawText(30,30,QString::fromStdString(str2));
                    if(!lastPoint.isNull())
                    {
                        QPen pen(Qt::green,3);
                        paint.setPen(pen);

                        paint.drawLine(lastPoint,QPoint(nowPointX,nowPointY));
                    }
                    else
                    {
                        firstPoint.setX(nowPointX);
                        firstPoint.setY(nowPointY);
                    }

                    lastPoint.setX(nowPointX);
                    lastPoint.setY(nowPointY);
                }

            }
            paint.drawLine(lastPoint,firstPoint);
        }
    }
    QPixmap pix = QPixmap::fromImage(tmpImage);
    this->setPixmap(pix);
    this->update();
  }
}

void VideoLabel::showText(QString str)
{
    this->setText(str);
}
void VideoLabel::clearROI()
{
    this->pointsMap.clear();
}


void VideoLabel::mousePressEvent(QMouseEvent *ev)
{
    if(this->pointsGetFlag)
    {
       QMap<QString,QPoint> pointMap;
       QString canvasSizeStr = QString::number(this->width()) + QString(",") + QString::number(this->height());
       QPoint pointValue(ev->x(),ev->y());
       pointMap[canvasSizeStr] = pointValue;
       this->pointsMap.append(pointMap);

       // emit getPointMapSignal(pointsMap);
    }
}
bool VideoLabel::setEnablePointsGet()
{
    this->contour.clear();
    this->pointsGetFlag = true;
    return true;
}
bool VideoLabel::setDisablePointGet()
{
    if(this->pointsMap.size()>2)
    {
       this->pointsGetFlag = false;
       emit getPointMapSignal(pointsMap);
        return true;
    }
    else
    {
        QMessageBox::critical(NULL,"Tips","Points is not enough, please continue..");
        return false;
    }

}
QVector< QMap<QString, QPoint> > VideoLabel::getPointsMap() const
{
    return pointsMap;
}
