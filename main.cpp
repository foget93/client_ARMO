#include "client.h"

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);



    Client client;

    client.show();

    return a.exec();
}
