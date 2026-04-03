#ifndef FSTATUS_CONFIG_H_
#define FSTATUS_CONFIG_H_

#include "types.h"

#define SHCMD(cmd_) { "/bin/sh", "-c", cmd_, NULL }

static const char* volume_cmd[] = 
    SHCMD("pactl get-sink-volume @DEFAULT_SINK@ | awk 'NR==1{print $5}'");
static const char* layout_cmd[] = { "xkb-switch", NULL };

#include "modules/temperature.h"
#include "modules/cpu.h"
#include "modules/customcmd.h"
#include "modules/time.h"
#include "modules/battery.h"

static const Module mod_temperature = 
    MODULE(Temperature, .thermal_zone = "thermal_zone5");
static const Module mod_cpu = 
    MODULE(Cpu, DEFAULTS);
static const Module mod_volume = 
    MODULE(CustomCmd, .command = volume_cmd);
static const Module mod_layout = 
    MODULE(CustomCmd, .command = layout_cmd);
static const Module mod_time = 
    MODULE(Time, .format = "%a %d %b %H:%M");
static const Module mod_battery = 
    MODULE(Battery, .battery_name = "BAT1", .adapter_name = "ACAD");

static const Format status_format[] = {
    FTEXT("[temp:"), FMOD(mod_temperature), FTEXT("C] "),
    FTEXT("[cpu:"), FMOD(mod_cpu), FTEXT("%] "),
    FTEXT("[vol:"), FMOD(mod_volume), FTEXT("] "),
    FTEXT("["), FMOD(mod_layout), FTEXT("] "),
    FTEXT("["), FMOD(mod_time), FTEXT("] "),
    FTEXT("[bat:"), FMOD(mod_battery), FTEXT("]"),
};

static const double status_update_interval_seconds = 1.0;

#endif // FSTATUS_CONFIG_H_
