#pragma once

#include <QQuickFramebufferObject>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>
#include <mpv/client.h>
#include <mpv/render_gl.h>

// Video surface backed by libmpv's render API. Lives in the Qt Quick scene,
// so QML effects (the glass panels) can sample the frames it draws.
class MpvItem : public QQuickFramebufferObject
{
    Q_OBJECT
    QML_ELEMENT
    // Observed mpv properties by name, e.g. state["time-pos"], state["track-list"].
    Q_PROPERTY(QVariantMap state READ state NOTIFY stateChanged)

public:
    explicit MpvItem(QQuickItem *parent = nullptr);
    ~MpvItem() override;

    Renderer *createRenderer() const override;
    QVariantMap state() const { return m_state; }

    Q_INVOKABLE void command(const QStringList &args);
    Q_INVOKABLE void setProperty(const QString &name, const QVariant &value);
    // Forwards a Qt key event to mpv as `keypress <name>`, so mpv's own
    // input.conf (IINA's bindings) decides what it does. Returns false if unmapped.
    Q_INVOKABLE bool keypress(int key, int modifiers, const QString &text);

    mpv_handle *mpv = nullptr;
    mpv_render_context *gl = nullptr;
    bool m_ready = false; // render context created; see command()
    QList<QStringList> m_pending;

signals:
    void stateChanged();
    void shutdown();

private:
    void drainEvents();
    QVariantMap m_state;
};
