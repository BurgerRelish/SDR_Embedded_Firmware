#include <Arduino.h>
#include <unity.h>
#include <ps_stl.h>
#include <functional>

#include "../lib/rule_engine/RuleEngineBase.h"

void log_memory_usage() {
    static const size_t tot_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    static const size_t tot_sram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);

    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

    log_printf("\n- PSRAM Usage: %f/%u kB (%f%%)", ((float)(tot_psram - free_psram)) / 1024, tot_psram / 1024, 100 *( 1 - ((float) free_psram) / ((float) tot_psram)));
    log_printf("\n- SRAM Usage: %f/%u kB (%f%%)\n\n", ((float)(tot_sram - free_sram)) / 1024, tot_sram / 1024, 100 *( 1 - ((float) free_sram) / ((float) tot_sram)));
}

void load_test_functions(std::shared_ptr<re::FunctionStorage>& functions) {
    std::function<bool(ps::vector<ps::string>&, re::VariableStorage*)> fn_0 = [](ps::vector<ps::string>& args, re::VariableStorage * vars){
        ps::stringstream arg_str;
        for (auto& arg : args) {
            arg_str << arg;
            arg_str << ",";
        }
        ESP_LOGI("fn_0", "Arg Val: %d, args: \'%s\'", vars->get_var<int>(args.at(0)), arg_str.str().c_str());
        vars->set_var<int>(re::VAR_INT, "res0", 45);
        return true;
    };

    ps::string id = "fn_0";
    functions -> add(id, fn_0);

    std::function<bool(ps::vector<ps::string>&, re::VariableStorage*)> fn_1 = [](ps::vector<ps::string>& args, re::VariableStorage * vars){
        ESP_LOGI("fn_1", "%s", args.at(0).c_str());
        return true;
    };
    log_memory_usage();

    ps::string id_1 = "fn_1";
    functions -> add(id_1, fn_1);

    log_memory_usage();
}

void load_test_rules(std::shared_ptr<re::RuleEngineBase> engine) {
    engine->set_var<int>(re::VAR_INT, "res0", 0);

    auto expr_0 = ps::string("(Val0 - 5) == 5 && res0 != 45"); // IF <this is true>
    auto fn_0 = ps::string("fn_0(Val0);");       // THEN <do this>

    auto expr_1 = ps::string("res0 == 45");
    auto fn_1 = ps::string("fn_1(res0)");

    auto rule_0 = std::make_tuple(2, expr_0, fn_0); // Highest Priority runs first.
    engine -> add_rule(rule_0);
    auto rule_1 = std::make_tuple(1, expr_1, fn_1);
    engine -> add_rule(rule_1);

    log_memory_usage();
}

void load_test_vars(std::shared_ptr<re::RuleEngineBase> engine) {
    engine -> set_var<int>(re::VAR_INT, "Val0", 10);
    log_memory_usage();
}

void test_separator() {
    ps::string fn_0 = "fn_0(1, 2, 3, 4);";
    re::CommandSeparator separator;
    auto tokens = separator.separate(fn_0);
    log_memory_usage();
    TEST_ASSERT_EQUAL(1, tokens.size());
    TEST_ASSERT_EQUAL(4, std::get<1>(tokens.at(0)).size());
}

void test_rule_engine() {
    auto functions = ps::make_shared<re::FunctionStorage>();
    load_test_functions(functions);
    auto rule_engine = ps::make_shared<re::RuleEngineBase>("UTL", functions);
    load_test_vars(rule_engine);
    load_test_rules(rule_engine);

    uint64_t start_tm = esp_timer_get_time();
    rule_engine -> reason();
    uint64_t end_tm = esp_timer_get_time();
    log_memory_usage();

    ESP_LOGD("Reason", "Took %u us", end_tm - start_tm);
    TEST_ASSERT_EQUAL_INT(45, rule_engine -> get_var<int>("res0"));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_separator);
    RUN_TEST(test_rule_engine);
    UNITY_END();
}

void loop() {

}