#ifndef BUBBLEFRAME_H
#define BUBBLEFRAME_H
#include <QFrame>
#include <QWidget>
#include <QString>

enum class MessageType{
    SEND,
    RECEIVE
};

class BubbleFrame:public QWidget{
    Q_OBJECT
public:
    explicit BubbleFrame(const QString& text, MessageType type, QWidget *parent = nullptr);
    void setText(const QString &text);
    void setBubbleColor(const QColor& color);
protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
private:
    QString m_text;          // 消息文本
    MessageType m_type;      // 消息类型
    QColor m_bubbleColor;    // 气泡颜色
    QColor m_textColor;      // 文本颜色
    int m_radius;            // 圆角半径
    int m_padding;           // 内边距
    int m_triangleWidth;     // 三角形宽度
};

#endif // BUBBLEFRAME_H
