/*
 * Copyright (c) 2019 Myles McNamara <myles@smyl.es>
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SMYLES_MOS_LIBS_CAPTIVE_PORTAL_WIFI_SETUP_H_
#define SMYLES_MOS_LIBS_CAPTIVE_PORTAL_WIFI_SETUP_H_

#include <stdbool.h>
#include <mgos.h>
#include "mgos_event.h"

typedef void (*wifi_setup_test_cb_t)(bool result, const char *ssid, const char* password, void *userdata);

#define MGOS_CAPTIVE_PORTAL_WIFI_SETUP_EV_BASE MGOS_EVENT_BASE('C', 'P', 'S')

enum mgos_wifi_captive_portal_event
{
    /**
     * Fired when WiFi Setup/Config testing is Started
     * 
     * ev_data: struct mgos_config_wifi_sta *sta
     */
    MGOS_CAPTIVE_PORTAL_WIFI_SETUP_TEST_START = MGOS_CAPTIVE_PORTAL_WIFI_SETUP_EV_BASE,
    /**
     * Fired when succesful connection test for Wifi
     * 
     * ev_data: struct mgos_config_wifi_sta *sta
     */
    MGOS_CAPTIVE_PORTAL_WIFI_SETUP_TEST_SUCCESS,
    /**
     * Fired when failed connection test Wifi
     * 
     * ev_data: struct mgos_config_wifi_sta *sta
     */
    MGOS_CAPTIVE_PORTAL_WIFI_SETUP_TEST_FAILED
};

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * @brief Start WiFi Credential/Connection test for ENTERPRISE WPA2 networks
 * 
 * @param ssid 
 * @param pass 
 * @param user 
 * @param cb 
 * @param userdata 
 * @return true 
 * @return false 
 */
bool mgos_captive_portal_wifi_setup_test_ent(char *ssid, char *pass, char* user, wifi_setup_test_cb_t cb, void *userdata );
/**
 * @brief Start WiFi Credential/Connection test for Standard OPEN/WEP/WPA2 networks
 * 
 * @param ssid 
 * @param pass 
 * @param cb 
 * @param userdata 
 * @param user 
 * @return true 
 * @return false 
 */
bool mgos_captive_portal_wifi_setup_test(char *ssid, char *pass, wifi_setup_test_cb_t cb, void *userdata );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SMYLES_MOS_LIBS_CAPTIVE_PORTAL_WIFI_SETUP_H_ */
