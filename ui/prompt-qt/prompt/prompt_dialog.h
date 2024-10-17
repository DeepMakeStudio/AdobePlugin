#ifndef PROMPT_DIALOG_H
#define PROMPT_DIALOG_H

#include <QObject>


class PromptDialog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString promptText READ getPromptText WRITE setPromptText NOTIFY promptTextChanged)

public:
    explicit PromptDialog(QObject *parent = nullptr);

    QString getPromptText() const;
    void setPromptText(const QString &text);
    QString getPromptText();

signals:
    void promptTextChanged();

private:
    QString m_promptText;
};


#endif // PROMPT_DIALOG_H


