#include "mpvitem.h"
#include "keys.h"

#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQuickWindow>
#include <QTemporaryFile>
#include <memory>

namespace {

QVariant toVariant(const mpv_node &n)
{
    switch (n.format) {
    case MPV_FORMAT_STRING: return QString::fromUtf8(n.u.string);
    case MPV_FORMAT_FLAG: return bool(n.u.flag);
    case MPV_FORMAT_INT64: return qlonglong(n.u.int64);
    case MPV_FORMAT_DOUBLE: return n.u.double_;
    case MPV_FORMAT_NODE_ARRAY: {
        QVariantList list;
        for (int i = 0; i < n.u.list->num; i++)
            list << toVariant(n.u.list->values[i]);
        return list;
    }
    case MPV_FORMAT_NODE_MAP: {
        QVariantMap map;
        for (int i = 0; i < n.u.list->num; i++)
            map.insert(QString::fromUtf8(n.u.list->keys[i]), toVariant(n.u.list->values[i]));
        return map;
    }
    default: return {};
    }
}

void *procAddress(void *, const char *name)
{
    auto ctx = QOpenGLContext::currentContext();
    return ctx ? reinterpret_cast<void *>(ctx->getProcAddress(name)) : nullptr;
}

class MpvRenderer : public QQuickFramebufferObject::Renderer
{
public:
    explicit MpvRenderer(MpvItem *item) : m_item(item) {}

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override
    {
        if (!m_item->gl) {
            mpv_opengl_init_params init{procAddress, nullptr};
            mpv_render_param params[]{
                {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
                {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &init},
                {MPV_RENDER_PARAM_INVALID, nullptr}};
            if (mpv_render_context_create(&m_item->gl, m_item->mpv, params) < 0)
                qFatal("failed to create mpv render context");
            mpv_render_context_set_update_callback(m_item->gl, [](void *ctx) {
                QMetaObject::invokeMethod(static_cast<MpvItem *>(ctx), &QQuickItem::update, Qt::QueuedConnection);
            }, m_item);
            QMetaObject::invokeMethod(m_item, [item = m_item] {
                item->m_ready = true;
                for (const auto &args : std::exchange(item->m_pending, {}))
                    item->command(args);
            }, Qt::QueuedConnection);
        }
        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

    void render() override
    {
        m_item->window()->beginExternalCommands();
        auto fbo = framebufferObject();
        mpv_opengl_fbo target{int(fbo->handle()), fbo->width(), fbo->height(), 0};
        int flip = 0;
        mpv_render_param params[]{
            {MPV_RENDER_PARAM_OPENGL_FBO, &target},
            {MPV_RENDER_PARAM_FLIP_Y, &flip},
            {MPV_RENDER_PARAM_INVALID, nullptr}};
        mpv_render_context_render(m_item->gl, params);
        m_item->window()->endExternalCommands();
    }

private:
    MpvItem *m_item;
};

} // namespace

MpvItem::MpvItem(QQuickItem *parent) : QQuickFramebufferObject(parent), mpv(mpv_create())
{
    if (!mpv)
        qFatal("could not create mpv context");

    // mpv reads input-conf from disk, so unpack the bundled IINA bindings once.
    std::unique_ptr<QTemporaryFile> conf(QTemporaryFile::createNativeFile(":/input.conf"));
    mpv_set_option_string(mpv, "vo", "libmpv");
    mpv_set_option_string(mpv, "hwdec", "auto-safe");
    mpv_set_option_string(mpv, "idle", "yes");
    mpv_set_option_string(mpv, "keep-open", "yes");
    mpv_set_option_string(mpv, "input-default-bindings", "no");
    if (conf)
        mpv_set_option_string(mpv, "input-conf", conf->fileName().toUtf8().constData());
    if (mpv_initialize(mpv) < 0)
        qFatal("could not initialize mpv");

    for (auto name : {"pause", "time-pos", "duration", "volume", "mute", "track-list",
                      "playlist", "playlist-pos", "fullscreen", "media-title", "idle-active"})
        mpv_observe_property(mpv, 0, name, MPV_FORMAT_NODE);

    mpv_set_wakeup_callback(mpv, [](void *ctx) {
        auto self = static_cast<MpvItem *>(ctx);
        QMetaObject::invokeMethod(self, [self] { self->drainEvents(); }, Qt::QueuedConnection);
    }, this);
}

MpvItem::~MpvItem()
{
    if (gl)
        mpv_render_context_free(gl);
    mpv_terminate_destroy(mpv);
}

QQuickFramebufferObject::Renderer *MpvItem::createRenderer() const
{
    window()->setPersistentGraphics(true);
    window()->setPersistentSceneGraph(true);
    return new MpvRenderer(const_cast<MpvItem *>(this));
}

void MpvItem::drainEvents()
{
    bool changed = false;
    while (auto ev = mpv_wait_event(mpv, 0)) {
        if (ev->event_id == MPV_EVENT_NONE)
            break;
        if (ev->event_id == MPV_EVENT_SHUTDOWN) {
            emit shutdown();
        } else if (ev->event_id == MPV_EVENT_PROPERTY_CHANGE) {
            auto p = static_cast<mpv_event_property *>(ev->data);
            m_state[p->name] = p->format == MPV_FORMAT_NODE ? toVariant(*static_cast<mpv_node *>(p->data)) : QVariant();
            changed = true;
        }
    }
    if (changed)
        emit stateChanged();
}

void MpvItem::command(const QStringList &args)
{
    // A file opened before the render context exists gets no video output in mpv.
    if (!m_ready && args.value(0) == "loadfile") {
        m_pending << args;
        return;
    }
    QList<QByteArray> utf8;
    QList<const char *> argv;
    for (const auto &a : args)
        utf8 << a.toUtf8();
    for (const auto &a : utf8)
        argv << a.constData();
    argv << nullptr;
    mpv_command_async(mpv, 0, argv.data());
}

void MpvItem::setProperty(const QString &name, const QVariant &value)
{
    // mpv parses every property from its string form ("yes"/"no", numbers, ids).
    auto str = value.typeId() == QMetaType::Bool ? QString(value.toBool() ? "yes" : "no") : value.toString();
    mpv_set_property_string(mpv, name.toUtf8().constData(), str.toUtf8().constData());
}

bool MpvItem::keypress(int key, int modifiers, const QString &text)
{
    const QString name = mpvKeyName(key, modifiers, text);
    if (!name.isEmpty())
        command({"keypress", name});
    return !name.isEmpty();
}
