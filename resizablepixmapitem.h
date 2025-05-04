#ifndef RESIZABLEPIXMAPITEM_H
#define RESIZABLEPIXMAPITEM_H

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>

class ResizeHandle;

class ResizablePixmapItem : public QGraphicsPixmapItem
{
public:
    enum HandlePosition {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    ResizablePixmapItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);
    ~ResizablePixmapItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    void updateHandles();
    void createHandles();

    bool m_resizing;
    QPointF m_startPos;
    QSizeF m_originalSize;
    QTransform m_originalTransform;
    ResizeHandle *m_handles[4]; // 四个角落的控制点
    friend class ResizeHandle;
};

class ResizeHandle : public QGraphicsRectItem
{
public:
    ResizeHandle(ResizablePixmapItem *parent, ResizablePixmapItem::HandlePosition position);
    
    void updatePosition();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    ResizablePixmapItem *m_parent;
    ResizablePixmapItem::HandlePosition m_position;
    QPointF m_startPos;
    QSizeF m_startSize;
    QTransform m_startTransform;
    bool m_isResizing;
};

#endif // RESIZABLEPIXMAPITEM_H 