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

    modbusSlaveID = 0;
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
    inputRegistersMB = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 4);
    inputRegistersMB2 = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 2, 2);
    discreteInputsMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 13);  // i can't read all at once. cos they are not initialized in the MPS
    discreteInputsMB1 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 13);
    discreteInputsMB2 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 2, 1);
    discreteInputsMB3 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 3, 1);
    discreteInputsMB4 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 4, 1);
    discreteInputsMB5 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 5, 1);
    discreteInputsMB6 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 6, 1);
    discreteInputsMB7 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 7, 1);
    discreteInputsMB8 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 8, 1);
    discreteInputsMB9 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 9, 1);
    discreteInputsMB10 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 10, 1);
    discreteInputsMB11 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 11, 1);
    discreteInputsMB12 = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 12, 1);
    //testDiscreteInputsMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 3, 1);
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
    else{
        ui->connectionLabel->setPalette(offPal);
    }
    if (auto *replyDiscreteInputs = modbus->sendReadRequest(*discreteInputsMB, modbusSlaveID)) {
        if (!replyDiscreteInputs->isFinished())
            connect(replyDiscreteInputs, &QModbusReply::finished, this, [this, replyDiscreteInputs](){
                emit readFinished(replyDiscreteInputs, 0);
            });
        else
            delete replyDiscreteInputs; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
    }
//    if (auto *replyDiscreteInputs1 = modbus->sendReadRequest(*discreteInputsMB1, modbusSlaveID)) {
//        if (!replyDiscreteInputs1->isFinished())
//            connect(replyDiscreteInputs1, &QModbusReply::finished, this, [this, replyDiscreteInputs1](){
//                emit readFinished(replyDiscreteInputs1, 1);
//            });
//        else
//            delete replyDiscreteInputs1; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs2 = modbus->sendReadRequest(*discreteInputsMB2, modbusSlaveID)) {
//        if (!replyDiscreteInputs2->isFinished())
//            connect(replyDiscreteInputs2, &QModbusReply::finished, this, [this, replyDiscreteInputs2](){
//                emit readFinished(replyDiscreteInputs2, 2);
//            });
//        else
//            delete replyDiscreteInputs2; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs3 = modbus->sendReadRequest(*discreteInputsMB3, modbusSlaveID)) {
//        if (!replyDiscreteInputs3->isFinished())
//            connect(replyDiscreteInputs3, &QModbusReply::finished, this, [this, replyDiscreteInputs3](){
//                emit readFinished(replyDiscreteInputs3, 3);
//            });
//        else
//            delete replyDiscreteInputs3; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs4 = modbus->sendReadRequest(*discreteInputsMB4, modbusSlaveID)) {
//        if (!replyDiscreteInputs4->isFinished())
//            connect(replyDiscreteInputs4, &QModbusReply::finished, this, [this, replyDiscreteInputs4](){
//                emit readFinished(replyDiscreteInputs4, 4);
//            });
//        else
//            delete replyDiscreteInputs4; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs5 = modbus->sendReadRequest(*discreteInputsMB5, modbusSlaveID)) {
//        if (!replyDiscreteInputs5->isFinished())
//            connect(replyDiscreteInputs5, &QModbusReply::finished, this, [this, replyDiscreteInputs5](){
//                emit readFinished(replyDiscreteInputs5, 5);
//            });
//        else
//            delete replyDiscreteInputs5; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs6 = modbus->sendReadRequest(*discreteInputsMB6, modbusSlaveID)) {
//        if (!replyDiscreteInputs6->isFinished())
//            connect(replyDiscreteInputs6, &QModbusReply::finished, this, [this, replyDiscreteInputs6](){
//                emit readFinished(replyDiscreteInputs6, 6);
//            });
//        else
//            delete replyDiscreteInputs6; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs7 = modbus->sendReadRequest(*discreteInputsMB7, modbusSlaveID)) {
//        if (!replyDiscreteInputs7->isFinished())
//            connect(replyDiscreteInputs7, &QModbusReply::finished, this, [this, replyDiscreteInputs7](){
//                emit readFinished(replyDiscreteInputs7, 7);
//            });
//        else
//            delete replyDiscreteInputs7; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs8 = modbus->sendReadRequest(*discreteInputsMB8, modbusSlaveID)) {
//        if (!replyDiscreteInputs8->isFinished())
//            connect(replyDiscreteInputs8, &QModbusReply::finished, this, [this, replyDiscreteInputs8](){
//                emit readFinished(replyDiscreteInputs8, 8);
//            });
//        else
//            delete replyDiscreteInputs8; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs9 = modbus->sendReadRequest(*discreteInputsMB9, modbusSlaveID)) {
//        if (!replyDiscreteInputs9->isFinished())
//            connect(replyDiscreteInputs9, &QModbusReply::finished, this, [this, replyDiscreteInputs9](){
//                emit readFinished(replyDiscreteInputs9, 9);
//            });
//        else
//            delete replyDiscreteInputs9; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs10 = modbus->sendReadRequest(*discreteInputsMB10, modbusSlaveID)) {
//        if (!replyDiscreteInputs10->isFinished())
//            connect(replyDiscreteInputs10, &QModbusReply::finished, this, [this, replyDiscreteInputs10](){
//                emit readFinished(replyDiscreteInputs10, 10);
//            });
//        else
//            delete replyDiscreteInputs10; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs11 = modbus->sendReadRequest(*discreteInputsMB11, modbusSlaveID)) {
//        if (!replyDiscreteInputs11->isFinished())
//            connect(replyDiscreteInputs11, &QModbusReply::finished, this, [this, replyDiscreteInputs11](){
//                emit readFinished(replyDiscreteInputs11, 11);
//            });
//        else
//            delete replyDiscreteInputs11; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
//    if (auto *replyDiscreteInputs12 = modbus->sendReadRequest(*discreteInputsMB12, modbusSlaveID)) {
//        if (!replyDiscreteInputs12->isFinished())
//            connect(replyDiscreteInputs12, &QModbusReply::finished, this, [this, replyDiscreteInputs12](){
//                emit readFinished(replyDiscreteInputs12, 12);
//            });
//        else
//            delete replyDiscreteInputs12; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }
    if (auto *replyInputRegisters = modbus->sendReadRequest(*inputRegistersMB, modbusSlaveID)) {
        if (!replyInputRegisters->isFinished())
            connect(replyInputRegisters, &QModbusReply::finished, this, [this, replyInputRegisters](){
                emit readFinished(replyInputRegisters, 1);
            });
        else
            delete replyInputRegisters; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
   }
//    if (auto *replyInputRegisters2 = modbus->sendReadRequest(*inputRegistersMB2, modbusSlaveID)) {
//        if (!replyInputRegisters2->isFinished())
//            connect(replyInputRegisters2, &QModbusReply::finished, this, [this, replyInputRegisters2](){
//                emit readFinished(replyInputRegisters2, 14);
//            });
//        else
//            delete replyInputRegisters2; // broadcast replies return immediately
//    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
//    }

    //ui->overheatLabel->setAutoFillBackground(true);
}

void MainWindow::onReadReady(QModbusReply* reply, int registerId){ // that's not right. there's four bytes.
    if (!reply)
        return;
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        //if (unit.value(0))
        switch (registerId) {
        case 0: // discrete ones
            ui->ReadyLabel->setPalette(unit.value(0) ? onPal : offPal);
            ui->OutputCurrentOkLabel->setPalette(unit.value(1) ? onPal : offPal);
            ui->NoPhaseLabel->setPalette(unit.value(3) ? offPal : onPal);
            ui->RadiatorOverheatLabel->setPalette(unit.value(4) ? offPal : onPal);
            ui->HuhLabel->setPalette(unit.value(5) ? offPal : onPal);
            ui->ExternalOneLabel->setPalette(unit.value(6) ? offPal : onPal);
            ui->ExternalTwoLabel->setPalette(unit.value(7) ? offPal : onPal);
            ui->OvercurrentLabel->setPalette(unit.value(8) ? offPal : onPal);
            ui->OutputOvervoltageLabel->setPalette(unit.value(9) ? offPal : onPal);
            ui->ModuleOneFaultLabel->setPalette(unit.value(10) ? offPal : onPal);
            ui->ModuleTwoFaultLabel->setPalette(unit.value(11) ? offPal : onPal);
            ui->ControlBoxFaultLabel->setPalette(unit.value(12) ? offPal : onPal);
            break;
        case 1: // input registers
            if (unit.value(0) < 65000)
                ui->ReadCurrentDoubleSpinBox->setValue((double)unit.value(0)/100);
            if (unit.value(1) < 65000)
                ui->ReadVoltageDoubleSpinBox->setValue((double)unit.value(1)/1000);
            ui->RadiatorTSpinBox->setValue(unit.value(2)/10);
            ui->ControlboxTSpinBox->setValue(unit.value(3)/10);
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
    QModbusDataUnit *writeUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, registerAddr, 1);
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
    on_StartPushButton_toggled(false);
    on_PowerONPushButton_toggled(false);
    delete ui;
}

void MainWindow::on_PowerONPushButton_toggled(bool checked)
{
    writeRegister(0, checked);
    writeRegister(10, ui->CurrentSpinBox->value()*100);
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
