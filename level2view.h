#ifndef LEVEL2VIEW_H
#define LEVEL2VIEW_H
#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVector>
#include <QPoint>
#include "witchscene.h"
struct L2Tile {
    enum Type { FLOOR, WALL, TRAP, TREASURE, DOOR, WITCH_ROOM };
    Type type = FLOOR;
    bool collected = false; 
    bool triggered = false; 
};
class Level2View : public QWidget
{
    Q_OBJECT
public:
    explicit Level2View(QWidget *parent = nullptr);
    void startLevel(const QString &playerName, const QString &role);
signals:
    void levelComplete();
    void levelFailed();
protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
private slots:
    void onSplashTimer();
    void onRiddleButton();
    void onSubmitAnswer();
private:
    void drawSplash(QPainter &p);
    void drawCorridor(QPainter &p);
    void drawWitchRoom(QPainter &p);
    void drawWitch(QPainter &p, int cx, int cy);
    void drawPlayer(QPainter &p, int px, int py);
    void drawHUD(QPainter &p);
    void drawWitchPanel(QPainter &p);
    void drawEscapeScreen(QPainter &p);
    void drawGameOverScreen(QPainter &p);
    void positionOverlayWidgets();
    void buildCorridor();
    enum Screen { SPLASH, CORRIDOR, WITCH_ROOM_SCREEN, ESCAPED_SCREEN, GAMEOVER_SCREEN };
    Screen      m_screen = SPLASH;
    QString     m_playerName;
    QString     m_role;
    int         m_health     = 3;       
    int         m_playerGX   = 1;       
    int         m_playerGY   = 2;       
    bool        m_hasTreasureHint   = false;  
    bool        m_hasBackupHint     = false;  
    static const int GRID_W = 10;
    static const int GRID_H = 5;
    L2Tile      m_grid[GRID_W][GRID_H];
    WitchScene *m_witch = nullptr;
    QTimer     *m_splashTimer   = nullptr;
    QPushButton *m_riddleBtn  = nullptr;
    QLineEdit   *m_answerEdit = nullptr;
    QPushButton *m_submitBtn  = nullptr;
    QString     m_flashMsg;
    int         m_flashFrames = 0;
};
#endif 
