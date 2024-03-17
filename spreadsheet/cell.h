#pragma once

#include <optional>
#include <string>
#include <vector>
#include <unordered_set>

#include "common.h"
#include "formula.h"

class Cell final : public CellInterface {
public:

	Cell(SheetInterface& sheet);
	~Cell() = default;

	void Set(std::string text);
	void Clear();

	Value GetValue() const override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;

private:

	class Impl;
	class EmptyCell;
	class TextCell;
	class FormulaCell;

	SheetInterface& cell_sheet_;
	std::unique_ptr<Impl> value_ = nullptr;
	std::unordered_set<Cell*> parent_cells_;
	std::unordered_set<Cell*> child_cells_;

	void CheckCycles(const std::vector<Position>& parents, const Cell* root_cell);
	
	void ClearCaches();
	
	void EraseParents();
	void UpdateDependeces();

};

std::ostream& operator<<(std::ostream& output, const CellInterface::Value& val);

class Cell::Impl {
public:

	virtual ~Impl() = default;

	virtual Value GetValue(const SheetInterface& sheet) = 0;
	virtual std::string GetText() const = 0;
	virtual std::vector<Position> GetReferencedCells() const = 0;

	virtual void ClearCache() {}

};

class Cell::EmptyCell final : public Cell::Impl {
public:

	EmptyCell() = default;

	Cell::Value GetValue(const SheetInterface& sheet) override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;

};

class Cell::TextCell final : public Cell::Impl {
public:

	explicit TextCell(std::string text);

	Cell::Value GetValue(const SheetInterface& sheet) override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;

private:

	std::string value_;

};

class Cell::FormulaCell final : public Cell::Impl {
public:

	explicit FormulaCell(std::string text);

	Value GetValue(const SheetInterface& sheet) override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;

	void ClearCache() override;

private:

	std::unique_ptr<FormulaInterface> formula_;
	std::optional<FormulaInterface::Value> cache_;

};