#include <Arduino.h>
#include <unity.h>
#include <any>
#include <FS.h>

#include "../sdr_containers/SDRUnit.h"
#include "../sdr_containers/SDRModule.h"

#include "../rule_engine/Expression.h"
#include "../rule_engine/VariableStorage.h"

#include "../ps_stl/ps_stl.h"

void test_storage();
void test_expression();
void test_lambda_in_storage();

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_storage);
    RUN_TEST(test_lambda_in_storage);
    RUN_TEST(test_expression);
    UNITY_END();
}

void loop() {}

void test_storage() {
    auto vars = ps::make_shared<re::VariableStorage>();
    vars -> set(re::VAR_INT, "TEST1", 1);
    vars -> set(re::VAR_STRING, "TEST2", ps::string("Hello World!!!"));

    auto int_var = vars -> get<int>("TEST1");
    auto str_var = vars -> get<ps::string>("TEST2");
    ESP_LOGI("T1", "Got %d", int_var);
    ESP_LOGI("T2", "Got: %s", str_var.c_str());
    TEST_ASSERT_EQUAL(1, int_var);
    TEST_ASSERT_EQUAL_STRING("Hello World!!!", str_var.c_str());
}

void test_lambda_in_storage() {
    auto vars = ps::make_shared<re::VariableStorage>();

    vars -> set(re::VAR_INT, "test", std::function<int(void)> ( []() { return 423; }));

    auto result = vars -> get<int>("test");
    TEST_ASSERT_TRUE(result);

    TEST_ASSERT_EQUAL_DOUBLE(423, vars -> get<double>("test"));
}

void test_expression() {
    const Reading test_reading_0 = {
        230.1, // double voltage;
        50.5, // double frequency;
        123.1, // double active_power;
        456.2, // double reactive_power;
        789.1, // double apparent_power;
        1.232, // double power_factor;
        0.1023,
        10000 // uint64_t timestamp;
    };

    const std::string mid = "moduleID";
    const std::string uid = "unitID";
    const std::vector<std::string> utag_list = {"unit", "test_unit_tag"};
    const std::vector<std::string> module_tag_list = {"mtag_1", "mtag_2"};

    Rule new_rule = {
        1,
        ps::string("((TAP <= 5) && (PS == 1)) && TS >= 0 && UID == \"unitID\" && UTL == [\"unit\",\"test_unit_tag\"]"),
        ps::string("TestCommand"),
    };

    const std::vector<Rule> rule_list = {new_rule};

    auto mod = ps::make_shared<Module>(
        mid,
        5,
        module_tag_list,
        rule_list,
        0,
        0
    );

    auto unit = ps::make_shared<SDRUnit>(
        uid, 
        1,
        utag_list,
        rule_list
    );

    mod -> addReading(test_reading_0);

    auto vars = re::load(unit, mod);

    auto expr = ps::make_shared<re::Expression>(new_rule.expression, vars);

    unit -> totalActivePower() = 5;
    unit -> powerStatus() = 1;

    TEST_ASSERT_TRUE(expr -> evaluate());
}