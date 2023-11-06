#include <Arduino.h>
#include <ps_stl.h>
#include <esp_brotli.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <time.h>
#include <LittleFS.h>

#include "Config.h"
#include "PinMap.h"
#include "Display.h"
#include "MQTTClient.h"
#include "Persistence.h"
#include "Scheduler.h"


#include "webpage_setup.h"

#include "App/Unit.h"
#include "App/Functions.h"
#include "SerializationHander.h"
#include "CommandHandler.h"

#include "RuleEngine.h"
#include <unity.h>

ps::string test_equality = "0 == 0";
ps::string test_parenthesis = "(1 == 1)";
ps::string test_inequality = "0 != 1";
ps::string test_greater_than = "1 > 0";
ps::string test_less_than = "0 < 1";
ps::string test_greater_than_equal = "1 >= 0";
ps::string test_less_than_equal = "0 <= 1";

ps::string test_and = "1 == 1 && 0 == 0";
ps::string test_or = "1 == 1 || 0 == 1";
ps::string test_not = "!0";

ps::string test_add = "(1 + 1) == 2";
ps::string test_subtract = "(1 - 1) == 0";
ps::string test_multiply = "(3 * 3) == 9";
ps::string test_divide = "(2 / 1) == 2";
ps::string test_modulo = "(3 % 2) == 1";
ps::string test_power = "(3 ^ 2) == 9";

ps::string test_string_equality = "\"a\" == \"a\"";
ps::string test_string_inequality = "\"a\" != \"b\"";

ps::string test_array_equality = "[\"a\", \"b\", \"c\"] == [\"a\", \"b\", \"c\"]";
ps::string test_array_inequality = "[\"a\", \"b\", \"c\"] != [\"a\", \"b\"]";
ps::string test_array_subset = "[\"a\"] |= [\"a\", \"b\"]";
ps::string test_array_common = "([\"a\"] && [\"a\", \"b\"]) == [\"a\"]";
ps::string test_array_unique = "([\"a\"] || [\"a\", \"b\"]) == [\"a\", \"b\"]";

ps::string test_array_contains_string = "[\"a\", \"b\", \"c\"] |= \"a\"";
ps::string test_array_string_equality = "[\"a\"] == \"a\"";


void log_memory_usage() {
    static const size_t tot_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    static const size_t tot_sram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);

    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

    float used_psram = (float)(tot_psram - free_psram) / 1024;
    float used_sram = (float)(tot_sram - free_sram) / 1024;
    float psram_percentage = 100 *( 1 - ((float) free_psram) / ((float) tot_psram));
    float sram_percentage = 100 *( 1 - ((float) free_sram) / ((float) tot_sram));


    ESP_LOGI("RAM", "Usage: RAM - %u/%u KB [%f%%], PSRAM - %u/%u KB [%f%%]", (uint32_t) used_sram, (uint32_t) tot_sram/1024, sram_percentage, (uint32_t) used_psram, (uint32_t) tot_psram/1024, psram_percentage);
}

void test_expressions() {
    uint64_t start_tm = micros();

    re::VariableStorage* vars = new re::VariableStorage();

    re::Expression expr(test_equality, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_parenthesis, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_inequality, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_greater_than, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_less_than, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_greater_than_equal, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_less_than_equal, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_and, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_or, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_not, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_add, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_subtract, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_multiply, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_divide, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_modulo, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_power, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_string_equality, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_string_inequality, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_array_equality, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_array_inequality, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_array_subset, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_array_common, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_array_unique, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_array_contains_string, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    expr = re::Expression(test_array_string_equality, vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    ps::ostringstream test_all;
    //Combine all the string tests to a single expression using AND.
    test_all << test_equality << "&&" << test_parenthesis << "&&" << test_inequality << "&&" << test_greater_than << "&&" << test_less_than << "&&" << test_greater_than_equal << "&&" << test_less_than_equal << "&&" << test_and << "&&" << test_or << "&&" << test_not << "&&" << test_add << "&&" << test_subtract << "&&" << test_multiply << "&&" << test_divide << "&&" << test_modulo << "&&" << test_power << "&&" << test_string_equality << "&&" << test_string_inequality << "&&" << test_array_equality << "&&" << test_array_inequality << "&&" << test_array_subset << "&&" << test_array_common << "&&" << test_array_unique << "&&" << test_array_contains_string << "&&" << test_array_string_equality;
    expr = re::Expression(test_all.str(), vars);
    TEST_ASSERT_EQUAL(true, expr.evaluate());

    ESP_LOGI("Expressions", "Complete. Took %uus", micros() - start_tm);
    delete vars;
}

void setup() {
    LittleFS.begin();
    psramInit();
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_expressions);
    UNITY_END();
}

void loop() {

}