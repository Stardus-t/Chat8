#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chatdialog.h"
#include "profilechange.h"
#include "applyfriend.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Chat8");
    _login_dlg=new LoginDialog(this);
    setCentralWidget(_login_dlg);
    _login_dlg->show();
    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotSwitchReg);
    connect(_login_dlg,&LoginDialog::switchChat,this,&MainWindow::SlotSwitchChat);
}

void MainWindow::SlotSwitchChat(){
    _login_dlg->hide();
    // if (chatDlg) delete chatDlg;
    QString username = _login_dlg->getUsername();
    chatDlg=new ChatDialog(username,this);

    //chatDlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(chatDlg);
    chatDlg->show();
    connect(chatDlg,&ChatDialog::switchProfiePage,this,&MainWindow::SlotSwitchProfile);
    connect(chatDlg,&ChatDialog::switchaddfriendPage,this,&MainWindow::ShowAddfriendPage);
    connect(chatDlg,&ChatDialog::search_his_page,this,&MainWindow::ShowHistoryPage);
    setFixedSize(chatDlg->width(), chatDlg->height());
}

void MainWindow::SlotSwitchProfile(){
    chatDlg->hide();
    // if (profile_dlg) delete profile_dlg;
    profile_dlg=new ProfileChange(this);
    setCentralWidget(profile_dlg);
    profile_dlg->show();
    connect(profile_dlg,&ProfileChange::destroyPage,this,&MainWindow::ClosProfileChange);
}

void MainWindow::ShowAddfriendPage(){
    friend_dlg=new ApplyFriend(this);
    // setCentralWidget(profile_dlg);
    if(chatDlg) friend_dlg->setTcpClient(chatDlg->getTcpClient());
    friend_dlg->show();
    connect(friend_dlg,&ApplyFriend::destroy_dlg,this,&MainWindow::ClosefriendPage);
}

void MainWindow::ShowHistoryPage(){
    his_dlg=new Search_history(this);
    if(chatDlg) his_dlg->setTcpClient(chatDlg->getTcpClient());
    his_dlg->setChatDialog(chatDlg);
    his_dlg->show();
    connect(his_dlg,&Search_history::destroy_dlg,this,&MainWindow::CloseHistoryPage);
}

void MainWindow::SlotSwitchReg(){
    _reg_dlg = new RegisterDialog(this);
    // _reg_dlg->hide();

    // _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);

    //连接注册界面返回登录信号
    connect(_reg_dlg, &RegisterDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLogin);
    setCentralWidget(_reg_dlg);
    _login_dlg->hide();
    _reg_dlg->show();
    connect(_reg_dlg,&RegisterDialog::exitRegister,this,&MainWindow::SlotSwitchLogin);
}

void MainWindow::SlotSwitchLogin()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    // if (_login_dlg) delete _login_dlg;
    _login_dlg = new LoginDialog(this);
    // _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _reg_dlg->hide();
    _login_dlg->show();
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
}

void MainWindow::ClosefriendPage(){
    friend_dlg->close();
}

void MainWindow::CloseHistoryPage(){
    his_dlg->close();
}

void MainWindow::ClosProfileChange(){
    profile_dlg->close();
    // chatDlg->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
