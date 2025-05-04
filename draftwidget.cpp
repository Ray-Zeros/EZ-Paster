#include "draftwidget.h"
#include "resizablepixmapitem.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <QImage>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QFileInfo>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTransform>

DraftWidget::DraftWidget(QWidget *parent)
    : QGraphicsView(parent),
      m_zoomFactor(1.0)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setAcceptDrops(true);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // 设置白色背景
    m_scene->setBackgroundBrush(Qt::white);
}

DraftWidget::~DraftWidget()
{
    // scene由QGraphicsView管理，不需要手动删除
}

void DraftWidget::setZoomFactor(qreal factor)
{
    // 限制缩放范围
    if (factor < 0.1)
        factor = 0.1;
    if (factor > 5.0)
        factor = 5.0;
        
    m_zoomFactor = factor;
    
    // 应用缩放
    QTransform transform;
    transform.scale(m_zoomFactor, m_zoomFactor);
    setTransform(transform);
}

void DraftWidget::pasteImageFromClipboard()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (mimeData->hasImage()) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        if (!image.isNull()) {
            // 使用自定义的 ResizablePixmapItem 替代 QGraphicsPixmapItem
            ResizablePixmapItem *item = new ResizablePixmapItem(QPixmap::fromImage(image));
            m_scene->addItem(item);
            
            // 将图像放置在视图中心
            item->setPos(mapToScene(viewport()->rect().center()) - QPointF(image.width()/2, image.height()/2));
            
            // 图像自带选择和移动功能，不需要额外设置
            // item->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
        }
    }
}

void DraftWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Paste)) {
        pasteImageFromClipboard();
        event->accept();
    } else if (event->key() == Qt::Key_Delete) {
        // 删除选中项 - 简化版，不需要处理连线
        QList<QGraphicsItem*> selectedItems = m_scene->selectedItems();
        for (QGraphicsItem *item : selectedItems) {
            m_scene->removeItem(item);
            delete item;
        }
        event->accept();
    } else {
        QGraphicsView::keyPressEvent(event);
    }
}

void DraftWidget::wheelEvent(QWheelEvent *event)
{
    // 获取当前缩放
    qreal factor = m_zoomFactor;
    
    // 根据滚轮方向调整缩放
    if (event->angleDelta().y() > 0) {
        // 放大
        factor *= 1.1;
    } else {
        // 缩小
        factor /= 1.1;
    }
    
    // 应用新的缩放
    setZoomFactor(factor);
    
    event->accept();
}

void DraftWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() || event->mimeData()->hasImage()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void DraftWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasImage()) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        if (!image.isNull()) {
            ResizablePixmapItem *item = new ResizablePixmapItem(QPixmap::fromImage(image));
            m_scene->addItem(item);
            item->setPos(mapToScene(event->position().toPoint()));
            event->acceptProposedAction();
            return;
        }
    } else if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);
                QStringList supportedFormats = {"png", "jpg", "jpeg", "bmp", "gif"};
                if (supportedFormats.contains(fileInfo.suffix().toLower())) {
                    QPixmap pixmap(filePath);
                    if (!pixmap.isNull()) {
                        ResizablePixmapItem *item = new ResizablePixmapItem(pixmap);
                        m_scene->addItem(item);
                        item->setPos(mapToScene(event->position().toPoint()));
                    }
                }
            }
        }
        event->acceptProposedAction();
        return;
    }

    event->ignore();
}

void DraftWidget::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
}

void DraftWidget::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void DraftWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
} 