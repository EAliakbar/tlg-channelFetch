#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QTime>
#include <QDateTime>
#include <QFile>

#include <QtTelegram/telegram.h>

#include <iostream>

int main(int argc, char *argv[])
{

    QCoreApplication a(argc, argv);

//    if(a.arguments().length() != 3)
//    {
//        qDebug() << "Telegram_Channel_Report <channel username> <message count limit>";
//        return 0;
//    }

//    QString channel_username = a.arguments().at(1);
//    int count_limit = a.arguments().at(2).toInt();

    QString channel_username = "sutmusic";
    int count_limit = 10;
    QString phoneNumber = QString("+98xx");

//    QString outputName = QString("Output_%1.csv").arg( QDateTime::currentDateTimeUtc().toTime_t());
//    QFile file(outputName);
//    if(!file.open(QFile::WriteOnly| QFile::Truncate))
//    {
//        qDebug() <<"Problem in file";
//    }

//    QTextStream stream(&file);

    QTextStream cout(stdout);
    cout<<"Channel_id,message_id,views_count,publish_date\n";

    qputenv("QT_LOGGING_RULES", "tg.*=false");

    qDebug() << "Creating Telegram object...";
    Telegram telegram("149.154.167.50", 443, 2, 100465, "444a6b8c4d18ee472e7504b13b928387",
                      phoneNumber,
                      QCoreApplication::applicationDirPath(),
                      ":/tg-server.pub");

    Telegram::connect(&telegram, &Telegram::authNeeded, [&](){
        telegram.authSendCode([&](TG_AUTH_SEND_CODE_CALLBACK){
            Q_UNUSED(msgId)
            Q_UNUSED(result)
            Q_UNUSED(error)
            int code;
            qDebug() << "Please enter the number:";
            std::cin >> code;
            QString sCode = QString::number(code);
            telegram.authSignIn(sCode, [&](TG_AUTH_SIGN_IN_CALLBACK){
              Q_UNUSED(msgId);
              if(!error.null) {
                qDebug() << error.errorText;
              }
            });
        });
    });

    Telegram::connect(&telegram, &Telegram::authLoggedIn, [&](){
        qDebug() << "Logged In";
        telegram.contactsResolveUsername(channel_username, [&](TG_CONTACTS_RESOLVE_USERNAME_CALLBACK){
            Q_UNUSED(msgId)
            if(!error.null) {
                qDebug() << error.errorText;
                return;
            }

            Chat chat = result.chats().at(0);
            InputPeer inputPeer;
            inputPeer.setChannelId(chat.id());
            inputPeer.setAccessHash(chat.accessHash());
            inputPeer.setClassType(InputPeer::typeInputPeerChannel);

            int reqTimes = count_limit / 20 ;
            if(count_limit % 20) reqTimes++;

            for (int i=0; i<reqTimes; i++)
            {
                int limit = (i == reqTimes-1) ? count_limit % 20 : 20;
                int offset = i * 20;
                qDebug() <<offset <<limit;
                telegram.messagesGetHistory(inputPeer, 0, 0, offset, limit, 0, 0, [&](TG_MESSAGES_GET_HISTORY_CALLBACK){
                    Q_UNUSED(msgId)
                    if(!error.null) {
                        qDebug() << error.errorText;
                        return;
                    }

                    qDebug() <<"Total: " <<result.messages().length();
                    Q_FOREACH(const Message &msg, result.messages())
                    {
                        if(msg.classType() == Message::typeMessage)
                        {
                            cout <<QString("%1,%2,%3,%4\n").arg(msg.toId().channelId()).arg(msg.id()).arg(msg.views()).arg(msg.date());
                            cout.flush();
                        }
                    }
                }, 30000);
            }

        }, 30000);
    });

    qDebug() << "Initializing telegram...";
    telegram.init();

    return a.exec();
}
