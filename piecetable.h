#ifndef PIECETABLE_H
#define PIECETABLE_H

#include <list>
#include <stack>
#include <string>
#include <tuple>

class PieceTable
{
    static const int MAX_HISTORY_CACHE_SIZE = 100;

    int len;
    std::string original, add;

    // Piece [refersToAddBuffer (bool), offset in buffer (int), length (int)]
    std::list<std::tuple<bool, int, int>> pieces;

    std::list<std::pair<int, std::list<std::tuple<bool, int, int>>>> history;
    std::list<std::pair<int, std::list<std::tuple<bool, int, int>>>>::iterator
        currentHistoryIt;

public:
    PieceTable(std::string initialText = std::string());
    void insert(std::string text, int offset);
    void remove(int offset, int length);

    std::string getText();
    std::string getTextAt(int offset, int length);

    int getLen();

    void historyForward();
    void historyBackward();
    void takeHistorySnapshot();

    void restoreFromHistoryIterator();

    std::string dump();

private:
    std::pair< std::list<std::tuple<bool, int, int>>::iterator, int >
        getPieceIteratorAndBufferOffset(int offset);
};

#endif // PIECETABLE_H
