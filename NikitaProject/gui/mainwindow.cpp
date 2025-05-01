#include "mainwindow.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      api_(nullptr),
      fileTable_(nullptr),
      nodeTable_(nullptr),
      hostEdit_(nullptr),
      portEdit_(nullptr),
      uploadButton_(nullptr),
      downloadButton_(nullptr),
      deleteButton_(nullptr),
      listFilesButton_(nullptr),
      fileNameLabel_(nullptr),
      progressBar_(nullptr),
      statusLabel_(nullptr) {
    try {
        api_ = new ClientToServerAPI(this);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать API: " + QString::fromStdString(e.what()));
        throw;
    }

    setupUi();

    connect(this, &MainWindow::progressUpdated, this, &MainWindow::updateProgressBar);
    connect(this, &MainWindow::statusUpdated, this, &MainWindow::updateStatusLabel);
    connect(this, &MainWindow::fileTableUpdated, this, &MainWindow::updateFileTable);
    connect(this, &MainWindow::nodeTableUpdated, this, &MainWindow::updateNodeTable);
    connect(this, &MainWindow::fileRowRemoved, this, &MainWindow::removeFileRow);
}

MainWindow::~MainWindow() {
    delete api_;
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout* connectLayout = new QHBoxLayout();
    hostEdit_ = new QLineEdit("127.0.0.1", this);
    portEdit_ = new QLineEdit("8080", this);
    QPushButton* connectButton = new QPushButton("Подключиться", this);
    connectLayout->addWidget(new QLabel("IP узла:", this));
    connectLayout->addWidget(hostEdit_);
    connectLayout->addWidget(new QLabel("Порт:", this));
    connectLayout->addWidget(portEdit_);
    connectLayout->addWidget(connectButton);
    mainLayout->addLayout(connectLayout);

    fileTable_ = new QTableWidget(this);
    fileTable_->setColumnCount(2);
    fileTable_->setHorizontalHeaderLabels({"ID файла", "Размер"});
    fileTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(fileTable_);

    nodeTable_ = new QTableWidget(this);
    nodeTable_->setColumnCount(2);
    nodeTable_->setHorizontalHeaderLabels({"IP узла", "Состояние"});
    mainLayout->addWidget(new QLabel("Список узлов:", this));
    mainLayout->addWidget(nodeTable_);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    uploadButton_ = new QPushButton("Загрузить", this);
    downloadButton_ = new QPushButton("Скачать", this);
    deleteButton_ = new QPushButton("Удалить", this);
    listFilesButton_ = new QPushButton("Обновить список", this);
    buttonLayout->addWidget(uploadButton_);
    buttonLayout->addWidget(downloadButton_);
    buttonLayout->addWidget(deleteButton_);
    buttonLayout->addWidget(listFilesButton_);
    mainLayout->addLayout(buttonLayout);

    fileNameLabel_ = new QLabel("Файл: не выбран", this);
    mainLayout->addWidget(fileNameLabel_);

    progressBar_ = new QProgressBar(this);
    progressBar_->setMinimum(0);
    progressBar_->setValue(0);
    mainLayout->addWidget(progressBar_);

    statusLabel_ = new QLabel("Готово", this);
    mainLayout->addWidget(statusLabel_);

    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(uploadButton_, &QPushButton::clicked, this, &MainWindow::onUploadClicked);
    connect(downloadButton_, &QPushButton::clicked, this, &MainWindow::onDownloadClicked);
    connect(deleteButton_, &QPushButton::clicked, this, &MainWindow::onDeleteClicked);
    connect(listFilesButton_, &QPushButton::clicked, this, &MainWindow::onListFilesClicked);
}

void MainWindow::onConnectClicked() {
    QString host = hostEdit_->text();
    bool ok;
    quint16 port = portEdit_->text().toUShort(&ok);
    if (!ok) {
        emit statusUpdated("Неверный порт");
        return;
    }
    std::string nodeIpPort = host.toStdString() + ":" + std::to_string(port);
    api_->connect(nodeIpPort);
    emit statusUpdated("Подключено к " + host + ":" + QString::number(port));
}

void MainWindow::onUploadClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите файл");
    if (filePath.isEmpty()) {
        emit statusUpdated("Выбор файла отменён");
        return;
    }
    fileNameLabel_->setText("Файл: " + filePath);
    emit statusUpdated("Начало загрузки...");
    api_->uploadFile(filePath.toStdString());
}

void MainWindow::onDownloadClicked() {
    if (fileTable_->currentRow() < 0) {
        emit statusUpdated("Выберите файл для скачивания");
        return;
    }
    QString fileId = fileTable_->item(fileTable_->currentRow(), 0)->text();
    QString savePath = QFileDialog::getSaveFileName(this, "Сохранить файл", fileId);
    if (savePath.isEmpty()) {
        emit statusUpdated("Сохранение отменено");
        return;
    }
    fileNameLabel_->setText("Файл: " + savePath);
    emit statusUpdated("Начало скачивания...");
    api_->downloadFile(fileId.toStdString(), savePath.toStdString());
}

void MainWindow::onDeleteClicked() {
    if (fileTable_->currentRow() < 0) {
        emit statusUpdated("Выберите файл для удаления");
        return;
    }
    QString fileId = fileTable_->item(fileTable_->currentRow(), 0)->text();
    int reply = QMessageBox::question(this, "Подтверждение удаления",
                                      "Удалить файл " + fileId + "?",
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        emit statusUpdated("Удаление...");
        api_->deleteFile(fileId.toStdString());
    }
}

void MainWindow::onListFilesClicked() {
    emit statusUpdated("Обновление списка...");
    api_->listFiles();
}

void MainWindow::onUploadProgress(const std::string& fileId, uint64_t partNumber, uint64_t totalParts) {
    if (totalParts > 0) {
        emit progressUpdated(static_cast<int>(partNumber), static_cast<int>(totalParts));
        emit statusUpdated("Загрузка " + QString::fromStdString(fileId) + ": " + QString::number(partNumber) + "/" + QString::number(totalParts));
    }
}

void MainWindow::onDownloadProgress(const std::string& fileId, uint64_t partNumber, uint64_t totalParts) {
    if (totalParts > 0) {
        emit progressUpdated(static_cast<int>(partNumber), static_cast<int>(totalParts));
        emit statusUpdated("Скачивание " + QString::fromStdString(fileId) + ": " + QString::number(partNumber) + "/" + QString::number(totalParts));
    }
}

void MainWindow::onUploadFinished(const std::string& fileId, bool success, const std::string& message) {
    emit statusUpdated(QString::fromStdString(message));
    if (success) {
        emit statusUpdated("Загрузка " + QString::fromStdString(fileId) + " завершена");
        api_->listFiles();
    }
}

void MainWindow::onDownloadFinished(const std::string& fileId, bool success, const std::string& message) {
    emit statusUpdated(QString::fromStdString(message));
    if (success) {
        emit statusUpdated("Скачивание " + QString::fromStdString(fileId) + " завершено");
    }
}

void MainWindow::onDeleteFinished(const std::string& fileId, bool success, const std::string& message) {
    emit statusUpdated(QString::fromStdString(message));
    if (success) {
        emit fileRowRemoved(fileId);
    }
}

void MainWindow::onListFilesResult(const std::vector<FileMetadata>& files) {
    emit fileTableUpdated(files);
    emit statusUpdated("Список файлов обновлён");
}

void MainWindow::onUploadComplete(const std::string& fileId) {
    emit statusUpdated("Загрузка завершена: " + QString::fromStdString(fileId));
}

void MainWindow::onDownloadComplete(const std::string& fileId, const std::string& outputPath) {
    emit statusUpdated("Скачивание завершено: " + QString::fromStdString(fileId) + " в " + QString::fromStdString(outputPath));
}

void MainWindow::onDeleteComplete(const std::string& fileId) {
    emit statusUpdated("Файл удалён: " + QString::fromStdString(fileId));
    emit fileRowRemoved(fileId);
}

void MainWindow::onError(const std::string& message) {
    emit statusUpdated("Ошибка: " + QString::fromStdString(message));
    QMessageBox::critical(this, "Ошибка", QString::fromStdString(message));
}

void MainWindow::onNodesUpdated(const std::vector<std::string>& nodes) {
    emit nodeTableUpdated(nodes);
}

void MainWindow::onFilesUpdated([[maybe_unused]] const nlohmann::json& files) {
    // Этот метод уже вызывает onListFilesResult
}

void MainWindow::updateProgressBar(int value, int maximum) {
    if (maximum > 0) {
        progressBar_->setMaximum(maximum);
        progressBar_->setValue(value);
    } else {
        progressBar_->setMaximum(1);
        progressBar_->setValue(0);
    }
}

void MainWindow::updateStatusLabel(const QString& message) {
    statusLabel_->setText(message);
}

void MainWindow::updateFileTable(const std::vector<FileMetadata>& files) {
    fileTable_->setRowCount(0);
    for (const auto& meta : files) {
        int row = fileTable_->rowCount();
        fileTable_->insertRow(row);
        fileTable_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(meta.id)));
        fileTable_->setItem(row, 1, new QTableWidgetItem(QString::number(meta.size)));
    }
}

void MainWindow::updateNodeTable(const std::vector<std::string>& nodes) {
    nodeTable_->setRowCount(0);
    for (const auto& node : nodes) {
        int row = nodeTable_->rowCount();
        nodeTable_->insertRow(row);
        nodeTable_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(node)));
        nodeTable_->setItem(row, 1, new QTableWidgetItem("Активен"));
    }
}

void MainWindow::removeFileRow(const std::string& fileId) {
    for (int r = 0; r < fileTable_->rowCount(); ++r) {
        if (fileTable_->item(r, 0)->text().toStdString() == fileId) {
            fileTable_->removeRow(r);
            break;
        }
    }
}