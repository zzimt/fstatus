#ifndef FSTATUS_CONFIG_H_
#define FSTATUS_CONFIG_H_

#include "types.h"

#define SHCMD(cmd_) { "/bin/sh", "-c", cmd_, NULL }

static const char* layout_cmd[] = { "xkb-switch", NULL };

#include "modules/temperature.h"
#include "modules/cpu.h"
#include "modules/volume.h"
#include "modules/customcmd.h"
#include "modules/time.h"
#include "modules/battery.h"

static const Module mod_temperature = 
    MODULE(Temperature, .thermal_zone = "thermal_zone5");
static const Module mod_cpu = 
    MODULE(Cpu, DEFAULTS);
static const Module mod_volume = 
    MODULE(Volume, .muted_text = "x");
static const Module mod_layout = 
    MODULE(CustomCmd, .command = layout_cmd);
static const Module mod_time = 
    MODULE(Time, .format = "%a %d %b %H:%M");
static const Module mod_battery = 
    MODULE(Battery, .battery_name = "BAT1", .adapter_name = "ACAD");

static const Format status_format[] = {
    FTEXT("[t:"), FMOD(mod_temperature), FTEXT("C]"),
    FTEXT("[c:"), FMOD(mod_cpu), FTEXT("%]"),
    FTEXT("[v:"), FMOD(mod_volume), FTEXT("]"),
    FTEXT("["), FMOD(mod_layout), FTEXT("]"),
    FTEXT("["), FMOD(mod_time), FTEXT("]"),
    FTEXT("[b:"), FMOD(mod_battery), FTEXT("]"),
};

static const double status_update_interval_seconds = 1.0;

#endif // FSTATUS_CONFIG_H_
