#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "client_app.h"
#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>

class MainWindow : public QMainWindow, public IBLClient {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // IBLClient interface
    void onUploadComplete(const std::string& fileId) override;
    void onDownloadComplete(const std::string& fileId, const std::string& outputPath) override;
    void onDeleteComplete(const std::string& fileId) override;
    void onError(const std::string& message) override;
    void onNodesUpdated(const std::vector<std::string>& nodes) override;
    void onFilesUpdated(const nlohmann::json& files) override;
    void onUploadProgress(const std::string& fileId, uint64_t partNumber, uint64_t totalParts) override;
    void onDownloadProgress(const std::string& fileId, uint64_t partNumber, uint64_t totalParts) override;
    void onUploadFinished(const std::string& fileId, bool success, const std::string& message) override;
    void onDownloadFinished(const std::string& fileId, bool success, const std::string& message) override;
    void onDeleteFinished(const std::string& fileId, bool success, const std::string& message) override;
    void onListFilesResult(const std::vector<FileMetadata>& files) override;

signals:
    void progressUpdated(int value, int maximum);
    void statusUpdated(const QString& message);
    void fileTableUpdated(const std::vector<FileMetadata>& files);
    void nodeTableUpdated(const std::vector<std::string>& nodes);
    void fileRowRemoved(const std::string& fileId);

private slots:
    void onConnectClicked();
    void onUploadClicked();
    void onDownloadClicked();
    void onDeleteClicked();
    void onListFilesClicked();

    void updateProgressBar(int value, int maximum);
    void updateStatusLabel(const QString& message);
    void updateFileTable(const std::vector<FileMetadata>& files);
    void updateNodeTable(const std::vector<std::string>& nodes);
    void removeFileRow(const std::string& fileId);

private:
    void setupUi();

    ClientToServerAPI* api_;
    QTableWidget* fileTable_;
    QTableWidget* nodeTable_;
    QLineEdit* hostEdit_;
    QLineEdit* portEdit_;
    QPushButton* uploadButton_;
    QPushButton* downloadButton_;
    QPushButton* deleteButton_;
    QPushButton* listFilesButton_;
    QLabel* fileNameLabel_;
    QProgressBar* progressBar_;
    QLabel* statusLabel_;
};

#endif // MAINWINDOW_H