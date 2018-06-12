#ifndef SHAPE_H
#define SHAPE_H

#include <QList>
#include <QPointF>
#include <QSizeF>
#include <memory>

class Shape;
typedef std::shared_ptr<Shape> ShapePtr;

class Shape {
public:

    struct Node {
        QPointF back;
        QPointF point;
        QPointF fore;

        void scale(QSizeF const &t)
        {
            double x = t.width();
            double y = t.height();
            point.rx() *= x;
            point.ry() *= y;
            back.rx() *= x;
            back.ry() *= y;
            fore.rx() *= x;
            fore.ry() *= y;
        }

        Node scaled(QSizeF const &t) const
        {
            Node r(*this);
            r.scale(t);
            return r;
        }
    };

    class Path {
    public:
        QList<Node> pts;
        bool closed = false;

        Node &node(int i)
        {
            return pts[i];
        }
        Node const &node(int i) const
        {
            return pts[i];
        }
    };

    enum class Unit {
        unknown,
        px,
        mm,
        cm,
        in,
        pt,
        pc,
    };

    static Unit parseUnit(char const *p);

    static const int DPI = 90;

    class Length {
    public:
        double value = 0;
        Unit unit = Unit::unknown;

        Length changeUnit(Unit new_unit) const;
    };

    QList<Path> paths;
    QSizeF scale;

    Shape();

    bool contains(QPointF const &point) const;


    static ShapePtr load(QString const &filepath);

    static double degree(QPointF const &org, QPointF const &pos);

    static double bezier(double a, double b, double c, double d, double t);
    static QPointF bezier(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d, double t);
};

#endif // SHAPE_H
