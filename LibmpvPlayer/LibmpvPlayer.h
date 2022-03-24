#pragma once

#include "ui_LibmpvPlayer.h"
#include <QtCore/QSettings>
#include <QtWidgets/QMainWindow>

class QTimer;
class LibmpvPlayer : public QMainWindow {
    Q_OBJECT

public:
    LibmpvPlayer(QWidget* parent = Q_NULLPTR);
    ~LibmpvPlayer();

private:
    void initialSignalSlot();

protected:
    virtual void changeEvent(QEvent* event) override;

private:
    Ui::LibmpvPlayerClass ui;
    QTimer* p_timer;
    QTranslator* p_translator;
};
