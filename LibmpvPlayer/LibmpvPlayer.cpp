#include "LibmpvPlayer.h"

#include <QDebug>
#include <QFileDialog>
#include <QTimer>
#include <QTranslator>

LibmpvPlayer::LibmpvPlayer(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    statusBar();

    p_timer = new QTimer(this);
    p_timer->setInterval(200);
    p_timer->start();

    p_translator = new QTranslator(this);

    initialSignalSlot();

    ui.actionEnglish->trigger(); // set english language as default
}

LibmpvPlayer::~LibmpvPlayer()
{
    p_timer->stop();
}

void LibmpvPlayer::initialSignalSlot()
{
    connect(ui.actionOpen, &QAction::triggered, [=] {
        const QString fileName = QFileDialog::getOpenFileName(this, tr("Open Media File"));
        qDebug() << "media file: " << fileName;
        if (!fileName.isEmpty()) {
            ui.videoOpenGLWidget->load(fileName);
        }
    });

    connect(ui.actionQuit, &QAction::triggered, [=] {
        qApp->quit();
    });

    connect(ui.menuLanguage, &QMenu::triggered, [=](QAction* action) {
        if (action == ui.actionEnglish) {
            const QString lanEn = qApp->applicationDirPath() + "/Language/qm/libmpvplayer_en.qm";
            p_translator->load(lanEn);
        } else if (action = ui.actionChinese) {
            const QString lanZh = qApp->applicationDirPath() + "/Language/qm/libmpvplayer_zh.qm";
            p_translator->load(lanZh);
        }
        qApp->installTranslator(p_translator);
    });

    connect(ui.playPushButton, &QPushButton::clicked, [=] {
        const bool paused = ui.videoOpenGLWidget->isPaused();
        if (paused) {
            ui.videoOpenGLWidget->play();
        } else {
            ui.videoOpenGLWidget->pause();
        }
    });

    connect(ui.videoOpenGLWidget, &LibmpvOpenGLWidget::durationChanged, [=](int position) {
        ui.progressSlider->setRange(0, position);
    });

    connect(ui.videoOpenGLWidget, &LibmpvOpenGLWidget::positionChanged, [=](int position) {
        ui.progressSlider->setValue(position);
        ui.statusBar->showMessage(QString("%1:%2:%3")
                                      .arg(position / 3600, 2, 10, QLatin1Char('0'))
                                      .arg(position / 60 % 60, 2, 10, QLatin1Char('0'))
                                      .arg(position % 60, 2, 10, QLatin1Char('0')));
    });

    connect(ui.progressSlider, &QSlider::sliderMoved, [=](int position) {
        ui.videoOpenGLWidget->seek(position);
    });

    connect(ui.progressSlider, &MediaSlider::sliderClicked, [=](int _val) {
        ui.videoOpenGLWidget->seek(_val);
    });

    connect(p_timer, &QTimer::timeout, [=] {
        const bool paused = ui.videoOpenGLWidget->isPaused();
        if (paused) {
            ui.playPushButton->setIcon(QIcon(":/Resource/play_black.png"));
        } else {
            ui.playPushButton->setIcon(QIcon(":/Resource/pause_black.png"));
        }
    });
}

void LibmpvPlayer::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);
    if (QEvent::LanguageChange == event->type()) {
        ui.retranslateUi(this);
    }
}
