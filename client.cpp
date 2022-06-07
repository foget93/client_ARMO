#include "client.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTime>

#include <QFileDialog>
/*
private:
    QTcpSocket* p_tcpSocket;
    QTextEdit*  p_txtInfo;
    QLineEdit*  p_txtInput;
    quint16     nextBlockSize;

private slots:
    void slotReadyRead ();
    void slotError (QAbstractSocket::SocketError);
    void slotSendToServer();
    void slotConnected();
*/
Client::Client(QWidget *parent)
    : QWidget(parent), nextBlockSize{0}
{
    p_tcpSocket = new QTcpSocket(this);
    p_btnconnect = new QPushButton("&Connect");

    p_btnOpenFile = new QPushButton("Open File");

    connect (p_btnOpenFile, &QPushButton::clicked,
             this,          &Client::slotOpenFile);

    connect (p_btnconnect,   &QPushButton::clicked,
             this,          &Client::slotBtnConnectClicked);
    //p_tcpSocket->connectToHost(strHost, nPort);

    connect(p_tcpSocket,    &QTcpSocket::connected,
            this,           &Client::slotConnected
            );

    connect(p_tcpSocket,    &QTcpSocket::readyRead,
            this,           &Client::slotReadyRead
            );

    connect(p_tcpSocket,    &QTcpSocket::errorOccurred,
            this,           &Client::slotError
            );

    p_txtInfo = new QTextEdit;
    p_txtInput = new QLineEdit;

    p_txtInfo->setReadOnly(true);

//    QPushButton* pcmd = new QPushButton("&Send");
//    connect (pcmd, &QPushButton::clicked,
//             this, &Client::slotSendToServer
//             );

    QPushButton* pcmd = new QPushButton("&Send");
    connect (pcmd, &QPushButton::clicked,
             this, &Client::slotSendPictureToServer
             );

//    connect (p_txtInput,    &QLineEdit::returnPressed,
//             this,          &Client::slotSendToServer
//             );

    p_lbl = new QLabel("For Picture");

    //Layoutsetup
    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(p_btnconnect);
    pvbxLayout->addWidget(p_btnOpenFile);
    pvbxLayout->addWidget(new QLabel("<H1>Client</H1>"));
    pvbxLayout->addWidget(p_txtInfo);
    pvbxLayout->addWidget(p_txtInput);
    pvbxLayout->addWidget(pcmd);

    pvbxLayout->addWidget(p_lbl);

    this->setLayout(pvbxLayout);

}

void Client::slotReadyRead()
{
    QDataStream in (p_tcpSocket);
    in.setVersion(QDataStream::Qt_6_2);

    for (;;) {
        if (!nextBlockSize) {
            if (p_tcpSocket->bytesAvailable() < (int)sizeof(quint16))
                break;

            in >> nextBlockSize;
        }
        if (p_tcpSocket->bytesAvailable() < nextBlockSize)
            break;

        QTime time;
        QString str;
        in >> time >> str;

        p_txtInfo->append(time.toString() + " " + str);
        nextBlockSize = 0;
    }
}

void Client::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
            "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                             "The host was not found." :
                             err == QAbstractSocket::RemoteHostClosedError ?
                                 "The remote host is closed." :
                                 err == QAbstractSocket::ConnectionRefusedError ?
                                     "The connection was refused." :
                                     QString(p_tcpSocket->errorString())
                                     );
    p_txtInfo->append(strError);
}

void Client::slotSendToServer()
{
    QByteArray arrBlock;
    QDataStream out (&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);

    out << quint16(0) << QTime::currentTime() << p_txtInput->text();

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    p_tcpSocket->write(arrBlock);
    p_txtInput->clear();
}

void Client::slotConnected()
{
    p_txtInfo->append("Received the connected() signal");
}

void Client::slotBtnConnectClicked()
{
    p_tcpSocket->connectToHost("localhost", 2323);
}
//==================================================
void Client::slotOpenFile()
{
    p_lbl->clear();
    this->fileName = QFileDialog::getOpenFileName(this);

    if (!fileName.isEmpty())
        this->currentImageName = fileName.right(fileName.size()
                                                - fileName.lastIndexOf('\\') - 1);

    QImage image(fileName);
    p_lbl->setPixmap(QPixmap::fromImage(image));
}
//============================================================
void Client::slotSendPictureToServer()
{
    QByteArray arrBlock;
    QDataStream out (&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);

    out << quint32(0) << QImage(fileName);

    out.device()->seek(0);
    out << quint32(arrBlock.size() - sizeof(quint32));

    p_tcpSocket->write(arrBlock);

    if(out.status() != QDataStream::Ok)
        p_txtInfo->setText("Error QDataStream");
    //p_txtInput->clear();

}
//============================================================




