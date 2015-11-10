/******************************************************************************
 * Copyright (C) 2015 Felix Rohrbach <kde@fxrh.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "messageeventmodel.h"

#include <QtCore/QDebug>

#include "lib/room.h"
#include "lib/events/event.h"
#include "lib/events/roommessageevent.h"
#include "lib/events/unknownevent.h"

MessageEventModel::MessageEventModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_currentRoom = 0;
}

MessageEventModel::~MessageEventModel()
{
}

void MessageEventModel::changeRoom(QMatrixClient::Room* room)
{
    beginResetModel();
    if( m_currentRoom )
    {
        disconnect( m_currentRoom, &QMatrixClient::Room::newMessage, this, &MessageEventModel::newMessage );
    }
    m_currentRoom = room;
    if( room )
    {
        m_currentMessages = room->messages();
        connect( room, &QMatrixClient::Room::newMessage, this, &MessageEventModel::newMessage );
        qDebug() << "connected" << room;
    }
    else
    {
        m_currentMessages = QList<QMatrixClient::Event*>();
    }
    endResetModel();
}

// QModelIndex LogMessageModel::index(int row, int column, const QModelIndex& parent) const
// {
//     if( parent.isValid() )
//         return QModelIndex();
//     if( row < 0 || row >= m_currentMessages.count() )
//         return QModelIndex();
//     return createIndex(row, column, m_currentMessages.at(row));
// }
//
// LogMessageModel::parent(const QModelIndex& index) const
// {
//     return QModelIndex();
// }

int MessageEventModel::rowCount(const QModelIndex& parent) const
{
    if( parent.isValid() )
        return 0;
    return m_currentMessages.count();
}

QVariant MessageEventModel::data(const QModelIndex& index, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();
    if( index.row() < 0 || index.row() >= m_currentMessages.count() )
        return QVariant();

    QMatrixClient::Event* event = m_currentMessages.at(index.row());
    if( event->type() == QMatrixClient::EventType::RoomMessage )
    {
        QMatrixClient::RoomMessageEvent* e = static_cast<QMatrixClient::RoomMessageEvent*>(event);
        return e->userId() + ": " + e->body();
    }
    if( event->type() == QMatrixClient::EventType::Unknown )
    {
        QMatrixClient::UnknownEvent* e = static_cast<QMatrixClient::UnknownEvent*>(event);
        return "Unknown Event: " + e->typeString() + "(" + e->content();
    }
    return "Unknown event";
}

void MessageEventModel::newMessage(QMatrixClient::Event* messageEvent)
{
    //qDebug() << "Message: " << message;
    if( messageEvent->type() == QMatrixClient::EventType::Typing )
    {
        return;
    }
    beginInsertRows(QModelIndex(), m_currentMessages.count(), m_currentMessages.count());
    m_currentMessages.append(messageEvent);
    endInsertRows();
}