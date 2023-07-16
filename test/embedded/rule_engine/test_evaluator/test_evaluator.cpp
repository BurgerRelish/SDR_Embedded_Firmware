#include <Arduino.h>
#include <unity.h>

#include "../rule_engine/Evaluator.h"
#include "../sdr_containers.h"
#include "ps_vector.h"
#include "ps_string.h"
#include "ps_stack.h"

void test_evaluator() {
    size_t highwatermark = uxTaskGetStackHighWaterMark(NULL);
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t tot_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);

    size_t free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t tot_sram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    ESP_LOGD("TEST_EVAL", "\n==== RAM Usage Statistics ====");
    log_printf("- Stack High Watermark: %u", highwatermark);
    log_printf("\n- PSRAM Usage: %f%", 100 *( 1 - ((float) free_psram) / ((float) tot_psram)));
    log_printf("\n- SRAM Usage: %f%", 100 * (1 - ((float) free_sram) / ((float) tot_sram)));
    log_printf("\n==============================\n\n");

    const std::string mid = "moduleID";
    const std::vector<std::string> mtag_list = {"module", "test_module_tag"};
    const int priority = 5;
    const uint8_t address = 10;
    const uint8_t offset = 2;

    const Reading test_reading_0 = {
        230.1, // double voltage;
        50.5, // double frequency;
        123.1, // double active_power;
        456.2, // double reactive_power;
        789.1, // double apparent_power;
        1.232, // double power_factor;
        true, // bool status;
        10000 // uint64_t timestamp;
    };

    Module sdr_module(mid, mtag_list, priority, address, offset);
    sdr_module.addReading(test_reading_0);

    const std::string uid = "unitID";
    const std::vector<std::string> utag_list = {"unit", "test_unit_tag"};
    SDRUnit sdr_unit(uid, 2, utag_list);

    sdr_unit.powerStatus() = true;
    sdr_unit.totalActivePower() = 5.00;

    Rule new_rule = {
        1,
        ps_string("((TAP <= 5) && (PS == 1)) && TS > 0 && UID == \"unitID\" && UTL == [\"unit\",\"test_unit_tag\"]"),
        ps_string("TestCommand"),
    };

    ps_queue<Rule> rules;
    rules.push(new_rule); 

    Evaluator evaluator(sdr_unit, sdr_module, rules);

    ps_queue<Token> commands;
    commands = evaluator.evaluate();

    TEST_ASSERT_EQUAL_INT(1, commands.size());

    highwatermark = uxTaskGetStackHighWaterMark(NULL);
    free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    tot_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);

    free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    tot_sram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);

    ESP_LOGD("TEST_EVAL", "\n==== RAM Usage Statistics ====");
    log_printf("- Stack High Watermark: %u", highwatermark);
    log_printf("\n- PSRAM Usage: %f%", 100 *( 1 - ((float) free_psram) / ((float) tot_psram)));
    log_printf("\n- SRAM Usage: %f%", 100 * (1 - ((float) free_sram) / ((float) tot_sram)));
    log_printf("\n==============================\n\n");
}

void setup() {
    psramInit();
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_evaluator);

    UNITY_END();
}

void loop() {

}