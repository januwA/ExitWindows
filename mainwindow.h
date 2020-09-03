#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QtDebug>
#include <QMessageBox>
#include <QtCore>
#include <QTimer>
#include <iostream>
#include <windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString version = "1.0.0";

    // 关机,重启,注销
    const DWORD ExitWindowsFlags[3] = {EWX_SHUTDOWN | EWX_FORCE,
                                       EWX_REBOOT | EWX_FORCE,
                                       EWX_LOGOFF | EWX_FORCE};
    QTimer *timer;
    DWORD exitFlags;
    int exitSeconds;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:


    void on_pushButton_clicked();
    void exit_timeout();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
