#include "global.h"

QString gate_url_prefix="";

std::function<void (QWidget*)>repolish=[](QWidget*w){
    w->style()->unpolish(w);//清除当前控件的样式
    w->style()->polish(w);//重新应用样式
};
QString ChatServer_host = "";  // 定义ChatServer_host
QString ChatServer_port = "";  // 定义ChatServer_port
QVector<FriendInfo>friendList;
GLOBAL::GLOBAL() {}
