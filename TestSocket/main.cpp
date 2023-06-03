#include <QCoreApplication>
#include <QWebSocket>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include "qcompressor.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (QSslSocket::supportsSsl())
    {

        QWebSocket* socket=new QWebSocket();
        socket->open(QUrl(QStringLiteral("wss://api.huobi.pro/ws"/*"wss://127.0.0.1:9999"*/)));
        QObject::connect(socket, &QWebSocket::connected, &a, [=](){
            qDebug()<<"connected!";
            QJsonObject json;
            json["sub"] = "market.wanbtc.depth.step0"; //для пары btcusdt обновление происходит чаще(около 1 секунды)
            json["id"] = "wanbtc";                      //для пары wanbtc обновление depth происходит раз в 8-10 секунд

            QString msg=QJsonDocument(json).toJson(QJsonDocument::Compact);
            socket->sendTextMessage(msg);
        });
        QObject::connect(socket, &QWebSocket::pong, &a, [=](){
           qDebug()<<"pong!";
        });
        QObject::connect(socket, &QWebSocket::textMessageReceived, &a, [=](){
           qDebug()<<"New message";
        });
        QObject::connect(socket, &QWebSocket::binaryMessageReceived, &a, [=](const QByteArray &message){
            qDebug()<<"QWebSocket::binaryMessageReceived: ";
            QByteArray decompressed;
            QCompressor::gzipDecompress(message, decompressed);
            qDebug()<<decompressed;
            QString text=decompressed;


            QJsonObject json = QJsonDocument::fromJson(decompressed).object();
            QJsonValue value = json["ping"];
            if (!value.isNull())
            {
                json.remove("ping");
                json["pong"]=value;
                QString sendmsg=QJsonDocument(json).toJson(QJsonDocument::Compact);
                socket->sendTextMessage(sendmsg);
            }
        });
    }
    else
    {
        qDebug()<<"Ssl dont support!";
        qDebug() << "QSslSocket::sslLibraryBuildVersionString()" << QSslSocket::sslLibraryBuildVersionString();
        qDebug() << "QSslSocket::sslLibraryVersionString()" << QSslSocket::sslLibraryVersionString();
        qDebug() << "Supports SSL: " << QSslSocket::supportsSsl();
        exit(-1);
    }

    return a.exec();
}
