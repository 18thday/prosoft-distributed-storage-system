#include "mainwindow.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QProgressBar>
#include <QDateTime>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), current_file_(nullptr) {
    setupUi();
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
    fileTable_->setHorizontalHeaderLabels({"Имя файла", "Размер"});
    fileTable_->setSelectionMode(QAbstractItemView::SingleSelection);
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
        statusLabel_->setText("Неверный порт");
        return;
    }
    statusLabel_->setText("Подключено к " + host + ":" + QString::number(port));

    // Имитация добавления узла в таблицу
    int row = nodeTable_->rowCount();
    nodeTable_->insertRow(row);
    nodeTable_->setItem(row, 0, new QTableWidgetItem(host));
    nodeTable_->setItem(row, 1, new QTableWidgetItem("ok"));
}

void MainWindow::onUploadClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите файл");
    if (filePath.isEmpty()) {
        statusLabel_->setText("Выбор файла отменен");
        return;
    }

    current_file_ = new QFile(filePath);
    if (!current_file_->open(QIODevice::ReadOnly)) {
        statusLabel_->setText("Не удалось открыть файл");
        delete current_file_;
        current_file_ = nullptr;
        return;
    }

    file_id_ = "file_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    total_parts_ = (current_file_->size() + 1024 * 1024 - 1) / (1024 * 1024); // MIN_CHUNK_SIZE = 1MB
    current_part_ = 0;
    progressBar_->setMaximum(total_parts_);
    progressBar_->setValue(0);
    fileNameLabel_->setText("Файл: " + filePath);
    statusLabel_->setText("Файл выбран, размер: " + QString::number(current_file_->size()) + " байт");

    current_file_->close();
    delete current_file_;
    current_file_ = nullptr;
}

void MainWindow::onDownloadClicked() {
    if (fileTable_->currentRow() < 0) {
        statusLabel_->setText("Выберите файл для скачивания");
        return;
    }
    QString fileId = fileTable_->item(fileTable_->currentRow(), 0)->text();
    QString savePath = QFileDialog::getSaveFileName(this, "Сохранить файл", fileId);
    if (savePath.isEmpty()) {
        statusLabel_->setText("Сохранение отменено");
        return;
    }
    fileNameLabel_->setText("Файл: " + savePath);
    statusLabel_->setText("Скачивание имитировано для " + fileId);
}

void MainWindow::onDeleteClicked() {
    if (fileTable_->currentRow() < 0) {
        statusLabel_->setText("Выберите файл для удаления");
        return;
    }
    QString fileId = fileTable_->item(fileTable_->currentRow(), 0)->text();
    int reply = QMessageBox::question(this, "Подтверждение удаления",
                                      "Вы уверены, что хотите удалить файл " + fileId + "?",
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        fileTable_->removeRow(fileTable_->currentRow());
        statusLabel_->setText("Файл " + fileId + " удален");
    }
}

void MainWindow::onListFilesClicked() {
    // Имитация обновления списка файлов
    fileTable_->setRowCount(0);
    int row = fileTable_->rowCount();
    fileTable_->insertRow(row);
    fileTable_->setItem(row, 0, new QTableWidgetItem("file_test"));
    fileTable_->setItem(row, 1, new QTableWidgetItem("1024"));
    statusLabel_->setText("Список файлов обновлен");
}