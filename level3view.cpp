#include "level3view.h"
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFont>
#include <QTimer>
#include <cmath>
#include <cstdlib>
Level3View::Level3View(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(800, 600);
    m_splashTimer = new QTimer(this);
    m_splashTimer->setSingleShot(true);
    connect(m_splashTimer, &QTimer::timeout, this, &Level3View::onSplashTimer);
    m_gameTick = new QTimer(this);
    m_gameTick->setInterval(20); 
    connect(m_gameTick, &QTimer::timeout, this, &Level3View::onGameTick);
}
void Level3View::startLevel(const QString &name, const QString &role)
{
    m_playerName=name.isEmpty()?"Adventurer":name;
    m_role=role;
    m_health=2; m_strikes=0;
    m_flashMsg=""; m_flashFrames=0;
    m_screen=SPLASH;
    m_px=60; m_py=240;
    for(int i=0;i<4;i++) m_keys[i]=false;
    m_shots.clear();
    m_arenaComplete=false; m_arenaCompleteTimer=0;
    m_gateOpen=false; m_sequenceProgress=0;
    m_gasTickCounter=0; m_gasFloodTimer=0; m_sequenceTimeout=0;
    spawnGargoyles();
    buildCorridor();
    m_splashTimer->start(3000);
    m_gameTick->start();
    setFocus(); update();
}
QString Level3View::leverSymbol(int i) const
{
    switch(i){ case 0:return "STAR"; case 1:return "MOON"; case 2:return "BOLT"; case 3:return "CROSS"; }
    return "";
}
QString Level3View::leverGlyph(int i) const
{
    switch(i){ case 0:return "★"; case 1:return "☽"; case 2:return "⚡"; case 3:return "✝"; }
    return "?";
}
QVector<int> Level3View::correctSequence() const
{
    if(m_role=="Wizard")  return {1,3,0,2}; 
    if(m_role=="Fighter") return {2,0,3,1}; 
    if(m_role=="Rogue")   return {3,2,1,0}; 
    return                       {0,1,2,3}; 
}
QString Level3View::poemTitle() const
{
    if(m_role=="Wizard")  return "~ The Arcane Verse ~";
    if(m_role=="Fighter") return "~ The Warrior's Oath ~";
    if(m_role=="Rogue")   return "~ The Thief's Creed ~";
    return                       "~ The Sacred Hymn ~";
}
QStringList Level3View::poem() const
{
    if(m_role=="Wizard") return {
        "I. What the tides obey and lovers name,\n"
        "   that hangs above the sleeping world in silver frame.",
        "II. Where two roads meet and martyrs kneel,\n"
        "    the sign that heals what swords reveal.",
        "III. I fell from heaven long ago,\n"
        "     five points of light in the dark below.",
        "IV. Zeus cast me down from cloud to stone,\n"
        "    I split the oak and crack the bone."
    };
    if(m_role=="Fighter") return {
        "I. The storm's own spear, the sky's bright scar,\n"
        "   it strikes before the thunder from afar.",
        "II. The navigator's guide through endless night,\n"
        "    five-pointed guardian of the sailor's right.",
        "III. Carved on every grave and chapel door,\n"
        "     the mark of faith from ancient lore.",
        "IV. She waxes full then hides her face,\n"
        "    commands the sea and marks time's pace."
    };
    if(m_role=="Rogue") return {
        "I. Two lines that intersect and bind,\n"
        "   on every church and grave you'll find.",
        "II. No thief can outrun what heaven throws,\n"
        "    the blinding flash before the thunder grows.",
        "III. She lights the Rogue's path through the night,\n"
        "     a crescent blade of stolen light.",
        "IV. Last of all — what pirates draw\n"
        "    on treasure maps to mark what's raw."
    };
    return {
        "I. What the Magi followed east and far,\n"
        "   the birth announced by heavenly star.",
        "II. She governs tides and women's ways,\n"
        "    the pale lantern of the midnight haze.",
        "III. The wrath of God made visible and bright,\n"
        "     a jagged sword of purifying light.",
        "IV. The last and holiest shape of all,\n"
        "    the mark of sacrifice upon the hall."
    };
}
void Level3View::buildCorridor()
{
    for(int x=0;x<GRID_W;++x)
        for(int y=0;y<GRID_H;++y)
            m_grid[x][y]={L3Tile::FLOOR,false,false,0,-1};
    for(int x=0;x<GRID_W;++x){ m_grid[x][0].type=L3Tile::WALL; m_grid[x][6].type=L3Tile::WALL; }
    for(int y=0;y<GRID_H;++y){ m_grid[0][y].type=L3Tile::WALL; m_grid[13][y].type=L3Tile::WALL; }
    m_grid[12][3].type=L3Tile::GATE;
    m_grid[4][1].type=L3Tile::SPIKE;
    m_grid[8][1].type=L3Tile::SPIKE;
    m_grid[3][3].type=L3Tile::SPIKE;
    m_grid[9][5].type=L3Tile::SPIKE;
    m_grid[5][5].type=L3Tile::SPIKE;
    m_grid[3][5].type=L3Tile::SPIKE;
    m_grid[10][3].type=L3Tile::SPIKE;
    m_grid[1][1].type=L3Tile::LEVER; m_grid[1][1].leverIndex=0;
    m_grid[1][5].type=L3Tile::LEVER; m_grid[1][5].leverIndex=1;
    m_grid[11][5].type=L3Tile::LEVER; m_grid[11][5].leverIndex=2;
    m_grid[11][1].type=L3Tile::LEVER; m_grid[11][1].leverIndex=3;
    m_playerGX=1; m_playerGY=3;
    m_gateOpen=false; m_sequenceProgress=0;
}
void Level3View::spawnGargoyles()
{
    m_gargoyles.clear(); m_shots.clear();
    m_arenaComplete=false;
    for(int i=0;i<3;i++){
        Gargoyle g;
        g.x=580+i*60; g.y=100+i*130; 
        g.hp=3;        
        g.alive=true;
        g.speed=2.2f+i*0.6f; 
        m_gargoyles.append(g);
    }
}
void Level3View::floodGas()
{
    m_gasFloodTimer=180; 
    for(int x=1;x<GRID_W-1;++x)
        for(int y=1;y<GRID_H-1;++y)
            if(m_grid[x][y].type==L3Tile::FLOOR||m_grid[x][y].type==L3Tile::GAS){
                m_grid[x][y].type=L3Tile::GAS;
                m_grid[x][y].gasTimer=240;
            }
}
void Level3View::onSplashTimer(){ m_screen=WITCH_BRIEFING; update(); }
void Level3View::onGameTick()
{
    if(m_screen==GARGOYLE_ARENA) updateGargoyleArena();
    else if(m_screen==CORRIDOR)  updateCorridor();
    if(m_flashFrames>0) m_flashFrames--;
    update();
}
void Level3View::updateGargoyleArena()
{
    const float spd=3.0f;
    if(m_keys[0]&&m_py>60)  m_py-=spd;
    if(m_keys[1]&&m_py<430) m_py+=spd;
    if(m_keys[2]&&m_px>20)  m_px-=spd;
    if(m_keys[3]&&m_px<680) m_px+=spd;
    for(auto &g:m_gargoyles){
        if(!g.alive) continue;
        float dx=m_px-g.x,dy=m_py-g.y,dist=std::sqrt(dx*dx+dy*dy);
        if(dist>0){ g.x+=g.speed*dx/dist; g.y+=g.speed*dy/dist; }
        if(dist<30){
            g.alive=false; m_health--;
            m_flashMsg="A gargoyle reached you! Lose a heart!"; m_flashFrames=80;
            if(m_health<=0){ m_screen=GAMEOVER; return; }
        }
    }
    for(auto &s:m_shots){
        if(!s.active) continue;
        s.x+=s.dx; s.y+=s.dy;
        if(s.x>820||s.x<-20||s.y>620||s.y<-20){ s.active=false; continue; }
        for(auto &g:m_gargoyles){
            if(!g.alive) continue;
            float dx=s.x-g.x,dy=s.y-g.y;
            if(std::sqrt(dx*dx+dy*dy)<24){ s.active=false; g.hp--; if(g.hp<=0) g.alive=false; }
        }
    }
    m_shots.erase(std::remove_if(m_shots.begin(),m_shots.end(),[](const Projectile&s){return !s.active;}),m_shots.end());
    bool allDead=true;
    for(auto &g:m_gargoyles) if(g.alive) allDead=false;
    if(allDead&&!m_arenaComplete){
        m_arenaComplete=true; m_arenaCompleteTimer=100;
        m_flashMsg="Gargoyles defeated! Entering the castle..."; m_flashFrames=100;
    }
    if(m_arenaComplete&&m_arenaCompleteTimer>0){
        m_arenaCompleteTimer--;
        if(m_arenaCompleteTimer<=0){ m_screen=POEM_CLUE; update(); }
    }
}
void Level3View::updateCorridor()
{
    m_gasTickCounter++;
    if(m_sequenceProgress>0&&m_sequenceProgress<4&&!m_gateOpen){
        m_sequenceTimeout--;
        if(m_sequenceTimeout<=0){
            m_sequenceProgress=0;
            for(int x=0;x<GRID_W;++x)
                for(int y=0;y<GRID_H;++y)
                    if(m_grid[x][y].type==L3Tile::LEVER)
                        m_grid[x][y].locked_in=false;
            m_flashMsg="Too slow! The levers reset — start again!";
            m_flashFrames=120;
        }
    }
    if(m_gasTickCounter%120==0){
        L3Tile &pt=m_grid[m_playerGX][m_playerGY];
        if(pt.type==L3Tile::GAS){
            m_health--;
            m_flashMsg="Poison gas burning you! Lose a heart!"; m_flashFrames=80;
            if(m_health<=0){ m_screen=GAMEOVER; m_gameTick->stop(); return; }
        }
    }
    if(m_gasFloodTimer>0){
        m_gasFloodTimer--;
        if(m_gasFloodTimer==0){
            for(int x=0;x<GRID_W;++x)
                for(int y=0;y<GRID_H;++y)
                    if(m_grid[x][y].type==L3Tile::GAS)
                        m_grid[x][y].type=L3Tile::FLOOR;
        }
    }
    if(m_gasTickCounter>=3){
        m_gasTickCounter=0;
        for(int x=0;x<GRID_W;++x)
            for(int y=0;y<GRID_H;++y)
                if(m_grid[x][y].type==L3Tile::GAS&&m_grid[x][y].gasTimer>0){
                    m_grid[x][y].gasTimer--;
                    if(m_grid[x][y].gasTimer<=0) m_grid[x][y].type=L3Tile::FLOOR;
                }
    }
}
void Level3View::playerShoot()
{
    if(m_screen!=GARGOYLE_ARENA) return;
    Projectile s; s.x=m_px+20; s.y=m_py+10;
    float bestDist=99999,tdx=1,tdy=0;
    for(auto &g:m_gargoyles){
        if(!g.alive) continue;
        float dx=g.x-s.x,dy=g.y-s.y,dist=std::sqrt(dx*dx+dy*dy);
        if(dist<bestDist){ bestDist=dist; tdx=dx/dist; tdy=dy/dist; }
    }
    s.dx=tdx*9; s.dy=tdy*9; s.active=true;
    m_shots.append(s);
}
void Level3View::pullLever(int leverIndex)
{
    const QVector<int> seq=correctSequence();
    if(m_sequenceProgress>=4) return; 
    if(leverIndex==seq[m_sequenceProgress]){
        for(int x=0;x<GRID_W;++x)
            for(int y=0;y<GRID_H;++y)
                if(m_grid[x][y].type==L3Tile::LEVER&&m_grid[x][y].leverIndex==leverIndex)
                    m_grid[x][y].locked_in=true;
        m_sequenceProgress++;
        m_sequenceTimeout = 480; 
        if(m_sequenceProgress==4){
            m_gateOpen=true;
            m_flashMsg="★ The gate unlocks! Reach it to free your friend! ★";
            m_flashFrames=180;
        } else {
            m_flashMsg=QString("Correct! Step %1/4 — keep going...").arg(m_sequenceProgress);
            m_flashFrames=80;
        }
    } else {
        m_strikes++;
        m_flashMsg=QString("WRONG LEVER! Strike %1/3 — gas floods the room!").arg(m_strikes);
        m_flashFrames=120;
        floodGas();
        m_sequenceProgress=0;
        m_sequenceTimeout=0;
        for(int gx=0;gx<GRID_W;++gx)
            for(int gy=0;gy<GRID_H;++gy)
                if(m_grid[gx][gy].type==L3Tile::LEVER)
                    m_grid[gx][gy].locked_in=false;
        if(m_strikes>=3){ m_screen=GAMEOVER; m_gameTick->stop(); }
    }
}
void Level3View::paintEvent(QPaintEvent *)
{
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    switch(m_screen){
    case SPLASH:          drawSplash(p);        break;
    case WITCH_BRIEFING:  drawWitchBriefing(p); break;
    case GARGOYLE_ARENA:  drawGargoyleArena(p); break;
    case POEM_CLUE:       drawPoemClue(p);      break;
    case CORRIDOR:        drawCorridor(p);       break;
    case VICTORY:         drawVictoryScreen(p);  break;
    case GAMEOVER:        drawGameOverScreen(p); break;
    }
    if(m_flashFrames>0){
        p.setFont(QFont("Georgia",11,QFont::Bold));
        const QColor fc=m_flashMsg.startsWith("★")||m_flashMsg.startsWith("Correct")?
            QColor(100,230,100,qMin(255,m_flashFrames*5)):
            QColor(230,80,50,qMin(255,m_flashFrames*5));
        p.setPen(fc);
        p.drawText(QRect(0,74,width(),22),Qt::AlignCenter,m_flashMsg);
    }
}
void Level3View::drawSplash(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());bg.setColorAt(0,QColor(8,4,2));bg.setColorAt(.5,QColor(24,8,4));bg.setColorAt(1,QColor(8,4,2));
    p.fillRect(rect(),bg);
    QRadialGradient vg(width()/2,height()/2,360);vg.setColorAt(0,QColor(140,40,10,80));vg.setColorAt(1,QColor(0,0,0,0));
    p.fillRect(rect(),vg);
    p.setPen(QPen(QColor(180,70,20,160),1));
    p.drawLine(80,height()/2-75,width()-80,height()/2-75);
    p.drawLine(80,height()/2+75,width()-80,height()/2+75);
    auto dm=[&](int x,int y){QPoint d[4]={{x,y-7},{x+7,y},{x,y+7},{x-7,y}};p.setBrush(QColor(200,80,20));p.setPen(Qt::NoPen);p.drawPolygon(d,4);};
    dm(80,height()/2-75);dm(width()-80,height()/2-75);dm(80,height()/2+75);dm(width()-80,height()/2+75);
    p.setPen(QColor(210,110,40));p.setFont(QFont("Georgia",14,QFont::Bold));
    p.drawText(rect().adjusted(0,-95,0,0),Qt::AlignCenter,"— LEVEL 3 —");
    p.setPen(QColor(255,220,160));p.setFont(QFont("Georgia",30,QFont::Bold));
    p.drawText(rect(),Qt::AlignCenter,"The Dragon's Castle");
    p.setPen(QColor(200,150,100,200));p.setFont(QFont("Georgia",13,QFont::StyleItalic));
    p.drawText(rect().adjusted(0,95,0,0),Qt::AlignCenter,"Stone guardians watch the gates...\nA cursed lock seals your friend within.");
}
void Level3View::drawWitchBriefing(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());bg.setColorAt(0,QColor(12,6,20));bg.setColorAt(1,QColor(20,10,30));
    p.fillRect(rect(),bg);
    int cx=110,cy=300;
    p.setPen(Qt::NoPen);p.setBrush(QColor(48,18,68));
    QPoint rb[5]={{cx-22,cy+80},{cx+22,cy+80},{cx+16,cy+30},{cx,cy+24},{cx-16,cy+30}};p.drawPolygon(rb,5);
    p.setBrush(QColor(176,148,110));p.drawEllipse(cx-11,cy,22,24);
    p.setBrush(QColor(22,14,32));p.drawEllipse(cx-17,cy+2,34,9);
    QPoint ht[3]={{cx-13,cy+5},{cx+13,cy+5},{cx+2,cy-32}};p.setBrush(QColor(28,16,40));p.drawPolygon(ht,3);
    p.setBrush(QColor(60,220,80));p.drawEllipse(cx-6,cy+8,5,5);p.drawEllipse(cx+1,cy+8,5,5);
    p.setBrush(QColor(28,14,42,230));p.setPen(QPen(QColor(100,50,140),1));
    p.drawRoundedRect(155,160,610,260,12,12);
    QPoint ptr[3]={{155,268},{135,288},{155,308}};p.setBrush(QColor(28,14,42,230));p.setPen(Qt::NoPen);p.drawPolygon(ptr,3);
    p.setFont(QFont("Georgia",10,QFont::Bold));p.setPen(QColor(180,120,220));p.drawText(170,185,"The Witch says:");
    p.setFont(QFont("Georgia",11,QFont::StyleItalic));p.setPen(QColor(228,200,255));
    QString b="\"Your friend is sealed behind a cursed gate, "+m_playerName+".\n"
              "Four levers line the corridor walls — each marked\n"
              "with a symbol. They must be pulled in the right order\n"
              "or poison gas will flood the chamber.\n\n"
              "Three wrong pulls and the curse consumes you.\n"
              "The poem I give you holds the answer — but\n"
              "it will not give it to you plainly. Think carefully.\n\n"
              "The gargoyles outside must fall first.\n"
              "Move with arrows. SPACE to shoot. Go.\"";
    p.drawText(QRect(170,195,580,220),Qt::TextWordWrap,b);
    p.setFont(QFont("Georgia",11,QFont::Bold));p.setPen(QColor(220,180,80));
    p.drawText(QRect(0,450,width(),28),Qt::AlignCenter,"Press SPACE to begin...");
    p.setPen(QColor(242,220,168));p.setFont(QFont("Georgia",16,QFont::Bold));
    p.drawText(24,34,"Level 3 – The Dragon's Castle");
    drawHUD(p);
}
void Level3View::drawPoemClue(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());bg.setColorAt(0,QColor(8,6,4));bg.setColorAt(1,QColor(18,13,7));
    p.fillRect(rect(),bg);
    QRadialGradient vg(width()/2,height()/2,320);vg.setColorAt(0,QColor(50,38,18,90));vg.setColorAt(1,QColor(0,0,0,0));
    p.fillRect(rect(),vg);
    p.setBrush(QColor(48,36,18,245));p.setPen(QPen(QColor(130,100,50),1));
    p.drawRoundedRect(60,50,680,470,14,14);
    p.setPen(QPen(QColor(150,118,65),1));
    p.drawRoundedRect(72,62,656,446,10,10);
    p.setFont(QFont("Georgia",15,QFont::Bold));p.setPen(QColor(220,188,100));
    p.drawText(QRect(60,68,680,36),Qt::AlignCenter,poemTitle());
    p.setPen(QPen(QColor(130,100,50),1));p.drawLine(100,106,700,106);
    p.setFont(QFont("Georgia",11,QFont::Bold));p.setPen(QColor(200,170,90));
    p.drawText(QRect(60,112,680,22),Qt::AlignCenter,"The four levers bear these marks:");
    p.setFont(QFont("Georgia",20));
    const int lx[]={130,260,430,560};
    const QString lname[]={"STAR","MOON","BOLT","CROSS"};
    for(int i=0;i<4;i++){
        p.setPen(QColor(240,210,120));
        p.drawText(lx[i],148,leverGlyph(i));
        p.setFont(QFont("Georgia",9));p.setPen(QColor(180,150,80));
        p.drawText(QRect(lx[i]-10,152,60,16),Qt::AlignCenter,lname[i]);
        p.setFont(QFont("Georgia",20));p.setPen(QColor(240,210,120));
    }
    p.setPen(QPen(QColor(130,100,50),1));p.drawLine(100,172,700,172);
    const QStringList lines=poem();
    int lineY=182;
    for(int i=0;i<4;i++){
        p.setFont(QFont("Georgia",11,QFont::StyleItalic));p.setPen(QColor(235,215,165));
        p.drawText(QRect(90,lineY,620,60),Qt::TextWordWrap,lines[i]);
        lineY+=68;
        if(i<3){ p.setPen(QPen(QColor(100,80,35,80),1));p.drawLine(110,lineY-4,690,lineY-4); }
    }
    p.setPen(QPen(QColor(130,100,50),1));p.drawLine(100,458,700,458);
    p.setFont(QFont("Georgia",10,QFont::Bold));p.setPen(QColor(180,140,70));
    p.drawText(QRect(60,462,680,30),Qt::AlignCenter,
        "Pull the 4 levers in the order hinted above. 3 wrong pulls = game over.");
    p.setFont(QFont("Georgia",11,QFont::Bold));p.setPen(QColor(160,210,140));
    p.drawText(QRect(0,528,width(),28),Qt::AlignCenter,"Press SPACE to enter the corridor...");
    p.setPen(QColor(242,220,168));p.setFont(QFont("Georgia",16,QFont::Bold));
    p.drawText(24,34,"Level 3 – The Dragon's Castle");
    drawHUD(p);
}
void Level3View::drawCorridor(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());bg.setColorAt(0,QColor(10,7,3));bg.setColorAt(1,QColor(20,14,6));
    p.fillRect(rect(),bg);
    for(int x=0;x<GRID_W;++x){
        for(int y=0;y<GRID_H;++y){
            QRect tile(OX+x*TILE,OY+y*TILE,TILE,TILE);
            L3Tile &t=m_grid[x][y];
            switch(t.type){
            case L3Tile::WALL:
                p.setBrush(QColor(48,36,24));p.setPen(QPen(QColor(28,20,10),1));p.drawRect(tile);
                p.setBrush(QColor(62,48,34));p.drawRoundedRect(tile.adjusted(4,4,-4,-4),3,3);
                break;
            case L3Tile::FLOOR:
                p.setBrush((x+y)%2==0?QColor(50,38,26):QColor(42,32,20));
                p.setPen(QPen(QColor(30,22,12),1));p.drawRect(tile);
                break;
            case L3Tile::SPIKE:
                p.setBrush(QColor(42,32,20));p.setPen(QPen(QColor(30,22,12),1));p.drawRect(tile);
                if(!t.triggered){
                    p.setPen(QPen(QColor(190,30,30),2));
                    for(int i=0;i<3;i++){int sx=tile.left()+10+i*14;p.drawLine(sx,tile.bottom()-5,sx,tile.top()+9);p.drawLine(sx-3,tile.bottom()-5,sx,tile.top()+9);p.drawLine(sx+3,tile.bottom()-5,sx,tile.top()+9);}
                    p.setPen(QPen(QColor(200,40,40,90),2));p.setBrush(Qt::NoBrush);p.drawRect(tile.adjusted(1,1,-1,-1));
                } else {
                    p.setBrush(QColor(120,15,15,160));p.setPen(Qt::NoPen);p.drawEllipse(tile.adjusted(8,8,-8,-8));
                }
                break;
            case L3Tile::GAS: {
                p.setBrush(QColor(42,32,20));p.setPen(QPen(QColor(30,22,12),1));p.drawRect(tile);
                const int gasA=qMin(190,(int)(t.gasTimer*0.9f+(m_gasFloodTimer>0?80:0)));
                QRadialGradient gg(tile.center(),TILE*0.7f);
                gg.setColorAt(0,QColor(50,200,30,gasA));
                gg.setColorAt(1,QColor(15,110,5,gasA/3));
                p.fillRect(tile,gg);
                p.setBrush(QColor(70,220,50,gasA/2));p.setPen(Qt::NoPen);
                p.drawEllipse(tile.left()+8,tile.top()+10,9,9);
                p.drawEllipse(tile.right()-17,tile.bottom()-19,9,9);
                break;
            }
            case L3Tile::LEVER:
                drawLever(p,tile,t,t.leverIndex);
                break;
            case L3Tile::GATE:
                p.setBrush(QColor(48,36,24));p.setPen(QPen(QColor(28,20,10),1));p.drawRect(tile);
                if(!m_gateOpen){
                    p.setBrush(QColor(65,65,75));p.setPen(QPen(QColor(38,38,48),2));
                    p.drawRoundedRect(tile.adjusted(5,3,-5,-2),4,4);
                    for(int bar=0;bar<3;bar++)
                        p.drawRect(tile.left()+10+bar*12,tile.top()+3,7,tile.height()-5);
                    p.setBrush(QColor(180,148,38));p.setPen(Qt::NoPen);
                    p.drawEllipse(tile.center().x()-5,tile.center().y()-5,10,10);
                } else {
                    p.setBrush(QColor(20,80,20,160));p.setPen(QPen(QColor(40,160,40),1));
                    p.drawRoundedRect(tile.adjusted(5,3,-5,-2),4,4);
                    p.setFont(QFont("Georgia",7,QFont::Bold));p.setPen(QColor(80,220,80));
                    p.drawText(tile,Qt::AlignCenter,"OPEN");
                }
                break;
            default:break;
            }
        }
    }
    for(int i:{2,6,10}){
        int tx=OX+i*TILE+20,ty=OY+4;
        p.setPen(QPen(QColor(80,55,30),2));p.drawLine(tx,ty+8,tx,ty+18);
        p.setBrush(QColor(255,170,50));p.setPen(Qt::NoPen);p.drawEllipse(tx-5,ty,10,14);
        QRadialGradient tg(tx,ty+7,26);tg.setColorAt(0,QColor(255,140,20,55));tg.setColorAt(1,QColor(0,0,0,0));
        p.fillRect(OX+i*TILE-8,OY,TILE+16,TILE*2,tg);
    }
    drawPlayer(p,OX+m_playerGX*TILE+8,OY+m_playerGY*TILE+6);
    drawHUD(p);
    p.setPen(QColor(255,220,160));p.setFont(QFont("Georgia",15,QFont::Bold));
    p.drawText(24,34,"Level 3 – The Dragon's Castle");
    p.setFont(QFont("Georgia",10,QFont::StyleItalic));p.setPen(QColor(180,150,110));
    p.drawText(24,52,"The Cursed Corridor  |  Arrow keys to move  |  Walk onto a lever to pull it");
    p.setFont(QFont("Georgia",10,QFont::Bold));
    p.setPen(QColor(220,180,80));
    QString progStr="Levers pulled: ";
    const QVector<int> seq=correctSequence();
    for(int i=0;i<4;i++){
        if(i<m_sequenceProgress) progStr+=leverGlyph(seq[i])+" ";
        else progStr+="_ ";
    }
    p.drawText(QRect(0,560,width()/2,24),Qt::AlignCenter,progStr);
    QString strikeStr="Strikes: ";
    for(int i=0;i<3;i++) strikeStr+=(i<m_strikes)?"✗ ":"○ ";
    p.setPen(m_strikes>0?QColor(220,80,60):QColor(180,180,180));
    p.drawText(QRect(width()/2,560,width()/2,24),Qt::AlignCenter,strikeStr);
    if(m_gateOpen){p.setPen(QColor(100,220,100));p.drawText(QRect(0,582,width(),18),Qt::AlignCenter,"★ Gate is open — walk to it! ★");}
    else if(m_sequenceProgress>0&&m_sequenceProgress<4){
        int secs=(m_sequenceTimeout/60)+1;
        p.setPen(secs<=3?QColor(220,80,60):QColor(160,130,70,180));
        p.setFont(QFont("Georgia",9,QFont::Bold));
        p.drawText(QRect(0,582,width(),18),Qt::AlignCenter,
            QString("Pull the next lever in %1 second%2 or the sequence resets!").arg(secs).arg(secs==1?"":"s"));
    } else {
        p.setPen(QColor(160,130,70,180));p.setFont(QFont("Georgia",9,QFont::StyleItalic));
        p.drawText(QRect(0,582,width(),18),Qt::AlignCenter,"Pull the levers in the correct order from your poem.");
    }
}
void Level3View::drawLever(QPainter &p, const QRect &tile, const L3Tile &t, int idx)
{
    p.setBrush(QColor(42,32,20));p.setPen(QPen(QColor(30,22,12),1));p.drawRect(tile);
    QColor baseCol  = t.locked_in ? QColor(60,180,60)  : QColor(90,70,40);
    QColor handleCol= t.locked_in ? QColor(80,220,80)  : QColor(160,120,50);
    QColor glowCol  = t.locked_in ? QColor(60,200,60,100) : QColor(200,160,60,60);
    QRadialGradient lg(tile.center(),28);lg.setColorAt(0,glowCol);lg.setColorAt(1,QColor(0,0,0,0));
    p.fillRect(tile,lg);
    p.setBrush(baseCol);p.setPen(QPen(baseCol.darker(140),1));
    p.drawRoundedRect(tile.center().x()-10,tile.bottom()-14,20,10,3,3);
    p.setPen(QPen(handleCol,3,Qt::SolidLine,Qt::RoundCap));
    const int hx=tile.center().x();
    const int hy_base=tile.bottom()-10;
    const int hy_tip =t.locked_in? tile.top()+16 : tile.top()+8;
    p.drawLine(hx,hy_base,hx+(t.locked_in?8:0),hy_tip);
    p.setFont(QFont("Georgia",18));
    p.setPen(t.locked_in?QColor(100,240,100):QColor(230,200,100));
    p.drawText(QRect(tile.left(),tile.top()+2,tile.width(),26),Qt::AlignCenter,leverGlyph(idx));
    const QVector<int> seq=correctSequence();
    if(m_sequenceProgress<4&&!t.locked_in&&idx==seq[m_sequenceProgress]){
        p.setPen(QPen(QColor(255,220,80,80+(int)(40*std::sin(m_gasTickCounter*0.2))),2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(tile.adjusted(2,2,-2,-2),4,4);
    }
}
void Level3View::drawGargoyleArena(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());bg.setColorAt(0,QColor(16,12,8));bg.setColorAt(1,QColor(28,20,12));
    p.fillRect(rect(),bg);
    p.setPen(QPen(QColor(30,22,12),1));
    for(int x=0;x<12;++x) for(int y=0;y<9;++y){p.setBrush((x+y)%2==0?QColor(52,40,26):QColor(44,34,20));p.drawRect(x*72,y*68,72,68);}
    p.setBrush(QColor(55,44,32));p.setPen(Qt::NoPen);p.drawRect(720,0,80,height());
    p.setPen(QPen(QColor(35,26,16),1));
    for(int y=0;y<9;++y) for(int x=0;x<2;++x){p.setBrush(QColor(62,50,36));p.drawRoundedRect(720+x*42+(y%2)*20,y*70,40,65,3,3);}
    p.setBrush(QColor(62,50,36));p.setPen(Qt::NoPen);
    for(int i=0;i<5;++i) p.drawRect(720+i*16,0,10,30);
    for(int ty:{80,280,460}){
        p.setPen(QPen(QColor(80,55,30),2));p.drawLine(718,ty+10,718,ty+22);
        p.setBrush(QColor(255,170,50));p.setPen(Qt::NoPen);p.drawEllipse(712,ty,12,16);
        QRadialGradient tg(718,ty+8,40);tg.setColorAt(0,QColor(255,140,20,80));tg.setColorAt(1,QColor(0,0,0,0));
        p.fillRect(680,ty-20,80,60,tg);
    }
    for(auto &g:m_gargoyles) drawGargoyle(p,g);
    for(auto &s:m_shots){
        if(!s.active) continue;
        if(m_role=="Wizard"){QRadialGradient sg(s.x,s.y,8);sg.setColorAt(0,QColor(180,100,255,220));sg.setColorAt(1,QColor(0,0,0,0));p.fillRect(s.x-10,s.y-10,20,20,sg);p.setBrush(QColor(200,140,255));p.setPen(Qt::NoPen);p.drawEllipse(s.x-5,s.y-5,10,10);}
        else if(m_role=="Rogue"){p.setBrush(QColor(160,160,180));p.setPen(Qt::NoPen);QPoint kn[4]={{(int)(s.x-8),(int)(s.y-2)},{(int)(s.x+8),(int)(s.y-2)},{(int)(s.x+8),(int)(s.y+2)},{(int)(s.x-8),(int)(s.y+2)}};p.drawPolygon(kn,4);}
        else if(m_role=="Cleric"){p.setBrush(QColor(220,220,255,200));p.setPen(Qt::NoPen);p.drawEllipse(s.x-6,s.y-6,12,12);}
        else{p.setBrush(QColor(180,150,100));p.setPen(Qt::NoPen);p.drawEllipse(s.x-5,s.y-5,10,10);}
    }
    drawPlayer(p,m_px,m_py);
    drawHUD(p);
    p.setPen(QColor(255,220,160));p.setFont(QFont("Georgia",15,QFont::Bold));
    p.drawText(24,34,"Level 3 – The Dragon's Castle");
    p.setFont(QFont("Georgia",10,QFont::StyleItalic));p.setPen(QColor(180,150,110));
    p.drawText(24,52,"Gargoyle Gauntlet  |  Arrows=Move  SPACE=Shoot");
    int alive=0; for(auto &g:m_gargoyles) if(g.alive) alive++;
    p.setFont(QFont("Georgia",11,QFont::Bold));p.setPen(QColor(220,160,60));
    p.drawText(QRect(0,560,width(),28),Qt::AlignCenter,QString("Gargoyles remaining: %1").arg(alive));
}
void Level3View::drawGargoyle(QPainter &p, const Gargoyle &g)
{
    if(!g.alive) return;
    const int cx=g.x,cy=g.y;
    p.setPen(Qt::NoPen);
    p.setBrush(g.hp==1?QColor(90,85,80):QColor(110,100,90));p.drawEllipse(cx-18,cy-10,36,40);
    p.setBrush(QColor(80,75,70,200));
    QPoint wL[4]={{cx-18,cy},{cx-50,cy-20},{cx-45,cy+20},{cx-18,cy+20}};
    QPoint wR[4]={{cx+18,cy},{cx+50,cy-20},{cx+45,cy+20},{cx+18,cy+20}};
    p.drawPolygon(wL,4);p.drawPolygon(wR,4);
    p.setBrush(g.hp==1?QColor(85,80,75):QColor(105,95,85));p.drawEllipse(cx-12,cy-28,24,26);
    p.setBrush(QColor(70,65,60));
    QPoint h1[3]={{cx-10,cy-26},{cx-6,cy-26},{cx-12,cy-42}};
    QPoint h2[3]={{cx+6,cy-26},{cx+10,cy-26},{cx+12,cy-42}};
    p.drawPolygon(h1,3);p.drawPolygon(h2,3);
    p.setBrush(QColor(220,40,20));p.drawEllipse(cx-7,cy-20,6,6);p.drawEllipse(cx+1,cy-20,6,6);
    if(g.hp==1){p.setPen(QPen(QColor(40,35,30),2));p.drawLine(cx-5,cy-28,cx+2,cy+10);p.drawLine(cx+3,cy-20,cx-4,cy+15);}
    p.setPen(Qt::NoPen);p.setBrush(QColor(30,10,10));p.drawRect(cx-16,cy-50,32,5);
    p.setBrush(QColor(200,40,20));p.drawRect(cx-16,cy-50,(g.hp*32)/3,5);
}
void Level3View::drawPlayer(QPainter &p, float px, float py)
{
    QColor cloak(76,124,215);
    if(m_role=="Wizard") cloak=QColor(85,102,224);
    else if(m_role=="Fighter") cloak=QColor(171,72,65);
    else if(m_role=="Rogue") cloak=QColor(65,138,94);
    else if(m_role=="Cleric") cloak=QColor(199,170,88);
    p.setPen(Qt::NoPen);p.setBrush(cloak);p.drawRoundedRect(px+12,py+14,24,26,8,8);
    p.setBrush(QColor(236,207,169));p.drawEllipse(px+16,py+4,16,16);
    p.setPen(QPen(QColor(210,190,126),3));
    p.drawLine(px+34,py+14,px+40,py+34);p.drawLine(px+14,py+14,px+8,py+34);
}
void Level3View::drawHUD(QPainter &p)
{
    for(int i=0;i<2;++i){
        const int hx=width()-44-i*34,hy=18;
        if(i<m_health){
            p.setPen(Qt::NoPen);p.setBrush(QColor(200,40,40));
            p.drawEllipse(hx,hy,13,13);p.drawEllipse(hx+10,hy,13,13);
            QPoint hp[3]={{hx,hy+12},{hx+24,hy+12},{hx+12,hy+24}};p.drawPolygon(hp,3);
        } else {
            p.setPen(QPen(QColor(120,40,40),2));p.setBrush(Qt::NoBrush);
            p.drawEllipse(hx,hy,13,13);p.drawEllipse(hx+10,hy,13,13);
            QPoint hp[3]={{hx,hy+12},{hx+24,hy+12},{hx+12,hy+24}};p.drawPolygon(hp,3);
        }
    }
}
void Level3View::drawVictoryScreen(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());bg.setColorAt(0,QColor(4,12,4));bg.setColorAt(.5,QColor(12,36,12));bg.setColorAt(1,QColor(4,12,4));
    p.fillRect(rect(),bg);
    QRadialGradient gw(width()/2,height()/2,300);gw.setColorAt(0,QColor(50,180,70,100));gw.setColorAt(1,QColor(0,0,0,0));p.fillRect(rect(),gw);
    p.setPen(QPen(QColor(60,150,60,160),1));
    p.drawLine(60,height()/2-95,width()-60,height()/2-95);p.drawLine(60,height()/2+95,width()-60,height()/2+95);
    p.setPen(QColor(150,230,150));p.setFont(QFont("Georgia",12,QFont::Bold));
    p.drawText(rect().adjusted(0,-120,0,0),Qt::AlignCenter,"— VICTORY —");
    p.setPen(QColor(190,255,190));p.setFont(QFont("Georgia",26,QFont::Bold));
    p.drawText(rect().adjusted(0,-30,0,0),Qt::AlignCenter,"Your friend is free!");
    p.setPen(QColor(130,210,130,210));p.setFont(QFont("Georgia",12,QFont::StyleItalic));
    p.drawText(rect().adjusted(0,60,0,0),Qt::AlignCenter,
               "The cursed gate groans and swings open.\nYou pull your friend from the dungeon.\n\nYour legend will be told for generations.");
    p.setFont(QFont("Georgia",11,QFont::Bold));p.setPen(QColor(180,230,180));
    p.drawText(rect().adjusted(0,190,0,0),Qt::AlignCenter,"Press R to play again.");
}
void Level3View::drawGameOverScreen(QPainter &p)
{
    QLinearGradient bg(0,0,0,height());bg.setColorAt(0,QColor(16,4,2));bg.setColorAt(.5,QColor(40,8,4));bg.setColorAt(1,QColor(16,4,2));
    p.fillRect(rect(),bg);
    QRadialGradient gw(width()/2,height()/2,300);gw.setColorAt(0,QColor(200,28,10,100));gw.setColorAt(1,QColor(0,0,0,0));p.fillRect(rect(),gw);
    p.setPen(QPen(QColor(180,40,20,160),1));
    p.drawLine(80,height()/2-80,width()-80,height()/2-80);p.drawLine(80,height()/2+80,width()-80,height()/2+80);
    p.setPen(QColor(240,150,130));p.setFont(QFont("Georgia",12,QFont::Bold));
    const QString reason=m_strikes>=3?"— THREE STRIKES —":"— POISONED —";
    p.drawText(rect().adjusted(0,-105,0,0),Qt::AlignCenter,reason);
    p.setPen(QColor(255,185,165));p.setFont(QFont("Georgia",24,QFont::Bold));
    p.drawText(rect(),Qt::AlignCenter,m_strikes>=3?"The curse consumed you.":"The poison claimed you.");
    p.setPen(QColor(200,135,115,200));p.setFont(QFont("Georgia",12,QFont::StyleItalic));
    p.drawText(rect().adjusted(0,90,0,0),Qt::AlignCenter,"Press R to try Level 3 again.");
}
void Level3View::keyPressEvent(QKeyEvent *event)
{
    if(m_screen==SPLASH){ m_splashTimer->stop(); onSplashTimer(); return; }
    if(m_screen==WITCH_BRIEFING){ if(event->key()==Qt::Key_Space||event->key()==Qt::Key_Return){ m_screen=GARGOYLE_ARENA; update(); } return; }
    if(m_screen==POEM_CLUE){ if(event->key()==Qt::Key_Space||event->key()==Qt::Key_Return){ m_screen=CORRIDOR; update(); } return; }
    if(m_screen==GAMEOVER&&event->key()==Qt::Key_R){ startLevel(m_playerName,m_role); return; }
    if(m_screen==VICTORY&&event->key()==Qt::Key_R){ startLevel(m_playerName,m_role); return; }
    if(m_screen==GARGOYLE_ARENA){
        if(event->key()==Qt::Key_Up)    m_keys[0]=true;
        if(event->key()==Qt::Key_Down)  m_keys[1]=true;
        if(event->key()==Qt::Key_Left)  m_keys[2]=true;
        if(event->key()==Qt::Key_Right) m_keys[3]=true;
        if(event->key()==Qt::Key_Space) playerShoot();
        return;
    }
    if(m_screen==CORRIDOR){
        int dx=0,dy=0;
        if(event->key()==Qt::Key_Up)   dy=-1;
        if(event->key()==Qt::Key_Down) dy= 1;
        if(event->key()==Qt::Key_Left) dx=-1;
        if(event->key()==Qt::Key_Right)dx= 1;
        if(!dx&&!dy) return;
        const int nx=m_playerGX+dx,ny=m_playerGY+dy;
        if(nx<0||nx>=GRID_W||ny<0||ny>=GRID_H) return;
        L3Tile &t=m_grid[nx][ny];
        if(t.type==L3Tile::WALL) return;
        if(t.type==L3Tile::SPIKE&&!t.triggered){
            t.triggered=true; m_health--;
            m_flashMsg="Spike trap! Lose a heart!"; m_flashFrames=90;
            m_playerGX=nx; m_playerGY=ny;
            if(m_health<=0){ m_screen=GAMEOVER; m_gameTick->stop(); }
            update(); return;
        }
        if(t.type==L3Tile::LEVER&&!t.locked_in){
            m_playerGX=nx; m_playerGY=ny;
            pullLever(t.leverIndex);
            update(); return;
        }
        if(t.type==L3Tile::GATE){
            if(m_gateOpen){
                m_screen=VICTORY; m_gameTick->stop();
                QTimer::singleShot(400,this,&Level3View::levelComplete);
            } else {
                m_flashMsg="The gate is sealed — pull the levers in order first!";
                m_flashFrames=80;
            }
            update(); return;
        }
        if(t.type==L3Tile::GAS){
            m_flashMsg="Poison gas! Move quickly!"; m_flashFrames=50;
        }
        m_playerGX=nx; m_playerGY=ny;
        update();
    }
}
void Level3View::keyReleaseEvent(QKeyEvent *event)
{
    if(m_screen==GARGOYLE_ARENA){
        if(event->key()==Qt::Key_Up)    m_keys[0]=false;
        if(event->key()==Qt::Key_Down)  m_keys[1]=false;
        if(event->key()==Qt::Key_Left)  m_keys[2]=false;
        if(event->key()==Qt::Key_Right) m_keys[3]=false;
    }
}
