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
    //modbus->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "192.168.0.82");//settings->value("MKON_IP", "192.168.1.99"));
    modbus->setConnectionParameter(QModbusDevice::NetworkAddressParameter, settings->value("ip", "192.168.0.82").toString());
    modbus->setConnectionParameter(QModbusDevice::NetworkPortParameter, "502");
    modbus->setTimeout(3000);
    if (!modbus->connectDevice()) {
        statusBar()->showMessage(tr("Connect failed: ") + modbus->errorString(), 5000);
        qDebug() << modbus->connectionParameter(QModbusDevice::NetworkAddressParameter);
        qDebug() << " modbus->connectDevice failed" + modbus->errorString();
    }
    inputRegistersMB = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 4);
    discreteInputsMB = new QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 13);

    connect(this, SIGNAL(readFinished(QModbusReply*, int)), this, SLOT(onReadReady(QModbusReply*, int)));
    readLoopTimer = new QTimer(this);
    connect(readLoopTimer, SIGNAL(timeout()), this, SLOT(readLoop()));
    readLoopTimer->start(1000);
    offPal.setColor(QPalette::WindowText, QColor("#ef5350"));
    onPal.setColor(QPalette::WindowText, QColor("#66bb6a"));
    QTimer::singleShot(500, this, &MainWindow::readCurrent);
}

void MainWindow::readLoop(){
    if (modbus->errorString().isEmpty()){
        ui->label->setPalette(onPal);
    }
    else{
        qDebug() << modbus->errorString();
        ui->label->setPalette(offPal);
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
    delete ui;
}

void MainWindow::on_PowerONPushButton_toggled(bool checked)
{
    writeRegister(0, checked);
    if (checked)
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


void MainWindow::on_exitButton_clicked()
{
    settings->setValue("I", ui->CurrentSpinBox->value());
    on_CurrentSpinBox_valueChanged(0);
    on_StartPushButton_toggled(false);
    on_PowerONPushButton_toggled(false);
    //QApplication::quit();
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeToStop()));
    timer->start(4000);

}

void MainWindow::timeToStop(){
    QCoreApplication::exit();
}

void MainWindow::readCurrent(){
    ui->CurrentSpinBox->setValue(settings->value("I", 0).toInt());
}
