
#include "qspicehelper.h"
#include "qspiceplaybackchannel.h"

void QSpiceHelper::playback_data(SpicePlaybackChannel *channel,
                                 gpointer data,
                                 gint data_size,
                                 gpointer user_data)
{
    Q_UNUSED(channel);
    QSpicePlaybackChannel *_playback =
            static_cast<QSpicePlaybackChannel*>(user_data);
    if ( NULL==_playback ) return;
    emit _playback->playbackData((void*)data, data_size);
}

void QSpiceHelper::playback_get_delay(SpicePlaybackChannel *channel,
                                      gpointer user_data)
{
    Q_UNUSED(channel);
    QSpicePlaybackChannel *_playback =
            static_cast<QSpicePlaybackChannel*>(user_data);
    if ( NULL==_playback ) return;
    emit _playback->playbackGetDelay();
}
void QSpiceHelper::playback_start(SpicePlaybackChannel *channel,
                                  gint format,
                                  gint channels,
                                  gint rate,
                                  gpointer user_data)
{
    Q_UNUSED(channel);
    QSpicePlaybackChannel *_playback =
            static_cast<QSpicePlaybackChannel*>(user_data);
    if ( NULL==_playback ) return;
    emit _playback->playbackStart(format, channels, rate);
}

void QSpiceHelper::playback_stop(SpicePlaybackChannel *channel,
                                 gpointer user_data)
{
    Q_UNUSED(channel);
    QSpicePlaybackChannel *_playback =
            static_cast<QSpicePlaybackChannel*>(user_data);
    if ( NULL==_playback ) return;
    emit _playback->playbackStop();
}

void QSpicePlaybackChannel::initCallbacks()
{
    g_signal_connect(gobject, "playback-data",
                     (GCallback) QSpiceHelper::playback_data, this);
    g_signal_connect(gobject, "playback-get-delay",
                     (GCallback) QSpiceHelper::playback_get_delay, this);
    g_signal_connect(gobject, "playback-start",
                     (GCallback) QSpiceHelper::playback_start, this);
    g_signal_connect(gobject, "playback-stop",
                     (GCallback) QSpiceHelper::playback_stop, this);
}
