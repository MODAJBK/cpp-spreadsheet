#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

//----------------FormulaError----------------

FormulaError::FormulaError(Category category)
    : category_(category) {
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

std::string_view FormulaError::ToString() const {
    using namespace std::string_view_literals;
    switch (category_) {
        case Category::Ref:
            return "#REF!"sv;
        case Category::Div0:
            return "#DIV/0!"sv;;
        case Category::Value:
            return "#VALUE!"sv;;
        default:
            return "#UNKNOWN ERROR!"sv;
    }
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

//------------------Formula------------------

namespace {
class Formula : public FormulaInterface {
public:

    explicit Formula(std::string expression) 
        try : ast_(ParseFormulaAST(expression)) {
        Position prev_cell = Position::NONE;
        for (const auto& curr_cell : ast_.GetCells()) {
            if (!(curr_cell == prev_cell)) {
                prev_cell = curr_cell;
                referenced_cells_.push_back(curr_cell);
            }
        }
        }
        catch (std::exception& exc) {
            throw FormulaException(exc.what());
        }

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute(sheet);
        }
        catch (FormulaError& error) {
            return error;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream ostr_stream;
        ast_.PrintFormula(ostr_stream);
        return ostr_stream.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        return referenced_cells_;
    }

private:

    FormulaAST ast_;
    std::vector<Position> referenced_cells_;

};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}