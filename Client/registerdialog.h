#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H
#include <QDialog>
#include "global.h"
#include <QWidget>
extern QMap<ReqId,std::function<void(const QJsonObject&)>> _handlers;
namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();
    void initHttpHandlers();
    void ChangeTipPage();
    bool enableBtn(bool enabled);

signals:
    void sigSwitchLogin();
    void exitRegister();

private slots:
    void on_sure_btn_clicked();
    void on_v_codepushButton_clicked();
    void showTip(QString str,bool b_ok);
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);

private:
    Ui::RegisterDialog *ui;
    QTimer *_countdown_timer;
    int _countdown;
};

#endif // REGISTERDIALOG_H
