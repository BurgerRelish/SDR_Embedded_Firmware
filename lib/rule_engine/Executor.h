#pragma once

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "../sdr_containers.h"
#include "ps_priority_queue.h"
#include "Language.h"
#include "Semantics.h"

class Executor {
    private:
    ps_priority_queue<Command> command_list;

    public:

    Executor() {
    }

    void ON();
    void OFF();
    void DELAY(Token period);
    void CLRQUE();

}

#endif