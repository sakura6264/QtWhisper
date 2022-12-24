#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QProcess>

class Worker : public QThread
{
    Q_OBJECT
public:
    explicit Worker(QObject * parent = nullptr);
    void setParams(QString filepath,QString modelpath, QString language,QString outputtype,bool translate,bool fast,int processors,int threads);
    void run() override;
signals:
    void raise(QString ms);
    void report(QString ms);
    void done(QString result);


private:
    QString filepath;
    QString modelpath;
    QString language;
    QString outputtype;
    bool translate;
    bool fast;
    int processors;
    int threads;
};

#endif // WORKER_H
