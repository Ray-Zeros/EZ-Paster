#ifndef DRAFTWIDGET_H
#define DRAFTWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
// #include <QGraphicsLineItem>  // 可以删除因为不再需要连线

// Forward declarations
class QKeyEvent;
class QDragEnterEvent;
class QDropEvent;
class QGraphicsItem;

// 删除整个 ConnectionLine 类

class DraftWidget : public QGraphicsView
{
    Q_OBJECT

public:
    explicit DraftWidget(QWidget *parent = nullptr);
    ~DraftWidget() override;

    void pasteImageFromClipboard();
    QGraphicsScene* scene() const { return m_scene; }
    QRectF sceneRect() const { return m_scene->sceneRect(); }
    
    // 设置缩放系数
    void setZoomFactor(qreal factor);
    qreal zoomFactor() const { return m_zoomFactor; }

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    // 确认 eventFilter 声明已删除
    // bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QGraphicsScene *m_scene;
    qreal m_zoomFactor;
    
    // 删除所有连线相关的成员变量
    // bool m_isDrawingLine;
    // QGraphicsItem *m_lineStartItem;
    // QGraphicsLineItem *m_tempLine;
    // QPointF m_lineStartPoint;
    // QList<ConnectionLine*> m_connectionLines;
    
    // 删除连线相关的函数
    // void updateConnectionLines();
    
    // 删除连线相关的辅助函数
    // bool isValidItem(QGraphicsItem* item) const { ... }
};

#endif // DRAFTWIDGET_H 