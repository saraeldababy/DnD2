#ifndef WITCHSCENE_H
#define WITCHSCENE_H
#include <QString>
#include <QStringList>
class WitchScene
{
public:
    enum Phase {
        PHASE_ENTER,       
        PHASE_TAUNT,       
        PHASE_RIDDLE,      
        PHASE_WRONG,       
        PHASE_ESCAPED      
    };
    explicit WitchScene(const QString &role = "Wizard");
    Phase   phase()           const { return m_phase; }
    bool    riddleShowing()   const { return m_phase == PHASE_RIDDLE || m_phase == PHASE_WRONG; }
    bool    escaped()         const { return m_phase == PHASE_ESCAPED; }
    QString witchLine()       const;   
    QString riddleText()      const;   
    QString hintText()        const;   
    QStringList acceptedAnswers() const { return m_answers; }
    void advanceDialogue();            
    void showRiddle();                 
    void submitAnswer(const QString &answer); 
    int wrongAttempts() const { return m_wrongAttempts; }
private:
    void buildRiddle(const QString &role);
    Phase       m_phase        = PHASE_ENTER;
    QString     m_role;
    QString     m_riddleText;
    QString     m_hintText;
    QStringList m_answers;
    int         m_wrongAttempts = 0;
};
#endif 
