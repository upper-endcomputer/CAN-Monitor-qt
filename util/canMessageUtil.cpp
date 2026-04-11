#include "canMessageUtil.h"


QString generateIdString(int id, bool IDE, bool RTR)
{
    QString idString;
    if(IDE) idString += "E";
    if(RTR) idString += "R";
    if(IDE || RTR) idString += " ";

    if(IDE)
        idString += QString("%1").arg(id, 8, 16, QLatin1Char('0')).toUpper();
    else
        idString += QString("%1").arg(id, 3, 16, QLatin1Char('0')).toUpper();

    return idString;
}


QString generateIdString(const can_message_t * cmsg)
{
    return generateIdString(cmsg->id, cmsg->IDE, cmsg->RTR);
}

QString generateDataString(const can_message_t * cmsg)
{
    QString dataString;
    if(!cmsg->RTR) for(int i=0; i<cmsg->dlc; i++){
        dataString += QString("%1 ").arg(cmsg->data[i], 2, 16, QLatin1Char('0')).toUpper();
    }
    return dataString;
}
