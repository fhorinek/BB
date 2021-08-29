
#include "gui/widgets/widget.h"
#include "drivers/power/pwr_mng.h"

REGISTER_WIDGET_IU
(
    Battery,
    "Battery percent",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
	lv_obj_t * sub_text;
);


static void Battery_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, NULL);

    local->value = widget_add_value(base, slot, "", &local->sub_text);
}

static void Battery_update(widget_slot_t * slot)
{
    char value[8];

    format_percent(value, pwr.fuel_gauge.battery_percentage);

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value, slot->obj);

    if (pwr.charger.charge_port == PWR_CHARGE_WEAK)
    	lv_label_set_text(local->sub_text, "weak chrg.");
    else if (pwr.charger.charge_port == PWR_CHARGE_SLOW)
    	lv_label_set_text(local->sub_text, "slow chrg.");
    else if (pwr.charger.charge_port == PWR_CHARGE_FAST)
    	lv_label_set_text(local->sub_text, "fast chrg.");
    else if (pwr.charger.charge_port == PWR_CHARGE_QUICK)
    	lv_label_set_text(local->sub_text, "quick chrg.");
    else if (pwr.data_port == PWR_DATA_CHARGE)
    	lv_label_set_text(local->sub_text, "slow chrg.");
    else
    	lv_label_set_text(local->sub_text, "");
}


