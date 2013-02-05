#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>

#include <serialport.h>
#include <serialportinfo.h>

QT_USE_NAMESPACE_SERIALPORT

namespace Ui {
class ConnectDialog;
}

enum ConnectionType
{
    Serial,
    File,
    NoConnection
};

class ConnectDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConnectDialog(QWidget *parent = 0);
    ~ConnectDialog();
    void detectSerialPorts();
    ConnectionType connectionType() const;
    QString serialPortName() const;
    QString fileName() const;

private slots:
    void onConnectionTypeChanged(QString);
    void onSelectFileButtonPressed();

private:
    ConnectionType parseConnectionName(QString connectionName);

    Ui::ConnectDialog *ui;
    QList<SerialPortInfo> _serialPorts;
    ConnectionType _connectionType;
};

#endif // CONNECTDIALOG_H
