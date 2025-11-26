#pragma once
#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QTableWidget>
#include <QComboBox>
#include <QTemporaryDir>
#include <QToolButton>
#include <QStackedLayout>
#include <QTimer>

class ThrobberWidget : public QWidget {
    Q_OBJECT
public:
    explicit ThrobberWidget(QWidget* parent = nullptr);
    void start();
    void stop();
    bool isSpinning() const { return timer_.isActive(); }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QTimer timer_;
    int angle_ = 0;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dropEvent(QDropEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void onOpen();
    void onRunAll();
    void onResultSourceChanged(int idx);

private:
    void setupUi();
    void loadImageFile(const QString& path);
    void setOriginal(const QImage& img);
    void setProcessed(const QImage& img);
    void scaleAndShow(QLabel* label, const QImage& img);
    void refreshPerfTable();
    QString pythonScriptPath() const;
    bool runPythonProcessor(const QString& inputPath, const QString& outputPath, qint64& elapsedMs, QString& notes);
    void pumpEvents();

    QImage original_;
    QImage cppImg_, asmImg_, pyImg_;
    qint64 cppMs_ = 0, asmMs_ = 0, pyMs_ = 0;
    QString cppNotes_ = "not run yet";
    QString asmNotes_ = "placeholder (not wired yet)";
    QString pyNotes_  = "not run yet";

    QLabel* originalView_ = nullptr;
    QLabel* processedView_ = nullptr;
    QWidget* processedContainer_ = nullptr;
    QStackedLayout* processedStack_ = nullptr;
    ThrobberWidget* throbber_ = nullptr;
    QTableWidget* perfTable_ = nullptr;
    QComboBox* resultSource_ = nullptr;
    QToolButton* runButton_ = nullptr;
};