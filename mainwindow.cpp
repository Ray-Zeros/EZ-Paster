#include "mainwindow.h"
#include "draftwidget.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QTabWidget>
#include <QIcon>
#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>
#include <QImageWriter>
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QSettings>
#include <QMenu>
#include <QRubberBand>
#include <QTimer>
#include <QSlider>
#include <QLabel>
#include <QStatusBar>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QScreen>
#else
#include <QDesktopWidget>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      zoomFactor(1.0),
      m_selectionWidget(nullptr), // 初始化截图成员
      m_rubberBand(nullptr),
      m_isSelecting(false)
{
    loadSettings();
    setupUI();
    setupConnections();
    setupZoomControls();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 直接退出应用程序
    qApp->quit();
    event->accept();
}

void MainWindow::setupUI()
{
    // Central Tab Widget
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    setCentralWidget(tabWidget);

    // Actions
    newAction = new QAction(QIcon::fromTheme("document-new"), tr("新建草稿"), this);
    newAction->setShortcuts(QKeySequence::New);
    newAction->setStatusTip(tr("创建一个新的草稿纸"));

    exportAction = new QAction(QIcon::fromTheme("document-save-as"), tr("导出为JPG..."), this);
    exportAction->setShortcuts(QKeySequence::SaveAs);
    exportAction->setStatusTip(tr("将当前草稿导出为JPG图像"));
    exportAction->setEnabled(false);

    quitAction = new QAction(QIcon::fromTheme("application-exit"), tr("退出"), this);
    quitAction->setShortcuts(QKeySequence::Quit);
    quitAction->setStatusTip(tr("退出应用程序"));

    screenshotAction = new QAction(QIcon::fromTheme("camera-photo"), tr("截图"), this);
    screenshotAction->setShortcut(QKeySequence("F11"));
    screenshotAction->setStatusTip(tr("截取屏幕并粘贴到当前草稿"));

    // 缩放操作
    zoomInAction = new QAction(QIcon::fromTheme("zoom-in"), tr("放大"), this);
    zoomInAction->setStatusTip(tr("放大视图"));
    
    zoomOutAction = new QAction(QIcon::fromTheme("zoom-out"), tr("缩小"), this);
    zoomOutAction->setStatusTip(tr("缩小视图"));
    
    resetZoomAction = new QAction(QIcon::fromTheme("zoom-original"), tr("重置缩放"), this);
    resetZoomAction->setStatusTip(tr("重置缩放到100%"));

    // Menu Bar
    QMenu *fileMenu = menuBar()->addMenu(tr("文件"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    QMenu *toolsMenu = menuBar()->addMenu(tr("工具"));
    toolsMenu->addAction(screenshotAction);
    
    QMenu *viewMenu = menuBar()->addMenu(tr("视图"));
    viewMenu->addAction(zoomInAction);
    viewMenu->addAction(zoomOutAction);
    viewMenu->addAction(resetZoomAction);

    // Toolbar
    QToolBar *fileToolBar = addToolBar(tr("文件"));
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(exportAction);
    fileToolBar->addSeparator();
    fileToolBar->addAction(screenshotAction);
    
    QToolBar *viewToolBar = addToolBar(tr("视图"));
    viewToolBar->addAction(zoomInAction);
    viewToolBar->addAction(zoomOutAction);
    viewToolBar->addAction(resetZoomAction);

    // Status Bar
    statusBar(); // Creates a status bar

    // Initial state
    setWindowTitle(tr("EZ Paster"));
    resize(800, 600);

    // Create an initial draft
    createNewDraft();
}

void MainWindow::setupConnections()
{
    connect(newAction, &QAction::triggered, this, &MainWindow::createNewDraft);
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportCurrentDraft);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);
    connect(screenshotAction, &QAction::triggered, this, &MainWindow::captureScreenshot);
    
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeDraftTab);
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::updateActions);
    
    // 连接缩放操作
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(resetZoomAction, &QAction::triggered, this, &MainWindow::resetZoom);
}

void MainWindow::setupZoomControls()
{
    // 创建缩放滑块
    zoomSlider = new QSlider(Qt::Horizontal);
    zoomSlider->setRange(-10, 10); // 对应10%到1000%缩放
    zoomSlider->setValue(0);       // 初始为100%
    zoomSlider->setTickInterval(1);
    zoomSlider->setTickPosition(QSlider::TicksBelow);
    zoomSlider->setMaximumWidth(150);
    
    // 创建缩放标签
    zoomLabel = new QLabel(tr("缩放: 100%"));
    
    // 将它们添加到状态栏
    statusBar()->addPermanentWidget(zoomLabel);
    statusBar()->addPermanentWidget(zoomSlider);
    
    // 连接滑块到缩放功能
    connect(zoomSlider, &QSlider::valueChanged, this, &MainWindow::updateZoomLevel);
}

void MainWindow::zoomIn()
{
    applyZoom(zoomFactor * 1.2);
}

void MainWindow::zoomOut()
{
    applyZoom(zoomFactor / 1.2);
}

void MainWindow::resetZoom()
{
    applyZoom(1.0);
}

void MainWindow::updateZoomLevel(int value)
{
    // 将滑块值转换为缩放因子
    qreal factor;
    if (value >= 0) {
        factor = 1.0 + value * 0.1; // 100% - 200%
    } else {
        factor = 1.0 / (1.0 - value * 0.1); // 100% - 10%
    }
    
    applyZoom(factor);
}

void MainWindow::applyZoom(qreal factor)
{
    // 限制缩放范围
    if (factor < 0.1) factor = 0.1;
    if (factor > 5.0) factor = 5.0;
    
    zoomFactor = factor;
    
    // 更新当前草稿的缩放
    DraftWidget *currentDraft = qobject_cast<DraftWidget*>(tabWidget->currentWidget());
    if (currentDraft) {
        currentDraft->setZoomFactor(zoomFactor);
    }
    
    // 更新滑块位置
    int sliderValue;
    if (zoomFactor >= 1.0) {
        sliderValue = qRound((zoomFactor - 1.0) * 10);
    } else {
        sliderValue = -qRound((1.0 / zoomFactor - 1.0) * 10);
    }
    
    // 更新UI，阻断信号避免递归
    zoomSlider->blockSignals(true);
    zoomSlider->setValue(sliderValue);
    zoomSlider->blockSignals(false);
    
    // 更新标签
    zoomLabel->setText(tr("缩放: %1%").arg(qRound(zoomFactor * 100)));
}

void MainWindow::createNewDraft()
{
    DraftWidget *draft = new DraftWidget(this);
    draft->setZoomFactor(zoomFactor); // 应用当前主窗口的缩放级别到新草稿
    int index = tabWidget->addTab(draft, tr("草稿 %1").arg(tabWidget->count() + 1));
    tabWidget->setCurrentIndex(index);
    updateActions();
}

void MainWindow::closeDraftTab(int index)
{
    if (index >= 0 && index < tabWidget->count()) {
        // Add confirmation dialog here if needed (check for unsaved changes)
        QWidget *widget = tabWidget->widget(index);
        tabWidget->removeTab(index);
        delete widget; // Delete the DraftWidget
        updateActions(); // Disable export if no tabs left
    }
}

void MainWindow::exportCurrentDraft()
{
    DraftWidget *currentDraft = qobject_cast<DraftWidget*>(tabWidget->currentWidget());
    if (!currentDraft)
        return;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                   tr("导出草稿"),
                                                   "",
                                                   tr("JPEG图像 (*.jpg);;所有文件 (*.*)"));

    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        if (fi.suffix().isEmpty()) {
            fileName += ".jpg";
        }

        // Render the scene to a QPixmap
        QPixmap pixmap(currentDraft->sceneRect().size().toSize());
        pixmap.fill(Qt::white); // Set a background color for export
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        currentDraft->scene()->render(&painter);
        painter.end();

        // Save the pixmap
        if (!pixmap.save(fileName)) {
            QMessageBox::warning(this, tr("导出失败"), tr("无法将图像保存到 %1。").arg(fileName));
        }
    }
}

void MainWindow::updateActions()
{
    // Enable/disable actions based on whether any tabs are open
    bool hasTabs = tabWidget->count() > 0;
    exportAction->setEnabled(hasTabs);
    screenshotAction->setEnabled(hasTabs);
}

void MainWindow::captureScreenshot()
{
    // 如果正在进行截图，则忽略
    if (m_selectionWidget) {
        return;
    }

    // 隐藏主窗口以便拍摄屏幕
    this->hide();
    // 稍作延迟确保窗口已隐藏
    QTimer::singleShot(200, this, [this]() {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (!screen) {
            qWarning("无法获取主屏幕");
            this->show();
            return;
        }
        // 抓取全屏
        m_fullScreenshot = screen->grabWindow(0);
        if (m_fullScreenshot.isNull()) {
             qWarning("抓取屏幕失败");
             this->show();
             return;
        }

        // 创建全屏半透明窗口用于选择区域
        m_selectionWidget = new QWidget(nullptr, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
        m_selectionWidget->setWindowState(Qt::WindowFullScreen);
        // 设置窗口透明度，让背景图片可见
        m_selectionWidget->setAttribute(Qt::WA_TranslucentBackground);
        m_selectionWidget->setCursor(Qt::CrossCursor); // 设置十字光标

        // 创建橡皮筋选择框
        m_rubberBand = new QRubberBand(QRubberBand::Rectangle, m_selectionWidget);
        // 可以自定义橡皮筋样式
        // QPalette pal;
        // pal.setBrush(QPalette::Highlight, QBrush(Qt::red));
        // m_rubberBand->setPalette(pal);
        m_rubberBand->setStyleSheet("border: 2px solid red; background-color: rgba(255, 255, 255, 10);");


        // 使用事件过滤器捕获鼠标事件
        m_selectionWidget->installEventFilter(this);
        m_isSelecting = false; // 重置选择状态

        // 连接销毁信号以进行清理
        connect(m_selectionWidget, &QWidget::destroyed, this, &MainWindow::cleanupScreenshot);

        // 显示选择窗口
        m_selectionWidget->show();
        m_selectionWidget->activateWindow(); // 确保窗口获得焦点以接收键盘事件

        // 在选择窗口上绘制半透明遮罩和背景（可选，另一种方法）
        // QPainter painter(m_selectionWidget);
        // QPixmap semiTransparentPixmap = m_fullScreenshot;
        // QPainter p(&semiTransparentPixmap);
        // p.fillRect(semiTransparentPixmap.rect(), QColor(0, 0, 0, 100)); // 半透明黑色
        // p.end();
        // painter.drawPixmap(0, 0, semiTransparentPixmap);

    });
}

void MainWindow::cleanupScreenshot()
{
    // 这个槽函数在 m_selectionWidget 被销毁时调用
    // 确保 MainWindow 中的指针被清理
    m_selectionWidget = nullptr;
    m_rubberBand = nullptr; // rubberBand 是 selectionWidget 的子控件，会被自动删除
    m_isSelecting = false;
    m_fullScreenshot = QPixmap(); // 清空截图缓存

    // 如果主窗口仍然隐藏，则显示它
    if (!this->isVisible()) {
        this->show();
        this->activateWindow();
    }
}

void MainWindow::handleScreenshotResult(const QPixmap &pixmap)
{
     if (pixmap.isNull()) return;

     // 复制到剪贴板
     QGuiApplication::clipboard()->setPixmap(pixmap);

     // 粘贴到当前草稿
     DraftWidget *currentDraft = qobject_cast<DraftWidget*>(tabWidget->currentWidget());
     if (currentDraft) {
         currentDraft->pasteImageFromClipboard();
     } else if (tabWidget->count() == 0) {
         // 如果没有标签，创建一个新的
         createNewDraft();
         currentDraft = qobject_cast<DraftWidget*>(tabWidget->currentWidget());
         if (currentDraft) {
             // 确保新草稿的缩放设置正确
             currentDraft->setZoomFactor(zoomFactor);
             currentDraft->pasteImageFromClipboard();
         }
     }

     // 如果主窗口仍然隐藏，则显示它
     if (!this->isVisible()) {
         this->show();
         this->activateWindow();
     }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // 只处理我们关心的选择窗口的事件
    if (watched == m_selectionWidget) {
        switch (event->type()) {
            case QEvent::MouseButtonPress: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::LeftButton) {
                    m_selStartPos = mouseEvent->pos();
                    m_rubberBand->setGeometry(QRect(m_selStartPos, QSize()));
                    m_rubberBand->show();
                    m_isSelecting = true;
                    return true; // 事件已处理
                }
                break;
            }
            case QEvent::MouseMove: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (m_isSelecting) {
                    m_selEndPos = mouseEvent->pos();
                    m_rubberBand->setGeometry(QRect(m_selStartPos, m_selEndPos).normalized());
                    return true; // 事件已处理
                }
                break;
            }
            case QEvent::MouseButtonRelease: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::LeftButton && m_isSelecting) {
                    m_selEndPos = mouseEvent->pos();
                    m_isSelecting = false;
                    QRect selectedRect = QRect(m_selStartPos, m_selEndPos).normalized();

                    // 检查选区大小，避免误操作
                    if (selectedRect.width() > 4 && selectedRect.height() > 4) {
                        // 截取选定区域
                        QPixmap selectedPixmap = m_fullScreenshot.copy(selectedRect);
                        // 处理截图结果
                        handleScreenshotResult(selectedPixmap);
                    }

                    // 关闭并标记删除选择窗口
                    m_selectionWidget->removeEventFilter(this); // 移除过滤器
                    m_selectionWidget->close();
                    m_selectionWidget->deleteLater(); // 安全删除

                    return true; // 事件已处理
                }
                break;
            }
            case QEvent::KeyPress: {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_Escape) {
                    m_isSelecting = false;
                    // 关闭并标记删除选择窗口
                    m_selectionWidget->removeEventFilter(this); // 移除过滤器
                    m_selectionWidget->close();
                    m_selectionWidget->deleteLater(); // 安全删除
                    // 如果用户取消截图，也需要显示主窗口
                     if (!this->isVisible()) {
                         this->show();
                         this->activateWindow();
                     }
                    return true; // 事件已处理
                }
                break;
            }
             case QEvent::Paint: {
                  // 在选择窗口上绘制背景和半透明遮罩
                  if (m_selectionWidget && !m_fullScreenshot.isNull()) {
                      QPainter painter(m_selectionWidget);
                      // 绘制背景图
                      painter.drawPixmap(0, 0, m_fullScreenshot);
                      // 绘制半透明遮罩
                      painter.fillRect(m_selectionWidget->rect(), QColor(0, 0, 0, 70));
                  }
                 // 不返回true，让窗口继续处理绘制
                 break;
             }
            default:
                break;
        }
    }

    // 对于其他对象或其他事件，调用基类的事件过滤器
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::loadSettings()
{
    QSettings settings("YourCompany", "EZ Paster");
    // screenshotHotkeyEnabled = settings.value("screenshotHotkeyEnabled", false).toBool();
    
    // Window position
    if (settings.contains("windowGeometry")) {
        restoreGeometry(settings.value("windowGeometry").toByteArray());
    }
}

void MainWindow::saveSettings()
{
    QSettings settings("YourCompany", "EZ Paster");
    // settings.setValue("screenshotHotkeyEnabled", screenshotHotkeyEnabled);
    settings.setValue("windowGeometry", saveGeometry());
}
