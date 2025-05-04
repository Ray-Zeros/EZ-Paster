#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QPixmap>

// Forward declarations to reduce header dependencies
class QAction;
class QTabWidget;
class DraftWidget; // Forward declare DraftWidget
class QSlider;
class QLabel;
class QRubberBand;
class QEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void createNewDraft();
    void closeDraftTab(int index);
    void exportCurrentDraft();
    void updateActions();
    void captureScreenshot();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void updateZoomLevel(int value);
    void cleanupScreenshot();

private:
    void setupUI();
    void setupConnections();
    void setupZoomControls();
    void loadSettings();
    void saveSettings();
    void applyZoom(qreal factor);
    void handleScreenshotResult(const QPixmap &pixmap);

    QTabWidget *tabWidget;

    // Actions
    QAction *newAction;
    QAction *exportAction;
    QAction *quitAction;
    QAction *screenshotAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *resetZoomAction;

    // Zoom controls
    QSlider *zoomSlider;
    QLabel *zoomLabel;
    qreal zoomFactor;

    // Screenshot temporary members
    QWidget *m_selectionWidget;
    QRubberBand *m_rubberBand;
    QPoint m_selStartPos;
    QPoint m_selEndPos;
    bool m_isSelecting;
    QPixmap m_fullScreenshot;
};
#endif // MAINWINDOW_H 