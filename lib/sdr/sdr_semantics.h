#pragma once

#ifndef SDR_SEMANTICS_H
#define SDR_SEMANTICS_H

/* SDR Unit Variables */
#define UNIT_CLASS "UNIT"
#define TOTAL_ACTIVE_POWER "TAP"
#define TOTAL_REACTIVE_POWER "TRP"
#define TOTAL_APPARENT_POWER "TSP"
#define POWER_STATUS "PS"
#define UNIT_ID "UID"
#define UNIT_TAG_LIST "UTL"
#define MODULE_COUNT "MC"


/* Module Variables */
#define MODULE_CLASS "MODULE"
#define ACTIVE_POWER "AP"
#define REACTIVE_POWER "RP"
#define APPARENT_POWER "SP"
#define VOLTAGE "V"
#define FREQUENCY "FR"
#define POWER_FACTOR "PF"
#define SWITCH_TIME "SWT"
#define CIRCUIT_PRIORITY "CP"
#define MODULE_ID "MID"
#define MODULE_TAG_LIST "MTL"
#define SWITCH_STATUS "MS"

/* Commands */
#define MOD_ON "ON" // ()
#define MOD_OFF "OFF" // ()

#define RE_CLR_QUEUE "CLRQUE" // ()
#define RE_DELAY "DELAY" // ()

#define REQUEST_UPDATE "REQUPD" // ()
#define PUBLISH_READINGS "PUBREAD" // ()
#define NOTIFY "NOTIFY" // ()

#define RESTART_UNIT "RESTART" // ()

#endif