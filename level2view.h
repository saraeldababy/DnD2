#ifndef LEVEL2VIEW_H
#define LEVEL2VIEW_H

#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

#include "witchscene.h"

// ---------------------------------------------------------------------------
// Level2View
//   Self-contained widget for the Level-2 experience:
//     1. Loading splash  ("Level 2 – The Witch's Cottage")
//     2. Cottage interior scene with witch dialogue
//     3. Riddle challenge (role-specific)
//     4. Escape / victory when riddle solved
// ---------------------------------------------------------------------------
class Level2View : public QWidget
{
    Q_OBJECT

public:
    explicit Level2View(QWidget *parent = nullptr);

    // Call before showing this widget
    void startLevel(const QString &playerName, const QString &role);

signals:
    void levelComplete();   // player solved the riddle and escapes
    void levelFailed();     // (reserved – currently unused for L2)

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onSplashTimer();
    void onRiddleButton();
    void onSubmitAnswer();

private:
    // --- helpers
    void drawSplash(QPainter &p);
    void drawCottageInterior(QPainter &p);
    void drawWitch(QPainter &p, int cx, int cy);
    void drawPlayer(QPainter &p, int px, int py);
    void drawDialogBubble(QPainter &p, const QRect &rect, const QString &text,
                          bool dark = false);
    void drawWitchPanel(QPainter &p);
    void drawEscapeScreen(QPainter &p);
    void positionOverlayWidgets();

    // --- state
    enum Screen { SPLASH, SCENE, ESCAPED_SCREEN };
    Screen      m_screen    = SPLASH;
    int         m_splashTick = 0;      // counts up while splash is shown

    QString     m_playerName;
    QString     m_role;
    WitchScene *m_witch     = nullptr;

    // Player walk-in animation
    int         m_playerX   = 0;      // pixel X inside interior (animated)
    bool        m_walkedIn  = false;

    QTimer     *m_splashTimer = nullptr;

    // Overlay widgets (riddle answer)
    QPushButton *m_riddleBtn   = nullptr;
    QLineEdit   *m_answerEdit  = nullptr;
    QPushButton *m_submitBtn   = nullptr;
};

#endif // LEVEL2VIEW_H
