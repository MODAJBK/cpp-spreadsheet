#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>

//-----------------Cell-----------------

Cell::Cell(SheetInterface& sheet)
	: cell_sheet_(sheet)
    , value_(std::move(std::make_unique<EmptyCell>())) {
}


void Cell::Set(std::string text) {
	if (text == value_->GetText()) {
		return;
	}
	if (text.size() > 0) {
		if (text.size() > 1 && text.front() == FORMULA_SIGN) {
			auto tmp_value = std::make_unique<Cell::FormulaCell>(std::move(text.substr(1)));
			CheckCycles(tmp_value->GetReferencedCells(), this);
			value_ = std::move(tmp_value);
		}
		else {
			value_ = std::make_unique<TextCell>(std::move(text));
		}
	}
	else {
		value_ = std::make_unique<EmptyCell>();
	}
	EraseParents();
	UpdateDependeces();
	ClearCaches();
}

void Cell::Clear() {
	value_ = std::move(std::make_unique<EmptyCell>());
	EraseParents();
	ClearCaches();
}

Cell::Value Cell::GetValue() const {
	return value_->GetValue(cell_sheet_);
}

std::string Cell::GetText() const {
	return value_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return value_->GetReferencedCells();
}

void Cell::CheckCycles(const std::vector<Position>& parents, const Cell* root_cell) {
	for (const auto& parent_cell : parents) {
		auto parent = dynamic_cast<Cell*>(cell_sheet_.GetCell(parent_cell));
		if (parent) {
			if (parent == root_cell) {
				throw CircularDependencyException("Cycle link between cells detected");
			}
			parent->CheckCycles(parent->GetReferencedCells(), root_cell);
		}
	}
}

void Cell::ClearCaches() {
	value_->ClearCache();
	for (const auto child_cell : child_cells_) {
		child_cell->ClearCaches();
	}
}

void Cell::UpdateDependeces() {
	for (const auto& parent_cell : GetReferencedCells()) {
		if (cell_sheet_.GetCell(parent_cell) == nullptr) {
			cell_sheet_.SetCell(parent_cell, {});
		}
		auto parent = dynamic_cast<Cell*>(cell_sheet_.GetCell(parent_cell));
		parent->child_cells_.insert(this);
		parent_cells_.insert(parent);
	}
}

void Cell::EraseParents() {
	for (const auto parent_cell : parent_cells_) {
		parent_cell->child_cells_.erase(this);
	}
	parent_cells_.clear();
}


std::ostream& operator<<(std::ostream& output, const CellInterface::Value& val) {
	std::visit([&output](const auto& v) {
		output << v;
		}, val);
	return output;
}

//-----------------EmptyCell-----------------

Cell::Value Cell::EmptyCell::GetValue(const SheetInterface& sheet) {
	return std::string();
}

std::string Cell::EmptyCell::GetText() const {
	return std::string();
}

std::vector<Position> Cell::EmptyCell::GetReferencedCells() const {
	return {};
}

//-----------------TextCell-----------------

Cell::TextCell::TextCell(std::string text)
	: value_(std::move(text)) {
}

Cell::Value Cell::TextCell::GetValue(const SheetInterface& sheet) {
	return value_.front() == ESCAPE_SIGN ? value_.substr(1) : value_;
}

std::string Cell::TextCell::GetText() const {
	return value_;
}

std::vector<Position> Cell::TextCell::GetReferencedCells() const {
	return {};
}

//-----------------FormulaCell-----------------

Cell::FormulaCell::FormulaCell(std::string text)
	: formula_(std::move(ParseFormula(text))) {
}

Cell::Value Cell::FormulaCell::GetValue(const SheetInterface& sheet)  {
	if (!cache_) {
		cache_ = formula_->Evaluate(sheet);
	}
	return std::visit([](auto val) { return CellInterface::Value(val); }, cache_.value());
}

std::string Cell::FormulaCell::GetText() const {
	return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaCell::GetReferencedCells() const {
	return formula_->GetReferencedCells();
}

void Cell::FormulaCell::ClearCache() {
	cache_.reset();
}