// Tower Kit documentation https://tower.hardwario.com/
// SDK API description https://sdk.hardwario.com/
// Forum https://forum.hardwario.com/

#include <application.h>

// LED instance
twr_led_t lcdLed_red;
twr_led_t lcdLed_blue;

// Button instance
twr_button_t button_left;
twr_button_t button_right;

// Thermometer instance
twr_tmp112_t tmp112;
uint16_t button_click_count = 0;

char *order_url= "testurl/start";
bool changesdone = false;

void twr_change_url(uint64_t *id, const char *topic, void *value, void *param);

// Button event callback
void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;
    // Log button event
    twr_log_info("APP: Button event: %i", event);

    // Check event source
    if (event == TWR_BUTTON_EVENT_PRESS && (int) event_param == 1)
    {
        twr_log_info("right button clicked");
        // Toggle LED pin state
        twr_led_set_mode(&lcdLed_blue, 2);
        twr_led_set_mode(&lcdLed_red, 1);
        bool variable;
        bool *ptr = &variable;
        *ptr = true;
        twr_radio_pub_bool("getnewurl", ptr);

         // Publish message on radio
        button_click_count++;
        twr_radio_pub_push_button(&button_click_count);
    } else if (event == TWR_BUTTON_EVENT_PRESS && (int) event_param == 0)
    {
        twr_log_info("left button clicked");
        twr_led_set_mode(&lcdLed_blue, 1);
        twr_led_set_mode(&lcdLed_red, 2);

         // Publish message on radio
        if(button_click_count >= 1)
        {
            button_click_count--;
        }
        twr_radio_pub_push_button(&button_click_count);
        /* code */
    }
 }

 static const twr_radio_sub_t subs[] = {
    {"denurity/urlchange", TWR_RADIO_SUB_PT_STRING, twr_change_url, NULL},
    {"/", TWR_RADIO_SUB_PT_STRING, twr_change_url, NULL},
};

void twr_change_url(uint64_t *id, const char *topic, void *value, void *param)
{
    twr_log_info("sub worked------");
    changesdone = true;
    //snprintf(order_url, sizeof(order_url), "%s", (char*)value);
    twr_log_info(value);
    twr_radio_pub_string("urlchange",order_url);
}
// void tmp112_event_handler(twr_tmp112_t *self, twr_tmp112_event_t event, void *event_param)
// {
//     if (event == TWR_TMP112_EVENT_UPDATE)
//     {
//         float celsius;
//         // Read temperature
//         twr_tmp112_get_temperature_celsius(self, &celsius);

//         twr_log_debug("APP: temperature: %.2f °C", celsius);

//         twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE, &celsius);
//     }
// }

// Application initialization function which is called once after boot
void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_ABS);

    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);
    twr_radio_set_rx_timeout_for_sleeping_node(500); // radio will be turned on for receiving a return message, time in milliseconds
    twr_radio_set_subs((twr_radio_sub_t *) subs, sizeof(subs)/sizeof(twr_radio_sub_t));
    twr_radio_pairing_request("duncan-test", VERSION);

    const twr_led_driver_t* driver = twr_module_lcd_get_led_driver();
    twr_led_init_virtual(&lcdLed_blue, TWR_MODULE_LCD_LED_GREEN, driver, 1);
    twr_led_init_virtual(&lcdLed_red, TWR_MODULE_LCD_LED_RED, driver, 1);

    const twr_button_driver_t* lcdButtonDriver =  twr_module_lcd_get_button_driver();
    twr_button_init_virtual(&button_left, 0, lcdButtonDriver, 0);
    twr_button_init_virtual(&button_right, 1, lcdButtonDriver, 0);

    twr_button_set_event_handler(&button_left, button_event_handler, (int*)0);
    twr_button_set_event_handler(&button_right, button_event_handler, (int*)1);

    twr_button_set_hold_time(&button_left, 500);
    twr_button_set_hold_time(&button_right, 500);

    twr_module_lcd_init();
    twr_module_lcd_set_font(&twr_font_ubuntu_15);
}

// Application task function (optional) which is called peridically if scheduled
void application_task(void)
{
    char * vOut = changesdone ? "true" : "false";
    twr_log_info(vOut);
    unsigned char ch;
    unsigned int number = sizeof(subs)/sizeof(twr_radio_sub_t);
    memcpy(ch, (char*)&number, 2);
    twr_log_info(ch);
    twr_module_lcd_clear();

    static char num[25];
    sprintf(num, "amount clicked: %i", button_click_count);

    twr_module_lcd_draw_string(10, 5, num, true);

    twr_module_lcd_update();

    twr_scheduler_plan_current_relative(500);
}
