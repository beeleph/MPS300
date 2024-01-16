#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModbusTcpClient>
#include <QModbusTcpServer>
#include <QModbusDataUnit>
#include <QDebug>
#include <QSettings>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QModbusTcpClient *modbus = nullptr;
    QSettings *settings;
    QModbusDataUnit *powerOnMB = nullptr;
    QModbusDataUnit *startMB = nullptr;
    QModbusDataUnit *setCurrentMB = nullptr;
    QModbusDataUnit *resetMB = nullptr;
    QModbusDataUnit *readCurrentMB = nullptr;
    QModbusDataUnit *readVoltageMB = nullptr;
    QModbusDataUnit *radiatorTMB = nullptr;
    QModbusDataUnit *controlBoxTMB = nullptr;
    QModbusDataUnit *readyMB = nullptr;
    QModbusDataUnit *outputCurrentOkMB = nullptr;
    QModbusDataUnit *noPhaseMB = nullptr;
    QModbusDataUnit *radiatorOverheatMB = nullptr;
    QModbusDataUnit *hzChoMB = nullptr;
    QModbusDataUnit *overcurrentMB = nullptr;
    QModbusDataUnit *externalOneMB = nullptr;
    QModbusDataUnit *externalTwoMB = nullptr;
    QModbusDataUnit *overvoltageMB = nullptr;
    QModbusDataUnit *controlBoxFaultMB = nullptr;
    QModbusDataUnit *moduleOneFaultMB = nullptr;
    QModbusDataUnit *moduleTwoFaultMB = nullptr;
    QModbusDataUnit *coilsMB = nullptr;
    QModbusDataUnit *inputRegistersMB = nullptr;
    QModbusDataUnit *discreteInputsMB = nullptr;
    QTimer *readLoopTimer;
    QPalette onPal, offPal;
    int modbusSlaveID = 0, msleep = 500;

signals:
    void readFinished(QModbusReply* reply, int relayId);

private slots:
    void onReadReady(QModbusReply* reply, int registerId);
    void readLoop();
    void writeRegister(int registerAddr, bool value);
    void writeRegister(int registerAddr, int value);
    void on_PowerONPushButton_toggled(bool checked);
    void on_StartPushButton_toggled(bool checked);
    void on_CurrentSpinBox_valueChanged(int arg1);
    void on_ResetPushButton_clicked();
    void resetResetButton(); // :)
};
#endif // MAINWINDOW_H
