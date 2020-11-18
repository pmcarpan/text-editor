#include "texteditor.h"

#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QLayout>

#include <iostream>

TextEditor::TextEditor(QWidget *parent)
    : QTextEdit(parent)
{
    setAcceptRichText(false);
}

bool TextEditor::isValidCharacter(char ch) {
    return isalnum(ch) || isspace(ch) || ispunct(ch);
}

void TextEditor::keyPressEvent(QKeyEvent *e)
{
    QTextCursor cursor = textCursor(); // document cursor
    QString textEntered = e->text();   // text detected
    int pos = cursor.position(),       // position
        anc = cursor.anchor();         // selection anchor, same as position if no selection
    bool reportKeyEventToSuperClass = true;

    std::cout << "(Pos, Anc, Text) = (" << pos << ", " << anc << ", " << textEntered.toStdString() << ")\n";

    if (e->key() == Qt::Key::Key_Backspace) {
        std::cout << "BACKSPACE\n";
        if (pos == anc) {
            if (pos - 1 >= 0)
                pieceTable.remove(pos, -1);
        }
        else {
            pieceTable.remove(pos, anc - pos);
        }
    }
    else if (e->key() == Qt::Key::Key_Delete) {
        std::cout << "DELETE\n";
        if (pos == anc) {
            if (pos + 1 < pieceTable.getLen())
                pieceTable.remove(pos, 1);
        }
        else {
            pieceTable.remove(pos, anc - pos);
        }
    }
    else if (e->matches(QKeySequence::Cut)) {
        cut();
        reportKeyEventToSuperClass = false;
    }
    else if (e->matches(QKeySequence::Copy)) {
        copy();
        reportKeyEventToSuperClass = false;
    }
    else if (e->matches(QKeySequence::Paste)) {
        paste();
        reportKeyEventToSuperClass = false;
    }
    else if (e->matches(QKeySequence::Undo)) {
        undo();
        reportKeyEventToSuperClass = false;
    }
    else if (e->matches(QKeySequence::Redo)) {
        redo();
        reportKeyEventToSuperClass = false;
    }
    else {
        std::string t = textEntered.toStdString();
        if (!t.empty() && t.size() == 1 && isValidCharacter(t[0])) {
            if (pos == anc)
                pieceTable.insert(t, pos);
            else {
                pieceTable.remove(pos, anc - pos);
                pieceTable.insert(t, std::min(pos, anc));
            }
        }
    }

    std::cout << "Len = " << pieceTable.getLen() << "\n";
    std::cout << "[STARTTOKEN]" << pieceTable.getText() << "[ENDTOKEN]\n";
    std::cout << pieceTable.dump();
    std::flush(std::cout);

    if (reportKeyEventToSuperClass)
        QTextEdit::keyPressEvent(e);
}

void TextEditor::cut()
{
    QTextCursor cursor = textCursor(); // document cursor
    int pos = cursor.position(),       // position
        anc = cursor.anchor();         // selection anchor, same as position if no selection

    pieceTable.remove(pos, anc - pos);

    QTextEdit::cut();

    QString clipBoardText = QApplication::clipboard()->text();
    std::string t = clipBoardText.toStdString();

    std::cout << "CUT | clipboardtext = " << t << "\n";
    std::flush(std::cout);
}

void TextEditor::copy()
{
    QTextEdit::copy();

    QString clipBoardText = QApplication::clipboard()->text();
    std::string t = clipBoardText.toStdString();

    std::cout << "COPY | clipboardtext = " << t << "\n";
    std::flush(std::cout);
}

void TextEditor::paste()
{
    QTextCursor cursor = textCursor(); // document cursor
    int pos = cursor.position(),       // position
        anc = cursor.anchor();         // selection anchor, same as position if no selection

    QString clipBoardText = QApplication::clipboard()->text();
    std::string t = clipBoardText.toStdString();

    std::cout << "PASTE | clipboardtext = " << t << "\n";
    std::flush(std::cout);

    if (pos == anc)
        pieceTable.insert(t, pos);
    else {
        pieceTable.remove(pos, anc - pos);
        pieceTable.insert(t, std::min(pos, anc));
    }

    QTextEdit::paste();
}

void TextEditor::undo()
{
    std::cout << "UNDO\n"; std::flush(std::cout);
    pieceTable.historyBackward();
    document()->setPlainText(QString::fromStdString(pieceTable.getText()));
}

void TextEditor::redo()
{
    std::cout << "REDO\n"; std::flush(std::cout);
    pieceTable.historyForward();
    document()->setPlainText(QString::fromStdString(pieceTable.getText()));
}

void TextEditor::clear()
{
    pieceTable = PieceTable();

    QTextEdit::clear();
}

void TextEditor::setPlainText(const QString &text)
{
    pieceTable = PieceTable(text.toStdString());

    QTextEdit::setPlainText(text);
}

TextEditor::~TextEditor()
{
}
