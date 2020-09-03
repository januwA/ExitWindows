#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    ui->comboBox->addItem("关机");
    ui->comboBox->addItem("重启");
    ui->comboBox->addItem("注销");

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(exit_timeout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 开始执行事件
void MainWindow::on_pushButton_clicked()
{
    exitFlags = ExitWindowsFlags[ui->comboBox->currentIndex()];
    QDateTime curDateTime = QDateTime::currentDateTime();
    QDateTime exitDateTime = QDateTime::fromString(ui->dateTimeEdit->text(), "yyyy-MM-dd hh:mm:ss");

    exitSeconds = exitDateTime.toTime_t() - curDateTime.toTime_t();
    if(exitSeconds < 0)
    {
        QMessageBox::warning(this, NULL, "必须大于当前时间!");
        return;
    }

    timer->start(1000);
}

void  MainWindow::exit_timeout()
{
    exitSeconds--;

    if(exitSeconds >= 0)
    {
        ui->statusbar->showMessage("距离关机还有: " + QString::number(exitSeconds) + "秒");
        return;
    }

    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;

    // 获取关闭特权的LUID
    TOKEN_PRIVILEGES tkp;
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // 获取此过程的关闭特权。
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
    {
        CloseHandle(hToken);
        return;
    }

    // 关闭系统并强制关闭所有应用程序。
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-exitwindowsex
    if (!ExitWindowsEx(exitFlags,
                       SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                       SHTDN_REASON_MINOR_UPGRADE |
                       SHTDN_REASON_FLAG_PLANNED))
    {
        CloseHandle(hToken);
        return;
    }

    timer->stop();
}

void MainWindow::on_pushButton_2_clicked()
{
    if(timer)
    {
        timer->stop();
        ui->statusbar->showMessage("");
    }
}
