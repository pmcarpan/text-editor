#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QKeyEvent>
#include <QTextEdit>
#include <QMenuBar>

#include "piecetable.h"

class TextEditor : public QTextEdit
{
    PieceTable pieceTable;

    bool isValidCharacter(char ch);

public:
    TextEditor(QWidget *parent = nullptr);
    ~TextEditor() override;

    void cut();
    void copy();
    void paste();

    void undo();
    void redo();

    void clear();

    void setPlainText(const QString &text);

protected:
    void keyPressEvent(QKeyEvent *e) override;

};

#endif // TEXTEDITOR_H
