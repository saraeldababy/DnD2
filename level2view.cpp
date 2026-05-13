#include "level2view.h"
#include <QPainter>
#include <QFont>
#include <QTimer>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QKeyEvent>
static const int TILE   = 60;   
static const int OX     = 80;   
static const int OY     = 90;   
Level2View::Level2View(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(800, 600);
    m_riddleBtn = new QPushButton("Ask for the Riddle", this);
    m_riddleBtn->setFont(QFont("Georgia", 12, QFont::Bold));
    m_riddleBtn->setStyleSheet(
        "QPushButton { background:#3b1f0e; color:#f2d98a; border:2px solid #8b5e3c;"
        " border-radius:8px; padding:8px 20px; }"
        "QPushButton:hover { background:#5a3118; border-color:#c8973a; }");
    m_riddleBtn->hide();
    connect(m_riddleBtn, &QPushButton::clicked, this, &Level2View::onRiddleButton);
    m_answerEdit = new QLineEdit(this);
    m_answerEdit->setPlaceholderText("Type your answer...");
    m_answerEdit->setFont(QFont("Georgia", 12));
    m_answerEdit->setStyleSheet(
        "QLineEdit { background:#1e120a; color:#f2e6c8; border:2px solid #7a5230;"
        " border-radius:6px; padding:6px 12px; }");
    m_answerEdit->hide();
    connect(m_answerEdit, &QLineEdit::returnPressed, this, &Level2View::onSubmitAnswer);
    m_submitBtn = new QPushButton("Submit", this);
    m_submitBtn->setFont(QFont("Georgia", 12, QFont::Bold));
    m_submitBtn->setStyleSheet(
        "QPushButton { background:#5c2d0b; color:#f2d98a; border:2px solid #9e6b3a;"
        " border-radius:8px; padding:8px 18px; }"
        "QPushButton:hover { background:#7a3d10; border-color:#d4a44c; }");
    m_submitBtn->hide();
    connect(m_submitBtn, &QPushButton::clicked, this, &Level2View::onSubmitAnswer);
    m_splashTimer = new QTimer(this);
    m_splashTimer->setSingleShot(true);
    connect(m_splashTimer, &QTimer::timeout, this, &Level2View::onSplashTimer);
}
void Level2View::startLevel(const QString &playerName, const QString &role)
{
    m_playerName      = playerName.isEmpty() ? "Adventurer" : playerName;
    m_role            = role;
    m_health          = 3;
    m_hasTreasureHint = false;
    m_hasBackupHint   = false;
    m_flashMsg        = "";
    m_flashFrames     = 0;
    m_screen          = SPLASH;
    m_playerGX        = 1;
    m_playerGY        = 2;
    delete m_witch;
    m_witch = new WitchScene(role);
    buildCorridor();
    m_riddleBtn->hide();
    m_answerEdit->hide();
    m_submitBtn->hide();
    m_splashTimer->start(2800);
    setFocus();
    update();
}
void Level2View::buildCorridor()
{
    for (int x = 0; x < GRID_W; ++x)
        for (int y = 0; y < GRID_H; ++y)
            m_grid[x][y].type      = L2Tile::FLOOR,
            m_grid[x][y].collected  = false,
            m_grid[x][y].triggered  = false;
    for (int x = 0; x < GRID_W; ++x) {
        m_grid[x][0].type = L2Tile::WALL;
        m_grid[x][4].type = L2Tile::WALL;
    }
    for (int y = 0; y < GRID_H; ++y) {
        m_grid[0][y].type = L2Tile::WALL;
        m_grid[9][y].type = L2Tile::WALL;
    }
    m_grid[9][2].type = L2Tile::DOOR;
    m_grid[5][2].type = L2Tile::TRAP;
    m_grid[7][3].type = L2Tile::TRAP;
    m_grid[3][1].type = L2Tile::TREASURE;
    m_grid[4][3].type = L2Tile::TREASURE;
}
void Level2View::onSplashTimer()
{
    m_screen = CORRIDOR;
    update();
}
void Level2View::onRiddleButton()
{
    m_witch->showRiddle();
    m_riddleBtn->hide();
    m_answerEdit->show();
    m_submitBtn->show();
    m_answerEdit->setFocus();
    positionOverlayWidgets();
    update();
}
void Level2View::onSubmitAnswer()
{
    if (!m_witch) return;
    m_witch->submitAnswer(m_answerEdit->text());
    m_answerEdit->clear();
    if (m_witch->escaped()) {
        m_answerEdit->hide();
        m_submitBtn->hide();
        m_screen = ESCAPED_SCREEN;
        update();
        QTimer::singleShot(2600, this, &Level2View::levelComplete);
    } else {
        m_health--;
        m_flashMsg    = "Wrong answer! You lose a heart!";
        m_flashFrames = 90;
        if (m_health <= 0) {
            m_answerEdit->hide();
            m_submitBtn->hide();
            m_riddleBtn->hide();
            m_screen = GAMEOVER_SCREEN;
            update();
            return;
        }
        if (m_witch->phase() == WitchScene::PHASE_WRONG) {
            m_answerEdit->show();
            m_submitBtn->show();
        }
        positionOverlayWidgets();
        update();
    }
}
void Level2View::positionOverlayWidgets()
{
    if (!m_witch) return;
    const WitchScene::Phase ph = m_witch->phase();
    if (ph == WitchScene::PHASE_TAUNT) {
        m_riddleBtn->move(width()/2 - m_riddleBtn->sizeHint().width()/2, 530);
    } else if (ph == WitchScene::PHASE_RIDDLE || ph == WitchScene::PHASE_WRONG) {
        m_answerEdit->setFixedWidth(300);
        m_answerEdit->move(width()/2 - 170, 534);
        m_submitBtn->move(width()/2 + 146, 530);
    }
}
void Level2View::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    switch (m_screen) {
    case SPLASH:            drawSplash(p);        break;
    case CORRIDOR:          drawCorridor(p);      break;
    case WITCH_ROOM_SCREEN: drawWitchRoom(p);     break;
    case ESCAPED_SCREEN:    drawEscapeScreen(p);  break;
    case GAMEOVER_SCREEN:   drawGameOverScreen(p);break;
    }
    if (m_screen != SPLASH && m_flashFrames > 0) {
        const int alpha = qMin(255, m_flashFrames * 4);
        p.setFont(QFont("Georgia", 13, QFont::Bold));
        p.setPen(QColor(220, 80, 60, alpha));
        p.drawText(QRect(0, 430, width(), 30), Qt::AlignCenter, m_flashMsg);
        m_flashFrames--;
        QTimer::singleShot(16, this, [this]{ update(); });
    }
}
void Level2View::drawSplash(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());
    bg.setColorAt(0.0, QColor(12,6,20));
    bg.setColorAt(0.5, QColor(28,14,38));
    bg.setColorAt(1.0, QColor(10,5,15));
    p.fillRect(rect(), bg);
    QRadialGradient vg(width()/2, height()/2, 340);
    vg.setColorAt(0.0, QColor(90,40,120,80));
    vg.setColorAt(1.0, QColor(0,0,0,0));
    p.fillRect(rect(), vg);
    p.setPen(QPen(QColor(140,80,180,160),1));
    p.drawLine(80, height()/2-70, width()-80, height()/2-70);
    p.drawLine(80, height()/2+70, width()-80, height()/2+70);
    auto diamond = [&](int x, int y){
        QPoint pts[4]={{x,y-6},{x+6,y},{x,y+6},{x-6,y}};
        p.setBrush(QColor(160,100,200)); p.setPen(Qt::NoPen);
        p.drawPolygon(pts,4);
    };
    diamond(80, height()/2-70); diamond(width()-80, height()/2-70);
    diamond(80, height()/2+70); diamond(width()-80, height()/2+70);
    p.setPen(QColor(170,120,210));
    p.setFont(QFont("Georgia",14,QFont::Bold));
    p.drawText(rect().adjusted(0,-90,0,0), Qt::AlignCenter, "— LEVEL 2 —");
    p.setPen(QColor(242,220,168));
    p.setFont(QFont("Georgia",30,QFont::Bold));
    p.drawText(rect(), Qt::AlignCenter, "The Witch's Cottage");
    p.setPen(QColor(180,150,210,200));
    p.setFont(QFont("Georgia",13,QFont::StyleItalic));
    p.drawText(rect().adjusted(0,90,0,0), Qt::AlignCenter,
               "Something stirs within the old stone walls...");
}
void Level2View::drawCorridor(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());
    bg.setColorAt(0.0, QColor(15,10,5));
    bg.setColorAt(1.0, QColor(25,16,8));
    p.fillRect(rect(), bg);
    for (int x = 0; x < GRID_W; ++x) {
        for (int y = 0; y < GRID_H; ++y) {
            QRect tile(OX + x*TILE, OY + y*TILE, TILE, TILE);
            const L2Tile &t = m_grid[x][y];
            switch (t.type) {
            case L2Tile::WALL:
                p.setBrush(QColor(55,42,30));
                p.setPen(QPen(QColor(35,26,16),1));
                p.drawRect(tile);
                p.setBrush(QColor(65,50,36));
                p.drawRoundedRect(tile.adjusted(4,4,-4,-4),3,3);
                break;
            case L2Tile::FLOOR:
                p.setBrush((x+y)%2==0 ? QColor(62,50,38) : QColor(55,44,32));
                p.setPen(QPen(QColor(40,30,20),1));
                p.drawRect(tile);
                break;
            case L2Tile::TRAP:
                p.setBrush(QColor(55,44,32));
                p.setPen(QPen(QColor(40,30,20),1));
                p.drawRect(tile);
                if (!t.triggered) {
                    p.setPen(QPen(QColor(160,30,30),2));
                    for (int i=0;i<3;i++) {
                        int sx = tile.left()+12+i*16;
                        p.drawLine(sx, tile.bottom()-8, sx, tile.top()+10);
                        p.drawLine(sx-4, tile.bottom()-8, sx, tile.top()+10);
                        p.drawLine(sx+4, tile.bottom()-8, sx, tile.top()+10);
                    }
                } else {
                    p.setBrush(QColor(120,20,20,120));
                    p.setPen(Qt::NoPen);
                    p.drawEllipse(tile.adjusted(10,10,-10,-10));
                }
                break;
            case L2Tile::TREASURE:
                p.setBrush(QColor(55,44,32));
                p.setPen(QPen(QColor(40,30,20),1));
                p.drawRect(tile);
                if (!t.collected) {
                    p.setBrush(QColor(120,80,30));
                    p.setPen(QPen(QColor(80,50,15),2));
                    p.drawRoundedRect(tile.adjusted(10,18,-10,-10),5,5);
                    p.setBrush(QColor(150,100,40));
                    p.drawRoundedRect(tile.adjusted(10,12,-10,-28),5,5);
                    p.setBrush(QColor(220,180,60));
                    p.setPen(Qt::NoPen);
                    p.drawEllipse(tile.center().x()-4, tile.top()+22, 8,8);
                    QRadialGradient cg(tile.center(), 28);
                    cg.setColorAt(0.0, QColor(220,180,60,80));
                    cg.setColorAt(1.0, QColor(0,0,0,0));
                    p.fillRect(tile, cg);
                } else {
                    p.setBrush(QColor(80,55,20));
                    p.setPen(QPen(QColor(60,40,10),2));
                    p.drawRoundedRect(tile.adjusted(10,22,-10,-10),5,5);
                    p.setBrush(QColor(100,70,25));
                    p.drawRoundedRect(tile.adjusted(10,14,-10,-34),5,5);
                }
                break;
            case L2Tile::DOOR:
                p.setBrush(QColor(55,42,30));
                p.setPen(QPen(QColor(35,26,16),1));
                p.drawRect(tile);
                p.setBrush(QColor(90,60,35));
                p.setPen(QPen(QColor(60,40,20),2));
                p.drawRoundedRect(tile.adjusted(8,6,-8,-2),6,6);
                p.setBrush(QColor(200,160,50));
                p.setPen(Qt::NoPen);
                p.drawEllipse(tile.center().x()-5, tile.center().y()-5, 10,10);
                p.setBrush(QColor(180,140,40));
                p.drawRect(tile.center().x()-3, tile.center().y(), 6,8);
                break;
            default: break;
            }
        }
    }
    drawPlayer(p, OX + m_playerGX*TILE + 10, OY + m_playerGY*TILE + 8);
    for (int i : {2, 6}) {
        int tx = OX + i*TILE + 22;
        int ty = OY + 0*TILE + TILE - 8;
        p.setPen(QPen(QColor(80,55,30),2));
        p.drawLine(tx, ty, tx, ty+10);
        p.setBrush(QColor(255,180,60));
        p.setPen(Qt::NoPen);
        p.drawEllipse(tx-5, ty-8, 10,14);
        QRadialGradient tg(tx, ty, 30);
        tg.setColorAt(0.0, QColor(255,150,30,60));
        tg.setColorAt(1.0, QColor(0,0,0,0));
        p.fillRect(OX+i*TILE, OY, TILE*2, TILE*2, tg);
    }
    drawHUD(p);
    p.setPen(QColor(242,220,168));
    p.setFont(QFont("Georgia",17,QFont::Bold));
    p.drawText(24, 34, "Level 2 – The Witch's Cottage");
    p.setFont(QFont("Georgia",10,QFont::StyleItalic));
    p.setPen(QColor(180,155,120));
    p.drawText(24, 52, "Room 1: The Entrance Corridor  |  Arrow keys to move");
    if (m_hasTreasureHint) {
        p.setFont(QFont("Georgia",10,QFont::Bold));
        p.setPen(QColor(220,180,60));
        p.drawText(QRect(0, 460, width(), 24), Qt::AlignCenter,
                   "★ Treasure found! You carry a hint for the witch's riddle. ★");
    }
    p.setFont(QFont("Georgia",10,QFont::StyleItalic));
    p.setPen(QColor(160,130,90,180));
    p.drawText(QRect(0,480,width(),20), Qt::AlignCenter,
               "Reach the door on the right to enter the witch's lair.");
}
void Level2View::drawWitchRoom(QPainter &p)
{
    if (!m_witch) return;
    QLinearGradient bg(0,0,0,height());
    bg.setColorAt(0.0, QColor(18,10,6));
    bg.setColorAt(0.4, QColor(42,22,12));
    bg.setColorAt(1.0, QColor(30,14,6));
    p.fillRect(rect(), bg);
    p.setPen(Qt::NoPen);
    for (int row=0; row<3; ++row)
        for (int col=0; col<10; ++col) {
            p.setBrush((col+row)%2==0 ? QColor(62,50,40) : QColor(55,44,34));
            p.drawRoundedRect(col*82+(row%2)*41, 430+row*34, 80,32, 4,4);
        }
    p.setBrush(QColor(38,28,20));
    p.drawRect(0,0,width(),430);
    p.setPen(QPen(QColor(28,20,14),1));
    for (int row=0; row<9; ++row)
        for (int col=0; col<12; ++col) {
            int wx=col*70+(row%2)*35-10, wy=60+row*42;
            if (wy>430) break;
            p.setBrush(QColor(48+(col*3)%12, 35+(row*2)%8, 22));
            p.drawRoundedRect(wx,wy,68,38,3,3);
        }
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(60,42,28));
    p.drawRect(580,220,180,210);
    p.setBrush(QColor(15,8,4));
    p.drawRoundedRect(608,280,124,150,10,10);
    auto flame=[&](int fx,int fy,int w,int h,QColor c){ p.setBrush(c); p.drawEllipse(fx,fy,w,h); };
    flame(618,310,40,90,QColor(200,80,10,220));
    flame(638,295,50,110,QColor(230,130,20,200));
    flame(668,318,38,85,QColor(210,90,15,210));
    flame(645,285,30,70,QColor(255,200,60,180));
    QRadialGradient fg(700,360,200);
    fg.setColorAt(0.0,QColor(220,110,20,70)); fg.setColorAt(1.0,QColor(0,0,0,0));
    p.fillRect(rect(),fg);
    p.setBrush(QColor(28,28,28)); p.setPen(QPen(QColor(80,80,80),2));
    p.drawEllipse(628,268,84,44);
    p.setBrush(QColor(10,48,30)); p.setPen(Qt::NoPen);
    p.drawEllipse(636,272,68,28);
    p.setBrush(QColor(20,180,100,160));
    p.drawEllipse(648,270,8,8); p.drawEllipse(668,266,6,6); p.drawEllipse(685,272,7,7);
    for (int shelf=0; shelf<2; ++shelf) {
        int sy=130+shelf*110;
        p.setBrush(QColor(72,50,32)); p.setPen(Qt::NoPen);
        p.drawRoundedRect(20,sy,160,12,3,3);
        const QColor bc[]={QColor(100,20,140),QColor(20,100,60),QColor(160,80,10),QColor(40,80,160)};
        for (int b=0;b<4;++b){
            int bx=28+b*38, by=sy-46;
            p.setBrush(bc[b]); p.drawRoundedRect(bx,by+14,18,32,5,5);
            p.setBrush(QColor(80,65,52)); p.drawRoundedRect(bx+5,by,8,16,3,3);
        }
    }
    drawWitch(p, 300, 220);
    drawPlayer(p, 100, 350);
    drawWitchPanel(p);
    drawHUD(p);
    p.setPen(QColor(242,220,168));
    p.setFont(QFont("Georgia",17,QFont::Bold));
    p.drawText(24,34,"Level 2 – The Witch's Cottage");
    p.setFont(QFont("Georgia",10,QFont::StyleItalic));
    p.setPen(QColor(180,155,120));
    p.drawText(24,52, "["+m_role+"]  "+m_playerName);
}
void Level2View::drawWitch(QPainter &p, int cx, int cy)
{
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(48,18,68));
    QPoint robe[5]={{cx-36,cy+130},{cx+36,cy+130},{cx+28,cy+50},{cx,cy+40},{cx-28,cy+50}};
    p.drawPolygon(robe,5);
    p.setBrush(QColor(55,22,78));
    p.drawRoundedRect(cx-22,cy+40,44,70,10,10);
    p.setBrush(QColor(176,148,110));
    p.drawEllipse(cx-18,cy,36,40);
    p.setBrush(QColor(22,14,32));
    p.drawEllipse(cx-28,cy+2,56,14);
    QPoint hat[3]={{cx-22,cy+8},{cx+22,cy+8},{cx+4,cy-52}};
    p.setBrush(QColor(28,16,40)); p.drawPolygon(hat,3);
    p.setBrush(QColor(120,50,160)); p.drawRect(cx-20,cy+2,40,8);
    p.setBrush(QColor(60,220,80));
    p.drawEllipse(cx-10,cy+14,8,8); p.drawEllipse(cx+2,cy+14,8,8);
    p.setBrush(QColor(10,60,16));
    p.drawEllipse(cx-8,cy+16,4,4); p.drawEllipse(cx+4,cy+16,4,4);
    p.setPen(QPen(QColor(120,90,60),2));
    p.drawLine(cx,cy+22,cx-4,cy+30); p.drawLine(cx-4,cy+30,cx+2,cy+32);
    p.setPen(QPen(QColor(80,55,30),3));
    p.drawLine(cx+26,cy+110,cx+42,cy-40);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(140,60,200,200));
    p.drawEllipse(cx+36,cy-52,18,18);
    QRadialGradient og(cx+45,cy-43,20);
    og.setColorAt(0.0,QColor(180,100,255,120)); og.setColorAt(1.0,QColor(0,0,0,0));
    p.fillRect(cx+24,cy-64,44,44,og);
}
void Level2View::drawPlayer(QPainter &p, int px, int py)
{
    QColor cloak(76,124,215);
    if      (m_role=="Wizard")  cloak=QColor(85,102,224);
    else if (m_role=="Fighter") cloak=QColor(171,72,65);
    else if (m_role=="Rogue")   cloak=QColor(65,138,94);
    else if (m_role=="Cleric")  cloak=QColor(199,170,88);
    p.setPen(Qt::NoPen);
    p.setBrush(cloak);
    p.drawRoundedRect(px+12,py+14,24,26,8,8);
    p.setBrush(QColor(236,207,169));
    p.drawEllipse(px+16,py+4,16,16);
    p.setPen(QPen(QColor(210,190,126),3));
    p.drawLine(px+34,py+14,px+40,py+34);
    p.drawLine(px+14,py+14,px+8,py+34);
}
void Level2View::drawHUD(QPainter &p)
{
    p.setFont(QFont("Georgia",20));
    const int heartY = 28;
    for (int i = 0; i < 3; ++i) {
        const int hx = width() - 44 - i*34;
        if (i < m_health) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(200,40,40));
            p.drawEllipse(hx,    heartY-10, 14, 14);
            p.drawEllipse(hx+10, heartY-10, 14, 14);
            QPoint hpts[3]={{hx,heartY+4},{hx+24,heartY+4},{hx+12,heartY+18}};
            p.drawPolygon(hpts,3);
        } else {
            p.setPen(QPen(QColor(120,40,40),2));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(hx,    heartY-10, 14, 14);
            p.drawEllipse(hx+10, heartY-10, 14, 14);
            QPoint hpts[3]={{hx,heartY+4},{hx+24,heartY+4},{hx+12,heartY+18}};
            p.drawPolygon(hpts,3);
        }
    }
}
void Level2View::drawWitchPanel(QPainter &p)
{
    if (!m_witch) return;
    const WitchScene::Phase ph = m_witch->phase();
    QLinearGradient panelBg(0,460,0,600);
    panelBg.setColorAt(0.0,QColor(20,10,30,230));
    panelBg.setColorAt(1.0,QColor(10,5,15,240));
    p.fillRect(QRect(0,460,width(),140),panelBg);
    p.setPen(QPen(QColor(100,60,140),1));
    p.drawLine(0,461,width(),461);
    p.setFont(QFont("Georgia",10,QFont::Bold));
    p.setPen(QColor(190,130,230));
    p.drawText(16,480,"The Witch:");
    p.setFont(QFont("Georgia",11,QFont::StyleItalic));
    p.setPen(QColor(232,210,255));
    p.drawText(QRect(16,484,560,60),Qt::TextWordWrap,m_witch->witchLine());
    if (ph==WitchScene::PHASE_RIDDLE || ph==WitchScene::PHASE_WRONG) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(60,38,18,220));
        p.drawRoundedRect(10,100,width()-20,340,12,12);
        p.setPen(QPen(QColor(160,110,60),1));
        p.drawRoundedRect(10,100,width()-20,340,12,12);
        p.setFont(QFont("Georgia",13,QFont::Bold));
        p.setPen(QColor(230,190,100));
        p.drawText(QRect(30,112,width()-60,30),Qt::AlignHCenter,"~ The Witch's Riddle ~");
        p.setFont(QFont("Georgia",12));
        p.setPen(QColor(242,224,178));
        p.drawText(QRect(40,148,width()-80,200),Qt::AlignHCenter|Qt::TextWordWrap,
                   m_witch->riddleText());
        QString hint = m_hasTreasureHint
            ? "★ Treasure Hint: " + m_witch->hintText()
            : m_witch->hintText();
        p.setFont(QFont("Georgia",10,QFont::StyleItalic));
        p.setPen(m_hasTreasureHint ? QColor(220,180,60) : QColor(160,130,90));
        p.drawText(QRect(40,350,width()-80,30),Qt::AlignHCenter,hint);
        if (m_hasBackupHint && m_witch->wrongAttempts() >= 2) {
            p.setFont(QFont("Georgia",10,QFont::Bold));
            p.setPen(QColor(100,220,100));
            p.drawText(QRect(40,374,width()-80,24),Qt::AlignHCenter,
                       "★★ Backup Hint: Think about what "+m_role+"s are known for in combat.");
        }
        if (ph==WitchScene::PHASE_WRONG) {
            p.setFont(QFont("Georgia",10,QFont::Bold));
            p.setPen(QColor(220,80,60));
            p.drawText(QRect(40,392,width()-80,24),Qt::AlignHCenter,
                       QString("Wrong! (%1 attempt%2) — Hearts remaining: %3")
                       .arg(m_witch->wrongAttempts())
                       .arg(m_witch->wrongAttempts()==1?"":"s")
                       .arg(m_health));
        }
    }
}
void Level2View::drawEscapeScreen(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());
    bg.setColorAt(0.0,QColor(8,18,8)); bg.setColorAt(0.5,QColor(20,42,20)); bg.setColorAt(1.0,QColor(8,18,8));
    p.fillRect(rect(),bg);
    QRadialGradient glow(width()/2,height()/2,280);
    glow.setColorAt(0.0,QColor(60,180,80,90)); glow.setColorAt(1.0,QColor(0,0,0,0));
    p.fillRect(rect(),glow);
    p.setPen(QPen(QColor(80,160,80,160),1));
    p.drawLine(80,height()/2-80,width()-80,height()/2-80);
    p.drawLine(80,height()/2+80,width()-80,height()/2+80);
    p.setPen(QColor(180,230,180));
    p.setFont(QFont("Georgia",13,QFont::Bold));
    p.drawText(rect().adjusted(0,-100,0,0),Qt::AlignCenter,"— ESCAPED —");
    p.setPen(QColor(220,255,220));
    p.setFont(QFont("Georgia",28,QFont::Bold));
    p.drawText(rect(),Qt::AlignCenter,"You outwitted the Witch!");
    p.setPen(QColor(160,210,160,200));
    p.setFont(QFont("Georgia",12,QFont::StyleItalic));
    p.drawText(rect().adjusted(0,100,0,0),Qt::AlignCenter,
               "The cursed door swings open...\nYour journey continues.");
}
void Level2View::drawGameOverScreen(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());
    bg.setColorAt(0.0,QColor(18,5,5)); bg.setColorAt(0.5,QColor(40,10,10)); bg.setColorAt(1.0,QColor(18,5,5));
    p.fillRect(rect(),bg);
    QRadialGradient glow(width()/2,height()/2,280);
    glow.setColorAt(0.0,QColor(180,30,30,90)); glow.setColorAt(1.0,QColor(0,0,0,0));
    p.fillRect(rect(),glow);
    p.setPen(QPen(QColor(160,40,40,160),1));
    p.drawLine(80,height()/2-80,width()-80,height()/2-80);
    p.drawLine(80,height()/2+80,width()-80,height()/2+80);
    p.setPen(QColor(230,150,150));
    p.setFont(QFont("Georgia",13,QFont::Bold));
    p.drawText(rect().adjusted(0,-100,0,0),Qt::AlignCenter,"— CURSED —");
    p.setPen(QColor(255,200,200));
    p.setFont(QFont("Georgia",28,QFont::Bold));
    p.drawText(rect(),Qt::AlignCenter,"The Witch claimed your soul!");
    p.setPen(QColor(200,150,150,200));
    p.setFont(QFont("Georgia",12,QFont::StyleItalic));
    p.drawText(rect().adjusted(0,90,0,0),Qt::AlignCenter,
               "Your hearts ran out...\nPress R to try Level 2 again.");
}
void Level2View::keyPressEvent(QKeyEvent *event)
{
    if (m_screen == SPLASH) {
        m_splashTimer->stop();
        onSplashTimer();
        return;
    }
    if (m_screen == GAMEOVER_SCREEN && event->key() == Qt::Key_R) {
        startLevel(m_playerName, m_role);
        return;
    }
    if (m_screen == CORRIDOR) {
        int dx=0, dy=0;
        if (event->key()==Qt::Key_Up)    dy=-1;
        if (event->key()==Qt::Key_Down)  dy= 1;
        if (event->key()==Qt::Key_Left)  dx=-1;
        if (event->key()==Qt::Key_Right) dx= 1;
        if (dx==0 && dy==0) return;
        const int nx = m_playerGX + dx;
        const int ny = m_playerGY + dy;
        if (nx<0||nx>=GRID_W||ny<0||ny>=GRID_H) return;
        L2Tile &target = m_grid[nx][ny];
        if (target.type == L2Tile::WALL) return;
        if (target.type == L2Tile::TRAP && !target.triggered) {
            target.triggered = true;
            m_health--;
            m_flashMsg    = "A trap! You lose a heart!";
            m_flashFrames = 90;
            m_playerGX = nx;
            m_playerGY = ny;
            if (m_health <= 0) {
                m_screen = GAMEOVER_SCREEN;
            }
            update();
            return;
        }
        if (target.type == L2Tile::TREASURE && !target.collected) {
            target.collected  = true;
            if (!m_hasTreasureHint) {
                m_hasTreasureHint = true;
                m_flashMsg    = "Chest 1: You found a clue for the witch's riddle!";
            } else {
                m_hasBackupHint = true;
                m_flashMsg    = "Chest 2: A backup hint — use it if you're stuck!";
            }
            m_flashFrames = 120;
            m_playerGX = target.collected ? m_playerGX : m_playerGX; 
            m_playerGX = m_playerGX + (int)(target.type == L2Tile::TREASURE) * 0; 
            m_playerGX = nx; m_playerGY = ny;
            update(); return;
        }
        if (target.type == L2Tile::DOOR) {
            m_screen = WITCH_ROOM_SCREEN;
            if (m_witch->phase() == WitchScene::PHASE_ENTER)
                m_witch->advanceDialogue();
            if (m_witch->phase() == WitchScene::PHASE_TAUNT) {
                m_riddleBtn->show();
                positionOverlayWidgets();
            }
            update();
            return;
        }
        m_playerGX = nx;
        m_playerGY = ny;
        update();
        return;
    }
    if (m_screen == WITCH_ROOM_SCREEN && m_witch) {
        if (event->key()==Qt::Key_Space || event->key()==Qt::Key_Return) {
            if (m_witch->phase()==WitchScene::PHASE_ENTER) {
                m_witch->advanceDialogue();
                if (m_witch->phase()==WitchScene::PHASE_TAUNT) {
                    m_riddleBtn->show();
                    positionOverlayWidgets();
                }
                update();
            }
        }
    }
}
