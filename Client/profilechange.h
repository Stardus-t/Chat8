#ifndef PROFILECHANGE_H
#define PROFILECHANGE_H

#include <QDialog>

namespace Ui {
class ProfileChange;
}

class ProfileChange : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileChange(QWidget *parent = nullptr);
    ~ProfileChange();

signals:
    void destroyPage();

private:
    Ui::ProfileChange *ui;
};

#endif // PROFILECHANGE_H
