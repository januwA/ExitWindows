#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 禁用最大化按钮
    // https://www.cnblogs.com/ajanuw/p/13563896.html
    Qt::WindowFlags flags = this->windowFlags();
    flags &= ~Qt::WindowMaximizeButtonHint;
    this->setWindowFlags(flags);

    // init ui
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    ui->comboBox->addItem("关机");
    ui->comboBox->addItem("重启");
    ui->comboBox->addItem("注销");

    // 定时器
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(exit_timeout()));

    // 托盘图标
    QIcon icon = QIcon(":/assets/images/time.ico");
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("ExitWindows");
    trayIcon->show();

    // 设置托盘右键菜单
    minimizeAction = new QAction("最小化", this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    maximizeAction = new QAction("最大化", this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction("还原", this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction("退出", this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    cancelExitWindowsAction = new QAction("取消任务", this);
    connect(cancelExitWindowsAction, SIGNAL(triggered()), this, SLOT(on_pushButton_2_clicked()));

    trayIconMenu = new QMenu(this);
    //trayIconMenu->addAction(minimizeAction);
    //trayIconMenu->addAction(maximizeAction);
    //trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(cancelExitWindowsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
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

    // 系统通知消息
    trayIcon->showMessage("通知","定时任务启动.",QSystemTrayIcon::Information,2000);
    timer->start(1000);
}

void  MainWindow::exit_timeout()
{
    exitSeconds--;
    if(exitSeconds >= 0)
    {
        ui->statusbar->showMessage("["+ ui->comboBox->currentText() +"]: " + QString::number(exitSeconds) + "秒");
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
    if(timer->isActive())
    {
        timer->stop();
        trayIcon->showMessage("通知","定时任务已取消.",QSystemTrayIcon::Information,2000);
        ui->statusbar->showMessage("");
    }
    else
    {
        trayIcon->showMessage("通知","当前没有任务.",QSystemTrayIcon::Information,2000);
    }
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        //qDebug() << "单击";
        break;
    case QSystemTrayIcon::DoubleClick:
        this->showNormal();
        break;
    case QSystemTrayIcon::MiddleClick:
        //qDebug() << "鼠标中键";
        break;
    default:
        break;
    }
}

void MainWindow::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    if(trayIcon->isVisible())
        hide();
}
