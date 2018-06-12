#ifndef SHAPEVIEW_H
#define SHAPEVIEW_H

#include <QWidget>
#include "Shape.h"

class ShapeView : public QWidget
{
    Q_OBJECT
private:
    struct Private;
	Private *m;
	void updateStatus();
public:
    explicit ShapeView(QWidget *parent = nullptr);
    ~ShapeView();

    void setPath(ShapePtr shape);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *);
	void mouseMoveEvent(QMouseEvent *event);
	void leaveEvent(QEvent *event);
};

#endif // SHAPEVIEW_H
