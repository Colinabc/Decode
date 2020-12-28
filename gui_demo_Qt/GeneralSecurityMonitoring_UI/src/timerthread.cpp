#include "timerthread.h"

timerThread::timerThread()
{

}
timerThread::~timerThread()
{

}
void timerThread::run()
{
    m_timer = new QTimer ;
    connect(m_timer , SIGNAL(timeout()) , this , SLOT(slot_time())) ;
    m_timer->start(6000) ;
}

void timerThread::slot_time()
{
    //QTime mytime = QTime::currentTime() ;
    emit sendtime() ;
}
