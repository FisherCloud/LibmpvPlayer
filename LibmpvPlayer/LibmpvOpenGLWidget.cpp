#include "LibmpvOpenGLWidget.h"

#include <QtCore/QMetaObject>
#include <QtGui/QOpenGLContext>
#include <stdexcept>

static void wakeup(void* ctx)
{
    QMetaObject::invokeMethod((LibmpvOpenGLWidget*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

static void* get_proc_address(void* ctx, const char* name)
{
    Q_UNUSED(ctx);
    QOpenGLContext* glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return nullptr;
    return reinterpret_cast<void*>(glctx->getProcAddress(QByteArray(name)));
}

LibmpvOpenGLWidget::LibmpvOpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    mpv = mpv_create();
    if (!mpv)
        throw std::runtime_error("could not create mpv context");

    mpv_set_option_string(mpv, "terminal", "yes");
    mpv_set_option_string(mpv, "msg-level", "all=v");
    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Request hw decoding, just for testing.
    mpv::qt::set_option_variant(mpv, "hwdec", "auto");

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_set_wakeup_callback(mpv, wakeup, this);

    pause();
}

LibmpvOpenGLWidget::~LibmpvOpenGLWidget()
{
    makeCurrent();
    if (mpv_gl)
        mpv_render_context_free(mpv_gl);
    mpv_terminate_destroy(mpv);
}

bool LibmpvOpenGLWidget::load(const QString url)
{
    play();
    QVariant v_ret = mpv::qt::command_variant(mpv, QStringList() << "loadfile" << url);
    return !v_ret.isNull() && v_ret.isValid();
}

bool LibmpvOpenGLWidget::isPaused()
{
    return mpv::qt::get_property_variant(mpv, "pause").toBool();
}

void LibmpvOpenGLWidget::play()
{
    const bool pause = mpv::qt::get_property_variant(mpv, "pause").toBool();
    if (pause) {
        mpv::qt::set_property_variant(mpv, "pause", false);
    }
}

void LibmpvOpenGLWidget::pause()
{
    const bool pause = mpv::qt::get_property_variant(mpv, "pause").toBool();
    if (!pause) {
        mpv::qt::set_property_variant(mpv, "pause", true);
    }
}

void LibmpvOpenGLWidget::stop()
{
    mpv::qt::command_variant(mpv, QVariantList() << "stop");
}

void LibmpvOpenGLWidget::seek(int position)
{
    mpv::qt::command_variant(mpv, QVariantList() << "seek" << position << "absolute");
}

void LibmpvOpenGLWidget::initializeGL()
{
    mpv_opengl_init_params gl_init_params { get_proc_address, nullptr, nullptr };
    mpv_render_param params[] {
        { MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL) },
        { MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params },
        { MPV_RENDER_PARAM_INVALID, nullptr }
    };

    if (mpv_render_context_create(&mpv_gl, mpv, params) < 0)
        throw std::runtime_error("failed to initialize mpv GL context");
    mpv_render_context_set_update_callback(mpv_gl, LibmpvOpenGLWidget::on_update, reinterpret_cast<void*>(this));
}

void LibmpvOpenGLWidget::paintGL()
{
    mpv_opengl_fbo mpfbo { static_cast<int>(defaultFramebufferObject()), width(), height(), 0 };
    int flip_y { 1 };

    mpv_render_param params[] = {
        { MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo },
        { MPV_RENDER_PARAM_FLIP_Y, &flip_y },
        { MPV_RENDER_PARAM_INVALID, nullptr }
    };
    // See render_gl.h on what OpenGL environment mpv expects, and
    // other API details.
    mpv_render_context_render(mpv_gl, params);
}

void LibmpvOpenGLWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event* event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void LibmpvOpenGLWidget::handle_mpv_event(mpv_event* event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property* prop = (mpv_event_property*)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double*)prop->data;
                Q_EMIT positionChanged(time);
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double*)prop->data;
                Q_EMIT durationChanged(time);
            }
        }
        break;
    }
    default:;
        // Ignore uninteresting or unknown events.
    }
}

// Make Qt invoke mpv_render_context_render() to draw a new/updated video frame.
void LibmpvOpenGLWidget::maybeUpdate()
{
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's render API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        doneCurrent();
    } else {
        update();
    }
}

void LibmpvOpenGLWidget::on_update(void* ctx)
{
    QMetaObject::invokeMethod((LibmpvOpenGLWidget*)ctx, "maybeUpdate");
}
