#include "roomshandler.h"

RoomsHandler::RoomsHandler()
{

}

void RoomsHandler::removeOldParticipantsFromQMap()
{
    Database* db = new Database(); //Each thread requires their own database connection.
    int mapParticipantsCounter = 0;
    int mapRoomsCounter = 0;
    int databaseCounter = 0;
    std::map<QString, std::map<QString, std::vector<QString>>>::iterator i = mMap.begin();
    while (i != mMap.end())
    {
        std::map<QString, std::vector<QString>>::iterator j = i->second.begin();
        while (j != i->second.end())
        {
            std::vector<QString> tempVector = j->second;
            int participantTimestampUnix = tempVector[1].toInt();
            QDateTime participantTimestamp;
            participantTimestamp.setTime_t(participantTimestampUnix);

            if ((participantTimestamp.secsTo(QDateTime::currentDateTime()) > 10)) //If the participant hasn't been active in the last minute
            {

                QSqlQuery q(db->mDb);
                q.prepare("DELETE FROM roomSession WHERE streamId = :streamId AND roomId = :roomId");
                q.bindValue(":streamId", j->first);
                q.bindValue(":roomId", i->first);
                if (q.exec())
                {
                    databaseCounter++;
                }
                else
                {
                    //qDebug() << "Failed Query" << Q_FUNC_INFO;
                }

                j = mMap[i->first].erase(j);
                mapParticipantsCounter++;
            }
            else
            {
                ++j;
            }
        }
        if (i->second.size() == 0)
        {
            i = mMap.erase(i);
            mapRoomsCounter++;
        }
        else
        {
            ++i;
        }
    }
    qDebug() << QDateTime::currentDateTime().toString("d.MMMM yyyy hh:mm:ss") << "Successfully removed"
             << mapParticipantsCounter << "(QMap P)" << mapRoomsCounter << "(QMap R)" << databaseCounter << "(Database).";
    delete db;
}

void RoomsHandler::initialInsert(QString roomId, QString streamId, QString ipAddress, QString firstHeader)
{
    std::vector<QString> tempVector = {ipAddress, QString::number(QDateTime::currentSecsSinceEpoch()), firstHeader};
    mMap[roomId][streamId] = tempVector;
    qDebug() << "Added streamId, ipAddress and timestamp:" << tempVector[0] << tempVector[1] << "to the QMap after confirming with database";
}

void RoomsHandler::printMap()
{
    qDebug() << "Printing start";
    std::map<QString, std::map<QString, std::vector<QString>>>::iterator i;
    for (i = mMap.begin(); i != mMap.end(); i++)
    {
        std::map<QString, std::vector<QString>>::iterator j;
        for (j = i->second.begin(); j != i->second.begin(); j++)
        {
            qDebug() << "Room:" << i->first << j->first;
            for (unsigned long k = 0; k < j->second.size(); k++)
            {
                qDebug() << k << j->second[k];
            }
        }
    }
    qDebug() << "Printing end";
}

void RoomsHandler::updateTimestamp(QString roomId, QString streamId)
{
    mMap[roomId][streamId][1] = QString::number(QDateTime::currentSecsSinceEpoch());
}

void RoomsHandler::startRemovalTimer(int seconds)
{
    qDebug() << "Removing inactive participants every" << seconds << "seconds.";
    int milliseconds = seconds * 1000;
    mAbortRemoval = false;
    std::thread t([=]()
    {
        while (!mAbortRemoval)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            removeOldParticipantsFromQMap();
        }
    });
    t.detach();
}

