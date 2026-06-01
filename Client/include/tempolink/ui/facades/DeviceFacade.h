#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <tempolink/audio/AudioEngine.h>

class DeviceFacade final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList inputDevices READ inputDevices NOTIFY devicesChanged)
    Q_PROPERTY(QStringList outputDevices READ outputDevices NOTIFY devicesChanged)
    Q_PROPERTY(QString selectedInputDevice READ selectedInputDevice NOTIFY devicesChanged)
    Q_PROPERTY(QString selectedOutputDevice READ selectedOutputDevice NOTIFY devicesChanged)
    Q_PROPERTY(double inputLevel READ inputLevel NOTIFY inputLevelChanged)
    Q_PROPERTY(double estimatedLatencyMs READ estimatedLatencyMs NOTIFY estimatedLatencyChanged)

public:
    explicit DeviceFacade(AudioEngine& audioEngine, QObject* parent = nullptr);

    QStringList inputDevices() const;
    QStringList outputDevices() const;
    QString selectedInputDevice() const;
    QString selectedOutputDevice() const;
    double inputLevel() const;
    double estimatedLatencyMs() const;

    Q_INVOKABLE void refreshDevices();
    Q_INVOKABLE void selectInputDevice(const QString& name);
    Q_INVOKABLE void selectOutputDevice(const QString& name);

signals:
    void devicesChanged();
    void inputLevelChanged();
    void estimatedLatencyChanged();

private:
    QStringList toStringList(const juce::StringArray& source) const;

    AudioEngine& m_audioEngine;
    QTimer m_pollTimer;
    QStringList m_inputDevices;
    QStringList m_outputDevices;
    QString m_selectedInputDevice;
    QString m_selectedOutputDevice;
    double m_inputLevel = 0.0;
    double m_estimatedLatencyMs = 0.0;
};
