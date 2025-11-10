#pragma once
#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QTableWidget>
#include <QComboBox>

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
    // UI 초기화/헬퍼
    void setupUi();
    void loadImageFile(const QString& path);
    void setOriginal(const QImage& img);
    void setProcessed(const QImage& img);
    void scaleAndShow(QLabel* label, const QImage& img);
    void refreshPerfTable();

    // 현재 상태
    QImage original_;
    QImage cppImg_, asmImg_, pyImg_;
    qint64 cppMs_ = 0, asmMs_ = 0, pyMs_ = 0;
    QString cppNotes_ = "placeholder (not wired yet)";
    QString asmNotes_ = "placeholder (not wired yet)";
    QString pyNotes_  = "placeholder (not wired yet)";

    // 위젯
    QLabel* originalView_ = nullptr;
    QLabel* processedView_ = nullptr;
    QTableWidget* perfTable_ = nullptr;
    QComboBox* resultSource_ = nullptr;

    // 유틸: 플레이스홀더 처리 (임시)
    QImage placeholderGrayscale(const QImage& src) const;
};
