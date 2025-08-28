#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

// Decode string number in given base into decimal (as long double for large values)
long double decodeBase(const string& num, int base) {
    long double value = 0;
    for (char c : num) {
        int digit;
        if (isdigit(c)) digit = c - '0';
        else if (isalpha(c)) digit = tolower(c) - 'a' + 10;
        else throw invalid_argument("Invalid character in base string");

        if (digit >= base) throw invalid_argument("Digit out of base range");

        value = value * base + digit;
    }
    return value;
}

// Lagrange interpolation: compute polynomial at point x
long double lagrangeInterpolation(const vector<long double>& X, const vector<long double>& Y, long double x) {
    int n = X.size();
    long double result = 0;

    for (int i = 0; i < n; i++) {
        long double term = Y[i];
        for (int j = 0; j < n; j++) {
            if (i != j) {
                term *= (x - X[j]) / (X[i] - X[j]);
            }
        }
        result += term;
    }
    return result;
}

int main() {
    // Read JSON file
    ifstream inFile("input2.json");
    if (!inFile) {
        cerr << "Error: Could not open input file" << endl;
        return 1;
    }

    json j;
    inFile >> j;

    int n = j["keys"]["n"];
    int k = j["keys"]["k"];

    vector<long double> X, Y;

    // Collect first k points
    int count = 0;
    for (auto& el : j.items()) {
        if (el.key() == "keys") continue;
        if (count >= k) break;

        int x = stoi(el.key());  // x is the object key
        int base = stoi(el.value()["base"].get<string>());
        string valStr = el.value()["value"].get<string>();
        long double y = decodeBase(valStr, base);

        X.push_back(x);
        Y.push_back(y);
        count++;
    }

    // Compute secret = P(0)
    long double secret = lagrangeInterpolation(X, Y, 0.0);

    cout << "Secret (constant term C) = " << fixed << setprecision(0) << secret << endl;

    return 0;
}
