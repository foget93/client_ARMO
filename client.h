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

    QTextEdit*      ptxtInfo;

    QPushButton*    pbtnconnect;
    QPushButton*    pbtnOpenFile;
    QPushButton*    pbtnSendPicture;

    QLineEdit*      ptxtInputHost;
    QLineEdit*      ptxtInputPort;

    quint32         nextBlockSize;

    QFile*          p_localFile;
    QString         fileName;
    QString         currentImageName;

protected:
    bool isHostAddress(const QString& ip);
    bool isPort(const QString& port);

public:
    explicit Client(QWidget *parent = nullptr);
    ~Client();

private slots:

    void slotReadyRead();
    void slotError(QAbstractSocket::SocketError);
    void slotBtnConnectClicked();
    void slotOpenFile();
    void slotSendPictureToServer();
};
// =====CLIENT_H=====
