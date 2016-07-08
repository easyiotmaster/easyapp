#include "label.h"

Label::Label(QWidget *parent):QLabel(parent)
{

}


void Label::mousePressEvent(QMouseEvent *ev)
{
    QLabel::mousePressEvent(ev);
}

void Label::mouseReleaseEvent(QMouseEvent *ev)
{
    QLabel::mouseReleaseEvent(ev);
    emit clicked();
}
