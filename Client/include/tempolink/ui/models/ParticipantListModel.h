#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct ParticipantSummary
{
    QString name;
    QString instrument;
    QString region;
    int rtt = 0;
};

class ParticipantListModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        NameRole = Qt::UserRole + 1,
        InstrumentRole,
        RegionRole,
        RttRole
    };

    explicit ParticipantListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setParticipants(QVector<ParticipantSummary> participants);

private:
    QVector<ParticipantSummary> m_participants;
};
