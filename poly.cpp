#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <map>
#include "json.hpp" // nlohmann's JSON library header

using json = nlohmann::json;

// BigInt Arithmetic Functions (operating on std::string)

// Helper to compare magnitudes of two number strings
bool is_smaller(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return a.length() < b.length();
    }
    return a < b;
}

// Function to add two positive number strings
std::string add(std::string a, std::string b) {
    std::string sum = "";
    int i = a.length() - 1, j = b.length() - 1, carry = 0;
    while (i >= 0 || j >= 0 || carry) {
        int digit_a = (i >= 0) ? a[i--] - '0' : 0;
        int digit_b = (j >= 0) ? b[j--] - '0' : 0;
        int current_sum = digit_a + digit_b + carry;
        sum += std::to_string(current_sum % 10);
        carry = current_sum / 10;
    }
    std::reverse(sum.begin(), sum.end());
    return sum;
}

// Function to subtract smaller positive string from larger positive string
std::string subtract(std::string a, std::string b) {
    if (is_smaller(a, b)) std::swap(a, b);
    std::string diff = "";
    int i = a.length() - 1, j = b.length() - 1, borrow = 0;
    while (i >= 0) {
        int digit_a = a[i--] - '0' - borrow;
        int digit_b = (j >= 0) ? b[j--] - '0' : 0;
        if (digit_a < digit_b) {
            digit_a += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        diff += std::to_string(digit_a - digit_b);
    }
    std::reverse(diff.begin(), diff.end());
    // Remove leading zeros
    size_t first_digit = diff.find_first_not_of('0');
    if (std::string::npos != first_digit) {
        return diff.substr(first_digit);
    }
    return "0";
}

// Function to handle signs for addition/subtraction
std::string add_signed(std::string num1, std::string num2) {
    bool neg1 = num1[0] == '-';
    bool neg2 = num2[0] == '-';
    if (neg1) num1 = num1.substr(1);
    if (neg2) num2 = num2.substr(1);

    if (neg1 == neg2) { // Both positive or both negative
        return (neg1 ? "-" : "") + add(num1, num2);
    } else { // One positive, one negative
        if (is_smaller(num1, num2)) {
            return (neg2 ? "" : "-") + subtract(num2, num1);
        } else {
            return (neg1 ? "-" : "") + subtract(num1, num2);
        }
    }
}


// Function to multiply a positive number string by a positive number string
std::string multiply(std::string num1, std::string num2) {
    if (num1 == "0" || num2 == "0") return "0";
    std::string product(num1.length() + num2.length(), '0');
    for (int i = num1.length() - 1; i >= 0; i--) {
        int carry = 0;
        for (int j = num2.length() - 1; j >= 0; j--) {
            int p = (num1[i] - '0') * (num2[j] - '0') + (product[i + j + 1] - '0') + carry;
            product[i + j + 1] = p % 10 + '0';
            carry = p / 10;
        }
        product[i] += carry;
    }
    size_t first_digit = product.find_first_not_of('0');
    if (std::string::npos != first_digit) {
        return product.substr(first_digit);
    }
    return "0";
}

// Function to handle signs for multiplication
std::string multiply_signed(std::string num1, std::string num2) {
    bool neg1 = num1[0] == '-';
    bool neg2 = num2[0] == '-';
    if (neg1) num1 = num1.substr(1);
    if (neg2) num2 = num2.substr(1);

    std::string result = multiply(num1, num2);
    if (result == "0") return "0";
    if (neg1 != neg2) {
        return "-" + result;
    }
    return result;
}


// Function to divide a large number string by a long long and get remainder
std::pair<std::string, long long> divide_by_long(std::string num_str, long long divisor) {
    std::string quotient = "";
    long long temp = 0;
    bool is_negative = num_str[0] == '-';
    if (is_negative) num_str = num_str.substr(1);
    if (divisor < 0) {
        is_negative = !is_negative;
        divisor = -divisor;
    }

    for (char digit : num_str) {
        temp = temp * 10 + (digit - '0');
        if (temp >= divisor) {
            quotient += std::to_string(temp / divisor);
            temp %= divisor;
        } else {
            if (!quotient.empty()) {
                quotient += '0';
            }
        }
    }
    if (quotient.empty()) quotient = "0";

    if (is_negative && quotient != "0") {
        quotient = "-" + quotient;
    }
    
    return {quotient, temp};
}


// Function to convert a number from any base (up to 36) to a decimal string
std::string baseToDecimal(std::string value, int base) {
    std::string result = "0";
    std::string power = "1";
    std::string base_str = std::to_string(base);

    for (int i = value.length() - 1; i >= 0; i--) {
        int digit;
        if (value[i] >= '0' && value[i] <= '9') {
            digit = value[i] - '0';
        } else if (value[i] >= 'a' && value[i] <= 'z') {
            digit = value[i] - 'a' + 10;
        } else { // Should not happen with valid input
            throw std::invalid_argument("Invalid character in number string");
        }

        if (digit >= base) {
             throw std::invalid_argument("Digit exceeds base");
        }

        if (digit != 0) {
            std::string term = multiply(std::to_string(digit), power);
            result = add(result, term);
        }
        
        power = multiply(power, base_str);
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_json_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    json data;
    try {
        data = json::parse(file);
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    int k = data["keys"]["k"];
    std::vector<long long> x_coords;
    std::vector<std::string> y_coords_str;
    
    int points_count = 0;
    for (auto& item : data.items()) {
        if (item.key() != "keys" && points_count < k) {
            long long x = std::stoll(item.key());
            std::string base_str = item.value()["base"];
            std::string value_str = item.value()["value"];
            int base = std::stoi(base_str);

            x_coords.push_back(x);
            y_coords_str.push_back(baseToDecimal(value_str, base));
            points_count++;
        }
    }

    std::string secret_c_str = "0";

    // Lagrange Interpolation for P(0)
    for (int j = 0; j < k; ++j) {
        std::string y_j = y_coords_str[j];
        
        std::string numerator_prod = "1";
        long long denominator_prod = 1;

        for (int i = 0; i < k; ++i) {
            if (i == j) continue;
            // L_j(0) = product of (-x_i) / (x_j - x_i)
            numerator_prod = multiply_signed(numerator_prod, std::to_string(-x_coords[i]));
            denominator_prod *= (x_coords[j] - x_coords[i]);
        }
        
        std::string term_numerator = multiply_signed(y_j, numerator_prod);
        
        auto division_result = divide_by_long(term_numerator, denominator_prod);
        
        if (division_result.second != 0) {
            std::cerr << "Error: The division for a term was not exact. Check the input points." << std::endl;
            return 1;
        }

        std::string term = division_result.first;
        secret_c_str = add_signed(secret_c_str, term);
    }

    std::cout << "The calculated value of constant c is: " << secret_c_str << std::endl;

    return 0;
}