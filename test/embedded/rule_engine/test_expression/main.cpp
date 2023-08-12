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
void test_sram_usage();

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_storage);
    RUN_TEST(test_lambda_in_storage);
    RUN_TEST(test_expression);
    RUN_TEST(test_sram_usage);
    UNITY_END();
}

void loop() {}

void log_memory_usage() {
    static const size_t tot_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    static const size_t tot_sram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);

    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

    log_printf("\n- PSRAM Usage: %f/%u kB (%f%%)", ((float)(tot_psram - free_psram)) / 1024, tot_psram / 1024, 100 *( 1 - ((float) free_psram) / ((float) tot_psram)));
    log_printf("\n- SRAM Usage: %f/%u kB (%f%%)\n\n", ((float)(tot_sram - free_sram)) / 1024, tot_sram / 1024, 100 *( 1 - ((float) free_sram) / ((float) tot_sram)));
}

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

void test_sram_usage() {
    log_memory_usage();

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
        ps::string("((TAP <= 5) && (PS == 1)) && AP < 123.2 && TS >= 0 && MS == 0 && UID == \"unitID\" && UTL || [\"unit\",\"test_unit_tag\",\"test\",\"test123\",\"test2\",\"test3\",\"test4\"]"),
        ps::string("TestCommand"),
    };

    
    const std::vector<Rule> rule_list = {new_rule};


    ps::vector<ps::string> commands = {
        "(TAP <= 5) && (PS == 1)", // 7 tokens
        "AP < 123.2", // 3 tokens
        "TS >= 0", // 3 tokens
        "MS == 0", // 3 tokens
        "UID == \"unitID\"", // 3 tokens
        "UTL || [\"unit\",\"test_unit_tag\",\"test\",\"test123\",\"test2\",\"test3\",\"test4\"]", // 3 tokens + 7 array tokens
        "((TSP > 100) || (TRP <= 50)) && (SW == 1)", // 11 tokens
        "(V > 220) && (F == 50)", //  7 tokens
        "(AP + RP) * SP", // 5 tokens
        "(MID == 3) && (PF >= 0.9)", // 7 tokens
        "((PS == 1) && (SW == 0)) || (V <= 240)", // 11 tokens
        // 27 tokens +  7 array tokens
        "((TAP <= 5) && (PS == 1)) && AP < 123.2 && TS >= 0 && MS == 0 && UID == \"unitID\" && UTL || [\"unit\",\"test_unit_tag\",\"test\",\"test123\",\"test2\",\"test3\",\"test4\"]",
        "(UTL || MTL) && 1", // 5 tokens
        "(AP + RP) >= 50", // 5 tokens
        "((PS == 1) && (MID == \"moduleID\")) || (V < 230)" // 11 tokens
    };

    auto unit = ps::make_shared<SDRUnit>(
            uid, 
            1,
            utag_list,
            rule_list
        );

    ps::vector<std::shared_ptr<Module>> mod_vect;
    ps::vector<std::shared_ptr<re::Expression>>expr_vect;
    ps::vector<std::shared_ptr<re::VariableStorage>>var_store_vect;

    /* Create modules */
    uint64_t start_tm = esp_timer_get_time();
    for (size_t i = 0; i < 32; i++){
        auto mod = ps::make_shared<Module>(
            mid,
            5,
            module_tag_list,
            rule_list,
            0,
            0
        );

        mod -> addReading(test_reading_0);
        mod_vect.push_back(mod);
    }
    uint64_t end_tm = esp_timer_get_time();
    ESP_LOGD("Mod", "Took %u us", end_tm - start_tm);
    
    log_memory_usage();

    start_tm = esp_timer_get_time();
    for (auto& module : mod_vect){
        /* 1 VariableStorage per Module */
        auto vars = re::load(unit, module);
        var_store_vect.push_back(vars);
        module -> addReading(test_reading_0);
    }

    end_tm = esp_timer_get_time();
    ESP_LOGD("Var+Expr", "Took %u us", end_tm - start_tm);
    
    log_memory_usage();

    /* Evaluate all */
    ps::vector<bool> results;
    try {
        for (size_t i = 0; i < 15; i++) {
            results.clear();
            /* Load 1 more expression per iteration. */
            for (size_t j = 0; j < mod_vect.size(); j++) {
                auto expr = ps::make_shared<re::Expression>(commands.at(i), var_store_vect.at(j));
                expr_vect.push_back(expr);
            }

            /* Check evaluation time */
            start_tm = esp_timer_get_time();
            for (auto& expression : expr_vect) {
                results.push_back(expression -> evaluate());
            }
            end_tm = esp_timer_get_time();
            ESP_LOGD("Eval", "Took %u us", end_tm - start_tm);
            
            log_memory_usage();

            ESP_LOGI("Module", "Count: %d", mod_vect.size());
            ESP_LOGI("Expr", "Count: %d", expr_vect.size());


        }
    } catch (std::exception &e){
        ESP_LOGE("Except","%s", e.what());
    } catch (...) {
        TEST_ASSERT_TRUE(false);
    }

    results.clear();
    start_tm = esp_timer_get_time();
    for (auto& expression : expr_vect) {
        results.push_back(expression -> evaluate());
    }
    end_tm = esp_timer_get_time();
    ESP_LOGD("Eval", "Took %u us", end_tm - start_tm);
    
    log_memory_usage();

    ESP_LOGI("Module", "Count: %d", mod_vect.size());
    ESP_LOGI("Expr", "Count: %d", expr_vect.size());

    TEST_ASSERT_EQUAL(1, 1);
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

    unit -> totalActivePower() = 4;
    unit -> powerStatus() = 1;
    uint64_t start_tm = esp_timer_get_time();
    auto result = expr -> evaluate();
    uint64_t end_tm = esp_timer_get_time();
    ESP_LOGD("Eval", "Took %u us", end_tm - start_tm);
    
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t tot_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t tot_sram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    log_printf("\n- PSRAM Usage: %f/%u kB (%f%%)", ((float)(tot_psram - free_psram)) / 1024, tot_psram / 1024, 100 *( 1 - ((float) free_psram) / ((float) tot_psram)));
    log_printf("\n- SRAM Usage: %f/%u kB (%f%%)\n\n", ((float)(tot_sram - free_sram)) / 1024, tot_sram / 1024, 100 *( 1 - ((float) free_sram) / ((float) tot_sram)));
    TEST_ASSERT_TRUE(result);
}