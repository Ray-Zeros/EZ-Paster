#include "resizablepixmapitem.h"
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QApplication>

const int HANDLE_SIZE = 10;

ResizablePixmapItem::ResizablePixmapItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsPixmapItem(pixmap, parent), m_resizing(false)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    
    m_originalSize = pixmap.size();
    createHandles();
}

ResizablePixmapItem::~ResizablePixmapItem()
{
    // 句柄是这个项的子项，会自动删除
}

void ResizablePixmapItem::createHandles()
{
    // 创建四个角落的控制点
    m_handles[TopLeft] = new ResizeHandle(this, TopLeft);
    m_handles[TopRight] = new ResizeHandle(this, TopRight);
    m_handles[BottomLeft] = new ResizeHandle(this, BottomLeft);
    m_handles[BottomRight] = new ResizeHandle(this, BottomRight);
    
    // 默认隐藏控制点，只在选择时显示
    for (int i = 0; i < 4; ++i) {
        m_handles[i]->setVisible(false);
    }
}

void ResizablePixmapItem::updateHandles()
{
    QRectF rect = boundingRect();
    
    // 更新控制点位置
    m_handles[TopLeft]->updatePosition();
    m_handles[TopRight]->updatePosition();
    m_handles[BottomLeft]->updatePosition();
    m_handles[BottomRight]->updatePosition();
}

void ResizablePixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPixmapItem::paint(painter, option, widget);
    
    // 当项被选中时，显示控制点
    bool showHandles = isSelected();
    for (int i = 0; i < 4; ++i) {
        m_handles[i]->setVisible(showHandles);
    }
}

QRectF ResizablePixmapItem::boundingRect() const
{
    return QGraphicsPixmapItem::boundingRect();
}

void ResizablePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_startPos = event->pos();
        m_originalTransform = transform();
    }
    QGraphicsPixmapItem::mousePressEvent(event);
}

void ResizablePixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPixmapItem::mouseMoveEvent(event);
}

void ResizablePixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPixmapItem::mouseReleaseEvent(event);
}

void ResizablePixmapItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsPixmapItem::hoverMoveEvent(event);
    setCursor(Qt::ArrowCursor);
}

QVariant ResizablePixmapItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged) {
        // 当选中状态改变时更新控制点可见性
        bool selected = value.toBool();
        for (int i = 0; i < 4; ++i) {
            m_handles[i]->setVisible(selected);
        }
    } else if (change == ItemTransformHasChanged || change == ItemPositionHasChanged) {
        // 当变换或位置改变时更新控制点位置
        updateHandles();
    }
    
    return QGraphicsPixmapItem::itemChange(change, value);
}

// ResizeHandle 实现

ResizeHandle::ResizeHandle(ResizablePixmapItem *parent, ResizablePixmapItem::HandlePosition position)
    : QGraphicsRectItem(0, 0, HANDLE_SIZE, HANDLE_SIZE, parent),
      m_parent(parent), m_position(position), m_isResizing(false)
{
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setAcceptHoverEvents(true);
    
    // 设置控制点样式
    setPen(QPen(Qt::white, 1));
    setBrush(QBrush(Qt::blue));
    
    updatePosition();
    
    // 设置不同角落的光标
    switch (position) {
        case ResizablePixmapItem::TopLeft:
        case ResizablePixmapItem::BottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case ResizablePixmapItem::TopRight:
        case ResizablePixmapItem::BottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
    }
}

void ResizeHandle::updatePosition()
{
    if (!m_parent)
        return;
    
    QRectF rect = m_parent->boundingRect();
    QPointF pos;
    
    // 计算控制点位置
    switch (m_position) {
        case ResizablePixmapItem::TopLeft:
            pos = rect.topLeft();
            break;
        case ResizablePixmapItem::TopRight:
            pos = rect.topRight();
            pos.rx() -= HANDLE_SIZE;
            break;
        case ResizablePixmapItem::BottomLeft:
            pos = rect.bottomLeft();
            pos.ry() -= HANDLE_SIZE;
            break;
        case ResizablePixmapItem::BottomRight:
            pos = rect.bottomRight();
            pos.rx() -= HANDLE_SIZE;
            pos.ry() -= HANDLE_SIZE;
            break;
    }
    
    // 转换到场景坐标
    QPointF scenePos = m_parent->mapToScene(pos);
    // 转换回控制点的父项坐标系（这里是ResizablePixmapItem）
    QPointF localPos = m_parent->mapFromScene(scenePos);
    
    setPos(localPos);
}

void ResizeHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);
}

void ResizeHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isResizing = true;
        m_startPos = event->scenePos();
        m_startSize = m_parent->boundingRect().size();
        m_startTransform = m_parent->transform();
        event->accept();
    } else {
        QGraphicsRectItem::mousePressEvent(event);
    }
}

void ResizeHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isResizing && m_parent) {
        QPointF delta = event->scenePos() - m_startPos;
        
        // 计算缩放比例
        qreal scaleX = 1.0;
        qreal scaleY = 1.0;
        
        QRectF rect = m_parent->boundingRect();
        QPointF center = rect.center();
        
        // 根据不同的控制点位置计算缩放方向
        switch (m_position) {
            case ResizablePixmapItem::TopLeft:
                scaleX = 1.0 - delta.x() / rect.width();
                scaleY = 1.0 - delta.y() / rect.height();
                break;
            case ResizablePixmapItem::TopRight:
                scaleX = 1.0 + delta.x() / rect.width();
                scaleY = 1.0 - delta.y() / rect.height();
                break;
            case ResizablePixmapItem::BottomLeft:
                scaleX = 1.0 - delta.x() / rect.width();
                scaleY = 1.0 + delta.y() / rect.height();
                break;
            case ResizablePixmapItem::BottomRight:
                scaleX = 1.0 + delta.x() / rect.width();
                scaleY = 1.0 + delta.y() / rect.height();
                break;
        }
        
        // 使用平均缩放比例实现等比例缩放
        qreal scale = (scaleX + scaleY) / 2.0;
        
        // 应用变换
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.scale(scale, scale);
        transform.translate(-center.x(), -center.y());
        
        m_parent->setTransform(transform);
        
        // 更新所有控制点位置
        m_parent->updateHandles();
        
        event->accept();
    } else {
        QGraphicsRectItem::mouseMoveEvent(event);
    }
}

void ResizeHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isResizing) {
        m_isResizing = false;
        event->accept();
    } else {
        QGraphicsRectItem::mouseReleaseEvent(event);
    }
}

void ResizeHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // 鼠标进入控制点时改变光标
    switch (m_position) {
        case ResizablePixmapItem::TopLeft:
        case ResizablePixmapItem::BottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case ResizablePixmapItem::TopRight:
        case ResizablePixmapItem::BottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
    }
    QGraphicsRectItem::hoverEnterEvent(event);
}

void ResizeHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(Qt::ArrowCursor);
    QGraphicsRectItem::hoverLeaveEvent(event);
} 