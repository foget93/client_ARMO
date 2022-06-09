#include "client.h"

#include <QPushButton>
#include <QLabel>
#include <QTime>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QMessageBox>

bool Client::isHostAddress(const QString &ip)
{
    QRegularExpression regForIp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$");
    QRegularExpressionMatch matchIp = regForIp.match(ip); // проверка формата строки ip

    if (matchIp.hasMatch()) { //проверка диапазона
        QStringList list = ip.split(".");
        for (auto& str : list) {
            if (str.toInt() > 255 || str.toInt() < 0 )
                return false;
        }
        return true;
    }
    else
        return false;
}

//============================================================

bool Client::isPort(const QString& port)
{
    //протокол TCP под port 16 bit => [1..65535] включительно, [0] - отсутствие порта
    bool convertingOk;
    int portDigit = port.toInt(&convertingOk);

    if  (!convertingOk || portDigit > 65535 || portDigit < 1)
        return false;
    return true;
}

//============================================================

Client::Client(QWidget *parent)
    : QWidget(parent), nextBlockSize{0}
{
    //===========настройка gui===========
    this->setWindowTitle("Client");
    this->resize(400,600);

    pbtnconnect = new QPushButton("&Connect");
    pbtnOpenFile = new QPushButton("Open File");
    pbtnSendPicture = new QPushButton("&Send");

    ptxtInfo = new QTextEdit;
    ptxtInfo->setReadOnly(true);

    QLabel* lblHost = new QLabel("ip");
    QLabel* lblPort = new QLabel("port");

    ptxtInputHost   = new QLineEdit("127.0.0.1");
    ptxtInputPort   = new QLineEdit("2323");

    //Layoutsetup
    QHBoxLayout* hostLayout = new QHBoxLayout;
    QHBoxLayout* portLayout = new QHBoxLayout;
    hostLayout->addWidget(lblHost);
    hostLayout->addStretch(3);
    hostLayout->addWidget(ptxtInputHost);


    portLayout->addWidget(lblPort);
    portLayout->addStretch(3);
    portLayout->addWidget(ptxtInputPort); //lbl host&&port + line edit host&&port


    QVBoxLayout* pleftvLayout = new QVBoxLayout;
    pleftvLayout->addLayout(hostLayout);
    pleftvLayout->addLayout(portLayout);
    pleftvLayout->addWidget(ptxtInfo); // layouts + txtInfo

    QVBoxLayout* prightLayout = new QVBoxLayout;
    prightLayout->addWidget(pbtnconnect,3);
    prightLayout->addWidget(pbtnOpenFile,3);
    prightLayout->addWidget(pbtnSendPicture,3);
    prightLayout->addStretch(1);//buttons right

    QHBoxLayout* pbossLayout = new QHBoxLayout;
    pbossLayout->addLayout(pleftvLayout);
    pbossLayout->addLayout(prightLayout);
    //pbossLayout->addStretch(3);

    this->setLayout(pbossLayout);

//============================================================

    p_tcpSocket = new QTcpSocket(this);

    connect (pbtnOpenFile,  &QPushButton::clicked,
             this,          &Client::slotOpenFile
            );// кликед кнопки "открыть файл" ->слот

    connect (pbtnconnect,   &QPushButton::clicked,
             this,          &Client::slotBtnConnectClicked
            );//кнопка конекта

    connect(p_tcpSocket,    &QTcpSocket::readyRead,
            this,           &Client::slotReadyRead
            );// ответ от сервера

    connect(p_tcpSocket,    &QTcpSocket::errorOccurred,
            this,           &Client::slotError
            );//обработка ошибок

    connect (pbtnSendPicture,   &QPushButton::clicked,
             this,              &Client::slotSendPictureToServer
             );//отправка картинки по кнопке
}

//============================================================

Client::~Client()
{
    if (this->p_tcpSocket!=nullptr) {
            this->p_tcpSocket->disconnectFromHost();
            if(this->p_tcpSocket->state() != QAbstractSocket::UnconnectedState)
                this->p_tcpSocket->waitForDisconnected(3000);
            delete this->p_tcpSocket;
    }//не обязательно
}

//============================================================

void Client::slotReadyRead()
{
/*
 * Цикл for нужен потому, что не все высланные сервером данные могут прийти одновременно.
 * Клиент должен "уметь" получать как весь блок целиком, так и часть блока, а так же все блоки сразу.
 * Каждый переданный блок начинается с nextBlockSize.
 * Размер блока (nextBlockSize) считывается при условии p_tcpSocket->bytesAvailable() не меньше 4 байт
 * и nextBlockSize == 0. Далее проверяем этот размер с p_tcpSocket->bytesAvailable() и считываем+выводим сообщение
*/
    QDataStream in (p_tcpSocket);
    in.setVersion(QDataStream::Qt_6_2);

    for (;;) {
        if (!nextBlockSize) {
            if (p_tcpSocket->bytesAvailable() < static_cast<qint64>(sizeof(quint32)) )
                break;

            in >> nextBlockSize;
        }

        if (p_tcpSocket->bytesAvailable() < nextBlockSize)
            break;

        QString str;
        in >>  str;

        ptxtInfo->append(str);
        nextBlockSize = 0; // анализ блока завершается тут
    }
}

//============================================================

void Client::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
            QTime::currentTime().toString() +
            " Ошибка: " + (err == QAbstractSocket::HostNotFoundError ?
                             "Хост не найден." :
                             err == QAbstractSocket::RemoteHostClosedError ?
                                 "Хост закрыт." :
                                 err == QAbstractSocket::ConnectionRefusedError ?
                                     "В соединении отказано." :
                                     QString(p_tcpSocket->errorString())
                                     );
    ptxtInfo->append(strError);
    ptxtInputHost->setReadOnly(false);
    ptxtInputPort->setReadOnly(false);
}

//============================================================

void Client::slotBtnConnectClicked()
{
    QString ip = ptxtInputHost->text();
    QString port = ptxtInputPort->text();

    if (!isHostAddress(ip)) {
        QMessageBox::critical(0,"Ошибка","Введён некорректный ip адрес.");
        return;
    }
    else if (!isPort(port)) {
        QMessageBox::critical(0,"Ошибка","Введён некорректный номер порта.");
        return;
    }
    else {
        p_tcpSocket->connectToHost(ip, port.toInt()); // <----соединение тут

        ptxtInputHost->setReadOnly(true);
        ptxtInputPort->setReadOnly(true);
    }

}

//============================================================

void Client::slotOpenFile()
{
    /*Читаем путь к выбранному файлу(fileName) + отрезаем имя файла (currentImageName) */
    currentImageName = "";
    this->fileName = QFileDialog::getOpenFileName(this);

    if (!fileName.isEmpty())
        this->currentImageName = fileName.right(fileName.size()
                                                - fileName.lastIndexOf('/') - 1);
    if(currentImageName != "")
        ptxtInfo->append(QString("%1 Файл %2 загружен.")
                         .arg(QTime::currentTime().toString())
                         .arg(currentImageName)
                        );
}

//============================================================

void Client::slotSendPictureToServer()
{
/*
 * По аналогии с void Client::slotReadyRead().
 * Нюанс: вместо реального размера кидаем 0, потом двигаем каретку и записываем реальный размер.
 * Далее запись в сокет.
*/
    if (p_tcpSocket->socketDescriptor() > 0) { //Returns the native socket descriptor of the QAbstractSocket object if this is available; otherwise returns -1.
        QByteArray arrBlock;
        QDataStream out (&arrBlock, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_2);

        QTime time = QTime::currentTime();
        out << quint32(0) << currentImageName << QImage(fileName) << time ;

        out.device()->seek(0);
        out << quint32(arrBlock.size() - sizeof(quint32));

        p_tcpSocket->write(arrBlock);

        if (out.status() != QDataStream::Ok)
            ptxtInfo->append("Error QDataStream");

        if (currentImageName != "")
            ptxtInfo->append(time.toString()
                              + " " + currentImageName
                              +" отправлен.");
        fileName.clear();
        currentImageName.clear();
    }
    else
        ptxtInfo->append("Пожалуйста, сначала подключитесь к серверу.");
}

//============================================================




