#ifndef GRADIENTLABEL_H
#define GRADIENTLABEL_H

#include <QLabel>
#include <QPainter>
#include <QLinearGradient>

class GradientLabel : public QLabel
{
    Q_OBJECT

public:
    explicit GradientLabel(
        QWidget *parent = nullptr
        )
        : QLabel(parent)
    {}

protected:
    void paintEvent(
        QPaintEvent *event
        ) override
    {
        Q_UNUSED(event)

        QPainter p(this);

        p.setRenderHint(
            QPainter::Antialiasing
            );

        QLinearGradient gradient(
            0,
            0,
            width(),
            0
            );

        // Orange → Yellow
        gradient.setColorAt(
            0.0,
            QColor(255,140,0)
            );

        gradient.setColorAt(
            1.0,
            QColor(255,220,0)
            );

        p.setFont(font());

        p.setPen(
            QPen(
                QBrush(gradient),
                0
                )
            );

        p.drawText(
            rect(),
            alignment(),
            text()
            );
    }
};

#endif