#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, ".");
    settings = new QSettings("MPS300.ini", QSettings::IniFormat);
    modbus = new QModbusTcpClient();

    modbusSlaveID = 5;
    connect(modbus, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        statusBar()->showMessage(modbus->errorString(), 5000);
    });
    if (!modbus) {
        statusBar()->showMessage(tr("Could not create Modbus master."), 5000);
    }
    modbus->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "192.168.0.82");//settings->value("MKON_IP", "192.168.1.99"));
    modbus->setConnectionParameter(QModbusDevice::NetworkPortParameter, "502");
    modbus->setTimeout(3000);
    if (!modbus->connectDevice()) {
        statusBar()->showMessage(tr("Connect failed: ") + modbus->errorString(), 5000);
        qDebug() << modbus->connectionParameter(QModbusDevice::NetworkAddressParameter);
        qDebug() << " modbus->connectDevice failed" + modbus->errorString();
    }
    voltageMB = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 2816, 2);
    currentMB = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 2818, 2);
    inputVMB = new QModbusDataUnit(QModbusDataUnit::Coils, 1296, 1);
    outputMB = new QModbusDataUnit(QModbusDataUnit::Coils, 1298, 1);
    overheatMB = new QModbusDataUnit(QModbusDataUnit::Coils, 1297, 1);
    overheatMBQ1 = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 51, 2);

    connect(this, SIGNAL(readFinished(QModbusReply*, int)), this, SLOT(onReadReady(QModbusReply*, int)));
    readLoopTimer = new QTimer(this);
    connect(readLoopTimer, SIGNAL(timeout()), this, SLOT(readLoop()));
    readLoopTimer->start(1000);
    onPal.setColor(QPalette::WindowText, QColor("#ef5350"));
    offPal.setColor(QPalette::WindowText, QColor("#66bb6a"));
//    ui->overheatLabel->setPalette(onPal);
//    ui->inputVlabel->setPalette(onPal);
//    ui->outputVlabel->setPalette(onPal);
//    ui->connectionLabel->setPalette(onPal);
//    ui->overheatLabelQ1->setPalette(onPal);
//    ui->setCurrentSpinBox->setValue(settings->value("Iset", 0.0).toFloat());
//    ui->startButton->setEnabled(false);
}

void MainWindow::readLoop(){
    if (modbus->errorString().isEmpty()){
        ui->connectionLabel->setPalette(offPal);
    }
    else
        ui->connectionLabel->setPalette(onPal);
    if (auto *replyVoltage = modbus->sendReadRequest(*voltageMB, modbusSlaveID)) {
        if (!replyVoltage->isFinished())
            connect(replyVoltage, &QModbusReply::finished, this, [this, replyVoltage](){
                emit readFinished(replyVoltage, 0);  // read fiinished connects to ReadReady()
            });
        else
            delete replyVoltage; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
    }
    if (auto *replyCurrent = modbus->sendReadRequest(*currentMB, modbusSlaveID)) {
        if (!replyCurrent->isFinished())
            connect(replyCurrent, &QModbusReply::finished, this, [this, replyCurrent](){
                emit readFinished(replyCurrent, 1);
            });
        else
            delete replyCurrent; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
    }
    if (auto *replyInputV = modbus->sendReadRequest(*inputVMB, modbusSlaveID)) {
        if (!replyInputV->isFinished())
            connect(replyInputV, &QModbusReply::finished, this, [this, replyInputV](){
                emit readFinished(replyInputV, 2);
            });
        else
            delete replyInputV; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
    }
    if (auto *replyOutputV = modbus->sendReadRequest(*outputMB, modbusSlaveID)) {
        if (!replyOutputV->isFinished())
            connect(replyOutputV, &QModbusReply::finished, this, [this, replyOutputV](){
                emit readFinished(replyOutputV, 3);
            });
        else
            delete replyOutputV; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
    }
    if (auto *replyOverheat = modbus->sendReadRequest(*overheatMB, modbusSlaveID)) {
        if (!replyOverheat->isFinished())
            connect(replyOverheat, &QModbusReply::finished, this, [this, replyOverheat](){
                emit readFinished(replyOverheat, 4);
            });
        else
            delete replyOverheat; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
    }
    //ui->overheatLabel->setAutoFillBackground(true);
}

void MainWindow::onReadReady(QModbusReply* reply, int registerId){ // that's not right. there's four bytes.
    if (!reply)
        return;
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        if (registerId == 2)
            if (unit.value(0))
                ui->inputVlabel->setPalette(onPal);
            else
                ui->inputVlabel->setPalette(offPal);
        else
        if (registerId == 3)
            if (unit.value(0))
                ui->outputVlabel->setPalette(onPal);
            else
                ui->outputVlabel->setPalette(offPal);
        else
        if (registerId == 4)
            if (unit.value(0))
                ui->overheatLabel->setPalette(onPal);
            else
                ui->overheatLabel->setPalette(offPal);
        else{
            float tmp = 0;
            unsigned short data[2];
            data[1] = unit.value(0);
            data[0] = unit.value(1);
            memcpy(&tmp, data, 4);
            if (registerId == 0){          // voltage register
                this->ui->Vlcd->display(tmp);
            }
            else if (registerId == 1) {                       // current register
                ui->Ilcd->display(tmp);
            }
        }
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16), 5000);
    }
    reply->deleteLater();
}

void MainWindow::writeRegister(int registerAddr, bool value){
    QModbusDataUnit *writeUnit = new QModbusDataUnit(QModbusDataUnit::Coils, registerAddr, 1); //???? or two?
    writeUnit->setValue(0, value);
    QModbusReply *reply;
    reply = modbus->sendWriteRequest(*writeUnit, modbusSlaveID);
    if (reply){
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        statusBar()->showMessage(tr("Write error: "), 5000);
    }
}

void MainWindow::writeRegister(int registerAddr, int value){
    QModbusDataUnit *writeUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, registerAddr, 2);
    writeUnit->setValue(0, value);
    QModbusReply *reply;
    reply = modbus->sendWriteRequest(*writeUnit, modbusSlaveID);
    if (reply){
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        statusBar()->showMessage(tr("Write error: "), 5000);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
