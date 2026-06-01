#pragma once

#include <QObject>
#include <QString>

class AppStateFacade final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentScreen READ currentScreen NOTIFY currentScreenChanged)
    Q_PROPERTY(bool showRail READ showRail NOTIFY currentScreenChanged)
    Q_PROPERTY(QString connectionStatusText READ connectionStatusText NOTIFY connectionStatusTextChanged)

public:
    explicit AppStateFacade(QObject* parent = nullptr);

    QString currentScreen() const;
    bool showRail() const;
    QString connectionStatusText() const;

    Q_INVOKABLE void navigate(const QString& screenId);
    void setConnectionStatusText(const QString& text);

signals:
    void currentScreenChanged();
    void connectionStatusTextChanged();

private:
    QString m_currentScreen { "start" };
    QString m_connectionStatusText { "Direct session standby" };
};
