#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>
#include <QString>

namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public QWidget
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();

    QSize sizeHint() const override{
        return QSize(parentWidget() ? parentWidget()->width() : 300, 70);
    }

    void SetInfo(QString name);
    QString getName(){
        return _name;
    }
private:
    Ui::ChatUserWid *ui;
    QString _name;

};

#endif // CHATUSERWID_H
