#include <gtest/gtest.h>
#include "task_fun.cpp"
#include <vector>
#include <cmath>
#include <string>

using namespace std;

vector<double> values = {2.0, 3.0, -2.0, 100.0};

TEST(MainGroup, Subtest_1) {
    TFunctionPtr func = funcFactory().Create("ident");
    EXPECT_EQ(func->ToString(), "x");
    for (double x : values)
        EXPECT_EQ(func->evaluate(x), x);
}

TEST(MainGroup, Subtest_2) {
    vector<string> ans{"2", "3", "-2", "100"};
    for (int i = 0; i < ans.size(); ++i) {
        TFunctionPtr func = funcFactory().Create("const", values[i]);
        EXPECT_EQ(func->ToString(), ans[i]);
	EXPECT_EQ(func->evaluate(values[i]), values[i]);
	EXPECT_EQ(func->evaluate(0), values[i]);
    }
}

TEST(MainGroup, Subtest_3) {
    TFunctionPtr func = funcFactory().Create("exp");
    EXPECT_EQ(func->ToString(), "e^x");
    for (int i = 0; i < values.size()-1; ++i)
        EXPECT_EQ(func->evaluate(values[i]), exp(values[i]));
}

TEST(MainGroup, Subtest_4) {
    vector<string> ans{"x^2", "x^3", "x^-2"};
    for (int i = 0; i < values.size()-1; ++i) {
        TFunctionPtr func = funcFactory().Create("power", values[i]);
        EXPECT_EQ(func->ToString(), ans[i]);
        EXPECT_EQ(func->evaluate(2.0), pow(2.0, values[i]));
    }
}

TEST(MainGroup, Subtest_5) {
   auto p = funcFactory().Create("polynomial", {-1, 2, 4});
   EXPECT_EQ(p->ToString(), "-1+2x^1+4x^2");
   EXPECT_EQ(p->evaluate(3.0), 41.0);
   EXPECT_EQ(p->evaluate(0.0), -1);
   EXPECT_EQ(p->evaluate(-1.0), 1);
}


TEST(MainGroup, Subtest_mixed) {
    TFunctionPtr ident = funcFactory().Create("ident");
    TFunctionPtr constFunc = funcFactory().Create("const", 3.0);
    TFunctionPtr powerFunc = funcFactory().Create("power", 2.0);
    TFunctionPtr expFunc = funcFactory().Create("exp");
    TFunctionPtr polyFunc = funcFactory().Create("polynomial", vector<double>{1, 2, 3});
    TFunctionPtr compositeFunc = ident + constFunc * powerFunc / expFunc + polyFunc;
    double x = 2.0;
    cout << "Evaluated result: " << compositeFunc->evaluate(x) << endl;
    EXPECT_NEAR(compositeFunc->evaluate(x), 20.624, 0.001);
    ASSERT_EQ("x+3*x^2/e^x+1+2x^1+3x^2", compositeFunc->ToString()); 
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
