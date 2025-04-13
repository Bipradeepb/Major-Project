#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QProcess *clientProcess;
    QString configFilePath;
    qint64 totalFileSize;
    QRegularExpression *expectBlkRegex;
    QRegularExpression *ackBlkRegex;

private slots:
    void on_saveButton_clicked();
    void on_startButton_clicked();
    void on_endButton_clicked();
    void clientOutputReady();
    void clientErrorReady();
    void parseClientOutput(const QString& output);
    void loadConfig(); // Declared loadConfig here

};
#endif // MAINWINDOW_H
