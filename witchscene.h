#ifndef WITCHSCENE_H
#define WITCHSCENE_H

#include <QString>
#include <QStringList>

// ---------------------------------------------------------------------------
// WitchScene
//   Manages the Level-2 witch encounter: dialogue phases, the riddle
//   challenge (role-specific), and escape tracking.
// ---------------------------------------------------------------------------
class WitchScene
{
public:
    enum Phase {
        PHASE_ENTER,       // Player just walked in; witch reacts
        PHASE_TAUNT,       // Witch taunts / blocks the door
        PHASE_RIDDLE,      // Player asked for the riddle; it is shown
        PHASE_WRONG,       // Player answered wrong
        PHASE_ESCAPED      // Riddle solved – player may leave
    };

    explicit WitchScene(const QString &role = "Wizard");

    // --- state
    Phase   phase()           const { return m_phase; }
    bool    riddleShowing()   const { return m_phase == PHASE_RIDDLE || m_phase == PHASE_WRONG; }
    bool    escaped()         const { return m_phase == PHASE_ESCAPED; }

    // --- content
    QString witchLine()       const;   // what the witch currently says
    QString riddleText()      const;   // the riddle question
    QString hintText()        const;   // small hint shown below riddle

    // --- answers  (multiple accepted phrasings)
    QStringList acceptedAnswers() const { return m_answers; }

    // --- transitions
    void advanceDialogue();            // ENTER -> TAUNT -> (stays)
    void showRiddle();                 // TAUNT -> RIDDLE
    void submitAnswer(const QString &answer); // may -> ESCAPED or WRONG

    // number of wrong attempts so far
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

#endif // WITCHSCENE_H
