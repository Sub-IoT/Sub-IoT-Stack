#include "connectdialog.h"
#include "ui_connectdialog.h"

#include <QFileDialog>
#include <QDebug>

ConnectDialog::ConnectDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);

    ui->connectionTypeComboBox->insertItem(0, "Serial port");
    ui->connectionTypeComboBox->insertItem(0, "Captured data file");

    detectSerialPorts();

    _connectionType = Serial;
    ui->stackedWidget->setCurrentWidget(ui->serialPortSettings);

    connect(ui->connectionTypeComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(onConnectionTypeChanged(QString)));
    connect(ui->selectFileButton, SIGNAL(pressed()), SLOT(onSelectFileButtonPressed()));
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

void ConnectDialog::onConnectionTypeChanged(QString connectionName)
{
    _connectionType = parseConnectionName(connectionName);
    switch(_connectionType)
    {
        case Serial:
            ui->stackedWidget->setCurrentWidget(ui->serialPortSettings);
            break;
        case File:
            ui->stackedWidget->setCurrentWidget(ui->fileSettings);
            break;
    }
}

void ConnectDialog::onSelectFileButtonPressed()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open binary file");
    ui->fileNameLineEdit->setText(fileName);
}

ConnectionType ConnectDialog::connectionType() const
{
    return _connectionType;
}

ConnectionType ConnectDialog::parseConnectionName(QString connectionName)
{
    if(connectionName == "Serial port")
        return Serial;
    if(connectionName == "Captured data file")
        return File;
}

void ConnectDialog::detectSerialPorts()
{
    _serialPorts = QSerialPortInfo::availablePorts();
    qDebug() << "Number of serial ports found: " << _serialPorts.count();
    for (int i = 0; i < _serialPorts.count(); i++)
    {
        const QSerialPortInfo &info = _serialPorts.at(i);
        QString s(QObject::tr("Port: %1\n"
                              "Location: %2\n"
                              "Description: %3\n"
                              "Manufacturer: %4\n"
                              "Vendor Identifier: %5\n"
                              "Product Identifier: %6\n"));

        s = s.arg(info.portName()).arg(info.systemLocation())
                .arg(info.description()).arg(info.manufacturer())
                .arg(info.vendorIdentifier()).arg(info.productIdentifier());

        qDebug() << s;
        ui->serialPortComboBox->insertItem(i, info.portName());
    }
}

QSerialPortInfo ConnectDialog::serialPortInfo() const
{
    QString serialPortName = ui->serialPortComboBox->currentText();
    for (int i = 0; i < _serialPorts.count(); i++)
    {
        const QSerialPortInfo &info = _serialPorts.at(i);
        if(info.portName() == serialPortName)
            return info;
    }
}

QString ConnectDialog::fileName() const
{
    return ui->fileNameLineEdit->text();
}
