#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "registerdialog.h"
#include "chatdialog.h"
#include "profilechange.h"
#include "applyfriend.h"
#include "search_history.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void SlotSwitchReg();
    void SlotSwitchLogin();
    void SlotSwitchChat();
    void SlotSwitchProfile();
    void ShowAddfriendPage();
    void ClosefriendPage();
    void ClosProfileChange();
    // void CloseRegisterPage();
    void ShowHistoryPage();
    void CloseHistoryPage();

private:
    Ui::MainWindow *ui;
    LoginDialog *_login_dlg;//登录界面
    RegisterDialog *_reg_dlg;//注册页面
    ChatDialog *chatDlg;
    ProfileChange *profile_dlg;
    ApplyFriend *friend_dlg;
    Search_history *his_dlg;
};
#endif // MAINWINDOW_H
