#pragma once
// =====CLIENT_H=====

#include <QWidget>
#include <QTcpSocket>

#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>


#include <QFile>
#include <QLabel>
//=======================================================================

class Client : public QWidget
{
    Q_OBJECT

private:
    QTcpSocket*     p_tcpSocket;
    QTextEdit*      p_txtInfo;
    QLineEdit*      p_txtInput;
    QPushButton*    p_btnconnect;
    QPushButton*    p_btnOpenFile;


    quint32         nextBlockSize;
    //quint16         nextBlockSize;
///////////////////////////////////////
    QFile*          p_localFile;
    QString         fileName;

    QImage image;
    QString currentImageName;

    QLabel* p_lbl;

public:
    explicit Client(QWidget *parent = nullptr);

private slots:

    void slotReadyRead ();
    void slotError (QAbstractSocket::SocketError);
    void slotSendToServer();
    void slotConnected();
    void slotBtnConnectClicked();

    void slotOpenFile();
    void slotSendPictureToServer();
};
// =====CLIENT_H=====
