#pragma once

#include "common.h"

#include <memory>
#include <vector>

// A formula that allows you to evaluate and update an arithmetic expression
// Supported Features:
// * Simple binary operations and numbers, parentheses: 1+2*3, 2.5*(2+3.5/7)
// * Cell values as variables: A1+B2*C3
// Cells specified in a formula can be either formulas or text. 
// If it is text but it represents a number, then it should be treated as a number. 
// An empty cell or a cell with empty text is treated as the number zero.
class FormulaInterface {
public:
    using Value = std::variant<double, FormulaError>;

    virtual ~FormulaInterface() = default;

    // Returns the calculated formula value for the given table or an error
    virtual Value Evaluate(const SheetInterface& sheet) const = 0;

    // Returns an expression that describes the formula
    // Contains no spaces or extra parentheses
    virtual std::string GetExpression() const = 0;

    // Returns a sorted array of cells that are directly 
    // involved in the calculation of the formula.
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

// Parses the expression and returns a formula object
// Throws FormulaException if the formula is syntactically incorrect
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression);
