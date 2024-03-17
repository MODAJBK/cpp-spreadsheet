#pragma once

#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// Cell position. Indexation from zero
struct Position {
    int row = 0;
    int col = 0;

    bool operator==(Position rhs) const;
    bool operator<(Position rhs) const;

    bool IsValid() const;
    std::string ToString() const;

    static Position FromString(std::string_view str);

    static const int MAX_ROWS = 16384;
    static const int MAX_COLS = 16384;
    static const Position NONE;
};

struct Size {
    int rows = 0;
    int cols = 0;

    bool operator==(Size rhs) const;
};

// Class describes errors that may occurre during formula evaluation
class FormulaError {
public:
    enum class Category {
        Ref,    // reference to a cell with an incorrect position
        Value,  // cell cannot be treated as a number
        Div0,  // as a result of the calculation, division by zero occurred
    };

    FormulaError(Category category);

    Category GetCategory() const;

    bool operator==(FormulaError rhs) const;

    std::string_view ToString() const;

private:
    Category category_;
};

std::ostream& operator<<(std::ostream& output, FormulaError fe);

// Exception thrown when trying to pass an incorrect position
class InvalidPositionException : public std::out_of_range {
public:
    using std::out_of_range::out_of_range;
};

// Exception thrown when trying to specify a syntactically incorrect formula
class FormulaException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Exception thrown when trying to set a formula that results in a circular 
// dependency between cells
class CircularDependencyException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Class describes Cell interface
class CellInterface {
public:
    // Cell can contain rather text, formula result 
    // or formula error message
    using Value = std::variant<std::string, double, FormulaError>;

    virtual ~CellInterface() = default;

    virtual Value GetValue() const = 0;

    virtual std::string GetText() const = 0;

    virtual std::vector<Position> GetReferencedCells() const = 0;
};

inline constexpr char FORMULA_SIGN = '=';
inline constexpr char ESCAPE_SIGN = '\'';

// Class describes Table interface
class SheetInterface {
public:
    virtual ~SheetInterface() = default;

    // Set the contents of a cell.
    // Clarifications on writing the formula:
    // * If the text contains only the "=" symbol and nothing else, 
    // then it is not considered a formula
    // * If the text begins with the character "'" (apostrophe), 
    // when displaying the cell value using the GetValue() method, it is omitted. 
    // Can be used if you need to start text with an "=" sign, 
    // but so that it is not interpreted as a formula
    virtual void SetCell(Position pos, std::string text) = 0;

    // Return the Cell value in position "pos" 
    // If cell is empty return nullptr
    virtual const CellInterface* GetCell(Position pos) const = 0;
    virtual CellInterface* GetCell(Position pos) = 0;

    // Clear the cell in position "por".
    // A subsequent GetCell() run on this cell will return 
    // either a nullptr or an object with empty text
    virtual void ClearCell(Position pos) = 0;

    // Calculates the size of the area that is involved in printing
    virtual Size GetPrintableSize() const = 0;

    // Outputs the entire table to the passed ostream
    virtual void PrintValues(std::ostream& output) const = 0;
    virtual void PrintTexts(std::ostream& output) const = 0;
};

// Create an empty table
std::unique_ptr<SheetInterface> CreateSheet();