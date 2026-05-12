#include "level2view.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QTimer>
#include <QLinearGradient>
#include <QRadialGradient>
#include <cmath>

// ============================================================
//  Construction
// ============================================================
Level2View::Level2View(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(800, 600);

    // --- Riddle button (shown during TAUNT phase)
    m_riddleBtn = new QPushButton("Ask for the Riddle", this);
    m_riddleBtn->setFont(QFont("Georgia", 12, QFont::Bold));
    m_riddleBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #3b1f0e;"
        "  color: #f2d98a;"
        "  border: 2px solid #8b5e3c;"
        "  border-radius: 8px;"
        "  padding: 8px 20px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #5a3118;"
        "  border-color: #c8973a;"
        "}");
    m_riddleBtn->hide();
    connect(m_riddleBtn, &QPushButton::clicked, this, &Level2View::onRiddleButton);

    // --- Answer input
    m_answerEdit = new QLineEdit(this);
    m_answerEdit->setPlaceholderText("Type your answer here...");
    m_answerEdit->setFont(QFont("Georgia", 12));
    m_answerEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #1e120a;"
        "  color: #f2e6c8;"
        "  border: 2px solid #7a5230;"
        "  border-radius: 6px;"
        "  padding: 6px 12px;"
        "}");
    m_answerEdit->hide();
    connect(m_answerEdit, &QLineEdit::returnPressed, this, &Level2View::onSubmitAnswer);

    // --- Submit button
    m_submitBtn = new QPushButton("Submit", this);
    m_submitBtn->setFont(QFont("Georgia", 12, QFont::Bold));
    m_submitBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #5c2d0b;"
        "  color: #f2d98a;"
        "  border: 2px solid #9e6b3a;"
        "  border-radius: 8px;"
        "  padding: 8px 18px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #7a3d10;"
        "  border-color: #d4a44c;"
        "}");
    m_submitBtn->hide();
    connect(m_submitBtn, &QPushButton::clicked, this, &Level2View::onSubmitAnswer);

    // --- Splash timer (auto-advance after 2.8 seconds)
    m_splashTimer = new QTimer(this);
    m_splashTimer->setSingleShot(true);
    connect(m_splashTimer, &QTimer::timeout, this, &Level2View::onSplashTimer);
}

// ============================================================
//  Public entry point
// ============================================================
void Level2View::startLevel(const QString &playerName, const QString &role)
{
    m_playerName = playerName.isEmpty() ? "Adventurer" : playerName;
    m_role       = role;

    delete m_witch;
    m_witch    = new WitchScene(role);
    m_screen   = SPLASH;
    m_playerX  = 0;
    m_walkedIn = false;

    m_riddleBtn->hide();
    m_answerEdit->hide();
    m_submitBtn->hide();

    m_splashTimer->start(2800);   // show splash for 2.8 s
    setFocus();
    update();
}

// ============================================================
//  Slots
// ============================================================
void Level2View::onSplashTimer()
{
    m_screen = SCENE;
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

    if (m_witch->escaped())
    {
        m_answerEdit->hide();
        m_submitBtn->hide();
        m_screen = ESCAPED_SCREEN;
        update();

        // Emit after a brief pause so player can read the victory screen
        QTimer::singleShot(2600, this, &Level2View::levelComplete);
    }
    else
    {
        // Wrong – keep answer widgets visible, redraw witch reaction
        positionOverlayWidgets();
        update();
    }
}

// ============================================================
//  Widget positioning helpers
// ============================================================
void Level2View::positionOverlayWidgets()
{
    const WitchScene::Phase ph = m_witch ? m_witch->phase() : WitchScene::PHASE_ENTER;

    if (ph == WitchScene::PHASE_TAUNT)
    {
        m_riddleBtn->move(width() / 2 - m_riddleBtn->sizeHint().width() / 2, 520);
    }
    else if (ph == WitchScene::PHASE_RIDDLE || ph == WitchScene::PHASE_WRONG)
    {
        const int cx = width() / 2;
        m_answerEdit->setFixedWidth(300);
        m_answerEdit->move(cx - 170, 525);
        m_submitBtn->move(cx + 146, 521);
    }
}

// ============================================================
//  paintEvent dispatcher
// ============================================================
void Level2View::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    switch (m_screen)
    {
    case SPLASH:          drawSplash(p);            break;
    case SCENE:           drawCottageInterior(p);   break;
    case ESCAPED_SCREEN:  drawEscapeScreen(p);      break;
    }
}

// ============================================================
//  SPLASH SCREEN
// ============================================================
void Level2View::drawSplash(QPainter &p)
{
    // Dark parchment-to-black gradient
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(12, 6, 20));
    bg.setColorAt(0.5, QColor(28, 14, 38));
    bg.setColorAt(1.0, QColor(10, 5, 15));
    p.fillRect(rect(), bg);

    // Subtle vignette glow in center
    QRadialGradient vignette(width() / 2, height() / 2, 340);
    vignette.setColorAt(0.0, QColor(90, 40, 120, 80));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(rect(), vignette);

    // Decorative horizontal rule
    p.setPen(QPen(QColor(140, 80, 180, 160), 1));
    p.drawLine(80, height() / 2 - 70, width() - 80, height() / 2 - 70);
    p.drawLine(80, height() / 2 + 70, width() - 80, height() / 2 + 70);

    // Small diamond ornaments on the rule
    auto drawDiamond = [&](int x, int y) {
        QPoint pts[4] = { {x, y-6}, {x+6, y}, {x, y+6}, {x-6, y} };
        p.setBrush(QColor(160, 100, 200));
        p.setPen(Qt::NoPen);
        p.drawPolygon(pts, 4);
    };
    drawDiamond(80,  height()/2 - 70);
    drawDiamond(width()-80, height()/2 - 70);
    drawDiamond(80,  height()/2 + 70);
    drawDiamond(width()-80, height()/2 + 70);

    // "LEVEL 2" label
    p.setPen(QColor(170, 120, 210));
    p.setFont(QFont("Georgia", 14, QFont::Bold));
    p.drawText(rect().adjusted(0, -90, 0, 0), Qt::AlignCenter, "— LEVEL 2 —");

    // Main title
    p.setPen(QColor(242, 220, 168));
    p.setFont(QFont("Georgia", 30, QFont::Bold));
    p.drawText(rect(), Qt::AlignCenter, "The Witch's Cottage");

    // Flavour subtitle
    p.setPen(QColor(180, 150, 210, 200));
    p.setFont(QFont("Georgia", 13, QFont::StyleItalic));
    p.drawText(rect().adjusted(0, 90, 0, 0), Qt::AlignCenter,
               "Something stirs within the old stone walls...");
}

// ============================================================
//  COTTAGE INTERIOR SCENE
// ============================================================
void Level2View::drawCottageInterior(QPainter &p)
{
    if (!m_witch) return;

    // ---- background: warm firelit room ----
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(18, 10, 6));
    bg.setColorAt(0.4, QColor(42, 22, 12));
    bg.setColorAt(1.0, QColor(30, 14, 6));
    p.fillRect(rect(), bg);

    // Stone floor
    p.setPen(Qt::NoPen);
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 10; ++col)
        {
            const int fx = col * 82 + (row % 2) * 41;
            const int fy = 430 + row * 34;
            const QColor stoneCol = (col + row) % 2 == 0
                ? QColor(62, 50, 40) : QColor(55, 44, 34);
            p.setBrush(stoneCol);
            p.drawRoundedRect(fx, fy, 80, 32, 4, 4);
        }
    }

    // Back wall (stone texture)
    p.setBrush(QColor(38, 28, 20));
    p.drawRect(0, 0, width(), 430);
    // Wall stones
    p.setPen(QPen(QColor(28, 20, 14), 1));
    for (int row = 0; row < 9; ++row)
    {
        for (int col = 0; col < 12; ++col)
        {
            const int wx = col * 70 + (row % 2) * 35 - 10;
            const int wy = 60 + row * 42;
            if (wy > 430) break;
            p.setBrush(QColor(48 + (col*3)%12, 35 + (row*2)%8, 22));
            p.drawRoundedRect(wx, wy, 68, 38, 3, 3);
        }
    }

    // Fireplace (right side)
    {
        p.setPen(Qt::NoPen);
        // Mantle
        p.setBrush(QColor(60, 42, 28));
        p.drawRect(580, 220, 180, 210);
        // Fire opening
        p.setBrush(QColor(15, 8, 4));
        p.drawRoundedRect(608, 280, 124, 150, 10, 10);
        // Flames
        auto drawFlame = [&](int fx, int fy, int w, int h, QColor col) {
            p.setBrush(col);
            p.drawEllipse(fx, fy, w, h);
        };
        drawFlame(618, 310, 40, 90, QColor(200, 80, 10, 220));
        drawFlame(638, 295, 50, 110, QColor(230, 130, 20, 200));
        drawFlame(668, 318, 38, 85, QColor(210, 90, 15, 210));
        drawFlame(645, 285, 30, 70, QColor(255, 200, 60, 180));

        // Fire glow on walls
        QRadialGradient fireGlow(700, 360, 200);
        fireGlow.setColorAt(0.0, QColor(220, 110, 20, 70));
        fireGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
        p.fillRect(rect(), fireGlow);

        // Cauldron above fire
        p.setBrush(QColor(28, 28, 28));
        p.setPen(QPen(QColor(80, 80, 80), 2));
        p.drawEllipse(628, 268, 84, 44);
        p.setBrush(QColor(10, 48, 30));
        p.setPen(Qt::NoPen);
        p.drawEllipse(636, 272, 68, 28); // liquid
        // Bubbles
        p.setBrush(QColor(20, 180, 100, 160));
        p.drawEllipse(648, 270, 8, 8);
        p.drawEllipse(668, 266, 6, 6);
        p.drawEllipse(685, 272, 7, 7);
    }

    // Shelves with potion bottles (left wall)
    {
        p.setPen(Qt::NoPen);
        for (int shelf = 0; shelf < 2; ++shelf)
        {
            const int sy = 130 + shelf * 110;
            p.setBrush(QColor(72, 50, 32));
            p.drawRoundedRect(20, sy, 160, 12, 3, 3);

            // Bottles
            const QColor bottleColors[] = {
                QColor(100, 20, 140), QColor(20, 100, 60),
                QColor(160, 80, 10), QColor(40, 80, 160)
            };
            for (int b = 0; b < 4; ++b)
            {
                const int bx = 28 + b * 38;
                const int by = sy - 46;
                p.setBrush(bottleColors[b]);
                p.drawRoundedRect(bx, by + 14, 18, 32, 5, 5); // body
                p.setBrush(QColor(80, 65, 52));
                p.drawRoundedRect(bx + 5, by, 8, 16, 3, 3);   // neck
            }
        }
    }

    // Witch NPC (center-left area)
    drawWitch(p, 300, 220);

    // Player (enters from left, walks toward center)
    if (!m_walkedIn)
    {
        m_playerX += 4;
        if (m_playerX >= 100)
        {
            m_playerX  = 100;
            m_walkedIn = true;
            m_witch->advanceDialogue();

            // Show riddle button once taunt phase starts
            if (m_witch->phase() == WitchScene::PHASE_TAUNT)
            {
                m_riddleBtn->show();
                positionOverlayWidgets();
            }
        }
        QTimer::singleShot(30, this, [this]{ update(); });
    }
    drawPlayer(p, m_playerX, 350);

    // ---- dialogue / riddle panel ----
    drawWitchPanel(p);

    // ---- HUD header ----
    p.setPen(QColor(242, 220, 168));
    p.setFont(QFont("Georgia", 17, QFont::Bold));
    p.drawText(24, 34, "Level 2 – The Witch's Cottage");

    p.setFont(QFont("Georgia", 10, QFont::StyleItalic));
    p.setPen(QColor(180, 155, 120));
    const QString roleTag = "[" + m_role + "]  " + m_playerName;
    p.drawText(24, 54, roleTag);
}

// ============================================================
//  Witch sprite
// ============================================================
void Level2View::drawWitch(QPainter &p, int cx, int cy)
{
    p.setPen(Qt::NoPen);

    // Cloak / robe (dark purple)
    p.setBrush(QColor(48, 18, 68));
    QPoint robe[5] = {
        {cx - 36, cy + 130},
        {cx + 36, cy + 130},
        {cx + 28, cy + 50},
        {cx,      cy + 40},
        {cx - 28, cy + 50}
    };
    p.drawPolygon(robe, 5);

    // Body
    p.setBrush(QColor(55, 22, 78));
    p.drawRoundedRect(cx - 22, cy + 40, 44, 70, 10, 10);

    // Head
    p.setBrush(QColor(176, 148, 110));
    p.drawEllipse(cx - 18, cy, 36, 40);

    // Hat brim
    p.setBrush(QColor(22, 14, 32));
    p.drawEllipse(cx - 28, cy + 2, 56, 14);
    // Hat cone
    QPoint hat[3] = {
        {cx - 22, cy + 8},
        {cx + 22, cy + 8},
        {cx + 4,  cy - 52}
    };
    p.setBrush(QColor(28, 16, 40));
    p.drawPolygon(hat, 3);
    // Hat band
    p.setBrush(QColor(120, 50, 160));
    p.drawRect(cx - 20, cy + 2, 40, 8);

    // Eyes (glowing green)
    p.setBrush(QColor(60, 220, 80));
    p.drawEllipse(cx - 10, cy + 14, 8, 8);
    p.drawEllipse(cx + 2,  cy + 14, 8, 8);
    // Pupils
    p.setBrush(QColor(10, 60, 16));
    p.drawEllipse(cx - 8, cy + 16, 4, 4);
    p.drawEllipse(cx + 4, cy + 16, 4, 4);

    // Nose (crooked)
    p.setPen(QPen(QColor(120, 90, 60), 2));
    p.drawLine(cx, cy + 22, cx - 4, cy + 30);
    p.drawLine(cx - 4, cy + 30, cx + 2, cy + 32);

    // Staff
    p.setPen(QPen(QColor(80, 55, 30), 3));
    p.drawLine(cx + 26, cy + 110, cx + 42, cy - 40);
    // Staff orb
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(140, 60, 200, 200));
    p.drawEllipse(cx + 36, cy - 52, 18, 18);
    // Glow
    QRadialGradient orbGlow(cx + 45, cy - 43, 20);
    orbGlow.setColorAt(0.0, QColor(180, 100, 255, 120));
    orbGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(cx + 24, cy - 64, 44, 44, orbGlow);
}

// ============================================================
//  Player sprite (replicates Level 1 style)
// ============================================================
void Level2View::drawPlayer(QPainter &p, int px, int py)
{
    QColor cloakColor(76, 124, 215);
    if      (m_role == "Wizard")  cloakColor = QColor(85,  102, 224);
    else if (m_role == "Fighter") cloakColor = QColor(171,  72,  65);
    else if (m_role == "Rogue")   cloakColor = QColor(65,  138,  94);
    else if (m_role == "Cleric")  cloakColor = QColor(199, 170,  88);

    p.setPen(Qt::NoPen);
    p.setBrush(cloakColor);
    p.drawRoundedRect(px + 12, py + 14, 24, 26, 8, 8);
    p.setBrush(QColor(236, 207, 169));
    p.drawEllipse(px + 16, py + 4, 16, 16);
    p.setPen(QPen(QColor(210, 190, 126), 3));
    p.drawLine(px + 34, py + 14, px + 40, py + 34);
    p.drawLine(px + 14, py + 14, px + 8,  py + 34);
}

// ============================================================
//  Dialogue / riddle panel at the bottom
// ============================================================
void Level2View::drawWitchPanel(QPainter &p)
{
    if (!m_witch) return;

    const WitchScene::Phase ph = m_witch->phase();

    // Panel background
    p.setPen(Qt::NoPen);
    QLinearGradient panelBg(0, 460, 0, 600);
    panelBg.setColorAt(0.0, QColor(20, 10, 30, 230));
    panelBg.setColorAt(1.0, QColor(10,  5, 15, 240));
    p.fillRect(QRect(0, 460, width(), 140), panelBg);
    p.setPen(QPen(QColor(100, 60, 140), 1));
    p.drawLine(0, 461, width(), 461);

    // Witch name tag
    p.setFont(QFont("Georgia", 10, QFont::Bold));
    p.setPen(QColor(190, 130, 230));
    p.drawText(16, 480, "The Witch:");

    // Witch dialogue
    p.setFont(QFont("Georgia", 11, QFont::StyleItalic));
    p.setPen(QColor(232, 210, 255));
    p.drawText(QRect(16, 484, 560, 60), Qt::TextWordWrap, m_witch->witchLine());

    // Riddle text
    if (ph == WitchScene::PHASE_RIDDLE || ph == WitchScene::PHASE_WRONG)
    {
        // Parchment riddle card
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(60, 38, 18, 220));
        p.drawRoundedRect(10, 100, width() - 20, 340, 12, 12);
        p.setPen(QPen(QColor(160, 110, 60), 1));
        p.drawRoundedRect(10, 100, width() - 20, 340, 12, 12);

        p.setFont(QFont("Georgia", 13, QFont::Bold));
        p.setPen(QColor(230, 190, 100));
        p.drawText(QRect(30, 112, width() - 60, 30), Qt::AlignHCenter, "~ The Witch's Riddle ~");

        p.setFont(QFont("Georgia", 12));
        p.setPen(QColor(242, 224, 178));
        p.drawText(QRect(40, 148, width() - 80, 200),
                   Qt::AlignHCenter | Qt::TextWordWrap,
                   m_witch->riddleText());

        p.setFont(QFont("Georgia", 10, QFont::StyleItalic));
        p.setPen(QColor(160, 130, 90));
        p.drawText(QRect(40, 350, width() - 80, 30),
                   Qt::AlignHCenter, m_witch->hintText());

        if (ph == WitchScene::PHASE_WRONG)
        {
            p.setFont(QFont("Georgia", 10, QFont::Bold));
            p.setPen(QColor(220, 80, 60));
            const QString wrongMsg = QString("Wrong! (%1 attempt%2) Try again...")
                .arg(m_witch->wrongAttempts())
                .arg(m_witch->wrongAttempts() == 1 ? "" : "s");
            p.drawText(QRect(40, 392, width()-80, 24), Qt::AlignHCenter, wrongMsg);
        }
    }
}

// ============================================================
//  ESCAPE / VICTORY SCREEN
// ============================================================
void Level2View::drawEscapeScreen(QPainter &p)
{
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(8, 18, 8));
    bg.setColorAt(0.5, QColor(20, 42, 20));
    bg.setColorAt(1.0, QColor(8, 18, 8));
    p.fillRect(rect(), bg);

    QRadialGradient glow(width()/2, height()/2, 280);
    glow.setColorAt(0.0, QColor(60, 180, 80, 90));
    glow.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(rect(), glow);

    p.setPen(QPen(QColor(80, 160, 80, 160), 1));
    p.drawLine(80, height()/2 - 80, width()-80, height()/2 - 80);
    p.drawLine(80, height()/2 + 80, width()-80, height()/2 + 80);

    p.setPen(QColor(180, 230, 180));
    p.setFont(QFont("Georgia", 13, QFont::Bold));
    p.drawText(rect().adjusted(0, -100, 0, 0), Qt::AlignCenter, "— ESCAPED —");

    p.setPen(QColor(220, 255, 220));
    p.setFont(QFont("Georgia", 28, QFont::Bold));
    p.drawText(rect(), Qt::AlignCenter, "You outwitted the Witch!");

    p.setPen(QColor(160, 210, 160, 200));
    p.setFont(QFont("Georgia", 12, QFont::StyleItalic));
    p.drawText(rect().adjusted(0, 100, 0, 0), Qt::AlignCenter,
               "The cursed door swings open...\nYour journey continues.");
}

// ============================================================
//  Key press (dismiss splash manually, or advance dialogue)
// ============================================================
void Level2View::keyPressEvent(QKeyEvent *event)
{
    if (m_screen == SPLASH)
    {
        m_splashTimer->stop();
        onSplashTimer();
        return;
    }

    // Space / Enter advance dialogue
    if (m_screen == SCENE && m_witch)
    {
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return)
        {
            if (m_witch->phase() == WitchScene::PHASE_ENTER)
            {
                m_witch->advanceDialogue();
                if (m_witch->phase() == WitchScene::PHASE_TAUNT)
                {
                    m_riddleBtn->show();
                    positionOverlayWidgets();
                }
                update();
            }
        }
    }
}
