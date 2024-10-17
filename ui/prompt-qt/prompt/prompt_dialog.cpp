#include "prompt_dialog.h"

PromptDialog::PromptDialog(QObject *parent) : QObject(parent)
{
    // Initialize the property if needed
}

QString PromptDialog::getPromptText() const
{
    return m_promptText;
}

void PromptDialog::setPromptText(const QString &text)
{
    if (m_promptText != text) {
        m_promptText = text;
        emit promptTextChanged();
    }
}

QString PromptDialog::getPromptText()
{
    return m_promptText;
}
