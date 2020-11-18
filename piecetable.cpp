#include "piecetable.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <iostream>

PieceTable::PieceTable(std::string initialText)
{
    len = 0;
    original = initialText;
    add = "";

    pieces.push_back(std::make_tuple(false, 0, int(initialText.length())));

    history.push_back(std::make_pair(len, pieces));
    currentHistoryIt = history.begin();
}

std::pair< std::list<std::tuple<bool, int, int>>::iterator, int >
PieceTable::getPieceIteratorAndBufferOffset(int offset)
{
    if (offset < 0) throw std::out_of_range("Offset < 0");

    int remainingOffset = offset;
    for (std::list<std::tuple<bool, int, int>>::iterator
         it = pieces.begin(); it != pieces.end(); it++)
    {
        std::tuple<bool, int, int> piece = *it;
        int pieceLength = std::get<2>(piece);
        if (remainingOffset <= pieceLength)
        {
            int pieceOffset = std::get<1>(piece);
            return std::make_pair(it, pieceOffset + remainingOffset);
        }
        remainingOffset -= pieceLength;
    }

    throw std::out_of_range("Offset > Length of Text");
}

void PieceTable::insert(std::string text, int offset)
{
    if (text.length() == 0) return;

    len += int(text.size());

    int addBufferOffset = int(add.length());
    add += text;
    auto pieceIteratorAndBufferOffset = getPieceIteratorAndBufferOffset(offset);
    auto& originalPiece = *pieceIteratorAndBufferOffset.first;
    int bufferOffset = pieceIteratorAndBufferOffset.second;

    // insert at end
    if (std::get<0>(originalPiece) && bufferOffset == std::get<1>(originalPiece) + std::get<2>(originalPiece)
            && std::get<1>(originalPiece) + std::get<2>(originalPiece) == addBufferOffset)
    {
        std::get<2>(originalPiece) += int(text.length());
        takeHistorySnapshot();
        return;
    }

    std::vector<std::tuple<bool, int, int>> insertPieces = {
std::make_tuple(std::get<0>(originalPiece),
                std::get<1>(originalPiece),
                bufferOffset - std::get<1>(originalPiece)),
std::make_tuple(true, addBufferOffset, int(text.length())),
std::make_tuple(std::get<0>(originalPiece),
                bufferOffset,
                std::get<2>(originalPiece) - (bufferOffset - std::get<1>(originalPiece)))
    };

    auto it = std::remove_if(insertPieces.begin(), insertPieces.end(),
                             [](std::tuple<bool, int, int> x) { return std::get<2>(x) <= 0; });

    pieces.insert(pieceIteratorAndBufferOffset.first, insertPieces.begin(), it);
    pieces.erase(pieceIteratorAndBufferOffset.first);

    takeHistorySnapshot();
}

void PieceTable::remove(int offset, int length)
{
    if (length == 0) return;
    if (length < 0) return remove(offset + length, -length);
    if (offset < 0) throw std::out_of_range("Offset < 0");

    len -= length;

    auto initialPieceIteratorAndBufferOffset = getPieceIteratorAndBufferOffset(offset);
    auto finalPieceIteratorAndBufferOffset = getPieceIteratorAndBufferOffset(offset + length);

    auto& initialAffectedPiece = *initialPieceIteratorAndBufferOffset.first;
    int initialBufferOffset = initialPieceIteratorAndBufferOffset.second;
    auto& finalAffectedPiece = *finalPieceIteratorAndBufferOffset.first;
    int finalBufferOffset = finalPieceIteratorAndBufferOffset.second;

    // delete at beginning or end of a single block
    if (initialAffectedPiece == finalAffectedPiece)
    {
        if (initialBufferOffset == std::get<1>(initialAffectedPiece))
        {
            std::get<1>(initialAffectedPiece) += length;
            std::get<2>(initialAffectedPiece) -= length;
            takeHistorySnapshot();
            return;
        }
        else if (finalBufferOffset == std::get<1>(initialAffectedPiece)
                                      + std::get<2>(initialAffectedPiece))
        {
            std::cout << "here " << offset << " " << length << "\n";
            std::flush(std::cout);
            std::get<2>(initialAffectedPiece) -= length;
            takeHistorySnapshot();
            return;
        }
    }

    std::vector<std::tuple<bool, int, int>> removePieces = {
std::make_tuple(std::get<0>(initialAffectedPiece),
                std::get<1>(initialAffectedPiece),
                initialBufferOffset - std::get<1>(initialAffectedPiece)),
std::make_tuple(std::get<0>(finalAffectedPiece),
                finalBufferOffset,
                std::get<2>(finalAffectedPiece) - (finalBufferOffset - std::get<1>(finalAffectedPiece)))
    };

    auto it = std::remove_if(removePieces.begin(), removePieces.end(),
                             [](std::tuple<bool, int, int> x) { return std::get<2>(x) <= 0; });

    pieces.insert(initialPieceIteratorAndBufferOffset.first, removePieces.begin(), it);
    pieces.erase(initialPieceIteratorAndBufferOffset.first,
                 std::next(finalPieceIteratorAndBufferOffset.first));
    takeHistorySnapshot();
}

std::string PieceTable::getText()
{
    std::string str;
    for (auto piece : pieces)
    {
        unsigned off = unsigned(std::get<1>(piece)),
                 len = unsigned(std::get<2>(piece));
        if (std::get<0>(piece))
            str += add.substr(off, len);
        else
            str += original.substr(off, len);
    }
    return str;
}

std::string PieceTable::getTextAt(int offset, int length)
{
    throw std::exception("Not Implemented");
}

int PieceTable::getLen()
{
    return len;
}

void PieceTable::historyForward()
{
    if (std::next(currentHistoryIt) == history.end()) return;

    currentHistoryIt++;
    restoreFromHistoryIterator();
}

void PieceTable::historyBackward()
{
    if (currentHistoryIt == history.begin()) return;

    currentHistoryIt--;
    restoreFromHistoryIterator();
}

void PieceTable::takeHistorySnapshot()
{
    history.erase(std::next(currentHistoryIt), history.end());
    history.push_back(std::make_pair(len, pieces));
    currentHistoryIt++;

    if (history.size() > MAX_HISTORY_CACHE_SIZE) history.pop_front();
}

void PieceTable::restoreFromHistoryIterator()
{
    len = currentHistoryIt->first;
    pieces = currentHistoryIt->second;
}

std::string PieceTable::dump()
{
    std::string str;
    str += "Original: " + original + "\n";
    str += "Add:      " + add + "\n";

    for (auto piece : pieces)
    {
        std::string tmp;
        tmp += (std::get<0>(piece) ? "True" : "False"); tmp += " ";
        tmp += std::to_string(std::get<1>(piece)); tmp += " ";
        tmp += std::to_string(std::get<2>(piece)); tmp += "\n";

        str += tmp;
    }

    return str;
}
