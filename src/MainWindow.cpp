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
#include <QProcess>
#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QApplication>
#include <QEventLoop>
#include <QToolButton>
#include <QKeySequence>
#include <QStandardPaths>

#include "../processors/cpp/noirify_cpp.h"
#include "../processors/asm/noirify_simd.h"

ThrobberWidget::ThrobberWidget(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    timer_.setInterval(80);
    connect(&timer_, &QTimer::timeout, this, [this] {
        angle_ = (angle_ + 30) % 360;
        update();
    });
}

void ThrobberWidget::start() {
    if (!timer_.isActive()) {
        timer_.start();
        show();
    }
}

void ThrobberWidget::stop() {
    timer_.stop();
    angle_ = 0;
    hide();
}

void ThrobberWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    if (!timer_.isActive()) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int spinnerSize = 48;
    const int radius = spinnerSize / 2 - 6;
    const QPointF center(width() / 2.0, height() / 2.0);

    for (int i = 0; i < 12; ++i) {
        p.save();
        p.translate(center);
        p.rotate(angle_ + i * 30);
        const qreal opacity = qMin<qreal>(1.0, 0.15 + (static_cast<qreal>(i) / 12.0));
        QColor c(255, 255, 255);
        c.setAlphaF(opacity);
        p.setBrush(c);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(radius, 0), 4, 4);
        p.restore();
    }
}


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    setAcceptDrops(true);
}

void MainWindow::setupUi() {
    menuBar()->setNativeMenuBar(false);

    auto fileMenu = menuBar()->addMenu("&File");
    auto actOpen = fileMenu->addAction("Open Image...");
    connect(actOpen, &QAction::triggered, this, &MainWindow::onOpen);

    actSaveResult_ = fileMenu->addAction("Save Result...");
    actSaveResult_->setShortcut(QKeySequence::Save);
    actSaveResult_->setEnabled(false);
    connect(actSaveResult_, &QAction::triggered, this, &MainWindow::onSaveResult);

    fileMenu->addSeparator();


    auto runMenu  = menuBar()->addMenu("&Run");
    auto actRunAll= runMenu->addAction("Run All (C++ / ASM / Python)");
    connect(actRunAll, &QAction::triggered, this, &MainWindow::onRunAll);

    resultSource_ = new QComboBox(this);
    resultSource_->addItems({"C++", "ASM", "Python", "Fastest"});
    resultSource_->setStyleSheet(
        "QComboBox {"
        "  color: #f6f6f6;"
        "  background: #222;"
        "  border: 1px solid #444;"
        "  border-radius: 6px;"
        "  padding: 4px 10px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  color: #f6f6f6;"
        "  background: #222;"
        "  selection-background-color: #444;"
        "  border: 1px solid #444;"
        "}"
    );

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

    processedContainer_ = new QWidget(this);
    processedStack_ = new QStackedLayout(processedContainer_);
    processedStack_->setStackingMode(QStackedLayout::StackAll);
    processedStack_->setContentsMargins(0, 0, 0, 0);
    processedStack_->addWidget(processedView_);
    throbber_ = new ThrobberWidget(processedContainer_);
    processedStack_->addWidget(throbber_);
    processedStack_->setAlignment(throbber_, Qt::AlignCenter);
    throbber_->hide();

    runButton_ = new QToolButton(this);
    runButton_->setCursor(Qt::PointingHandCursor);
    runButton_->setIcon(QIcon(":/noirify.png"));
    runButton_->setIconSize(QSize(96, 96));
    runButton_->setText("Noirify");
    runButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    runButton_->setMinimumSize(120, 100);
    runButton_->setStyleSheet(
        "QToolButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2f2f2f, stop:1 #1f1f1f);"
        "  color: #f6f6f6;"
        "  border: 1px solid #3b3b3b;"
        "  border-radius: 12px;"
        "  padding: 18px 22px 14px;"
        "  font-size: 18px;"
        "  font-weight: 700;"
        "  letter-spacing: 1px;"
        "}"
        "QToolButton:hover { background: #383838; border-color: #565656; }"
        "QToolButton:pressed { background: #2c2c2c; }"
    );
    connect(runButton_, &QToolButton::clicked, this, &MainWindow::onRunAll);

    auto buttonWrapper = new QWidget(this);
    auto buttonLayout = new QVBoxLayout(buttonWrapper);
    buttonLayout->setContentsMargins(12, 0, 12, 0);
    // buttonLayout->addStretch();
    // buttonLayout->addWidget(runButton_, 0, Qt::AlignCenter);
    // buttonLayout->addStretch();
    saveButton_ = new QToolButton(this);
    saveButton_->setCursor(Qt::PointingHandCursor);
    saveButton_->setText("Save Result");
    saveButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    saveButton_->setIcon(QIcon::fromTheme("document-save"));
    saveButton_->setIconSize(QSize(36, 36));
    saveButton_->setMinimumSize(120, 90);
    saveButton_->setEnabled(false); // ✅ 기본 비활성
    saveButton_->setStyleSheet(
        "QToolButton {"
        "  background: #1f1f1f;"
        "  color: #f6f6f6;"
        "  border: 1px solid #3b3b3b;"
        "  border-radius: 12px;"
        "  padding: 12px 18px 10px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QToolButton:hover { background: #2a2a2a; border-color: #565656; }"
        "QToolButton:pressed { background: #181818; }"
    );
    connect(saveButton_, &QToolButton::clicked, this, &MainWindow::onSaveResult);
    buttonLayout->addStretch();
    buttonLayout->addWidget(runButton_, 0, Qt::AlignCenter);
    buttonLayout->addSpacing(12);
    buttonLayout->addWidget(saveButton_, 0, Qt::AlignCenter);
    buttonLayout->addStretch();


    auto views = new QHBoxLayout();
    views->setSpacing(24);
    views->addWidget(originalView_, 1);
    views->addWidget(buttonWrapper, 0);
    views->addWidget(processedContainer_, 1);

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

    originalPath_ = path;

    cppImg_ = QImage();
    asmImg_ = QImage();
    pyImg_  = QImage();

    cppMs_ = 0; asmMs_ = 0; pyMs_ = 0;
    cppNotes_ = "not run yet";
    asmNotes_ = "not run yet";
    pyNotes_  = "not run yet";

    processedView_->setPixmap(QPixmap());
    processedView_->setText("Ready. Run All to process.");

    refreshPerfTable();
    updateSaveEnabled();

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
QImage prepareImageForASM(const QImage& img) {
    return img.convertToFormat(QImage::Format_RGBA8888);
}


void MainWindow::onRunAll() {
    if (original_.isNull()) {
        QMessageBox::information(this, "No image", "Open or drop an image first.");
        return;
    }

    throbber_->start();
    throbber_->raise();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    QElapsedTimer t;

    t.start();
    cppImg_ = noirify_cpp::convertToGrayscale(original_);
    cppMs_  = t.elapsed();
    cppNotes_ = cppImg_.isNull() ? "C++ processor failed" : "C++ processor executed successfully";
    refreshPerfTable();
    pumpEvents();

    asmNotes_ = "Processing...";
    refreshPerfTable();
    pumpEvents();

    QElapsedTimer tAsm;
    tAsm.start();

    // Przygotowanie obrazu dla ASM
    QImage asmCopy = prepareImageForASM(original_);
    uint8_t* buffer = asmCopy.bits();
    int width  = asmCopy.width();
    int height = asmCopy.height();

    // Ustawienie wag kolorów
    set_rgb(77, 150, 29);

    // Wywołanie ASM
    to_grayscale(buffer, width, height);

    // Zapis wyniku
    asmImg_ = asmCopy;
    asmMs_ = tAsm.elapsed();
    asmNotes_ = "ASM processor executed successfully";

    refreshPerfTable();
    pumpEvents();

    pyNotes_ = "Processing...";
    refreshPerfTable();
    pumpEvents();
    t.restart();
    pyImg_  = cppImg_;
    pyMs_   = -1;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        pyNotes_ = "Python3 processor unavailable (no temp dir)";
    } else {
        const QString inputPath = tempDir.filePath("noirify_input.png");
        const QString outputPath = tempDir.filePath("noirify_output.png");

        if (!original_.save(inputPath)) {
            pyNotes_ = "Python processor unavailable (cannot write input)";
        } else {
            if (runPythonProcessor(inputPath, outputPath, pyMs_, pyNotes_)) {
                const QImage result(outputPath);
                if (!result.isNull()) {
                    pyImg_ = result;
                } else {
                    pyNotes_ = "Python processor output missing";
                }
            }
        }
    }

    refreshPerfTable();

    setProcessed(cppImg_);
    resultSource_->setCurrentIndex(0);

    throbber_->stop();
    throbber_->hide();

    updateSaveEnabled();

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

void MainWindow::pumpEvents() {
    QApplication::processEvents(QEventLoop::AllEvents, 16);
}

QString MainWindow::pythonScriptPath() const {
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList candidates = {
        QDir(appDir.filePath(".."))
            .filePath("processors/python/noirify.py"),
        appDir.filePath("processors/python/noirify.py")
    };

    for (const auto& candidate : candidates) {
        if (QFile::exists(candidate)) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }
    return {};
}

bool MainWindow::runPythonProcessor(const QString& inputPath, const QString& outputPath,
                                    qint64& elapsedMs, QString& notes) {
    const QString script = pythonScriptPath();
    if (script.isEmpty()) {
        elapsedMs = 0;
        notes = "Python processor script not found";
        return false;
    }

    QProcess proc;
    QStringList args{script, inputPath, outputPath};

    QElapsedTimer timer;
    timer.start();
    proc.start("python", args);

    while (proc.state() == QProcess::Starting || proc.state() == QProcess::Running) {
        if (proc.waitForFinished(50)) break;
        QApplication::processEvents(QEventLoop::AllEvents, 16);
        if (timer.elapsed() > 30000) {
            proc.kill();
            proc.waitForFinished();
            elapsedMs = timer.elapsed();
            notes = "Python processor timed out";
            return false;
        }
    }

    elapsedMs = timer.elapsed();

    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        const QString stderrOut = QString::fromUtf8(proc.readAllStandardError()).trimmed();
        notes = stderrOut.isEmpty()
                ? QStringLiteral("Python processor failed")
                : QStringLiteral("Python error: %1").arg(stderrOut);
        return false;
    }

    if (!QFile::exists(outputPath)) {
        notes = "Python processor did not create output";
        return false;
    }

    notes = "Python processor executed successfully";
    return true;
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
    updateSaveEnabled();
}

QImage MainWindow::currentResultImage() const {
    const int idx = resultSource_ ? resultSource_->currentIndex() : 0;

    switch (idx) {
        case 0: return cppImg_;
        case 1: return asmImg_;
        case 2: return pyImg_;
        case 3: { // Fastest
            struct Cand { qint64 ms; QImage img; };
            QList<Cand> cands;

            if (!cppImg_.isNull() && cppMs_ > 0) cands.push_back({cppMs_, cppImg_});
            if (!asmImg_.isNull() && asmMs_ > 0) cands.push_back({asmMs_, asmImg_});
            if (!pyImg_.isNull()  && pyMs_  > 0) cands.push_back({pyMs_,  pyImg_});

            if (cands.isEmpty()) {
                // ms가 0이거나 아직 정확히 없을 때도 "있는 이미지"라도 반환
                if (!cppImg_.isNull()) return cppImg_;
                if (!asmImg_.isNull()) return asmImg_;
                if (!pyImg_.isNull())  return pyImg_;
                return {};
            }

            std::sort(cands.begin(), cands.end(), [](const Cand& a, const Cand& b){
                return a.ms < b.ms;
            });
            return cands.first().img;
        }
        default:
            return cppImg_;
    }
}

QString MainWindow::suggestedSavePath() const {
    const QString baseDir = !originalPath_.isEmpty()
        ? QFileInfo(originalPath_).absolutePath()
        : QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    const QString baseName = !originalPath_.isEmpty()
        ? QFileInfo(originalPath_).completeBaseName()
        : QStringLiteral("noirify");

    QString tag = "cpp";
    switch (resultSource_ ? resultSource_->currentIndex() : 0) {
        case 0: tag = "cpp"; break;
        case 1: tag = "asm"; break;
        case 2: tag = "py";  break;
        case 3: tag = "fastest"; break;
    }

    return QDir(baseDir).filePath(QString("%1_noirify_%2.png").arg(baseName, tag));
}

void MainWindow::updateSaveEnabled() {
    const bool canSave = !currentResultImage().isNull();

    if (saveButton_) saveButton_->setEnabled(canSave);
    if (actSaveResult_) actSaveResult_->setEnabled(canSave);

    if (!canSave && saveButton_) {
        saveButton_->setToolTip("Run All first to generate a result image.");
    } else if (saveButton_) {
        saveButton_->setToolTip("Save the currently selected result.");
    }
}

void MainWindow::onSaveResult() {
    const QImage img = currentResultImage();
    if (img.isNull()) {
        QMessageBox::information(this, "No result", "No processed result to save.\nRun All first.");
        updateSaveEnabled();
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this,
        "Save Result",
        suggestedSavePath(),
        "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;BMP Image (*.bmp)"
    );

    if (path.isEmpty()) return;

    // 확장자 없으면 png로
    if (QFileInfo(path).suffix().isEmpty()) {
        path += ".png";
    }

    if (!img.save(path)) {
        QMessageBox::warning(this, "Save failed", "Could not save file:\n" + path);
    }
}
