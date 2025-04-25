#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QFile>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onConnectClicked();
    void onUploadClicked();
    void onDownloadClicked();
    void onDeleteClicked();
    void onListFilesClicked();

private:
    void setupUi();

    QLineEdit* hostEdit_;
    QLineEdit* portEdit_;
    QTableWidget* fileTable_;
    QTableWidget* nodeTable_;
    QPushButton* uploadButton_;
    QPushButton* downloadButton_;
    QPushButton* deleteButton_;
    QPushButton* listFilesButton_;
    QLabel* fileNameLabel_;
    QProgressBar* progressBar_;
    QLabel* statusLabel_;
    QFile* current_file_;
    QString file_id_;
    uint64_t total_parts_;
    uint64_t current_part_;
};

#endif