#ifndef INTERNETSHARE_H
#define INTERNETSHARE_H

#include <QMainWindow>

namespace Ui {
class InternetShare;
}

class InternetShare : public QMainWindow
{
    Q_OBJECT

public:
    explicit InternetShare(QWidget *parent = 0);
    ~InternetShare();

private slots:
    void on_internetInterface_currentIndexChanged(const QString &arg1);

    void on_sharedInterface_currentIndexChanged(int index);

    bool on_pushButton_2_clicked();
    bool startSharing();
bool runSharing();
    void on_pushButton_clicked();

    void on_manualGateway_clicked();

    void on_autoGateway_clicked();

    void on_autoGateways_currentIndexChanged(int index);

private:
    Ui::InternetShare *ui;
};

#endif // INTERNETSHARE_H
