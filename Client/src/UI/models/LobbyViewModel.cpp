#include <tempolink/ui/models/LobbyViewModel.h>

LobbyViewModel::LobbyViewModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int LobbyViewModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_visibleRows.size();
}

QVariant LobbyViewModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_visibleRows.size())
        return {};

    const auto& room = m_rooms[m_visibleRows[index.row()]];

    switch (role)
    {
        case NameRole: return room.name;
        case HostRole: return room.host;
        case RegionRole: return room.region;
        case BpmRole: return room.bpm;
        case RttRole: return room.rtt;
        case LiveRole: return room.live;
        case MemberCountRole: return room.members.size();
        case SourceIndexRole: return m_visibleRows[index.row()];
        default: return {};
    }
}

QHash<int, QByteArray> LobbyViewModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { HostRole, "host" },
        { RegionRole, "region" },
        { BpmRole, "bpm" },
        { RttRole, "rtt" },
        { LiveRole, "live" },
        { MemberCountRole, "memberCount" },
        { SourceIndexRole, "sourceIndex" }
    };
}

QString LobbyViewModel::searchQuery() const
{
    return m_searchQuery;
}

void LobbyViewModel::setSearchQuery(const QString& query)
{
    if (m_searchQuery == query)
        return;

    m_searchQuery = query;
    rebuildVisibleRows();
    emit searchQueryChanged();
}

QString LobbyViewModel::statusFilter() const
{
    return m_statusFilter;
}

void LobbyViewModel::setStatusFilter(const QString& filter)
{
    if (m_statusFilter == filter)
        return;

    m_statusFilter = filter;
    rebuildVisibleRows();
    emit statusFilterChanged();
}

void LobbyViewModel::setRooms(QVector<RoomSummary> rooms)
{
    beginResetModel();
    m_rooms = std::move(rooms);
    endResetModel();
    rebuildVisibleRows();
}

RoomSummary LobbyViewModel::roomAt(int index) const
{
    if (index < 0 || index >= m_visibleRows.size())
        return {};

    return m_rooms[m_visibleRows[index]];
}

void LobbyViewModel::rebuildVisibleRows()
{
    beginResetModel();
    m_visibleRows.clear();

    const auto query = m_searchQuery.trimmed();
    for (auto i = 0; i < m_rooms.size(); ++i)
    {
        const auto& room = m_rooms[i];

        if (!query.isEmpty())
        {
            const auto haystack = room.name + " " + room.host;
            if (!haystack.contains(query, Qt::CaseInsensitive))
                continue;
        }

        if (m_statusFilter == "Live" && !room.live)
            continue;
        if (m_statusFilter == "Idle" && room.live)
            continue;
        if ((m_statusFilter == "KR" || m_statusFilter == "JP") && !room.region.contains(m_statusFilter, Qt::CaseInsensitive))
            continue;

        m_visibleRows.push_back(i);
    }

    endResetModel();
}
