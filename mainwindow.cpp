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
    //coilsMB = new QModbusDataUnit(QModbusDataUnit::Coils, 0, 3);
    //setCurrentMB = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 10, 1);
    inputRegistersMB = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 4);
    discreteInputsMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 13);
    /*powerOnMB = new QModbusDataUnit(QModbusDataUnit::Coils, 1, 1);
    startMB = new QModbusDataUnit(QModbusDataUnit::Coils, 2, 1);
    setCurrentMB = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 11, 1);
    resetMB = new QModbusDataUnit(QModbusDataUnit::Coils, 3, 1);
    readCurrentMB = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 1, 1);
    readVoltageMB = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 2, 1);
    radiatorTMB = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 3, 1);
    controlBoxTMB = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 4, 1);
    readyMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 1, 1);
    outputCurrentOkMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 2, 1);
    noPhaseMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 4, 1);
    radiatorOverheatMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 5, 1);
    hzChoMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 6, 1);
    overcurrentMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 9, 1);
    externalOneMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 7, 1);
    externalTwoMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 8, 1);
    overvoltageMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 10, 1);
    controlBoxFaultMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 13, 1);
    moduleOneFaultMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 11, 1);
    moduleTwoFaultMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 12, 1);*/

    connect(this, SIGNAL(readFinished(QModbusReply*, int)), this, SLOT(onReadReady(QModbusReply*, int)));
    readLoopTimer = new QTimer(this);
    connect(readLoopTimer, SIGNAL(timeout()), this, SLOT(readLoop()));
    readLoopTimer->start(1000);
    offPal.setColor(QPalette::WindowText, QColor("#ef5350"));
    onPal.setColor(QPalette::WindowText, QColor("#66bb6a"));
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
        ui->connectionLabel->setPalette(onPal);
    }
    else
        ui->connectionLabel->setPalette(offPal);
    /*if (auto *replyCoils = modbus->sendReadRequest(*coilsMB, modbusSlaveID)) {
        if (!replyCoils->isFinished())
            connect(replyCoils, &QModbusReply::finished, this, [this, replyCoils](){
                emit readFinished(replyCoils, 0);  // read fiinished connects to ReadReady()
            });
        else
            delete replyCoils; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
    }
    if (auto *replyCurrent = modbus->sendReadRequest(*setCurrentMB, modbusSlaveID)) {
        if (!replyCurrent->isFinished())
            connect(replyCurrent, &QModbusReply::finished, this, [this, replyCurrent](){
                emit readFinished(replyCurrent, 1);
            });
        else
            delete replyCurrent; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
    }*/
    if (auto *replyInputRegisters = modbus->sendReadRequest(*inputRegistersMB, modbusSlaveID)) {
        if (!replyInputRegisters->isFinished())
            connect(replyInputRegisters, &QModbusReply::finished, this, [this, replyInputRegisters](){
                emit readFinished(replyInputRegisters, 0);
            });
        else
            delete replyInputRegisters; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
    }
    if (auto *replyDiscreteInputs = modbus->sendReadRequest(*discreteInputsMB, modbusSlaveID)) {
        if (!replyDiscreteInputs->isFinished())
            connect(replyDiscreteInputs, &QModbusReply::finished, this, [this, replyDiscreteInputs](){
                emit readFinished(replyDiscreteInputs, 1);
            });
        else
            delete replyDiscreteInputs; // broadcast replies return immediately
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
        if (unit.value(0))
        switch (registerId) {
        case 0: // input registers
            ui->ReadCurrentSpinBox->setValue(unit.value(0)/100);
            ui->ReadVoltageSpinBox->setValue(unit.value(1)/1000);
            ui->RadiatorTSpinBox->setValue(unit.value(2/10));
            ui->ControlboxTSpinBox->setValue(unit.value(3/10));
            break;
        case 1:
            ui->ReadyLabel->setPalette(unit.value(0) ? onPal : offPal);
            ui->OutputCurrentOkLabel->setPalette(unit.value(1) ? onPal : offPal);
            ui->NoPhaseLabel->setPalette(unit.value(3) ? onPal : offPal);
            ui->RadiatorOverheatLabel->setPalette(unit.value(4) ? onPal : offPal);
            ui->HuhLabel->setPalette(unit.value(5) ? onPal : offPal);
            ui->ExternalOneLabel->setPalette(unit.value(6) ? onPal : offPal);
            ui->ExternalTwoLabel->setPalette(unit.value(7) ? onPal : offPal);
            ui->OvercurrentLabel->setPalette(unit.value(8) ? onPal : offPal);
            ui->OutputOvervoltageLabel->setPalette(unit.value(9) ? onPal : offPal);
            ui->ModuleOneFaultLabel->setPalette(unit.value(10) ? onPal : offPal);
            ui->ModuleTwoFaultLabel->setPalette(unit.value(11) ? onPal : offPal);
            ui->ControlBoxFaultLabel->setPalette(unit.value(12) ? onPal : offPal);
            break;
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

void MainWindow::on_PowerONPushButton_toggled(bool checked)
{
    writeRegister(0, checked);
}

void MainWindow::on_StartPushButton_toggled(bool checked)
{
    writeRegister(1, checked);
}

void MainWindow::on_CurrentSpinBox_valueChanged(int arg1)
{
    writeRegister(10, arg1*100);
}

void MainWindow::on_ResetPushButton_clicked()
{
    writeRegister(2, true);
    QTimer::singleShot(500, this, &MainWindow::resetResetButton);
}

void MainWindow::resetResetButton(){
    writeRegister(2, false);
}
