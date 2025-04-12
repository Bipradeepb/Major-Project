#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>
#include <QFileDialog>
/*
Copy the exe from ClientUi build to Client build folder
cd Client
Run -> ./build/ClientUI
==> Config File Path = ./config.txt
==> Client exe = ./build/cli_exe
==> File Path in Config = ./Demo/FileName
*/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , clientProcess(nullptr)
    , configFilePath("./config.txt") // Set the default config file path
    , totalFileSize(0)
{
    ui->setupUi(this);
    setWindowTitle("go-back-N Client@pb-dot"); // Set the new window title here
    setStyleSheet("QMainWindow { background-color: #f0f0f0; border: 1px solid #c0c0c0; }"); // Light gray background, thin border

    // Style for Labels - Assuming these are your label object names
    QString labelStyle = "color: black;"; // Set label text color to black
    ui->label->setStyleSheet(labelStyle);
    ui->label_1->setStyleSheet(labelStyle);
    ui->label_2->setStyleSheet(labelStyle);
    ui->label_3->setStyleSheet(labelStyle);
    ui->label_4->setStyleSheet(labelStyle);
    ui->label_5->setStyleSheet(labelStyle);
    ui->label_6->setStyleSheet(labelStyle);
    ui->label_7->setStyleSheet(labelStyle);


    // Style for LineEdits - Set text color to black
    QString textBoxStyle = "border: 1px solid #a0a0a0; padding: 5px; border-radius: 3px; background-color: white; color: black;";
    ui->sw_ip->setStyleSheet(textBoxStyle);
    ui->sw_port->setStyleSheet(textBoxStyle);
    ui->win_size->setStyleSheet(textBoxStyle);
    ui->choice->setStyleSheet(textBoxStyle);
    ui->file_size->setStyleSheet(textBoxStyle);
    ui->file_path->setStyleSheet(textBoxStyle);

    // Style for Log TextEdit
    ui->logTextEdit->setStyleSheet("border: 1px solid #808080; background-color: #e0e0e0; font-family: 'Courier New', monospace; color: black;");

    loadConfig(); // Load configuration when the UI starts
    ui->progressBar->setRange(0, 100); // Initialize progress bar range
    ui->progressBar->setValue(0);      // Explicitly set initial value to 0
    ui->logTextEdit->setReadOnly(true); // Make log area read-only
    ui->endButton->setEnabled(false); // Disable end button initially
}

MainWindow::~MainWindow()
{
    if (clientProcess && clientProcess->state() == QProcess::Running) {
        clientProcess->kill(); // Ensure client is stopped on exit
        clientProcess->waitForFinished();
    }
    delete ui;
}

void MainWindow::loadConfig() // Definition of loadConfig
{
    QFile configFile(configFilePath);
    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&configFile);
        if (!in.atEnd()) ui->sw_ip->setText(in.readLine());
        if (!in.atEnd()) ui->sw_port->setText(in.readLine());
        if (!in.atEnd()) ui->win_size->setText(in.readLine());
        if (!in.atEnd()) ui->choice->setText(in.readLine());
        if (!in.atEnd()) ui->file_path->setText(in.readLine());
        configFile.close();
    }
}

void MainWindow::on_saveButton_clicked()
{
    QFile configFile(configFilePath);
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&configFile);
        out << ui->sw_ip->text() << "\n";
        out << ui->sw_port->text() << "\n";
        out << ui->win_size->text() << "\n";
        out << ui->choice->text() << "\n";
        out << ui->file_path->text() << "\n";
        configFile.close();
        QMessageBox::information(this, "Configuration Saved", "Configuration saved successfully!");
    } else {
        QMessageBox::critical(this, "Error", "Could not open config.txt for writing.");
    }
}

void MainWindow::on_startButton_clicked()
{
    if (clientProcess && clientProcess->state() == QProcess::Running) {
        QMessageBox::warning(this, "Warning", "Client is already running.");
        return;
    }

    // Save the config before starting the client
    on_saveButton_clicked(); // Call the updated save function

    clientProcess = new QProcess(this);
    connect(clientProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::clientOutputReady);
    connect(clientProcess, &QProcess::readyReadStandardError, this, &MainWindow::clientErrorReady);
    connect(clientProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus){
                ui->logTextEdit->append(QString("Client finished with exit code: %1, status: %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? "Normal" : "Crashed"));
                ui->startButton->setEnabled(true);
                // Removed ui->endButton->setEnabled(false); from here
                clientProcess->deleteLater();
                clientProcess = nullptr;
                totalFileSize = 0;
            });

    QString clientExecutablePath = "./build/cli_exe";
    if (clientExecutablePath.isEmpty()) {
        delete clientProcess;
        clientProcess = nullptr;
        return;
    }

    QStringList arguments;
    arguments << configFilePath; // Pass the config file path as an argument to client_exe

    clientProcess->start(clientExecutablePath, arguments);
    if (!clientProcess->waitForStarted(5000)) {
        QMessageBox::critical(this, "Error", QString("Could not start client: %1").arg(clientProcess->errorString()));
        delete clientProcess;
        clientProcess = nullptr;
        return;
    }

    ui->logTextEdit->append(QString("Starting client: %1 %2").arg(clientExecutablePath).arg(arguments.join(" ")));
    ui->startButton->setEnabled(false);
    ui->endButton->setEnabled(true); // Ensure End Client button is enabled when starting

    bool ok;
    totalFileSize = ui->file_size->text().toLongLong(&ok);
    //qDebug() << "File Size Input:" << ui->file_size->text() << ", Converted Size:" << totalFileSize << ", OK:" << ok;

    if (!ok || totalFileSize <= 0) {
        QMessageBox::warning(this, "Warning", "Invalid file size provided. Progress bar might not work correctly.");
        totalFileSize = 0;
    }
}

void MainWindow::on_endButton_clicked()
{
    if (clientProcess && clientProcess->state() == QProcess::Running) {
        clientProcess->kill(); // Or clientProcess->terminate();
        ui->logTextEdit->append("Stopping client...");
        ui->startButton->setEnabled(true);
        ui->endButton->setEnabled(false);
    }
    ui->progressBar->setValue(0); // Reset progress bar
    ui->logTextEdit->clear();    // Clear logs
}

void MainWindow::clientOutputReady()
{
    QByteArray output = clientProcess->readAllStandardOutput();
    QString outputString = QString::fromLocal8Bit(output);
    ui->logTextEdit->append(outputString);
    parseClientOutput(outputString);
}

void MainWindow::clientErrorReady()
{
    QByteArray error = clientProcess->readAllStandardError();
    QString errorString = QString::fromLocal8Bit(error);
    ui->logTextEdit->append(QString("Error: %1").arg(errorString));
}

//Progress Bar Tracking
void MainWindow::parseClientOutput(const QString& output)
{
    // Track 'expect_blk_num' from regular ACK (optional newline at the end)
    QRegularExpression expectBlkRegex("Full Win Recv :- Sending ACK with blk = (\\d+)");
    QRegularExpressionMatch expectBlkMatch = expectBlkRegex.match(output);
    if (expectBlkMatch.hasMatch() && totalFileSize > 0) {
        qint64 currentBlock = expectBlkMatch.captured(1).toLongLong();
        qint64 totalBlocks = (totalFileSize + 1) / 512; // Calculate total blocks
        //qDebug() << "Regular ACK found - Block Number:" << currentBlock << "Total Blocks:" << totalBlocks;
        if (totalBlocks > 0) {
            int progress = static_cast<int>((static_cast<double>(currentBlock) / totalBlocks) * 100);
            ui->progressBar->setValue(progress);
            //qDebug() << "Progress (Regular ACK):" << progress;
        }
    }

    // Track 'ack.block_number' (optional newline at the end)
    QRegularExpression ackBlkRegex("ACK recv with blk num = (\\d+)");
    QRegularExpressionMatch ackBlkMatch = ackBlkRegex.match(output);
    if (ackBlkMatch.hasMatch() && totalFileSize > 0) {
        qint64 currentBlock = ackBlkMatch.captured(1).toLongLong();
        qint64 totalBlocks = (totalFileSize + 1) / 512; // Calculate total blocks
        //qDebug() << "ACK Received found - Block Number:" << currentBlock << "Total Blocks:" << totalBlocks;
        if (totalBlocks > 0) {
            int progress = static_cast<int>((static_cast<double>(currentBlock) / totalBlocks) * 100);
            ui->progressBar->setValue(progress);
            //qDebug() << "Progress (ACK Recv):" << progress;
        }
    }

    // Check for completion messages (optional newline at the end)
    if (output.contains("Received Full File ") || output.contains("File Transfer Complete ")) {
        ui->progressBar->setValue(100);
        //qDebug() << "File transfer complete - Progress: 100%";
    }
}
