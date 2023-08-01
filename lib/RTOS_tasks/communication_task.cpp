#include "communication_task.h"
#include "config.h"
#include "VariableDelay.h"

#define TARGET_LOOP_FREQUENCY 1
void ruleEngineTaskFunction(void* pvParameters) {


    VariableDelay vd(RULE_ENGINE_TASK_NAME, TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.
    while(1) {

        vd.delay(); // Wait for remainder of frame to reach target frequency.
    }
}