#ifndef TERMINALHIGHLIGHTER_H
#define TERMINALHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>

class TerminalHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    TerminalHighlighter(QTextDocument *parent = nullptr);
    void setTheme(bool isDark);

protected:
    void highlightBlock(const QString &text) override;

private:
    bool m_isDarkTheme = true;
    
    QTextCharFormat errorFormat;
    QTextCharFormat commandFormat;
    QTextCharFormat pathFormat;
    QTextCharFormat folderFormat;
    QTextCharFormat fileFormat;
    QTextCharFormat promptFormat;
    QTextCharFormat userHostFormat;

    void updateFormats();
};

#endif // TERMINALHIGHLIGHTER_H
