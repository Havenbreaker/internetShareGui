#include "internetshare.h"
#include "ui_internetshare.h"
#include "QtNetwork/QNetworkInterface"
#include "QList"
#include "QProcess"
#include <QMessageBox>

QList<QNetworkInterface> interface = QNetworkInterface::allInterfaces();
QProcess process1;
QProcess process2;


QStringList getGateway(){
    /**
     * @brief Tries to get the current gateways and returns a list
     * of all possible entries
     */

    //Command to get Gateways
    QString cmd= "netstat -nr ";//| awk 'BEGIN {while ($3!=\"0.0.0.0\") getline; print $2}'";
    //Run command
    process1.start("bash", QStringList() << "-c" << cmd);
    process1.waitForFinished(3000);

   //Get output from stdout
    QString out = process1.readAll();
    QStringList out2 = out.split("\n");
    QStringList retval;
    retval.append("0.0.0.0");
    //Cycle through all lines and extract possible gateways
    for(int idx=0;idx<out2.count();idx++){
        QStringList tmp=out2.at(idx).split(QRegExp("\\s+"));
        if(tmp.at(0)=="0.0.0.0"){
            retval.append(tmp.at(1));
        }
    }
    //If gateway was found, delete first dummy entry
    if(retval.count()>1){
        retval.removeFirst();
    }
    return retval;
}


InternetShare::InternetShare(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InternetShare)
{
    ui->setupUi(this);
    // List all interfaces' names
    qDebug() << "Interfaces found:";
    foreach (QNetworkInterface iface, interface)
    {
        qDebug() << iface.name() ;
        try{
        ui->internetInterface->addItem(iface.name());
       ui->sharedInterface->addItem(iface.name());
        }
        catch(...){
            qDebug()<<"Issue";
        }
    }
    qDebug() << "**************";
    QStringList gatew=getGateway();

    //Add all gateways to drop down list
    foreach (QString g, gatew) {
        ui->autoGateways->addItem(g);
    }
    //Set initial default values
    ui->autoGateways->setCurrentIndex(ui->autoGateways->count()-1);
    //Create IP regex for text fields
    QRegExpValidator* validator = new QRegExpValidator(QRegExp("([0-1][0-9][0-9]|2[0-4][0-9]|25[0-5])\\.([0-1][0-9][0-9]|2[0-4][0-9]|25[0-5])\\.([0-1][0-9][0-9]|2[0-4][0-9]|25[0-5])\\.([0-1][0-9][0-9]|2[0-4][0-9]|25[0-5])"));
    ui->shareMask->setValidator(validator);
    ui->shareIp->setValidator(validator);
    ui->hostIp->setValidator(validator);
    ui->gateway->setValidator(validator);

    ui->shareMask->setText("255.255.255.0");
    ui->shareIp->setText("172.16.42.1");
    ui->shareNet->setText("172.16.42.0/24");
    ui->hostIp->setText("172.16.42.42");
    ui->sharedInterface->setCurrentIndex(1);
    ui->internetInterface->setCurrentIndex(interface.count()-1);


}


QString pineapplelan="eth0";
QString pineapplehostip="172.16.42.42";
QString pineapplenetmask="255.255.255.0";
QString pineapplewan="wlan0";
QString pineapplenet="172.16.42.0/24";
QString pineapplegw=getGateway().at(0);
QString pineappleip="172.16.42.1";


InternetShare::~InternetShare()
{
    //delete ui;
   //this->close();
   //QApplication::exit(0);
}

void InternetShare::on_internetInterface_currentIndexChanged(const QString &arg1)
{

    int idx=ui->internetInterface->currentIndex();
    //QString txt=interface.at(idx).addressEntries().at(0).ip().toString();
    //ui->internetIp->setText(txt);
    //ui->gateway->setText(getGateway());
    //ui->InternetShare.centralWidget
}
bool InternetShare::runSharing(){
    QStringList cmds;
    //Bring up Ethernet Interface directly connected to Pineapple
    cmds.append("ifconfig "+pineapplelan + " "+pineapplehostip+" "+"netmask "+pineapplenetmask+" up");
    // Enable IP Forwarding
    cmds.append("echo '1' > /proc/sys/net/ipv4/ip_forward");
    //clear chains
    cmds.append("iptables -X");
    //clear rules
    cmds.append("iptables -F");
    //Setup IP forwarding
    cmds.append("iptables -A FORWARD -i "+pineapplewan+" -o "+pineapplelan+" -s "+ pineapplenet+" -m state --state NEW -j ACCEPT");
    cmds.append("iptables -A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT");
    cmds.append("iptables -A POSTROUTING -t nat -j MASQUERADE");
    //remove default route
    cmds.append("route del default");
    //add default gateway
    cmds.append("route add default gw "+ pineapplegw +" "+ pineapplewan);

    foreach(QString cmd,cmds){
        qDebug()<<cmd;
        process1.start("bash", QStringList() << "-c" << cmd);
        process1.waitForFinished(3000);
        qDebug()<< "Done";
    }
    QMessageBox msgBox;
    msgBox.setText("Finished setting up internet sharing");
    msgBox.exec();
    return true;

}

void InternetShare::on_sharedInterface_currentIndexChanged(int index)
{
    //QString txt=interface.at(index).addressEntries().at(0).ip().toString();
    //ui->hostIp->setText(txt);
    //ui->gateway->setText(interface.at(index).addressEntries().at(0).ip().toString());

}
bool InternetShare::startSharing(){
    pineapplelan=ui->sharedInterface->currentText();
    pineapplehostip=ui->hostIp->text();
    pineapplenetmask=ui->shareMask->text();
    pineapplewan=ui->internetInterface->currentText();
    pineapplenet=ui->shareNet->text();
    if(ui->autoGateways->isEnabled()){
    pineapplegw=ui->autoGateways->currentText();
    }
    else{
        pineapplegw=ui->gateway->text();
    }
    pineappleip=ui->shareIp->text();
    QMessageBox msgBox;
    if(pineapplelan==pineapplewan){
        msgBox.setText("Same interfaces selected, please check your settings");
        msgBox.exec();
        return false;
    }
    else{
        QString stats;
        stats.append("Pineapple Interface: " + pineapplelan + "\n");
        stats.append("Internet  Interface: " + pineapplewan+ "\n");
        msgBox.setText("Save and start internet Sharing?");
        msgBox.setInformativeText(stats);
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        bool retval=false;
        switch (ret) {
        case QMessageBox::Save:
            // Save was clicked
            runSharing();
           // delete ui;
            break;
        case QMessageBox::Cancel:
            // Cancel was clicked
            break;
        default:
            // should never be reached
            break;
        }

        return retval;

    }
    qDebug() << pineapplelan;


}

bool InternetShare::on_pushButton_2_clicked()
{
QCoreApplication::exit();
return true;

}



void InternetShare::on_pushButton_clicked()
{
    startSharing();
}

void InternetShare::on_manualGateway_clicked()
{
    ui->autoGateways->setEnabled(false);
    ui->gateway->setEnabled(true);
    ui->autoGateway->setChecked(false);
}

void InternetShare::on_autoGateway_clicked()
{
    ui->gateway->setEnabled(false);
    ui->autoGateways->setEnabled(true);
    ui->manualGateway->setChecked(false);
}

void InternetShare::on_autoGateways_currentIndexChanged(int index)
{
    pineapplegw=ui->autoGateways->currentText();
}


