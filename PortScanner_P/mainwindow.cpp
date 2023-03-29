#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <SFML/Network.hpp>
#include <QTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <helpdialog.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->runScanButton,&QPushButton::released,this,&MainWindow::RunScanHandler);
    connect(ui->helpButton,&QPushButton::released,this,&MainWindow::HelpHandler);

}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearTable()
{
    ui->screenTableWidget->clear();
    ui->screenTableWidget->setColumnCount(3);
    ui->screenTableWidget->setRowCount(0);
    ui->screenTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->screenTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Port"));
    ui->screenTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("State"));
    ui->screenTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("Service"));
    ui->screenTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
bool IsPortOpen(const std::string& address, int port)
{
    sf::TcpSocket socket;
    sf::Time t1 = sf::milliseconds(300);//timeout
    bool state = (socket.connect(sf::IpAddress(address), port,t1) == sf::Socket::Done);
    socket.disconnect();
    return state;
}
void MainWindow::RunScanHandler()
{
    portStatsAll=0;
    portStatsOpen=0;

    clearTable();

    ui->onlyOpenCheckBox->setDisabled(true);

    ui->warningTextBrowser->setText("");
    ui->warningTextBrowser->setAlignment(Qt::AlignHCenter);

    PrepareScan();

}
bool MainWindow::ValidateIP(const QString ipAddress)
{
    //if there are 4 parts separated by dots
    QStringList ipPart=ipAddress.split('.');
    if(ipPart.count()!=4)
    {
        return false;
    }
    char cat='x';

    //check address category
    if(ipPart[0].toInt()==10&&ipPart[1].toInt()<=255&&ipPart[2].toInt()<=255&&ipPart[3].toInt()<=255)
    {
        cat='a';
    }
    if(ipPart[0].toInt()==172&&ipPart[1].toInt()>=16&&ipPart[1].toInt()<=31&&ipPart[2].toInt()<=255&&ipPart[3].toInt()<=255)
    {
        cat='b';
    }
    if(ipPart[0].toInt()==192&&ipPart[1].toInt()==168)
    {
        cat='c';    }
    if(ipAddress=="127.0.0.1")
    {
        return true;
    }
    qDebug() << cat;
    if(cat=='x')
    {
        return false;
    }
    //check if there are numbers form 0 to 255 or '-' or '*'
        bool isInterval=false;
        for(int i=0;i<ipPart.count();i++)
        {
            bool ok;
            int dec = ipPart.at(i).toInt(&ok, 10);
            if(!ok||dec>255||dec<0)
            {
                if(ipPart.at(i).contains('-')&&isInterval==false)
                {
                    isInterval=true;
                }
                if(!(ipPart.at(i).contains('-')||ipPart.at(i).contains('*')))
                {
                    return false;
                }
            }
         }
    return true;
}
bool MainWindow::ValidatePorts(const QString bPort,const QString ePort)
{
    bool begPort,endPort;
    int begPortInt=bPort.toInt(&begPort,10);
    int endPortInt=ePort.toInt(&endPort,10);
    if(!begPort||!endPort||(endPortInt<begPortInt)||begPortInt<1||(endPortInt>65535))
    {
        return false;
    }
    return true;
}
QString MainWindow::IdentifyService(const int port)
{
    //check what service runs at port
    //original file with services comes from C:\Windows\System32\drivers\etc\services
    //but program uses modified version
    QString service="Unknown";
    QFile file(DATAFILE);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << file.errorString();
    }

    while (!file.atEnd())
    {
        QString line = file.readLine();
        QStringList fields = line.split(";");
        if(fields[0].toInt()==port)
        {
           service=fields[1];
        }
    }
    file.close();

    return service;
}
void MainWindow::ScanPorts(const QString addressip)
{
    int eport=ui->endPortLineEdit->text().toInt();
    int bport=ui->beginnigPortLineEdit->text().toInt();

    QFont  font;
    font.setWeight(QFont::Bold);

    bool portClosed;

    QTableWidgetItem * item1;
    QTableWidgetItem * item2;
    QTableWidgetItem * item3;

    ui->screenTableWidget->insertRow(ui->screenTableWidget->rowCount());
    ui->screenTableWidget->setSpan(ui->screenTableWidget->rowCount()-1,0,1,3);
    item1=new QTableWidgetItem("IP Address: "+addressip);
    ui->screenTableWidget->setItem(ui->screenTableWidget->rowCount()-1,0,item1);
    item1->setTextAlignment(Qt::AlignCenter);
    ui->screenTableWidget->item(ui->screenTableWidget->rowCount()-1,0)->setFont(font);

    for(int i=bport;i<=eport;i++)
    {
        ui->screenTableWidget->insertRow(ui->screenTableWidget->rowCount());
        item1=new QTableWidgetItem(QString::number(i));
        ui->screenTableWidget->setItem(ui->screenTableWidget->rowCount()-1,0,item1);
        item1->setTextAlignment(Qt::AlignCenter);
        portClosed=false;

        if(IsPortOpen(addressip.toStdString(), i))
        {
            portStatsOpen++;
            item2=new QTableWidgetItem("OPEN");
            ui->screenTableWidget->setItem(ui->screenTableWidget->rowCount()-1,1,item2);
            item2->setForeground(QBrush(QColor(133,191,70)));

            item3=new QTableWidgetItem(IdentifyService(i));
            ui->screenTableWidget->setItem(ui->screenTableWidget->rowCount()-1,2,item3);

        }
        else
        {
            item2=new QTableWidgetItem("CLOSED");
            ui->screenTableWidget->setItem(ui->screenTableWidget->rowCount()-1,1,item2);
            item2->setForeground(QBrush(QColor(193,56,68)));
            item3=new QTableWidgetItem(IdentifyService(i));
            ui->screenTableWidget->setItem(ui->screenTableWidget->rowCount()-1,2,item3);

            if(ui->onlyOpenCheckBox->isChecked())
            {
                portClosed=true;
                ui->screenTableWidget->removeRow(ui->screenTableWidget->rowCount()-1);
            }

        }
        if(!portClosed)
        {
            ui->screenTableWidget->item(ui->screenTableWidget->rowCount()-1,1)->setFont(font);
            item2->setTextAlignment(Qt::AlignCenter);
            item3->setTextAlignment(Qt::AlignCenter);
        }
        portStatsAll++;
    }
}
void MainWindow::HelpHandler()
{
    helpDialog helpD;
    helpD.setModal(true);
    helpD.exec();
}
void MainWindow::PrepareScan()
{
    if(ValidateIP(ui->beginningIpLineEdit->text()))
    {
        if(ValidatePorts(ui->beginnigPortLineEdit->text(),ui->endPortLineEdit->text()))
        {
            clearTable();
            QElapsedTimer timer;
            timer.start();

            QString ipAddress1=ui->beginningIpLineEdit->text();
            QStringList ipPart=ipAddress1.split('.');

            QString iptogive;
            int minus=4;
            bool isMinus=false;
            int star=4;
            for(int q=0;q<ipPart.count();q++)
            {
                if(ipPart.at(q).contains('-'))
                {
                    minus=q;
                    isMinus=true;
                }
            }
            if(isMinus)
            {
                if(minus==4)
                {
                    ScanPorts(ipAddress1);
                }
                else
                {
                    for(int i=0;i<minus;i++)
                    {
                        iptogive+=ipPart.at(i)+'.';
                    }
                    QStringList ipPart2;
                    QString iptogive2;
                    switch(minus)
                    {
                    case 1:
                        ipPart2=ipPart.at(minus).split('-');
                        for(int i=ipPart2.at(0).toInt();i<=ipPart2.at(1).toInt();i++)
                        {
                            for(int y=0;y<=255;y++)
                            {
                                for(int q=0;q<=255;q++)
                                {
                                    iptogive2=iptogive+(QString::number(i)+'.'+QString::number(y)+'.'+QString::number(q));
                                    ScanPorts(iptogive2);
                                }
                            }
                        }
                    break;
                    case 2:
                        ipPart2=ipPart.at(minus).split('-');
                        for(int i=ipPart2.at(0).toInt();i<=ipPart2.at(1).toInt();i++)
                        {
                            for(int y=0;y<=255;y++)
                            {
                                iptogive2=iptogive+(QString::number(i)+'.'+QString::number(y));
                                ScanPorts(iptogive2);
                            }
                        }
                    break;
                    case 3:
                        ipPart2=ipPart.at(minus).split('-');
                        for(int i=ipPart2.at(0).toInt();i<=ipPart2.at(1).toInt();i++)
                        {
                            iptogive2=iptogive+QString::number(i);
                            ScanPorts(iptogive2);
                        }
                    break;
                    }
                }
            }
            else
            {
                for(int q=0;q<ipPart.count();q++)
                {
                    if(ipPart.at(q).contains('*'))
                    {
                        star=q;
                    }
                }
                if(star==4)
                {
                    ScanPorts(ipAddress1);
                }
                else
                {
                    for(int i=0;i<star;i++)
                    {
                        iptogive+=ipPart.at(i)+'.';
                    }
                    QString iptogive2;
                    switch(star)
                    {
                    case 1:
                        for(int i=16;i<=255;i++)
                        {
                            for(int y=0;y<=255;y++)
                            {
                                for(int q=0;q<=255;q++)
                                {
                                    iptogive2=iptogive+(QString::number(i)+'.'+QString::number(y)+'.'+QString::number(q));
                                    ScanPorts(iptogive2);
                                }
                            }
                        }
                    break;
                    case 2:
                        for(int i=0;i<=255;i++)
                        {
                            for(int y=0;y<=255;y++)
                            {
                                iptogive2=iptogive+(QString::number(i)+'.'+QString::number(y));
                                ScanPorts(iptogive2);
                            }
                        }
                    break;
                    case 3:
                        for(int i=0;i<=255;i++)
                        {
                            iptogive2=iptogive+QString::number(i);
                            ScanPorts(iptogive2);
                        }
                    break;
                    }
                }
            }
            ui->onlyOpenCheckBox->setDisabled(false);
            QTableWidgetItem *stats;
            QFont font;
            font.setWeight(QFont::Bold);
            QString timeStats=QTime::fromMSecsSinceStartOfDay(timer.restart()).toString();

            ui->screenTableWidget->insertRow(ui->screenTableWidget->rowCount());
            ui->screenTableWidget->setSpan(ui->screenTableWidget->rowCount()-1,0,1,3);
            stats=new QTableWidgetItem("Scan complete! It took: "+timeStats+"\n Scanned "+QString::number(portStatsAll)+" ports. "+QString::number(portStatsOpen)+" ports were open.");
            ui->screenTableWidget->setItem(ui->screenTableWidget->rowCount()-1,0,stats);
            stats->setTextAlignment(Qt::AlignHCenter);
            ui->screenTableWidget->item(ui->screenTableWidget->rowCount()-1,0)->setFont(font);
        }
        else
        {
            ui->warningTextBrowser->setTextColor(QColor(QColor(193,56,68)));
            ui->warningTextBrowser->append("Incorrect ports! Try again.");
        }
    }
    else
    {
        ui->warningTextBrowser->setTextColor(QColor(QColor(193,56,68)));
        ui->warningTextBrowser->append("Incorrect IP address! Try again.");
        ui->onlyOpenCheckBox->setDisabled(false);
    }

}
