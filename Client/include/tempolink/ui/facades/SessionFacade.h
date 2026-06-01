#pragma once

#include <QObject>
#include <QString>
#include <QVector>

#include <tempolink/ui/models/LobbyViewModel.h>
#include <tempolink/ui/models/ParticipantListModel.h>

class AppStateFacade;

class SessionFacade final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString authMessage READ authMessage NOTIFY authMessageChanged)
    Q_PROPERTY(QString roomName READ roomName NOTIFY roomChanged)
    Q_PROPERTY(QString roomHost READ roomHost NOTIFY roomChanged)
    Q_PROPERTY(QString roomRegion READ roomRegion NOTIFY roomChanged)
    Q_PROPERTY(int roomBpm READ roomBpm NOTIFY roomChanged)
    Q_PROPERTY(int roomRtt READ roomRtt NOTIFY roomChanged)
    Q_PROPERTY(bool roomLive READ roomLive NOTIFY roomChanged)
    Q_PROPERTY(QString meName READ meName CONSTANT)
    Q_PROPERTY(QString meInstrument READ meInstrument CONSTANT)
    Q_PROPERTY(QString meRegion READ meRegion CONSTANT)
    Q_PROPERTY(QString meQuality READ meQuality CONSTANT)

public:
    SessionFacade(AppStateFacade& appState,
                  LobbyViewModel& lobbyModel,
                  ParticipantListModel& participantModel,
                  QObject* parent = nullptr);

    QString authMessage() const;
    QString roomName() const;
    QString roomHost() const;
    QString roomRegion() const;
    int roomBpm() const;
    int roomRtt() const;
    bool roomLive() const;
    QString meName() const;
    QString meInstrument() const;
    QString meRegion() const;
    QString meQuality() const;

    Q_INVOKABLE void login(const QString& email, const QString& password);
    Q_INVOKABLE void signup(const QString& name, const QString& email, const QString& password);
    Q_INVOKABLE void continueWithGooglePrototype();
    Q_INVOKABLE void joinRoom(int row);
    Q_INVOKABLE void leaveRoom();

signals:
    void authMessageChanged();
    void roomChanged();

private:
    struct RoomRecord
    {
        RoomSummary summary;
        QVector<ParticipantSummary> participants;
    };

    void initialiseRooms();
    void enterLobby(const QString& message);
    void setAuthMessage(const QString& message);
    void setCurrentRoom(int index);

    AppStateFacade& m_appState;
    LobbyViewModel& m_lobbyModel;
    ParticipantListModel& m_participantModel;
    QVector<RoomRecord> m_rooms;
    QString m_authMessage;
    int m_currentRoomIndex { -1 };
};
