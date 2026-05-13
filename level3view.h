#ifndef LEVEL3VIEW_H
#define LEVEL3VIEW_H
#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>
#include <QVector>
#include <QString>
struct L3Tile {
    enum Type { FLOOR, WALL, SPIKE, LEVER, DOOR, GATE, GAS };
    Type type      = FLOOR;
    bool triggered = false;  
    bool locked_in = false;  
    int  gasTimer  = 0;
    int  leverIndex = -1;    
};
struct Gargoyle {
    float x, y;
    int   hp    = 2;
    bool  alive = true;
    float speed = 1.2f;
};
struct Projectile {
    float x, y, dx, dy;
    bool  active = true;
};
class Level3View : public QWidget
{
    Q_OBJECT
public:
    explicit Level3View(QWidget *parent = nullptr);
    void startLevel(const QString &playerName, const QString &role);
signals:
    void levelComplete();
protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
private slots:
    void onSplashTimer();
    void onGameTick();
private:
    void drawSplash(QPainter &p);
    void drawWitchBriefing(QPainter &p);
    void drawGargoyleArena(QPainter &p);
    void drawPoemClue(QPainter &p);
    void drawCorridor(QPainter &p);
    void drawVictoryScreen(QPainter &p);
    void drawGameOverScreen(QPainter &p);
    void drawGargoyle(QPainter &p, const Gargoyle &g);
    void drawPlayer(QPainter &p, float px, float py);
    void drawHUD(QPainter &p);
    void drawLever(QPainter &p, const QRect &tile, const L3Tile &t, int idx);
    void spawnGargoyles();
    void buildCorridor();
    void floodGas();
    void updateGargoyleArena();
    void updateCorridor();
    void playerShoot();
    void pullLever(int leverIndex);
    QString     poemTitle()       const;
    QStringList poem()            const;  
    QVector<int> correctSequence() const; 
    QString     leverSymbol(int i) const; 
    QString     leverGlyph(int i)  const; 
    enum Screen { SPLASH, WITCH_BRIEFING, GARGOYLE_ARENA, POEM_CLUE, CORRIDOR, VICTORY, GAMEOVER };
    Screen  m_screen = SPLASH;
    QString m_playerName;
    QString m_role;
    int     m_health  = 2;
    int     m_strikes = 0;   
    float   m_px = 60, m_py = 240;
    bool    m_keys[4] = {};
    QVector<Gargoyle>   m_gargoyles;
    QVector<Projectile> m_shots;
    bool    m_arenaComplete      = false;
    int     m_arenaCompleteTimer = 0;
    static const int GRID_W = 14;
    static const int GRID_H = 7;
    static const int TILE   = 50;
    static const int OX     = 25;
    static const int OY     = 80;
    L3Tile  m_grid[GRID_W][GRID_H];
    int     m_playerGX = 1, m_playerGY = 3;
    int     m_sequenceProgress = 0;  
    bool    m_gateOpen = false;
    int     m_gasTickCounter = 0;
    int     m_gasFloodTimer  = 0;
    int     m_sequenceTimeout = 0;  
    QString m_flashMsg;
    int     m_flashFrames = 0;
    QTimer *m_splashTimer = nullptr;
    QTimer *m_gameTick    = nullptr;
};
#endif
