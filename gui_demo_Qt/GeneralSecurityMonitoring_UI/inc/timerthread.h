#ifndef TIMERTHREAD_H
#define TIMERTHREAD_H
#include<QThread>
#include<QTimer>
#include<QTime>
class timerThread : public QThread
{
    Q_OBJECT

public:
    timerThread();
    ~timerThread();

protected:
    void run() ;

public slots:
    void slot_time() ;
signals:
    void sendtime() ;
private:
    QTimer *m_timer ;

};

#endif // TIMERTHREAD_H
