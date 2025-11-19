#include "MainWindow.h"
#include <QMenuBar>
#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QMessageBox>
#include <QImageReader>
#include <QElapsedTimer>
#include <QPixmap>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    setAcceptDrops(true);
}

void MainWindow::setupUi() {
    auto fileMenu = menuBar()->addMenu("&File");
    auto actOpen = fileMenu->addAction("Open Image...");
    connect(actOpen, &QAction::triggered, this, &MainWindow::onOpen);

    auto runMenu  = menuBar()->addMenu("&Run");
    auto actRunAll= runMenu->addAction("Run All (C++ / ASM / Python)");
    connect(actRunAll, &QAction::triggered, this, &MainWindow::onRunAll);

    resultSource_ = new QComboBox(this);
    resultSource_->addItems({"C++", "ASM", "Python", "Fastest"});
    connect(resultSource_, &QComboBox::currentIndexChanged,
            this, &MainWindow::onResultSourceChanged);
    menuBar()->setCornerWidget(resultSource_, Qt::TopRightCorner);

    originalView_  = new QLabel("Drop or Open an image", this);
    processedView_ = new QLabel("Processed will appear here", this);
    originalView_->setAlignment(Qt::AlignCenter);
    processedView_->setAlignment(Qt::AlignCenter);
    originalView_->setMinimumSize(200,200);
    processedView_->setMinimumSize(200,200);
    originalView_->setStyleSheet("QLabel{background:#222;color:#bbb;border:1px solid #444;}");
    processedView_->setStyleSheet("QLabel{background:#222;color:#bbb;border:1px solid #444;}");

    auto views = new QHBoxLayout();
    views->addWidget(originalView_, 1);
    views->addWidget(processedView_, 1);

    perfTable_ = new QTableWidget(3, 3, this);
    perfTable_->setHorizontalHeaderLabels({"Processor","Time (ms)","Notes"});
    perfTable_->verticalHeader()->setVisible(false);
    perfTable_->horizontalHeader()->setStretchLastSection(true);
    perfTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    perfTable_->setSelectionMode(QAbstractItemView::NoSelection);
    perfTable_->setFixedHeight(140);

    auto central = new QWidget(this);
    auto v = new QVBoxLayout(central);
    v->addLayout(views, 1);
    v->addWidget(perfTable_, 0);
    setCentralWidget(central);

    refreshPerfTable();
}

void MainWindow::onOpen() {
    const QString path = QFileDialog::getOpenFileName(
        this, "Open Image", QString(),
        "Images (*.png *.jpg *.jpeg *.bmp *.gif *.tiff)");
    if (!path.isEmpty()) loadImageFile(path);
}

void MainWindow::loadImageFile(const QString& path) {
    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QImage img = reader.read();
    if (img.isNull()) {
        QMessageBox::warning(this, "Load failed", "Could not load image:\n" + path);
        return;
    }
    setOriginal(img);
    processedView_->setText("Ready. Run All to process.");
}

void MainWindow::setOriginal(const QImage& img) {
    original_ = img;
    scaleAndShow(originalView_, original_);
}

void MainWindow::setProcessed(const QImage& img) {
    scaleAndShow(processedView_, img);
}

void MainWindow::scaleAndShow(QLabel* label, const QImage& img) {
    if (img.isNull()) { label->setText("No image"); return; }
    const QSize area = label->size() * label->devicePixelRatioF();
    QPixmap pm = QPixmap::fromImage(img);
    label->setPixmap(pm.scaled(area, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e) {
    if (e->mimeData()->hasUrls()) e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* e) {
    const auto urls = e->mimeData()->urls();
    if (urls.isEmpty()) return;
    const auto path = urls.first().toLocalFile();
    if (!path.isEmpty()) loadImageFile(path);
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    if (!original_.isNull())  scaleAndShow(originalView_, original_);
    if (!cppImg_.isNull() && resultSource_->currentIndex()==0)      setProcessed(cppImg_);
    else if (!asmImg_.isNull() && resultSource_->currentIndex()==1) setProcessed(asmImg_);
    else if (!pyImg_.isNull()  && resultSource_->currentIndex()==2) setProcessed(pyImg_);
}

QImage MainWindow::placeholderGrayscale(const QImage& src) const {
    return src.convertToFormat(QImage::Format_Grayscale8);
}

void MainWindow::onRunAll() {
    if (original_.isNull()) {
        QMessageBox::information(this, "No image", "Open or drop an image first.");
        return;
    }

    QElapsedTimer t;

    t.start();
    cppImg_ = placeholderGrayscale(original_);
    cppMs_  = t.elapsed();
    cppNotes_ = "placeholder (replace with C++ processor)";

    t.restart();
    asmImg_ = cppImg_;
    asmMs_  = t.elapsed();
    asmNotes_ = "placeholder (replace with ASM processor)";

    t.restart();
    pyImg_  = cppImg_;
    pyMs_   = t.elapsed();
    pyNotes_ = "placeholder (replace with Python processor)";

    refreshPerfTable();

    setProcessed(cppImg_);
    resultSource_->setCurrentIndex(0);
}

void MainWindow::refreshPerfTable() {
    struct Row { const char* name; qint64 ms; const QString* notes; };
    Row rows[3] = {
        {"C++",    cppMs_, &cppNotes_},
        {"ASM",    asmMs_, &asmNotes_},
        {"Python", pyMs_,  &pyNotes_}
    };
    perfTable_->setRowCount(3);
    for (int i=0;i<3;++i) {
        perfTable_->setItem(i, 0, new QTableWidgetItem(rows[i].name));
        perfTable_->setItem(i, 1, new QTableWidgetItem(QString::number(rows[i].ms)));
        perfTable_->setItem(i, 2, new QTableWidgetItem(*rows[i].notes));
    }
}

void MainWindow::onResultSourceChanged(int idx) {
    if (original_.isNull()) return;
    switch (idx) {
        case 0: if (!cppImg_.isNull()) setProcessed(cppImg_); break;
        case 1: if (!asmImg_.isNull()) setProcessed(asmImg_); break;
        case 2: if (!pyImg_.isNull())  setProcessed(pyImg_);  break;
        case 3: {
            qint64 best = cppMs_; QImage bestImg = cppImg_;
            if (asmMs_ >= 0 && (best <= 0 || (asmMs_ > 0 && asmMs_ < best))) { best = asmMs_; bestImg = asmImg_; }
            if (pyMs_  >= 0 && (best <= 0 || (pyMs_  > 0 && pyMs_  < best))) { best = pyMs_;  bestImg = pyImg_;  }
            if (!bestImg.isNull()) setProcessed(bestImg);
            break;
        }
        default: break;
    }
}