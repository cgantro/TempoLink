#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct LobbyMemberPreview
{
    QString name;
    QString instrument;
};

struct RoomSummary
{
    QString name;
    QString host;
    QString region;
    int bpm = 0;
    int rtt = 0;
    bool live = false;
    QVector<LobbyMemberPreview> members;
};

class LobbyViewModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(QString statusFilter READ statusFilter WRITE setStatusFilter NOTIFY statusFilterChanged)

public:
    enum Role
    {
        NameRole = Qt::UserRole + 1,
        HostRole,
        RegionRole,
        BpmRole,
        RttRole,
        LiveRole,
        MemberCountRole,
        SourceIndexRole
    };

    explicit LobbyViewModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString searchQuery() const;
    void setSearchQuery(const QString& query);

    QString statusFilter() const;
    void setStatusFilter(const QString& filter);

    void setRooms(QVector<RoomSummary> rooms);
    RoomSummary roomAt(int index) const;

signals:
    void searchQueryChanged();
    void statusFilterChanged();

private:
    void rebuildVisibleRows();

    QVector<RoomSummary> m_rooms;
    QVector<int> m_visibleRows;
    QString m_searchQuery;
    QString m_statusFilter { "All" };
};
