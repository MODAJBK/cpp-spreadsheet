#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::Sheet() = default;
Sheet::~Sheet() = default;

bool Sheet::CheckTableSize(Position pos) const {
    return pos.row >= static_cast<int>(sheet_.size()) || pos.col >= static_cast<int>(sheet_[pos.row].size());
}

void Sheet::Resize(Position pos) {
    if (pos.row >= static_cast<int>(sheet_.size())) {
        sheet_.resize(pos.row + 1);
    }
    if (pos.col >= static_cast<int>(sheet_[pos.row].size())) {
        sheet_[pos.row].resize(pos.col + 1);
    }
}

void Sheet::ChangeSize() {
    int max_col = -1;
    int max_row = -1;
    for (int row = 0; row < static_cast<int>(sheet_.size()); ++row) {
        for (int col = 0; col < static_cast<int>(sheet_[row].size()); ++col) {
            const auto cell = GetCell({ row, col });
            if (cell && !cell->GetText().empty()) {
                max_col = std::max(max_col, col);
                max_row = std::max(max_row, row);
            }
        }
    }
    printable_size_.rows = max_row + 1;
    printable_size_.cols = max_col + 1;
}

void Sheet::ChangePrintableSize(Position pos) {
    if (printable_size_.rows < pos.row + 1) {
        printable_size_.rows = pos.row + 1;
    }
    if (printable_size_.cols < pos.col + 1) {
        printable_size_.cols = pos.col + 1;
    }
}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    }
    if (const auto cell = GetCell(pos)) {
        if (cell->GetText() == text) {
            return;
        }
        sheet_[pos.row][pos.col]->Set(std::move(text));
        return;
    }
    if (!text.empty()) {
        ChangePrintableSize(pos);
    }
    Resize(pos);
    sheet_[pos.row][pos.col] = std::move(std::make_unique<Cell>(*this));
    sheet_[pos.row][pos.col]->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell posiiton");
    }
    if (CheckTableSize(pos)) {
        return nullptr;
    }
    return sheet_[pos.row][pos.col].get();
}

CellInterface* Sheet::GetCell(Position pos) {
    return const_cast<CellInterface*>(const_cast<const Sheet*>(this)->GetCell(pos));
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid cell position");
    }
    if (CheckTableSize(pos)) {
        return;
    }
    sheet_[pos.row][pos.col].reset();
    if (pos.row + 1 == printable_size_.rows || pos.col + 1 == printable_size_.cols) {
        ChangeSize();
    }
}

Size Sheet::GetPrintableSize() const {
    return printable_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < printable_size_.rows; ++row) {
        bool is_first = true;
        for (int col = 0; col < printable_size_.cols; ++col) {
            if (is_first) {
                is_first = false;
            }
            else {
                output << '\t';
            }
            if (const auto cell = GetCell({ row, col })) {
                output << sheet_[row][col]->GetValue();
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < printable_size_.rows; ++row) {
        bool is_first = true;
        for (int col = 0; col < printable_size_.cols; ++col) {
            if (is_first) {
                is_first = false;
            }
            else {
                output << '\t';
            }
            if (const auto cell = GetCell({ row, col })) {
                output << sheet_[row][col]->GetText();
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}