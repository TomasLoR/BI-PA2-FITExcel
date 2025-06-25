#ifndef __PROGTEST__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <climits>
#include <cfloat>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <variant>
#include <optional>
#include <compare>
#include <charconv>
#include <span>
#include <utility>
#include "expression.h"

using namespace std::literals;
using CValue = std::variant<std::monostate, double, std::string>;

constexpr unsigned SPREADSHEET_CYCLIC_DEPS = 0x01;
constexpr unsigned SPREADSHEET_FUNCTIONS = 0x02;
constexpr unsigned SPREADSHEET_FILE_IO = 0x04;
constexpr unsigned SPREADSHEET_SPEED = 0x08;
constexpr unsigned SPREADSHEET_PARSER = 0x10;
#endif /* __PROGTEST__ */


/** Class representing a position in the spreadsheet */
class CPos {

public:

    CPos(std::string_view str) {
        if (!setRowCol(str)) {
            throw std::invalid_argument("Invalid position");
        }
    };

    CPos(size_t col, size_t row) : m_col(col), m_row(row) {};

    /** Method for updating the position
     * @param[in] offset - offset to be added to the position
    */
    void updatePos(const std::pair<int, int> &offset);

    /** Method for converting the position represented by size_t to a string
     * @param[out] os - output stream
    */
    void toStr(std::ostream &os) const;

    /** Method for comparing two positions
     * @param[in] other - position to be compared with
     * @return true if the position is less than the other position, false otherwise
    */
    bool operator<(const CPos &other) const;

    /** Method for comparing two positions
     * @param[in] other - position to be compared with
     * @return true if the positions are equal, false otherwise
    */
    bool operator==(const CPos &other) const;

    /** Method for getting the column
     * @return column of the position
    */
    size_t getCol() const;

    /** Method for getting the row
     * @return row of the position
    */
    size_t getRow() const;

private:

    size_t m_col;
    size_t m_row;
    bool m_absRow = false;
    bool m_absCol = false;

    /** Method for setting the row and column
     * @param[in] str - string representing the position
     * @return true if the position is valid, false otherwise
    */
    bool setRowCol(const std::string_view &str);

    /** Static method for converting a string to a size_t number
     * @param[in] str - string to be converted
     * @param[out] i - index of the string
     * @return number represented by the string
    */
    static size_t toNum(const std::string_view &str, size_t &i) ;

    /** Method for processing the column
     * @param[in] str - string representing the position
     * @param[out] i - index of the string
     * @return true if the column is valid, false otherwise
    */
    bool processCol(const std::string_view &str, size_t &i);

    /** Method for processing the row
     * @param[in] str - string representing the position
     * @param[out] i - index of the string
     * @return true if the row is valid, false otherwise
    */
    bool processRow(const std::string_view &str, size_t &i);

};

void CPos::updatePos(const std::pair<int, int> &offset) {
    if (!m_absCol) {
        m_col += offset.first;
    }

    if (!m_absRow) {
        m_row += offset.second;
    }
}

void CPos::toStr(std::ostream &os) const {
    if (m_absCol) {
        os << "$";
    }

    std::string str;
    size_t num = m_col;
    while (num > 0) {
        char ch = 'A' + (num - 1) % 26;
        str = ch + str;
        num = (num - 1) / 26;
    }
    os << str;

    if (m_absRow) {
        os << "$";
    }
    os << std::to_string(m_row);
}

bool CPos::operator<(const CPos &other) const {
    return std::tie(m_col, m_row) < std::tie(other.m_col, other.m_row);
}

bool CPos::operator==(const CPos &other) const {
    return std::tie(m_col, m_row) == std::tie(other.m_col, other.m_row);
}

size_t CPos::getCol() const {
    return m_col;
}

size_t CPos::getRow() const {
    return m_row;
}

bool CPos::setRowCol(const std::string_view &str) {
    if (str.size() < 2) {
        return false;
    }

    size_t i = 0;
    if (!processCol(str, i) || !processRow(str, i)) {
        return false;
    }

    return true;
}

size_t CPos::toNum(const std::string_view &str, size_t &i) {
    size_t num = 0;
    while (i < str.size() && std::isalpha(str[i])) {
        num = num * 26 + (std::toupper(str[i]) - 'A' + 1);
        i++;
    }

    return num;
}

bool CPos::processCol(const std::string_view &str, size_t &i) {
    if (str[i] == '$') {
        m_absCol = true;
        i++;
    }

    if (i == str.size() || !std::isalpha(str[i])) {
        return false;
    }

    m_col = toNum(str, i);

    return true;
}

bool CPos::processRow(const std::string_view &str, size_t &i) {
    if (str[i] == '$') {
        m_absRow = true;
        i++;
    }

    if (i == str.size() || !std::isdigit(str[i])) {
        return false;
    }

    std::string numbers(str);
    numbers = numbers.substr(i);

    try {
        m_row = std::stoull(numbers);
    } catch (const std::invalid_argument &e) {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------

/** Base class representing an expression node */
class CNode {

public:

    virtual ~CNode() = default;

    /** Method for evaluating the node
     * @param[in] visited - set of visited nodes
     * @return value of the node
    */
    virtual CValue evaluate(std::set<CPos> &visited) const = 0;

    /** Method for cloning the node
     * @param[in] map - map of the nodes
     * @return shared pointer to the cloned node
    */
    virtual std::shared_ptr<CNode> clone(const std::map<CPos, std::shared_ptr<CNode>> &map) const = 0;

    /** Method for updating the reference
     * @param[in] offset - offset to be added to the position
    */
    virtual void updateRef(std::pair<int, int> offset) = 0;

    /** Method for saving the node
     * @param[out] os - output stream
    */
    virtual void save(std::ostream &os) const = 0;

    /** Method for checking if the node is an expression
     * @return true if the node is an expression, false otherwise
    */
    bool isExpr() const;

    /** Method for setting the node as an expression */
    void setExpr();


protected:
    bool m_expr = false;
};

bool CNode::isExpr() const {
    return m_expr;
}

void CNode::setExpr() {
    m_expr = true;
}

using ANode = std::shared_ptr<CNode>;

/** Derived class from Node representing a CValue node*/
class CValueNode : public CNode {

public:
    CValueNode(const CValue &val) : m_val(val) {};

    CValueNode(const CValue &val, const std::string &str) : m_val(val), m_valToSave(str), m_exprStr(true) {}

    CValue evaluate(std::set<CPos> &visited) const override {
        return m_val;
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<CValueNode> tmp = std::make_shared<CValueNode>(m_val);
        tmp->m_exprStr = m_exprStr;
        tmp->m_valToSave = m_valToSave;
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {}

    void save(std::ostream &os) const override {
        if (m_val.index() == 1) {
            os << std::to_string(std::get<double>(m_val));
        } else {
            if (m_expr) {
                os << m_valToSave;
            } else {
                os << std::get<std::string>(m_val);
            }
        }
    }

private:
    CValue m_val;
    std::string m_valToSave;
    bool m_exprStr = false;

};

/** Derived class from Node representing a reference node */
class CRefNode : public CNode {

public:
    CRefNode(const CPos &pos, const std::map<CPos, ANode> &map) : m_pos(pos), m_map(map) {}

    CValue evaluate(std::set<CPos> &visited) const override {
        if (m_map.find(m_pos) == m_map.end() || visited.count(m_pos) > 0) {
            return {};
        }

        visited.emplace(m_pos);
        CValue result = m_map.at(m_pos)->evaluate(visited);
        visited.erase(m_pos);

        return result;
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<CRefNode> tmp = std::make_shared<CRefNode>(m_pos, map);
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_pos.updatePos(offset);
    }

    void save(std::ostream &os) const override {
        m_pos.toStr(os);
    }

private:
    CPos m_pos;
    const std::map<CPos, ANode> &m_map;

};

/** Derived class from Node representing an addition node */
class opAddNode : public CNode {

public:
    opAddNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);

        if (val1.index() == 1 && val2.index() == 1) {
            return std::get<double>(val1) + std::get<double>(val2);
        } else if (val1.index() == 2 && val2.index() == 2) {
            return std::get<std::string>(val1) + std::get<std::string>(val2);
        } else if (val1.index() == 1 && val2.index() == 2) {
            return std::to_string(std::get<double>(val1)) + std::get<std::string>(val2);
        } else if (val1.index() == 2 && val2.index() == 1) {
            return std::get<std::string>(val1) + std::to_string(std::get<double>(val2));
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opAddNode> tmp = std::make_shared<opAddNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << "+";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

/** Derived class from Node representing a subtraction node */
class opSubNode : public CNode {

public:
    opSubNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {

        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);
        if (val1.index() == 1 && val2.index() == 1) {
            return std::get<double>(val1) - std::get<double>(val2);
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opSubNode> tmp = std::make_shared<opSubNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << "-";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

/** Derived class from Node representing a multiplication node */
class opMulNode : public CNode {

public:
    opMulNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);

        if (val1.index() == 1 && val2.index() == 1) {
            return std::get<double>(val1) * std::get<double>(val2);
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opMulNode> tmp = std::make_shared<opMulNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;
        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << "*";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

/** Derived class from Node representing a division node */
class opDivNode : public CNode {

public:
    opDivNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);

        if (val1.index() == 1 && val2.index() == 1 && std::get<double>(val2) != 0) {
            return std::get<double>(val1) / std::get<double>(val2);
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opDivNode> tmp = std::make_shared<opDivNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << "/";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

/** Derived class from Node representing a power node */
class opPowNode : public CNode {

public:
    opPowNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);

        if (val1.index() == 1 && val2.index() == 1) {
            return std::pow(std::get<double>(val1), std::get<double>(val2));
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opPowNode> tmp = std::make_shared<opPowNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << "^";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

/** Derived class from Node representing a negation node */
class opNegNode : public CNode {

public:
    opNegNode(const ANode &left) : m_left(left) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val = m_left->evaluate(visited);

        if (val.index() == 1) {
            return -std::get<double>(val);
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opNegNode> tmp = std::make_shared<opNegNode>(m_left->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        os << "-";
        m_left->save(os);
        os << ")";
    }

private:
    ANode m_left;

};

/** Derived class from Node representing an equality node */
class opEqNode : public CNode {

public:
    opEqNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);

        if (val1.index() == 1 && val2.index() == 1) {
            return double(std::get<double>(val1) == std::get<double>(val2));
        } else if (val1.index() == 2 && val2.index() == 2) {
            return double(std::get<std::string>(val1) == std::get<std::string>(val2));
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opEqNode> tmp = std::make_shared<opEqNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << "=";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

/** Derived class from Node representing a non-equality node */
class opNeNode : public CNode {

public:
    opNeNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);

        if (val1.index() == 1 && val2.index() == 1) {
            return double(std::get<double>(val1) != std::get<double>(val2));
        } else if (val1.index() == 2 && val2.index() == 2) {
            return double(std::get<std::string>(val1) != std::get<std::string>(val2));
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opNeNode> tmp = std::make_shared<opNeNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << "<>";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

/** Derived class from Node representing a less than node */
class opLtNode : public CNode {

public:
    opLtNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);

        if (val1.index() == 1 && val2.index() == 1) {
            return double(std::get<double>(val1) < std::get<double>(val2));
        } else if (val1.index() == 2 && val2.index() == 2) {
            return double(std::get<std::string>(val1) < std::get<std::string>(val2));
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opLtNode> tmp = std::make_shared<opLtNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << "<";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

/** Derived class from Node representing a less than or equal node */
class opLeNode : public CNode {

public:
    opLeNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);

        if (val1.index() == 1 && val2.index() == 1) {
            return double(std::get<double>(val1) <= std::get<double>(val2));
        } else if (val1.index() == 2 && val2.index() == 2) {
            return double(std::get<std::string>(val1) <= std::get<std::string>(val2));
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opLeNode> tmp = std::make_shared<opLeNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << "<=";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

/** Derived class from Node representing a greater than node */
class opGtNode : public CNode {

public:
    opGtNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);

        if (val1.index() == 1 && val2.index() == 1) {
            return double(std::get<double>(val1) > std::get<double>(val2));
        } else if (val1.index() == 2 && val2.index() == 2) {
            return double(std::get<std::string>(val1) > std::get<std::string>(val2));
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opGtNode> tmp = std::make_shared<opGtNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << ">";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

/** Derived class from Node representing a greater than or equal node */
class opGeNode : public CNode {

public:
    opGeNode(const ANode &left, const ANode &right) : m_left(left), m_right(right) {};

    CValue evaluate(std::set<CPos> &visited) const override {
        CValue val1 = m_left->evaluate(visited);
        CValue val2 = m_right->evaluate(visited);

        if (val1.index() == 1 && val2.index() == 1) {
            return double(std::get<double>(val1) >= std::get<double>(val2));
        } else if (val1.index() == 2 && val2.index() == 2) {
            return double(std::get<std::string>(val1) >= std::get<std::string>(val2));
        } else {
            return {};
        }
    }

    std::shared_ptr<CNode> clone(const std::map<CPos, ANode> &map) const override {
        std::shared_ptr<opGeNode> tmp = std::make_shared<opGeNode>(m_left->clone(map), m_right->clone(map));
        tmp->m_expr = m_expr;

        return tmp;
    }

    void updateRef(std::pair<int, int> offset) override {
        m_left->updateRef(offset);
        m_right->updateRef(offset);
    }

    void save(std::ostream &os) const override {
        os << "(";
        m_left->save(os);
        os << "<=";
        m_right->save(os);
        os << ")";
    }

private:
    ANode m_left;
    ANode m_right;

};

//----------------------------------------------------------------------------------------------------------------------

/** Derived class from CExprBuilder representing my expression builder */
class CMyExprBuilder : public CExprBuilder {
public:

    CMyExprBuilder() = default;

    CMyExprBuilder(const CMyExprBuilder &other);

    CMyExprBuilder &operator=(const CMyExprBuilder &other);

    /** Method for creating an addition node */
    void opAdd() override;

    /** Method for creating a subtraction node */
    void opSub() override;

    /** Method for creating a multiplication node */
    void opMul() override;

    /** Method for creating a division node */
    void opDiv() override;

    /** Method for creating a power node */
    void opPow() override;

    /** Method for creating a negation node */
    void opNeg() override;

    /** Method for creating an equality node */
    void opEq() override;

    /** Method for creating a non-equality node */
    void opNe() override;

    /** Method for creating a less than node */
    void opLt() override;

    /** Method for creating a less than or equal node */
    void opLe() override;

    /** Method for creating a greater than node */
    void opGt() override;

    /** Method for creating a greater than or equal node */
    void opGe() override;

    /** Method creating CValueNode with a number
     * @param[in] val - number to be stored in the node
    */
    void valNumber(double val) override;

    /** Method creating CValueNode with a string
     * @param[in] val - string to be stored in the node
    */
    void valString(std::string val) override;

    /** Method creating CRefNode with a reference to another cell
     * @param[in] val - reference to be stored in the node
    */
    void valReference(std::string val) override;

    void valRange(std::string val) override {}

    void funcCall(std::string fnName, int paramCount) override {}

    /** Method for getting the value of a cell
     * @param[in] pos - position of the cell
     * @return value of the cell
    */
    CValue getVal(const CPos &pos) const;

    /** Method for updating the nodes
     * @param[in] pos - position of the cell
     * @param[in] contents - contents of the cell
    */
    void updateNodes(const CPos &pos, const std::string &contents);

    /** Method for checking if a node exists
     * @param[in] pos - position of the cell
     * @return true if the node exists, false otherwise
    */
    bool nodeExists(const CPos &pos) const;

    /** Method for adding a CValueNode to the map
     * @param[in] pos - position of the cell
     * @param[in] val - value of the cell
    */
    void addCValNode(const CPos &pos, const CValue &val);

    /** Method for adding a node to the map
     * @param[in] pos - position of the cell
     * @param[in] tmp - reference to the cell
    */
    void addNode(const CPos &dst, const ANode &tmp);

    /** Method for updating the references
     * @param[in] pos - position of the cell
     * @param[in] offset - offset to be added to the position
    */
    void callUpdateRef(const CPos &pos, const std::pair<int, int> &offset);

    /** Method for getting the nodes
     * @return map of the nodes
    */
    const std::map<CPos, ANode> &getNodes() const;

private:

    std::stack<ANode> m_stack;
    std::map<CPos, ANode> m_nodes;

    /** Static method for adding double quotes to a string
     * @param[out] val - string to be modified
    */
    static void doubleQuotes(std::string &val);
};

CMyExprBuilder::CMyExprBuilder(const CMyExprBuilder &other) {
    for (const auto &pair: other.m_nodes) {
        m_nodes[pair.first] = pair.second->clone(m_nodes);
    }
}

CMyExprBuilder &CMyExprBuilder::operator=(const CMyExprBuilder &other) {
    if (this != &other) {
        m_nodes.clear();
        for (const auto &pair: other.m_nodes) {
            m_nodes[pair.first] = pair.second->clone(m_nodes);
        }
    }

    return *this;
}

void CMyExprBuilder::opAdd() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot add only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opAddNode>(left, right));
}

void CMyExprBuilder::opSub() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot subtract only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opSubNode>(left, right));
}

void CMyExprBuilder::opMul() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot multiply only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opMulNode>(left, right));
}

void CMyExprBuilder::opDiv() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot divide only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opDivNode>(left, right));
}

void CMyExprBuilder::opPow() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot power only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opPowNode>(left, right));
}

void CMyExprBuilder::opNeg() {
    if (m_stack.empty()) {
        throw std::invalid_argument("Cannot negate only zero elements");
    }

    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opNegNode>(left));
}

void CMyExprBuilder::opEq() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot compare only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opEqNode>(left, right));
}

void CMyExprBuilder::opNe() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot compare only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();

    m_stack.push(std::make_shared<opNeNode>(left, right));

}

void CMyExprBuilder::opLt() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot compare only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opLtNode>(left, right));
}

void CMyExprBuilder::opLe() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot compare only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opLeNode>(left, right));
}

void CMyExprBuilder::opGt() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot compare only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opGtNode>(left, right));
}

void CMyExprBuilder::opGe() {
    if (m_stack.size() < 2) {
        throw std::invalid_argument("Cannot compare only one element");
    }

    ANode right = m_stack.top();
    m_stack.pop();
    ANode left = m_stack.top();
    m_stack.pop();
    m_stack.push(std::make_shared<opGeNode>(left, right));
}

void CMyExprBuilder::valReference(std::string val) {
    m_stack.emplace(std::make_shared<CRefNode>(CPos(val), m_nodes));
}

void CMyExprBuilder::valNumber(double val) {
    m_stack.emplace(std::make_shared<CValueNode>(CValue(val)));
}

void CMyExprBuilder::valString(std::string val) {
    std::string valToSave = val;
    doubleQuotes(valToSave);
    m_stack.emplace(std::make_shared<CValueNode>(CValue(val), valToSave));
}


CValue CMyExprBuilder::getVal(const CPos &pos) const {
    std::set<CPos> visited;
    visited.emplace(pos);

    return m_nodes.at(pos)->evaluate(visited);
}

void CMyExprBuilder::updateNodes(const CPos &pos, const std::string &contents) {
    if (m_stack.size() != 1) {
        throw std::invalid_argument("Stack size is not 1 when updating nodes");
    }

    m_nodes[pos] = std::move(m_stack.top());
    m_stack.pop();
    m_nodes[pos]->setExpr();
}

bool CMyExprBuilder::nodeExists(const CPos &pos) const {
    if (m_nodes.find(pos) == m_nodes.end()) {
        return false;
    }

    return true;
}

void CMyExprBuilder::addCValNode(const CPos &pos, const CValue &val) {
    m_nodes[pos] = std::make_shared<CValueNode>(val);

}

void CMyExprBuilder::addNode(const CPos &dst, const ANode &tmp) {
    m_nodes[dst] = tmp->clone(m_nodes);
}

void CMyExprBuilder::callUpdateRef(const CPos &pos, const std::pair<int, int> &offset) {
    m_nodes[pos]->updateRef(offset);
}

const std::map<CPos, ANode> &CMyExprBuilder::getNodes() const {
    return m_nodes;
}

void CMyExprBuilder::doubleQuotes(std::string &str) {
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '"') {
            str.insert(i, 1, '"');
            i++;
        }
    }
    str.insert(str.begin(), '"');
    str.push_back('"');
}

//----------------------------------------------------------------------------------------------------------------------

/** Class represenring an excel-like spreadsheet */
class CSpreadsheet {
public:
    static unsigned capabilities() {
        return SPREADSHEET_CYCLIC_DEPS;
    }

    CSpreadsheet() = default;

    /** Method for loading the spreadsheet from a stream
     * @param[out] is input stream
     * @return true if the loading was successful, false otherwise
    */
    bool load(std::istream &is);

    /** Method for saving the spreadsheet to a stream
     * @param[out] os output stream
     * @return true if the saving was successful
    */
    bool save(std::ostream &os) const;

    /** Method for setting the contents of a cell
     * @param[in] pos position of the cell
     * @param[in] contents contents of the cell
     * @return true if the setting was successful, false otherwise
    */
    bool setCell(CPos pos, std::string contents);

    /** Method for getting the value of a cell
     * @param[in] pos position of the cell
     * @return value of the cell
    */
    CValue getValue(CPos pos);

    /** Method for copying a rectangle of cells
     * @param[in] dst position of the top-left corner of the destination rectangle
     * @param[in] src position of the top-left corner of the source rectangle
     * @param[in] w width of the rectangle
     * @param[in] h height of the rectangle
    */
    void copyRect(CPos dst, CPos src, int w = 1, int h = 1);

private:
    CMyExprBuilder m_builder;

    /** Method for cloning the nodes from the source rectangle
     * @param[in] src position of the top-left corner of the source rectangle
     * @param[in] w width of the rectangle
     * @param[in] h height of the rectangle
     * @param[out] tmp map of the cloned nodes
    */
    void cloneSourceNodes(const CPos &src, int w, int h, std::map<CPos, ANode> &tmp) const;

};

bool CSpreadsheet::save(std::ostream &os) const {
    char delim = '~';
    std::map<CPos, ANode> nodes = m_builder.getNodes();

    for (const auto &pair: nodes) {
        os << pair.first.getCol() << " " << pair.first.getRow() << " ";
        if (nodes.at(pair.first)->isExpr()) {
            os << "=";
        }
        pair.second->save(os);
        os << delim;
    }

    return true;
}

bool CSpreadsheet::load(std::istream &is) {
    m_builder = CMyExprBuilder();

    std::string line;
    while (std::getline(is, line, '~')) {
        line.append("~");
        std::istringstream iss(line);
        size_t col, row;
        std::string contents;
        if (!(iss >> col >> row)) {
            return false;
        }
        std::getline(iss, contents, '~');
        contents = contents.substr(1);

        this->setCell(CPos(col, row), contents);
    }

    return true;
}

bool CSpreadsheet::setCell(CPos pos, std::string contents) {
    if (contents[0] == '=') {
        try {
            parseExpression(contents, m_builder);
            m_builder.updateNodes(pos, contents);
        } catch (std::invalid_argument &) {
            return false;
        }

    } else {
        try {
            double num = std::stod(contents);
            m_builder.addCValNode(pos, num);
        } catch (std::invalid_argument &) {
            m_builder.addCValNode(pos, contents);
        }

    }

    return true;
}

CValue CSpreadsheet::getValue(CPos pos) {
    if (!m_builder.nodeExists(pos)) {
        return {};
    }

    return m_builder.getVal(pos);
}

void CSpreadsheet::copyRect(CPos dst, CPos src, int w, int h) {
    std::pair<int, int> offset = {dst.getCol() - src.getCol(), dst.getRow() - src.getRow()};
    std::map<CPos, ANode> tmp;
    cloneSourceNodes(src, w, h, tmp);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            const CPos srcPos = CPos(src.getCol() + x, src.getRow() + y);
            const CPos dstPos = CPos(dst.getCol() + x, dst.getRow() + y);

            if (m_builder.nodeExists(srcPos)) {
                m_builder.addNode(dstPos, tmp[srcPos]);
                m_builder.callUpdateRef(dstPos, offset);
            }
        }
    }
}

void CSpreadsheet::cloneSourceNodes(const CPos &src, int w, int h, std::map<CPos, ANode> &tmp) const {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            const CPos srcPos = CPos(src.getCol() + x, src.getRow() + y);

            if (m_builder.nodeExists(srcPos)) {
                tmp[srcPos] = m_builder.getNodes().at(srcPos)->clone(m_builder.getNodes());
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

#ifndef __PROGTEST__

bool valueMatch(const CValue &r,
                const CValue &s) {
    if (r.index() != s.index())
        return false;
    if (r.index() == 0)
        return true;
    if (r.index() == 2)
        return std::get<std::string>(r) == std::get<std::string>(s);
    if (std::isnan(std::get<double>(r)) && std::isnan(std::get<double>(s)))
        return true;
    if (std::isinf(std::get<double>(r)) && std::isinf(std::get<double>(s)))
        return (std::get<double>(r) < 0 && std::get<double>(s) < 0)
               || (std::get<double>(r) > 0 && std::get<double>(s) > 0);
    return fabs(std::get<double>(r) - std::get<double>(s)) <= 1e8 * DBL_EPSILON * fabs(std::get<double>(r));
}

int main() {
    CSpreadsheet x0, x1;
    std::ostringstream oss;
    std::istringstream iss;
    std::string data;
    assert (x0.setCell(CPos("A1"), "10"));
    assert (x0.setCell(CPos("A2"), "20.5"));
    assert (x0.setCell(CPos("A3"), "3e1"));
    assert (x0.setCell(CPos("A4"), "=40"));
    assert (x0.setCell(CPos("A5"), "=5e+1"));
    assert (x0.setCell(CPos("A6"), "raw text with any characters, including a quote \" or a newline\n"));
    assert (x0.setCell(CPos("A7"),
                       "=\"quoted string, quotes must be doubled: \"\". Moreover, backslashes are needed for C++.\""));
    assert (valueMatch(x0.getValue(CPos("A1")), CValue(10.0)));
    assert (valueMatch(x0.getValue(CPos("A2")), CValue(20.5)));
    assert (valueMatch(x0.getValue(CPos("A3")), CValue(30.0)));
    assert (valueMatch(x0.getValue(CPos("A4")), CValue(40.0)));
    assert (valueMatch(x0.getValue(CPos("A5")), CValue(50.0)));
    assert (valueMatch(x0.getValue(CPos("A6")),
                       CValue("raw text with any characters, including a quote \" or a newline\n")));
    assert (valueMatch(x0.getValue(CPos("A7")),
                       CValue("quoted string, quotes must be doubled: \". Moreover, backslashes are needed for C++.")));
    assert (valueMatch(x0.getValue(CPos("A8")), CValue()));
    assert (valueMatch(x0.getValue(CPos("AAAA9999")), CValue()));
    assert (x0.setCell(CPos("B1"), "=A1+A2*A3"));
    assert (x0.setCell(CPos("B2"), "= -A1 ^ 2 - A2 / 2   "));
    assert (x0.setCell(CPos("B3"), "= 2 ^ $A$1"));
    assert (x0.setCell(CPos("B4"), "=($A1+A$2)^2"));
    assert (x0.setCell(CPos("B5"), "=B1+B2+B3+B4"));
    assert (x0.setCell(CPos("B6"), "=B1+B2+B3+B4+B5"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(625.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-110.25)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(1024.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(930.25)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(2469.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(4938.0)));
    assert (x0.setCell(CPos("A1"), "12"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(627.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-154.25)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(1056.25)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(5625.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(11250.0)));
    x1 = x0;
    assert (x0.setCell(CPos("A2"), "100"));
    assert (x1.setCell(CPos("A2"), "=A3+A5+A4"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(3012.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-194.0)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(12544.0)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(19458.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(38916.0)));
    assert (valueMatch(x1.getValue(CPos("B1")), CValue(3612.0)));
    assert (valueMatch(x1.getValue(CPos("B2")), CValue(-204.0)));
    assert (valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x1.getValue(CPos("B4")), CValue(17424.0)));
    assert (valueMatch(x1.getValue(CPos("B5")), CValue(24928.0)));
    assert (valueMatch(x1.getValue(CPos("B6")), CValue(49856.0)));
    oss.clear();
    oss.str("");
    assert (x0.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    assert (valueMatch(x1.getValue(CPos("B1")), CValue(3012.0)));
    assert (valueMatch(x1.getValue(CPos("B2")), CValue(-194.0)));
    assert (valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x1.getValue(CPos("B4")), CValue(12544.0)));
    assert (valueMatch(x1.getValue(CPos("B5")), CValue(19458.0)));
    assert (valueMatch(x1.getValue(CPos("B6")), CValue(38916.0)));
    assert (x0.setCell(CPos("A3"), "4e1"));
    assert (valueMatch(x1.getValue(CPos("B1")), CValue(3012.0)));
    assert (valueMatch(x1.getValue(CPos("B2")), CValue(-194.0)));
    assert (valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x1.getValue(CPos("B4")), CValue(12544.0)));
    assert (valueMatch(x1.getValue(CPos("B5")), CValue(19458.0)));
    assert (valueMatch(x1.getValue(CPos("B6")), CValue(38916.0)));
    oss.clear();
    oss.str("");
    assert (x0.save(oss));
    data = oss.str();
    for (size_t i = 0; i < std::min<size_t>(data.length(), 10); i++)
        data[i] ^= 0x5a;
    iss.clear();
    iss.str(data);
    assert (!x1.load(iss));
    assert (x0.setCell(CPos("D0"), "10"));
    assert (x0.setCell(CPos("D1"), "20"));
    assert (x0.setCell(CPos("D2"), "30"));
    assert (x0.setCell(CPos("D3"), "40"));
    assert (x0.setCell(CPos("D4"), "50"));
    assert (x0.setCell(CPos("E0"), "60"));
    assert (x0.setCell(CPos("E1"), "70"));
    assert (x0.setCell(CPos("E2"), "80"));
    assert (x0.setCell(CPos("E3"), "90"));
    assert (x0.setCell(CPos("E4"), "100"));
    assert (x0.setCell(CPos("F10"), "=D0+5"));
    assert (x0.setCell(CPos("F11"), "=$D0+5"));
    assert (x0.setCell(CPos("F12"), "=D$0+5"));
    assert (x0.setCell(CPos("F13"), "=$D$0+5"));
    x0.copyRect(CPos("G11"), CPos("F10"), 1, 4);

    oss.clear();
    oss.str("");
    assert (x0.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x0.load(iss));

    assert (valueMatch(x0.getValue(CPos("F10")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F11")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F12")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F13")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F14")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G10")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G11")), CValue(75.0)));
    assert (valueMatch(x0.getValue(CPos("G12")), CValue(25.0)));
    assert (valueMatch(x0.getValue(CPos("G13")), CValue(65.0)));
    assert (valueMatch(x0.getValue(CPos("G14")), CValue(15.0)));
    x0.copyRect(CPos("G11"), CPos("F10"), 2, 4);


    oss.clear();
    oss.str("");
    assert (x0.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x0.load(iss));

    assert (valueMatch(x0.getValue(CPos("F10")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F11")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F12")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F13")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F14")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G10")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G11")), CValue(75.0)));
    assert (valueMatch(x0.getValue(CPos("G12")), CValue(25.0)));
    assert (valueMatch(x0.getValue(CPos("G13")), CValue(65.0)));
    assert (valueMatch(x0.getValue(CPos("G14")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("H10")), CValue()));
    assert (valueMatch(x0.getValue(CPos("H11")), CValue()));
    assert (valueMatch(x0.getValue(CPos("H12")), CValue()));
    assert (valueMatch(x0.getValue(CPos("H13")), CValue(35.0)));
    assert (valueMatch(x0.getValue(CPos("H14")), CValue()));
    assert (x0.setCell(CPos("F0"), "-27"));

    assert (valueMatch(x0.getValue(CPos("H14")), CValue(-22.0)));
    x0.copyRect(CPos("H12"), CPos("H13"), 1, 2);

    oss.clear();
    oss.str("");
    assert (x0.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x0.load(iss));

    assert (valueMatch(x0.getValue(CPos("H12")), CValue(25.0)));
    assert (valueMatch(x0.getValue(CPos("H13")), CValue(-22.0)));
    assert (valueMatch(x0.getValue(CPos("H14")), CValue(-22.0)));

    x0.setCell(CPos("A1"), "20");
    x0.setCell(CPos("A$1"), "10");


    CSpreadsheet x4;
    assert (x4.setCell(CPos("B3"), "=B1 + 5"));
    assert (x4.setCell(CPos("A1"), "=B3"));
    assert (x4.setCell(CPos("B1"), "=A1"));
    assert (valueMatch(x4.getValue(CPos("A1")), CValue()));
    assert (valueMatch(x4.getValue(CPos("B1")), CValue()));

    assert (x4.setCell(CPos("A1"), "= 1 + 5 * 3 / 2 ^ 2 > A1"));
    assert (valueMatch(x4.getValue(CPos("A1")), CValue()));

    CSpreadsheet x5;
    assert (x5.setCell(CPos("A1"), "=B1"));
    assert (x5.setCell(CPos("B1"), "=C1"));
    assert (x5.setCell(CPos("C1"), "=D1"));
    assert (x5.setCell(CPos("D1"), "=E1"));
    assert (x5.setCell(CPos("E1"), "=C1"));


    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
