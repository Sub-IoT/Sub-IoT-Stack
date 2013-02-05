#ifndef CLILOGGER_H
#define CLILOGGER_H

#include <QtCore>

class CliLogger : public QObject
{
    Q_OBJECT

public:
    explicit CliLogger(QObject *parent = 0);
    
signals:
    
private slots:
    void onMessageReceived(QString);
    
private:
    bool isSerialPort(QString arg) const;
    QString serialErrorString() const;

    QIODevice* _ioDevice;
};

#endif // CLILOGGER_H
