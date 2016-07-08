#ifndef LABEL_H
#define LABEL_H
#include <QLabel>
#include <QMouseEvent>
class Label : public QLabel
{
    Q_OBJECT
public:
    explicit Label(QWidget *parent = 0);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
};

#endif // LABEL_H
