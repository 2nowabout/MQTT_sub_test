// Tower Kit documentation https://tower.hardwario.com/
// SDK API description https://sdk.hardwario.com/
// Forum https://forum.hardwario.com/

#include <application.h>

// LED instance
bc_led_t lcdLed_red;
bc_led_t lcdLed_blue;

// Button instance
bc_button_t button_left;
bc_button_t button_right;

// Thermometer instance
bc_tmp112_t tmp112;
uint16_t button_click_count = 0;

char *order_url= "testurl/start";
bool changesdone = false;

void bc_change_url(uint64_t *id, const char *topic, void *value, void *param);

 static const bc_radio_sub_t subs[] = {
    {"denurity/-/url/change", BC_RADIO_SUB_PT_INT, bc_change_url, NULL},
};

void bc_change_url(uint64_t *id, const char *topic, void *value, void *param)
{
    bc_log_info("sub worked------");
    changesdone = true;
    //snprintf(order_url, sizeof(order_url), "%s", (char*)value);
    bc_log_info(value);
    bc_radio_pub_string("urlIsChanged",order_url);
}
// void tmp112_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param)
// {
//     if (event == bc_TMP112_EVENT_UPDATE)
//     {
//         float celsius;
//         // Read temperature
//         bc_tmp112_get_temperature_celsius(self, &celsius);

//         bc_log_debug("APP: temperature: %.2f °C", celsius);

//         bc_radio_pub_temperature(bc_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE, &celsius);
//     }
// }

// Button event callback
void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;
    // Log button event
    bc_log_info("APP: Button event: %i", event);

    // Check event source
    if (event == BC_BUTTON_EVENT_PRESS && (int) event_param == 1)
    {
        bc_log_info("right button clicked");
        // Toggle LED pin state
        bc_led_set_mode(&lcdLed_blue, 2);
        bc_led_set_mode(&lcdLed_red, 1);
        bool variable;
        bool *ptr = &variable;
        *ptr = true;
        bc_radio_pub_bool("getnewurl", ptr);

         // Publish message on radio
        button_click_count++;
        bc_radio_pub_push_button(&button_click_count);
    } else if (event == BC_BUTTON_EVENT_PRESS && (int) event_param == 0)
    {
        bc_log_info("left button clicked");
        bc_led_set_mode(&lcdLed_blue, 1);
        bc_led_set_mode(&lcdLed_red, 2);

         // Publish message on radio
        if(button_click_count >= 1)
        {
            button_click_count--;
        }
        bc_radio_pub_push_button(&button_click_count);
        /* code */
    }
 }

// Application initialization function which is called once after boot
void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DEBUG, BC_LOG_TIMESTAMP_ABS);

    bc_radio_init(BC_RADIO_MODE_NODE_LISTENING);
    bc_radio_set_rx_timeout_for_sleeping_node(500); // radio will be turned on for receiving a return message, time in milliseconds
    bc_radio_set_subs((bc_radio_sub_t *) subs, sizeof(subs)/sizeof(bc_radio_sub_t));
    bc_radio_pairing_request("duncan", VERSION);

    const bc_led_driver_t* driver = bc_module_lcd_get_led_driver();
    bc_led_init_virtual(&lcdLed_blue, BC_MODULE_LCD_LED_GREEN, driver, 1);
    bc_led_init_virtual(&lcdLed_red, BC_MODULE_LCD_LED_RED, driver, 1);

    const bc_button_driver_t* lcdButtonDriver =  bc_module_lcd_get_button_driver();
    bc_button_init_virtual(&button_left, 0, lcdButtonDriver, 0);
    bc_button_init_virtual(&button_right, 1, lcdButtonDriver, 0);

    bc_button_set_event_handler(&button_left, button_event_handler, (int*)0);
    bc_button_set_event_handler(&button_right, button_event_handler, (int*)1);

    bc_button_set_hold_time(&button_left, 500);
    bc_button_set_hold_time(&button_right, 500);

    bc_module_lcd_init();
    bc_module_lcd_set_font(&bc_font_ubuntu_15);
}

// Application task function (optional) which is called peridically if scheduled
void application_task(void)
{
    char * vOut = changesdone ? "true" : "false";
    bc_log_info(vOut);
    bc_module_lcd_clear();

    static char num[25];
    sprintf(num, "amount clicked: %i", button_click_count);

    bc_module_lcd_draw_string(10, 5, num, true);

    bc_module_lcd_update();

    bc_scheduler_plan_current_relative(500);
}
