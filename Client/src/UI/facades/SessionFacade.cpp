#include <tempolink/ui/facades/SessionFacade.h>

#include <tempolink/ui/facades/AppStateFacade.h>

namespace
{
QString connectionPhrase(int rtt)
{
    if (rtt <= 15)
        return "Direct link stable";
    if (rtt <= 30)
        return "Direct link moderate";
    return "Latency rising";
}
}

SessionFacade::SessionFacade(AppStateFacade& appState,
                             LobbyViewModel& lobbyModel,
                             ParticipantListModel& participantModel,
                             QObject* parent)
    : QObject(parent),
      m_appState(appState),
      m_lobbyModel(lobbyModel),
      m_participantModel(participantModel)
{
    initialiseRooms();
}

QString SessionFacade::authMessage() const
{
    return m_authMessage;
}

QString SessionFacade::roomName() const
{
    return m_currentRoomIndex >= 0 ? m_rooms[m_currentRoomIndex].summary.name : QString();
}

QString SessionFacade::roomHost() const
{
    return m_currentRoomIndex >= 0 ? m_rooms[m_currentRoomIndex].summary.host : QString();
}

QString SessionFacade::roomRegion() const
{
    return m_currentRoomIndex >= 0 ? m_rooms[m_currentRoomIndex].summary.region : QString();
}

int SessionFacade::roomBpm() const
{
    return m_currentRoomIndex >= 0 ? m_rooms[m_currentRoomIndex].summary.bpm : 0;
}

int SessionFacade::roomRtt() const
{
    return m_currentRoomIndex >= 0 ? m_rooms[m_currentRoomIndex].summary.rtt : 0;
}

bool SessionFacade::roomLive() const
{
    return m_currentRoomIndex >= 0 && m_rooms[m_currentRoomIndex].summary.live;
}

QString SessionFacade::meName() const
{
    return "Moonlight";
}

QString SessionFacade::meInstrument() const
{
    return QString::fromUtf8("건반");
}

QString SessionFacade::meRegion() const
{
    return "Seoul KR";
}

QString SessionFacade::meQuality() const
{
    return QString::fromUtf8("저압축");
}

void SessionFacade::login(const QString& email, const QString& password)
{
    if (email.trimmed().isEmpty() || password.trimmed().isEmpty())
    {
        setAuthMessage(QString::fromUtf8("이메일과 비밀번호를 입력해 주세요."));
        return;
    }

    enterLobby(QString::fromUtf8("로그인 프로토타입이 완료되었습니다."));
}

void SessionFacade::signup(const QString& name, const QString& email, const QString& password)
{
    if (name.trimmed().isEmpty() || email.trimmed().isEmpty() || password.trimmed().isEmpty())
    {
        setAuthMessage(QString::fromUtf8("이름, 이메일, 비밀번호를 입력해 주세요."));
        return;
    }

    enterLobby(QString::fromUtf8("회원가입 프로토타입이 완료되었습니다."));
}

void SessionFacade::continueWithGooglePrototype()
{
    enterLobby(QString::fromUtf8("Google 로그인 프로토타입입니다."));
}

void SessionFacade::joinRoom(int row)
{
    if (row < 0 || row >= m_rooms.size())
        return;

    setCurrentRoom(row);
    m_appState.navigate("room");
}

void SessionFacade::leaveRoom()
{
    m_appState.navigate("lobby");
}

void SessionFacade::initialiseRooms()
{
    m_rooms = {
        {
            { QString::fromUtf8("Aurora 새벽 세션"), QString::fromUtf8("윤도현"), "Seoul KR", 92, 12, true,
              { { QString::fromUtf8("윤도현"), QString::fromUtf8("기타") }, { QString::fromUtf8("하루"), QString::fromUtf8("드럼") }, { "KENTA", QString::fromUtf8("베이스") }, { QString::fromUtf8("서린"), QString::fromUtf8("보컬") } } },
            { { QString::fromUtf8("윤도현"), QString::fromUtf8("기타"), "Seoul KR", 9 },
              { QString::fromUtf8("하루"), QString::fromUtf8("드럼"), "Busan KR", 14 },
              { "KENTA", QString::fromUtf8("베이스"), "Tokyo JP", 28 },
              { QString::fromUtf8("서린"), QString::fromUtf8("보컬"), "Seoul KR", 11 } }
        },
        {
            { QString::fromUtf8("Moonchild 합주실"), "AKIRA", "Tokyo JP", 124, 26, true,
              { { "AKIRA", QString::fromUtf8("건반") }, { "MIYU", QString::fromUtf8("기타") }, { QString::fromUtf8("준"), QString::fromUtf8("드럼") } } },
            { { "AKIRA", QString::fromUtf8("건반"), "Tokyo JP", 24 },
              { "MIYU", QString::fromUtf8("기타"), "Osaka JP", 31 },
              { QString::fromUtf8("준"), QString::fromUtf8("드럼"), "Tokyo JP", 22 } }
        },
        {
            { QString::fromUtf8("심야 재즈 트리오"), QString::fromUtf8("강이든"), "Seoul KR", 78, 8, false,
              { { QString::fromUtf8("강이든"), QString::fromUtf8("베이스") }, { QString::fromUtf8("노아"), QString::fromUtf8("건반") } } },
            { { QString::fromUtf8("강이든"), QString::fromUtf8("베이스"), "Seoul KR", 7 },
              { QString::fromUtf8("노아"), QString::fromUtf8("건반"), "Seoul KR", 10 } }
        },
        {
            { QString::fromUtf8("Eclipse 리허설"), "RINA", "Osaka JP", 140, 33, false,
              { { "RINA", QString::fromUtf8("보컬") }, { "SORA", QString::fromUtf8("기타") }, { QString::fromUtf8("타쿠"), QString::fromUtf8("드럼") }, { QString::fromUtf8("미카"), QString::fromUtf8("베이스") } } },
            { { "RINA", QString::fromUtf8("보컬"), "Osaka JP", 30 },
              { "SORA", QString::fromUtf8("기타"), "Tokyo JP", 35 },
              { QString::fromUtf8("타쿠"), QString::fromUtf8("드럼"), "Tokyo JP", 29 },
              { QString::fromUtf8("미카"), QString::fromUtf8("베이스"), "Osaka JP", 38 } }
        }
    };

    QVector<RoomSummary> summaries;
    summaries.reserve(m_rooms.size());
    for (const auto& room : m_rooms)
        summaries.push_back(room.summary);

    m_lobbyModel.setRooms(std::move(summaries));
    setCurrentRoom(0);
}

void SessionFacade::enterLobby(const QString& message)
{
    setAuthMessage(message);
    m_appState.setConnectionStatusText("Direct session ready");
    m_appState.navigate("lobby");
}

void SessionFacade::setAuthMessage(const QString& message)
{
    if (m_authMessage == message)
        return;

    m_authMessage = message;
    emit authMessageChanged();
}

void SessionFacade::setCurrentRoom(int index)
{
    if (index < 0 || index >= m_rooms.size())
        return;

    m_currentRoomIndex = index;
    m_participantModel.setParticipants(m_rooms[index].participants);
    m_appState.setConnectionStatusText(connectionPhrase(m_rooms[index].summary.rtt));
    emit roomChanged();
}
