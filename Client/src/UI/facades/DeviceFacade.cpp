#include <tempolink/ui/facades/DeviceFacade.h>

DeviceFacade::DeviceFacade(AudioEngine& audioEngine, QObject* parent)
    : QObject(parent),
      m_audioEngine(audioEngine)
{
    refreshDevices();

    m_pollTimer.setInterval(60);
    connect(&m_pollTimer, &QTimer::timeout, this, [this]
    {
        const auto nextLevel = static_cast<double>(m_audioEngine.getInputLevel());
        if (!qFuzzyCompare(m_inputLevel, nextLevel))
        {
            m_inputLevel = nextLevel;
            emit inputLevelChanged();
        }

        const auto nextLatency = m_audioEngine.getEstimatedLatencyMs();
        if (!qFuzzyCompare(m_estimatedLatencyMs, nextLatency))
        {
            m_estimatedLatencyMs = nextLatency;
            emit estimatedLatencyChanged();
        }
    });
    m_pollTimer.start();
}

QStringList DeviceFacade::inputDevices() const
{
    return m_inputDevices;
}

QStringList DeviceFacade::outputDevices() const
{
    return m_outputDevices;
}

QString DeviceFacade::selectedInputDevice() const
{
    return m_selectedInputDevice;
}

QString DeviceFacade::selectedOutputDevice() const
{
    return m_selectedOutputDevice;
}

double DeviceFacade::inputLevel() const
{
    return m_inputLevel;
}

double DeviceFacade::estimatedLatencyMs() const
{
    return m_estimatedLatencyMs;
}

void DeviceFacade::refreshDevices()
{
    m_inputDevices = toStringList(m_audioEngine.getAvailableInputDevices());
    m_outputDevices = toStringList(m_audioEngine.getAvailableOutputDevices());
    m_selectedInputDevice = QString::fromStdString(m_audioEngine.getCurrentInputDevice().toStdString());
    m_selectedOutputDevice = QString::fromStdString(m_audioEngine.getCurrentOutputDevice().toStdString());
    m_estimatedLatencyMs = m_audioEngine.getEstimatedLatencyMs();
    emit devicesChanged();
    emit estimatedLatencyChanged();
}

void DeviceFacade::selectInputDevice(const QString& name)
{
    if (m_audioEngine.setInputDevice(juce::String(name.toStdString())))
        refreshDevices();
}

void DeviceFacade::selectOutputDevice(const QString& name)
{
    if (m_audioEngine.setOutputDevice(juce::String(name.toStdString())))
        refreshDevices();
}

QStringList DeviceFacade::toStringList(const juce::StringArray& source) const
{
    QStringList list;
    for (const auto& value : source)
        list.push_back(QString::fromStdString(value.toStdString()));

    return list;
}
