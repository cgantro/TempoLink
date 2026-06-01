#include <tempolink/ui/models/ParticipantListModel.h>

ParticipantListModel::ParticipantListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ParticipantListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_participants.size();
}

QVariant ParticipantListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_participants.size())
        return {};

    const auto& participant = m_participants[index.row()];

    switch (role)
    {
        case NameRole: return participant.name;
        case InstrumentRole: return participant.instrument;
        case RegionRole: return participant.region;
        case RttRole: return participant.rtt;
        default: return {};
    }
}

QHash<int, QByteArray> ParticipantListModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { InstrumentRole, "instrument" },
        { RegionRole, "region" },
        { RttRole, "rtt" }
    };
}

void ParticipantListModel::setParticipants(QVector<ParticipantSummary> participants)
{
    beginResetModel();
    m_participants = std::move(participants);
    endResetModel();
}
