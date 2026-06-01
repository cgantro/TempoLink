#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QTextStream>

#include <juce_events/juce_events.h>

#include <tempolink/audio/AudioEngine.h>
#include <tempolink/ui/facades/AppStateFacade.h>
#include <tempolink/ui/facades/DeviceFacade.h>
#include <tempolink/ui/facades/SessionFacade.h>
#include <tempolink/ui/models/LobbyViewModel.h>
#include <tempolink/ui/models/ParticipantListModel.h>

namespace
{
void appendQtLog(const QString& message)
{
    QFile file(QDir::currentPath() + "/tempolink-qt.log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream << message << Qt::endl;
}
}

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationDisplayName("TempoLink");
    app.setOrganizationName("TempoLink");
    QQuickStyle::setStyle("Basic");

    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    AudioEngine audioEngine;
    audioEngine.start();

    AppStateFacade appState;
    LobbyViewModel lobbyModel;
    ParticipantListModel participantModel;
    SessionFacade sessionFacade(appState, lobbyModel, participantModel);
    DeviceFacade deviceFacade(audioEngine);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appState", &appState);
    engine.rootContext()->setContextProperty("sessionFacade", &sessionFacade);
    engine.rootContext()->setContextProperty("deviceFacade", &deviceFacade);
    engine.rootContext()->setContextProperty("lobbyModel", &lobbyModel);
    engine.rootContext()->setContextProperty("participantModel", &participantModel);

    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app, [](const QList<QQmlError>& warnings)
    {
        for (const auto& warning : warnings)
            appendQtLog(warning.toString());
    });

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []()
    {
        appendQtLog("QML object creation failed");
        QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.loadFromModule("TempoLink", "Main");

    const auto exitCode = app.exec();
    audioEngine.stop();
    return exitCode;
}
