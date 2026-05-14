#include "witchscene.h"

WitchScene::WitchScene(const QString &role) : m_role(role)
{
    buildRiddle(role);
}

QString WitchScene::witchLine() const
{
    switch (m_phase)
    {
    case PHASE_ENTER:
        return "GET OUT of my cottage, wretch!\nYou dare enter uninvited?!";
    case PHASE_TAUNT:
        return "The door is sealed by my curse.\nOnly a worthy mind may leave.\nPress Space to face the riddle... if you dare.";
    case PHASE_RIDDLE:
        return "Answer correctly and you shall pass.\nAnswer wrongly... and you stay. Forever.";
    case PHASE_WRONG:
        if (m_wrongAttempts == 1)
            return "Bwahahaha! Wrong! Try again, fool.\nMy patience wears thin...";
        else if (m_wrongAttempts == 2)
            return "Still wrong?! You disappoint me.\nThink harder, adventurer!";
        else
            return "You are hopeless... yet my curse demands\nI give you one last chance.";
    case PHASE_ESCAPED:
        return "Impossible... you solved it.\nBe gone from my sight!";
    }
    return "";
}

QString WitchScene::riddleText() const { return m_riddleText; }
QString WitchScene::hintText()   const { return m_hintText;   }

void WitchScene::buildRiddle(const QString &role)
{
    if (role == "Wizard")
    {
        m_riddleText =
            "I have cities, but no houses live there.\n"
            "I have mountains, but no trees grow.\n"
            "I have water, but no fish swim.\n"
            "I have roads, but no carts travel.\n"
            "What am I?";
        m_hintText = "Hint: A Wizard consults me before every journey.";
        m_answers  = { "map", "a map" };
    }
    else if (role == "Fighter")
    {
        m_riddleText =
            "The more you take, the more you leave behind.\n"
            "A warrior knows me well after every march.\n"
            "What am I?";
        m_hintText = "Hint: You make me with every step you take.";
        m_answers  = { "footsteps", "steps", "a footstep", "footstep" };
    }
    else if (role == "Rogue")
    {
        m_riddleText =
            "I follow you all day in the light,\n"
            "yet I vanish when darkness falls.\n"
            "You cannot touch me, yet I mimic\n"
            "your every move without a sound.\n"
            "What am I?";
        m_hintText = "Hint: A Rogue knows how to use me to hide.";
        m_answers  = { "shadow", "my shadow", "a shadow", "your shadow" };
    }
    else // Cleric
    {
        m_riddleText =
            "I speak without a mouth,\n"
            "I hear without ears.\n"
            "I have no body, yet I come alive\n"
            "with the wind. What am I?";
        m_hintText = "Hint: A Cleric hears me in prayer and in mountains.";
        m_answers  = { "echo", "an echo" };
    }
}

void WitchScene::advanceDialogue()
{
    if (m_phase == PHASE_ENTER)
        m_phase = PHASE_TAUNT;
}

void WitchScene::showRiddle()
{
    if (m_phase == PHASE_TAUNT || m_phase == PHASE_WRONG)
        m_phase = PHASE_RIDDLE;
}

void WitchScene::submitAnswer(const QString &answer)
{
    const QString trimmed = answer.trimmed().toLower();
    for (const QString &accepted : m_answers)
    {
        if (trimmed == accepted.toLower())
        {
            m_phase = PHASE_ESCAPED;
            return;
        }
    }
    ++m_wrongAttempts;
    m_phase = PHASE_WRONG;
}

void WitchScene::forceEscaped() { m_phase = PHASE_ESCAPED; }
