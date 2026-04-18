#include "TerminalHighlighter.h"

TerminalHighlighter::TerminalHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
    updateFormats();
}

void TerminalHighlighter::setTheme(bool isDark) {
    m_isDarkTheme = isDark;
    updateFormats();
    rehighlight();
}

void TerminalHighlighter::updateFormats() {
    if (m_isDarkTheme) {
        // Dark theme colors
        errorFormat.setForeground(QColor("#ff5555"));       // Red
        commandFormat.setForeground(QColor("#50fa7b"));     // Green
        pathFormat.setForeground(QColor("#8be9fd"));        // Cyan/Blue
        folderFormat.setForeground(QColor("#bd93f9"));      // Purple
        fileFormat.setForeground(QColor("#f8f8f2"));        // White
        promptFormat.setForeground(QColor("#f1fa8c"));      // Yellow for prompt symbol
        userHostFormat.setForeground(QColor("#50fa7b"));    // Green
    } else {
        // Light theme colors
        errorFormat.setForeground(QColor("#d73a49"));       // Red
        commandFormat.setForeground(QColor("#22863a"));     // Green
        pathFormat.setForeground(QColor("#005cc5"));        // Blue
        folderFormat.setForeground(QColor("#6f42c1"));      // Purple
        fileFormat.setForeground(QColor("#24292e"));        // Black
        promptFormat.setForeground(QColor("#b08800"));      // Yellow/Brown for prompt symbol
        userHostFormat.setForeground(QColor("#22863a"));    // Green
    }

    errorFormat.setFontWeight(QFont::Bold);
    commandFormat.setFontWeight(QFont::Bold);
    folderFormat.setFontWeight(QFont::Bold);
    userHostFormat.setFontWeight(QFont::Bold);
}

void TerminalHighlighter::highlightBlock(const QString &text) {
    // 1. Errors
    QRegularExpression errorRegex("(?i)(\\[ERROR\\]|\\[BLOCKED\\]|error:|failed:|exception:|permission denied).*");
    QRegularExpressionMatchIterator errIt = errorRegex.globalMatch(text);
    while (errIt.hasNext()) {
        QRegularExpressionMatch match = errIt.next();
        setFormat(match.capturedStart(), match.capturedLength(), errorFormat);
    }

    // 2. Folder/file parsing heuristics
    // E.g., paths starting with / or ~/ or just simple extensions
    QRegularExpression pathRegex("(?<!\\w)(/|~/)[\\w\\-\\./]+");
    QRegularExpressionMatchIterator pathIt = pathRegex.globalMatch(text);
    while (pathIt.hasNext()) {
        QRegularExpressionMatch match = pathIt.next();
        setFormat(match.capturedStart(), match.capturedLength(), pathFormat);
    }

    // Identify folders ending with / in list outputs (ls -F style) or normal words ending with /
    QRegularExpression folderRegex("(?<!\\w)([\\w\\-\\.]+/)");
    QRegularExpressionMatchIterator folderIt = folderRegex.globalMatch(text);
    while (folderIt.hasNext()) {
        QRegularExpressionMatch match = folderIt.next();
        setFormat(match.capturedStart(), match.capturedLength(), folderFormat);
    }

    // Identify typical files (e.g., with extension like .cpp, .txt, .h)
    QRegularExpression fileRegex("(?<!\\w)([\\w\\-]+\\.[a-zA-Z0-9]+)(?!/)");
    QRegularExpressionMatchIterator fileIt = fileRegex.globalMatch(text);
    while (fileIt.hasNext()) {
        QRegularExpressionMatch match = fileIt.next();
        // If it's not already formatted as an error, format as file
        setFormat(match.capturedStart(), match.capturedLength(), fileFormat);
    }

    // 3. Prompt and Command
    // Prompt pattern: user@host:path$ command
    QRegularExpression promptRegex("^([^@]+@[^:]+):([^\\$]+)(\\$\\s*)(.*)$");
    QRegularExpressionMatch promptMatch = promptRegex.match(text);
    if (promptMatch.hasMatch()) {
        // Highlight user@host
        setFormat(promptMatch.capturedStart(1), promptMatch.capturedLength(1), userHostFormat);
        
        // Highlight path
        setFormat(promptMatch.capturedStart(2), promptMatch.capturedLength(2), pathFormat);
        
        // Highlight $
        setFormat(promptMatch.capturedStart(3), promptMatch.capturedLength(3), promptFormat);

        // Highlight the command part
        QString cmdPart = promptMatch.captured(4);
        if (!cmdPart.isEmpty()) {
            setFormat(promptMatch.capturedStart(4), promptMatch.capturedLength(4), commandFormat);
        }
    } else {
        // Fallback for old prompt format
        QRegularExpression oldPromptRegex("^(\\[.*\\]\\$\\s*)(.*)$");
        QRegularExpressionMatch oldMatch = oldPromptRegex.match(text);
        if (oldMatch.hasMatch()) {
            QString cmdPart = oldMatch.captured(2);
            if (!cmdPart.isEmpty()) {
                setFormat(oldMatch.capturedStart(2), oldMatch.capturedLength(2), commandFormat);
            }
        }
    }
}
