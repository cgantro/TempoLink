#include <tempolink/ui/facades/AppStateFacade.h>

AppStateFacade::AppStateFacade(QObject* parent)
    : QObject(parent)
{
}

QString AppStateFacade::currentScreen() const
{
    return m_currentScreen;
}

bool AppStateFacade::showRail() const
{
    return m_currentScreen != "start";
}

QString AppStateFacade::connectionStatusText() const
{
    return m_connectionStatusText;
}

void AppStateFacade::navigate(const QString& screenId)
{
    if (m_currentScreen == screenId)
        return;

    m_currentScreen = screenId;
    emit currentScreenChanged();
}

void AppStateFacade::setConnectionStatusText(const QString& text)
{
    if (m_connectionStatusText == text)
        return;

    m_connectionStatusText = text;
    emit connectionStatusTextChanged();
}
