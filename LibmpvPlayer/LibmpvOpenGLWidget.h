#pragma once

#include <QOpenGLWidget>

#include "LibmpvHelper.hpp"
#include "client.h"
#include "render_gl.h"

class LibmpvOpenGLWidget : public QOpenGLWidget {
    Q_OBJECT

public:
    LibmpvOpenGLWidget(QWidget* parent = nullptr);
    ~LibmpvOpenGLWidget();

public:
    bool load(const QString url);
    bool isPaused();

public Q_SLOTS:
    void play();
    void pause();
    void stop();
    void seek(int position);

Q_SIGNALS:
    void durationChanged(int value);
    void positionChanged(int value);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void on_mpv_events();
    void maybeUpdate();

private:
    void handle_mpv_event(mpv_event* event);
    static void on_update(void* ctx);

    mpv_handle* mpv;
    mpv_render_context* mpv_gl;
};
