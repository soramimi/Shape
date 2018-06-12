#include "ShapeView.h"
#include "Shape.h"
#include "Shape.h"
#include <cmath>
#include <QPainter>

struct ShapeView::Private {
    ShapePtr shape;
	bool contains = false;
};

ShapeView::ShapeView(QWidget *parent)
    : QWidget(parent)
    , m(new Private)
{
	setMouseTracking(true);
}

ShapeView::~ShapeView()
{
    delete m;
}

void ShapeView::setPath(ShapePtr shape)
{
    m->shape = shape;
    update();
}

void ShapeView::paintEvent(QPaintEvent *)
{
    QPainter pr(this);

    if (m->shape) {
        QPainterPath painterpath;
        for (Shape::Path const &path : m->shape->paths) {
            if (path.pts.size() > 1) {
                painterpath.moveTo(path.pts[0].scaled(m->shape->scale).point);
                int n = path.pts.size();
                for (int i = 0; i + 1 < n; i++) {
                    Shape::Node a = path.pts[i];
                    Shape::Node b = path.pts[i + 1];
                    a.scale(m->shape->scale);
                    b.scale(m->shape->scale);
                    painterpath.cubicTo(a.point + a.fore, b.point + b.back, b.point);
                }
                if (path.closed) {
                    Shape::Node b = path.pts.front();
                    b.scale(m->shape->scale);
                    painterpath.lineTo(b.point);
                }
            }
        }
        pr.drawPath(painterpath);
	}

	int x = 4;
	int y = pr.fontMetrics().height();
	pr.drawText(x, y, m->contains ? "inside" : "outside");
}

void ShapeView::updateStatus()
{
	if (m->shape) {
		QPoint pt = QCursor::pos();
		pt = mapFromGlobal(pt);
		m->contains = m->shape->contains(pt);
	} else {
		m->contains = false;
	}
	update();
}

void ShapeView::mouseMoveEvent(QMouseEvent *event)
{
	updateStatus();
}

void ShapeView::leaveEvent(QEvent *event)
{
	updateStatus();
}






