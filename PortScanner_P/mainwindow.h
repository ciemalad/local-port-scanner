#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define DATAFILE "services.csv"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void clearTable();
    void PrepareScan(); // use validation, write warnings and statistics. deconstruct ip address ant decide how to scan it
    bool ValidateIP(const QString ipAddress); // Validate that ip addresses are correct
    bool ValidatePorts(const QString bPort,const QString ePort); // Validate that ports are correct
    QString IdentifyService(const int port); // Identify what service works on particular port
    void ScanPorts(const QString addressip); // Scan secified ports of given ip address and complete table
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void RunScanHandler(); // manage events after pressing "Scan" button
    void HelpHandler(); // manage events after pressing "Help" button
    int portStatsOpen; // Statistics of opened ports
    int portStatsAll; // Statistics of opened and closed ports
};
#endif // MAINWINDOW_H
