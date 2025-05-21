#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cctype>

struct Triple {
    int row, col, value;
};

class SparseMatrix {
public:
    int rows, cols;
    std::vector<Triple> data;

    SparseMatrix(int r = 0, int c = 0) : rows(r), cols(c) {}

    void loadFromFile(const std::string& filename);
    SparseMatrix add(const SparseMatrix& other) const;
    SparseMatrix subtract(const SparseMatrix& other) const;
    SparseMatrix multiply(const SparseMatrix& other) const;
    void print() const;

private:
    void validateFormat(const std::string& content);
    bool isInteger(const std::string& s);
};

// Helper: remove all whitespace
std::string removeWhitespace(const std::string& input) {
    std::string result;
    for (char ch : input) {
        if (!isspace(ch)) result += ch;
    }
    return result;
}

// Check if a string is a valid integer
bool SparseMatrix::isInteger(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(c) && c != '-') return false;
    }
    return true;
}

void SparseMatrix::validateFormat(const std::string& content) {
    if (content.empty() || content.front() != '[' || content.back() != ']')
        throw std::invalid_argument("Input file has wrong format");

    for (char c : content) {
        if (c == '[' || c == ']' || c == ',' || c == '(' || c == ')' || isdigit(c) || c == '-')
            continue;
        throw std::invalid_argument("Input file has wrong format");
    }
}

// Load sparse matrix from a file
void SparseMatrix::loadFromFile(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) throw std::invalid_argument("Cannot open file");

    std::string line, total;
    while (std::getline(infile, line)) {
        total += removeWhitespace(line);
    }

    validateFormat(total);

    size_t i = 1; // skip '['
    while (i < total.length() && total[i] != ']') {
        if (total[i] != '(') throw std::invalid_argument("Input file has wrong format");
        ++i;

        std::string rowStr, colStr, valStr;
        while (i < total.length() && total[i] != ',') rowStr += total[i++];
        ++i;
        while (i < total.length() && total[i] != ',') colStr += total[i++];
        ++i;
        while (i < total.length() && total[i] != ')') valStr += total[i++];
        ++i;

        if (!isInteger(rowStr) || !isInteger(colStr) || !isInteger(valStr))
            throw std::invalid_argument("Input file has wrong format");

        Triple t = {std::stoi(rowStr), std::stoi(colStr), std::stoi(valStr)};
        rows = std::max(rows, t.row + 1);
        cols = std::max(cols, t.col + 1);
        data.push_back(t);

        if (total[i] == ',') ++i; // skip comma after tuple
    }
}

SparseMatrix SparseMatrix::add(const SparseMatrix& other) const {
    if (rows != other.rows || cols != other.cols)
        throw std::invalid_argument("Matrix dimensions do not match for addition");

    SparseMatrix result(rows, cols);
    result.data = data;

    for (const auto& t : other.data) {
        bool found = false;
        for (auto& r : result.data) {
            if (r.row == t.row && r.col == t.col) {
                r.value += t.value;
                found = true;
                break;
            }
        }
        if (!found) result.data.push_back(t);
    }
    return result;
}

SparseMatrix SparseMatrix::subtract(const SparseMatrix& other) const {
    SparseMatrix negated = other;
    for (auto& t : negated.data) t.value *= -1;
    return this->add(negated);
}

SparseMatrix SparseMatrix::multiply(const SparseMatrix& other) const {
    if (cols != other.rows)
        throw std::invalid_argument("Matrix dimensions do not match for multiplication");

    SparseMatrix result(rows, other.cols);

    for (const auto& a : data) {
        for (const auto& b : other.data) {
            if (a.col == b.row) {
                bool found = false;
                for (auto& r : result.data) {
                    if (r.row == a.row && r.col == b.col) {
                        r.value += a.value * b.value;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    result.data.push_back({a.row, b.col, a.value * b.value});
            }
        }
    }
    return result;
}

void SparseMatrix::print() const {
    std::cout << "[";
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << "(" << data[i].row << "," << data[i].col << "," << data[i].value << ")";
        if (i != data.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";
}

// Main Program
int main() {
    SparseMatrix A, B;
    std::string file1, file2;
    std::cout << "Enter path of first matrix file: ";
    std::cin >> file1;
    std::cout << "Enter path of second matrix file: ";
    std::cin >> file2;

    try {
        A.loadFromFile(file1);
        B.loadFromFile(file2);

        int choice;
        std::cout << "Choose operation:\n1. Addition\n2. Subtraction\n3. Multiplication\n";
        std::cin >> choice;

        SparseMatrix result;
        switch (choice) {
            case 1:
                result = A.add(B);
                break;
            case 2:
                result = A.subtract(B);
                break;
            case 3:
                result = A.multiply(B);
                break;
            default:
                std::cout << "Invalid choice.\n";
                return 1;
        }

        std::cout << "Result:\n";
        result.print();
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

