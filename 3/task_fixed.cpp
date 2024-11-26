#include <iostream>
#include <memory>
#include <vector>
#include <cmath>
#include <sstream>
#include <stdexcept>

using namespace std;

class TFunction;
using TFunctionPtr = shared_ptr<TFunction>;

class TFunction {
protected:
    TFunctionPtr left, right;
    char op;
public:
    TFunction() : left(nullptr), right(nullptr), op('0') {}
    TFunction(TFunctionPtr left, TFunctionPtr right, char op) : left(left), right(right), op(op) {}

    virtual double evaluate(double x) const {
        double left_val = left->evaluate(x);
        double right_val = right->evaluate(x);
        switch (op) {
            case '+':
                return left_val + right_val;
            case '-':
                return left_val - right_val;
            case '*':
                return left_val * right_val;
            case '/':
                return left_val / right_val;
            default:
                throw invalid_argument("Unknown operation");
        }
    }

    virtual string ToString() const {
        stringstream ss;
        ss << left->ToString() << op << right->ToString();
        return ss.str();
    }

    friend TFunctionPtr operator+(TFunctionPtr self, TFunctionPtr other);
    friend TFunctionPtr operator-(TFunctionPtr self, TFunctionPtr other);
    friend TFunctionPtr operator*(TFunctionPtr self, TFunctionPtr other);
    friend TFunctionPtr operator/(TFunctionPtr self, TFunctionPtr other);
    template<typename T>
    friend TFunctionPtr operator+(TFunctionPtr self, T other);
    template<typename T>
    friend TFunctionPtr operator-(TFunctionPtr self, T other);
    template<typename T>
    friend TFunctionPtr operator*(TFunctionPtr self, T other);
    template<typename T>
    friend TFunctionPtr operator/(TFunctionPtr self, T other);
};

class funcFactory {
public:
    TFunctionPtr Create(string type);
    TFunctionPtr Create(string type, double value);
    TFunctionPtr Create(string type, vector<double> vect);
};

class IdentFunction : public TFunction {
public:
    IdentFunction() : TFunction() {}

    double evaluate(double x) const override {
        return x;
    }

    string ToString() const override {
        stringstream ss;
        ss << "x";
        return ss.str();
    }
};

class ConstFunction : public TFunction {
    double value;
public:
    ConstFunction(double value) : TFunction(), value(value) {}

    double evaluate(double x) const override {
        return value;
    }

    string ToString() const override {
        stringstream ss;
        ss << value;
        return ss.str();
    }
};

class PowerFunction : public TFunction {
    double exponent;
public:
    PowerFunction(double exponent) : TFunction(), exponent(exponent) {}

    double evaluate(double x) const override {
        return pow(x, exponent);
    }

    string ToString() const override {
        stringstream ss;
        ss << "x^" << exponent;
        return ss.str();
    }
};

class ExpFunction : public TFunction {
public:
    ExpFunction() : TFunction() {}

    double evaluate(double x) const override {
        return exp(x);
    }

    string ToString() const override {
        stringstream ss;
        ss << "e^x";
        return ss.str();
    }
};

class PolynomialFunction : public TFunction {
    vector<double> coefficients;
public:
    PolynomialFunction(const vector<double>& coefficients) : TFunction(), coefficients(coefficients) {}

    double evaluate(double x) const override {
        double result = 0;
        double x_pow = 1;
        for (double coeff : coefficients) {
            result += coeff * x_pow;
            x_pow *= x;
        }
        return result;
    }

    string ToString() const override {
        int pw = 0;
        stringstream ss;
        for (int i = 0; i < coefficients.size(); ++i) {
            if (i != 0)
                ss << '+';
            ss << coefficients[i];
            if (pw != 0) 
                ss << "x^" << pw;
            pw++;
        }
        return ss.str();
    }
};

TFunctionPtr operator+(TFunctionPtr self, TFunctionPtr other) {
    return make_shared<TFunction>(self, other, '+');
}

TFunctionPtr operator-(TFunctionPtr self, TFunctionPtr other) {
    return make_shared<TFunction>(self, other, '-');
}

TFunctionPtr operator*(TFunctionPtr self, TFunctionPtr other) {
    return make_shared<TFunction>(self, other, '*');
}

TFunctionPtr operator/(TFunctionPtr self, TFunctionPtr other) {
    return make_shared<TFunction>(self, other, '/');
}

template<typename T>
TFunctionPtr operator+(TFunctionPtr self, T other) {
    throw logic_error("Invalid argument");
}

template<typename T>
TFunctionPtr operator-(TFunctionPtr self, T other) {
    throw logic_error("Invalid argument");
}

template<typename T>
TFunctionPtr operator*(TFunctionPtr self, T other) {
    throw logic_error("Invalid argument");
}

template<typename T>
TFunctionPtr operator/(TFunctionPtr self, T other) {
    throw logic_error("Invalid argument");
}


TFunctionPtr funcFactory::Create(string type, vector<double> value) {
    if (type == "polynomial")
        return make_shared<PolynomialFunction>(value);
    throw invalid_argument("Invalid argument");
}

TFunctionPtr funcFactory::Create(string type) {
    if (type == "exp")
        return make_shared<ExpFunction>();
    if (type == "ident")
        return make_shared<IdentFunction>();
    throw invalid_argument("Invalid argument");
}

TFunctionPtr funcFactory::Create(string type, double value) {
    if (type == "power")
        return make_shared<PowerFunction>(value);
    if (type == "const")
        return make_shared<ConstFunction>(value);
    throw invalid_argument("Invalid argument");
}
/*
int main() {
    TFunctionPtr ident = funcFactory().Create("ident");
    TFunctionPtr constFunc = funcFactory().Create("const", 3.0);
    TFunctionPtr powerFunc = funcFactory().Create("power", 2.0);
    TFunctionPtr expFunc = funcFactory().Create("exp");
    TFunctionPtr polyFunc = funcFactory().Create("polynomial", vector<double>{1, 2, 3});

    TFunctionPtr compositeFunc = ident + constFunc * powerFunc / expFunc + polyFunc;

    double x = 2.0;
    cout << "Evaluated result: " << compositeFunc->evaluate(x) << endl;
    auto y = funcFactory().Create("ident");
    y + 11;
    cout << compositeFunc->ToString();
    return 0;
}*/
