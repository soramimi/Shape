#include "Shape.h"
#include <cmath>
#include <QDebug>
#include <QFile>
#include <QRect>
#include <QXmlStreamReader>

Shape::Shape()
{
    scale = QSizeF(1, 1);
}

bool Shape::contains(const QPointF &pt) const
{
    int count = 0;
    for (Path const &path : paths) {
		double angle = 0;
        size_t n = path.pts.size();
        double theta1 = 0;
        for (size_t i = 0; i + 1 < n; i++) {
            Shape::Node r = path.pts[i];
            Shape::Node s = path.pts[i + 1];
            r.scale(scale);
            s.scale(scale);
            QPointF a = r.point;
            QPointF b = r.point + r.fore;
            QPointF c = s.point + s.back;
            QPointF d = s.point;
            if (i == 0) {
				double x = a.x() - pt.x();
				double y = a.y() - pt.y();
                theta1 = atan2(y, x);
            }
            for (int j = 0; j < 16; j++) {
                double t = (j + 1) / 16.0;
                QPointF to = bezier(a, b, c, d, t);
				double x = to.x() - pt.x();
				double y = to.y() - pt.y();
                double theta2 = atan2(y, x);
				double a = theta2 - theta1;
				if (a >  M_PI) a -= M_PI * 2;
				if (a < -M_PI) a += M_PI * 2;
				angle += a;
                theta1 = theta2;
            }
        }
		if (angle > M_PI) {
            count++;
        }
    }
    return count & 1;
}

ShapePtr Shape::load(const QString &filepath)
{
    ShapePtr shape;
    QFile file(filepath);
    if (file.open(QFile::ReadOnly)) {
        shape = ShapePtr(new Shape);
        QXmlStreamReader r(&file);
        QString state;
        while (1) {
            QXmlStreamReader::TokenType t = r.readNext();
            if (t == QXmlStreamReader::NoToken || t == QXmlStreamReader::Invalid) break;
            if (t == QXmlStreamReader::StartElement) {
                state = state + '/' + r.name().toString();
                if (r.name() == "path") {
                    Shape::Path path;
                    Shape::Node last;
                    QStringRef d = r.attributes().value("d");
                    QByteArray ba = d.toLatin1();
                    if (!ba.isEmpty()) {
                        char const *begin = ba.data();
                        char const *end = begin + ba.size();
                        char const *ptr = begin;
                        char command = 0;
                        std::vector<double> nums;
                        enum class Sign {
                            None,
                            Plus,
                            Minus,
                        } sign = Sign::None;
                        double number = 0;
                        double div = 0;
                        while (1) {
                            int c = 0;
                            if (ptr < end) {
                                c = (unsigned char)*ptr;
                            }
                            if (c == '.' && div == 0) {
                                div = 1;
                            } else if (isdigit(c)) {
                                if (sign == Sign::None) {
                                    sign = Sign::Plus;
                                }
                                number = number * 10 + (c - '0');
                                if (div > 0) {
                                    div *= 10;
                                }
                            } else {
                                if (div > 0) {
                                    number /= div;
                                }
                                if (sign == Sign::Minus) {
                                    number = -number;
                                }
                                if (sign != Sign::None) {
                                    nums.push_back(number);
                                }
                                if (isalpha(c) || c == 0) {
                                    if (command != 0) {
                                        auto StartLine = [&](){
                                            int n = path.pts.size();
                                            if (n == 0) {
                                                Shape::Node node;
                                                node.point = last.point;
                                                path.pts.push_back(node);
                                                n++;
                                            }
                                            return n;
                                        };
                                        switch (command) {
                                        case 'M':
                                            last.point = QPointF(nums[0], nums[1]);
                                            break;
                                        case 'c':
                                        case 'C':
                                            if (nums.size() == 6) {
                                                int n = StartLine();
                                                Shape::Node node;
                                                path.node(n - 1).fore = QPointF(nums[0], nums[1]);
                                                node.point = QPointF(nums[4], nums[5]);
                                                node.back = QPointF(nums[2], nums[3]) - node.point;
                                                if (command == 'c') {
                                                    node.point += last.point;
                                                } else {
                                                    path.node(n - 1).fore -= last.point;
                                                }
                                                last = node;
                                                path.pts.push_back(node);
                                            }
                                            break;
                                        case 'q':
                                        case 'Q':
                                            if (nums.size() == 4) {
                                                int n = StartLine();
                                                Shape::Node node;
                                                path.node(n - 1).fore = QPointF(nums[0], nums[1]);
                                                node.point = QPointF(nums[2], nums[3]);
                                                node.back = QPointF(nums[0], nums[1]) - node.point;
                                                if (command == 'q') {
                                                    node.point += last.point;
                                                } else {
                                                    path.node(n - 1).fore -= last.point;
                                                }
                                                last = node;
                                                path.node(n - 1).fore = path.node(n - 1).fore * 2 / 3;
                                                node.back = node.back * 2 / 3;
                                                path.pts.push_back(node);
                                            }
                                            break;
                                        case 's':
                                        case 'S':
                                            if (nums.size() >= 4) {
                                                int n = StartLine();
                                                for (size_t i = 0; i + 3 < nums.size(); i += 4) {
                                                    Shape::Node node;
                                                    path.node(n - 1).fore = -path.node(n - 1).back;
                                                    node.point = QPointF(nums[i + 2], nums[i + 3]);
                                                    node.back = QPointF(nums[i + 0], nums[i + 1]) - node.point;
                                                    if (command == 's') {
                                                        node.point += path.node(n - 1).point;
                                                    }
                                                    last = node;
                                                    path.pts.push_back(node);
                                                    n++;
                                                }
                                            }
                                            break;
                                        case 't':
                                        case 'T':
                                            if (nums.size() >= 2) {
                                                int n = StartLine();
                                                for (size_t i = 0; i + 1 < nums.size(); i += 2) {
                                                    Shape::Node node;
                                                    path.node(n - 1).fore = -path.node(n - 1).back;
                                                    node.point = QPointF(nums[i + 0], nums[i + 1]);
                                                    node.back = last.point + last.point - (last.point + last.back) - node.point;
                                                    if (command == 't') {
                                                        node.point += path.node(n - 1).point;
                                                    }
                                                    last = node;
                                                    node.back = node.back * 2 / 3;
                                                    path.pts.push_back(node);
                                                    n++;
                                                }
                                            }
                                            break;
                                        case 'L':
                                            if (nums.size() >= 2) {
                                                StartLine();
                                                for (size_t i = 0; i + 1 < nums.size(); i += 2) {
                                                    Shape::Node node;
                                                    node.point = QPointF(nums[i], nums[i + 1]);
                                                    path.pts.push_back(node);
                                                    last = node;
                                                }
                                            }
                                            break;
                                        case 'l':
                                            if (nums.size() >= 2) {
                                                StartLine();
                                                for (size_t i = 0; i + 1 < nums.size(); i += 2) {
                                                    Shape::Node node;
                                                    node.point = QPointF(nums[i], nums[i + 1]) + last.point;
                                                    path.pts.push_back(node);
                                                    last = node;
                                                }
                                            }
                                            break;
                                        case 'H':
                                            if (nums.size() >= 1) {
                                                StartLine();
                                                for (size_t i = 0; i < nums.size(); i++) {
                                                    Shape::Node node;
                                                    last.point.rx() = nums[i];
                                                    node.point = last.point;
                                                    path.pts.push_back(node);
                                                }
                                            }
                                            break;
                                        case 'h':
                                            if (nums.size() >= 1) {
                                                StartLine();
                                                for (size_t i = 0; i < nums.size(); i++) {
                                                    Shape::Node node;
                                                    last.point.rx() += nums[i];
                                                    node.point = last.point;
                                                    path.pts.push_back(node);
                                                }
                                            }
                                            break;
                                        case 'V':
                                            if (nums.size() >= 1) {
                                                StartLine();
                                                for (size_t i = 0; i < nums.size(); i++) {
                                                    Shape::Node node;
                                                    last.point.ry() = nums[i];
                                                    node.point = last.point;
                                                    path.pts.push_back(node);
                                                }
                                            }
                                            break;
                                        case 'v':
                                            if (nums.size() >= 1) {
                                                StartLine();
                                                for (size_t i = 0; i < nums.size(); i++) {
                                                    Shape::Node node;
                                                    last.point.ry() += nums[i];
                                                    node.point = last.point;
                                                    path.pts.push_back(node);
                                                }
                                            }
                                            break;
										case 'a':
										case 'A':
											qDebug() << "not supported";
											break;
										}
                                        QString text;
                                        text += command;
                                        for (double v : nums) {
                                            text += ' ' + QString::number(v);
                                        }
                                    }
                                    if (c == 'z' || c == 'Z') {
                                        path.closed = true;
                                        break;
                                    }
                                    if (c == 0) {
                                        break;
                                    }
                                    nums.clear();
                                    command = c;
                                }
                                sign = c == '-' ? Sign::Minus : Sign::None;
                                number = 0;
                                div = 0;
                            }
                            ptr++;
                        }
                    }
                    shape->paths.push_back(path);
                } else if (state == "/svg") {
                    Shape::Length width;
                    Shape::Length height;
                    {
                        char *p, *q;
                        QByteArray ba;
                        ba = r.attributes().value("width").toLatin1();
                        if (!ba.isEmpty()) {
                            p = q = ba.data();
                            width.value = strtod(p, &q);
                            if (p < q) {
                                width.unit = Shape::parseUnit(q);
                            }
                        }
                        if (!ba.isEmpty()) {
                            ba = r.attributes().value("height").toLatin1();
                            p = q = ba.data();
                            height.value = strtod(p, &q);
                            if (p < q) {
                                height.unit = Shape::parseUnit(q);
                            }
                        }
                        if (!ba.isEmpty()) {
                            ba = r.attributes().value("viewBox").toLatin1();
                            p = q = ba.data();
                            std::vector<double> nums;
                            while (1) {
                                while (isspace((unsigned char)*p)) p++;
                                double v = strtod(p, &q);
                                if (!q || p == q) break;
                                nums.push_back(v);
                                p = q;
                            }
                            shape->scale = QSizeF(1, 1);
                            if (nums.size() == 4) {
                                QRect viewBox(nums[0], nums[1], nums[2], nums[3]);
                                double scale_x = viewBox.width();
                                double scale_y = viewBox.height();
                                if (scale_x > 0 && scale_y > 0) {
                                    scale_x = width.changeUnit(Shape::Unit::px).value / scale_x;
                                    scale_y = height.changeUnit(Shape::Unit::px).value / scale_y;
                                    shape->scale = QSizeF(scale_x, scale_y);
                                }
                            }
                        }
                    }
                }
            } else if (t == QXmlStreamReader::EndElement) {
                int i = state.lastIndexOf('/');
                if (i >= 0) {
                    state = state.mid(0, i);
                }
            }
        }
    }
    return shape;
}

double Shape::degree(const QPointF &org, const QPointF &pos)
{
    double x = pos.x() - org.x();
    double y = pos.y() - org.y();
    double a = atan2(y, x) * 180 / 3.1459265358979323846;
    return fmod(a + 450, 360);
}

double Shape::bezier(double a, double b, double c, double d, double t)
{
    double u = 1 - t;
    return u * u * u * a + u * u * t * b * 3 + u * t * t * c * 3 + t * t * t * d;
}

QPointF Shape::bezier(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d, double t)
{
    double x = bezier(a.x(), b.x(), c.x(), d.x(), t);
    double y = bezier(a.y(), b.y(), c.y(), d.y(), t);
    return QPointF(x, y);
}

Shape::Unit Shape::parseUnit(const char *p)
{
    while (isspace((unsigned char)*p)) p++;
    if (p[0] == 'p' && p[1] == 'x') return Unit::px;
    if (p[0] == 'm' && p[1] == 'm') return Unit::mm;
    if (p[0] == 'c' && p[1] == 'm') return Unit::cm;
    if (p[0] == 'i' && p[1] == 'n') return Unit::in;
    return Unit::unknown;
}

Shape::Length Shape::Length::changeUnit(Shape::Unit new_unit) const
{
    Length r;
    r.value = value;
    r.unit = unit;
    if (new_unit != r.unit) {
        if (new_unit == Unit::px) {
            switch (r.unit) {
            case Unit::cm:
                r.value *= 10;
                // fallthru
            case Unit::mm:
                r.value /= 25.4;
                break;
            case Unit::pt:
                r.value /= 72;
                break;
            case Unit::pc:
                r.value /= 6;
                break;
            }
            r.value *= DPI;
        } else if (r.unit == Unit::px) {
            if (DPI > 0) {
                switch (new_unit) {
                case Unit::cm:
                    r.value /= 10;
                    // fallthru
                case Unit::mm:
                    r.value *= 25.4;
                    break;
                case Unit::pt:
                    r.value *= 72;
                    break;
                case Unit::pc:
                    r.value *= 6;
                    break;
                }
                r.value /= DPI;
            }
        }
    }
    r.unit = new_unit;
    return r;
}
