#include "bubbleframe.h"
#include <QPainter>//负责所有绘制
#include <QFontMetrics>//计算文字占用的像素尺寸，动态挑战气泡大小
#include <QStyleOption>
#include <QApplication>
#include <QRect>

BubbleFrame::BubbleFrame(const QString& text,MessageType type,QWidget *parent)
    :QWidget(parent), m_text(text), m_type(type), m_radius(8), m_padding(10), m_triangleWidth(10)
{
    if(type==MessageType::SEND){
        m_bubbleColor=QColor(135, 206, 235);
        m_textColor=Qt::black;
    }else{
        m_bubbleColor=Qt::white;
        m_textColor=Qt::black;
    }
    setMinimumHeight(sizeHint().height());
}

void BubbleFrame::setText(const QString &text){
    m_text=text;
    updateGeometry();
    repaint();
}

void BubbleFrame::paintEvent(QPaintEvent *event){//绘制
    Q_UNUSED(event);//不用这个参数
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing,true);//抗锯齿
    //绘制圆角矩形
    QRect bubbleRect=rect();//整个控件
    if(m_type==MessageType::SEND)
        bubbleRect.setWidth(bubbleRect.width() - m_triangleWidth);
    else bubbleRect.setX(m_triangleWidth);

    painter.setBrush(m_bubbleColor);//填充颜色
    painter.setPen(Qt::transparent);//边框透明
    painter.drawRoundedRect(bubbleRect, m_radius, m_radius);//绘制
    //绘制三角形
    QPolygon triangle;
    if (m_type == MessageType::SEND) {
        triangle << QPoint(bubbleRect.right(), bubbleRect.top() + m_triangleWidth)
        << QPoint(rect().right(), bubbleRect.center().y())
        << QPoint(bubbleRect.right(), bubbleRect.bottom() - m_triangleWidth);
    } else {
        triangle << QPoint(bubbleRect.left(), bubbleRect.top() + m_triangleWidth)
        << QPoint(0, bubbleRect.center().y())
        << QPoint(bubbleRect.left(), bubbleRect.bottom() - m_triangleWidth);
    }
    painter.drawPolygon(triangle);
    //绘制文本
    painter.setPen(m_textColor);
    QRect textRect=bubbleRect.adjusted(m_padding, m_padding, -m_padding, -m_padding);
    painter.drawText(textRect, Qt::TextWordWrap, m_text);// Qt::TextWordWrap允许自动换行
}

QSize BubbleFrame::sizeHint()const{
    QFontMetrics fm(font());//font()返回当前使用的控件
    QSize textSize = fm.size(Qt::TextWordWrap, m_text);
    return QSize(textSize.width() + 2 * m_padding + m_triangleWidth,
                 textSize.height() + 2 * m_padding);
}
QSize BubbleFrame::minimumSizeHint() const {
    return sizeHint();
}
