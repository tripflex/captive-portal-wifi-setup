/*
 * Copyright (c) 2019 Myles McNamara <https://smyl.es>
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
#include <stdlib.h>
#include <string.h>

#include "mgos_wifi.h"
#include "mgos_utils.h"
#include "mgos_timers.h"
#include "mgos_config.h"
#include "mgos_mongoose.h"
#include "mgos_captive_portal_wifi_setup.h"

#include "mongoose.h"

static int s_connection_retries = 0;
static mgos_timer_id s_connect_timer_id = MGOS_INVALID_TIMER_ID;

static wifi_setup_test_cb_t s_wifi_setup_test_cb = NULL;
static void *s_wifi_setup_test_userdata = NULL;

static struct mgos_config_wifi_sta *sp_test_sta_vals = NULL;

static void remove_event_handlers(void);
static void add_event_handlers(void);

static void clear_timeout_vals(void){
    mgos_clear_timer(s_connect_timer_id);
    s_connect_timer_id = MGOS_INVALID_TIMER_ID;
    remove_event_handlers();
    s_connection_retries = 0;

    if( s_wifi_setup_test_cb != NULL ){
        s_wifi_setup_test_cb( false, sp_test_sta_vals->ssid, sp_test_sta_vals->pass, s_wifi_setup_test_userdata );
    }
}

static void sta_connect_timeout_timer_cb(void *arg) {
    clear_timeout_vals();
    mgos_event_trigger(MGOS_CAPTIVE_PORTAL_WIFI_SETUP_TEST_FAILED, sp_test_sta_vals);
    LOG(LL_ERROR, ("Captive Portal WiFi Setup STA: Connect timeout"));

    (void) arg;
}

static void ip_aquired_cb(int ev, void *ev_data, void *userdata){
    char *connectedto = mgos_wifi_get_connected_ssid();
    // struct mgos_config_wifi_sta *sta = (struct mgos_config_wifi_sta *) sp_test_sta_vals;

    LOG(LL_INFO, ("Captive Portal WiFi Setup -- IP Acquired from SSID %s", connectedto ) );

    // TODO: maybe check that connected SSID matches the test SSID

    free(connectedto);

    // Clear timeout timer on IP Acquired
    clear_timeout_vals();
    remove_event_handlers();

    int sta_index = mgos_sys_config_get_cportal_setup_copy();

    if ( sp_test_sta_vals != NULL ){

        if( sta_index > -1 ){
            LOG(LL_INFO, ("Copying SSID %s and Password %s to STA 1 config (wifi.sta)", sp_test_sta_vals->ssid, sp_test_sta_vals->pass ) );

            // TODO: figure out a way to dynamically set the fn to call instead of all this extra code
            // Maybe check mamuesp common tools for
            // value = tools_config_get_dyn("my.configuration.%s.my_value", my_key, true);
            if( sta_index == 0 ){
                mgos_sys_config_set_wifi_sta_enable(true);
                mgos_sys_config_set_wifi_sta_ssid(sp_test_sta_vals->ssid);
                mgos_sys_config_set_wifi_sta_pass(sp_test_sta_vals->pass);

                if (!mgos_conf_str_empty(sp_test_sta_vals->user)){
                    mgos_sys_config_set_wifi_sta_user(sp_test_sta_vals->user);
                    mgos_sys_config_set_wifi_sta_anon_identity(sp_test_sta_vals->user);
                    mgos_sys_config_set_wifi_sta_ca_cert("");
                } else {
                    mgos_sys_config_set_wifi_sta_user("");
                    mgos_sys_config_set_wifi_sta_anon_identity("");
                    mgos_sys_config_set_wifi_sta_ca_cert("");
                }

            } else if( sta_index == 1){
                mgos_sys_config_set_wifi_sta1_enable(true);
                mgos_sys_config_set_wifi_sta1_ssid(sp_test_sta_vals->ssid);
                mgos_sys_config_set_wifi_sta1_pass(sp_test_sta_vals->pass);

                if (!mgos_conf_str_empty(sp_test_sta_vals->user)){
                    mgos_sys_config_set_wifi_sta1_user(sp_test_sta_vals->user);
                    mgos_sys_config_set_wifi_sta1_anon_identity(sp_test_sta_vals->user);
                    mgos_sys_config_set_wifi_sta1_ca_cert("");
                } else {
                    mgos_sys_config_set_wifi_sta1_user("");
                    mgos_sys_config_set_wifi_sta1_anon_identity("");
                    mgos_sys_config_set_wifi_sta1_ca_cert("");
                }

            } else if( sta_index == 2){
                mgos_sys_config_set_wifi_sta2_enable(true);
                mgos_sys_config_set_wifi_sta2_ssid(sp_test_sta_vals->ssid);
                mgos_sys_config_set_wifi_sta2_pass(sp_test_sta_vals->pass);

                if (!mgos_conf_str_empty(sp_test_sta_vals->user)){
                    mgos_sys_config_set_wifi_sta2_user(sp_test_sta_vals->user);
                    mgos_sys_config_set_wifi_sta2_anon_identity(sp_test_sta_vals->user);
                    mgos_sys_config_set_wifi_sta2_ca_cert("");
                } else {
                    mgos_sys_config_set_wifi_sta2_user("");
                    mgos_sys_config_set_wifi_sta2_anon_identity("");
                    mgos_sys_config_set_wifi_sta2_ca_cert("");
                }

            }
        }

        int disable = mgos_sys_config_get_cportal_setup_disable();
        if ( disable == 1 || disable == 2 ){
            mgos_sys_config_set_wifi_ap_enable(false);
        }
        // Disable captive portal
        if( disable == 2 ){
            mgos_sys_config_set_cportal_enable(false);
        }

        int enable = mgos_sys_config_get_cportal_setup_enable();
        if( enable == 1 ){
            mgos_sys_config_set_dns_sd_enable(true);
        }

        char *err = NULL;
        if (!save_cfg(&mgos_sys_config, &err)){
            LOG(LL_ERROR, ("Copy STA Values, Save Config Error: %s", err));
            free(err);
        } else {

            int reboot_ms = (mgos_sys_config_get_cportal_setup_reboot() * 1000);
            if (reboot_ms > 0){
                mgos_system_restart_after(reboot_ms);
            }
        }

        mgos_event_trigger(MGOS_CAPTIVE_PORTAL_WIFI_SETUP_TEST_SUCCESS, sp_test_sta_vals);
        if( s_wifi_setup_test_cb != NULL ){
            s_wifi_setup_test_cb( true, sp_test_sta_vals->ssid, sp_test_sta_vals->pass, s_wifi_setup_test_userdata );
        }
    }

    (void)ev;
    (void)ev_data;
    (void)userdata;
}

static void maybe_reconnect(int ev, void *ev_data, void *userdata){
    s_connection_retries++;
    LOG(LL_INFO, ("Wifi Captive Portal - Retrying Connection... Attempt %d", s_connection_retries ) );
    mgos_wifi_connect();
    (void)ev;
    (void)ev_data;
    (void)userdata;
}

static void remove_event_handlers(void){
    mgos_event_remove_handler(MGOS_WIFI_EV_STA_DISCONNECTED, maybe_reconnect, NULL);
    mgos_event_remove_handler(MGOS_WIFI_EV_STA_IP_ACQUIRED, ip_aquired_cb, NULL);
}

static void add_event_handlers(void){
    // We use NULL for userdata to make sure they are removed correctly
    mgos_event_add_handler(MGOS_WIFI_EV_STA_IP_ACQUIRED, ip_aquired_cb, NULL);
    mgos_event_add_handler(MGOS_WIFI_EV_STA_DISCONNECTED, maybe_reconnect, NULL);
}

static bool wifi_setup_test_start(wifi_setup_test_cb_t cb, void *userdata){
    s_wifi_setup_test_cb = cb;
    s_wifi_setup_test_userdata = userdata;

    // Make sure to remove any existing handlers (in case of previous test already set)
    remove_event_handlers();

    if ( s_connect_timer_id == MGOS_INVALID_TIMER_ID ){
        int timeout_ms = (mgos_sys_config_get_cportal_setup_timeout() * 1000);
        s_connect_timer_id = mgos_set_timer(timeout_ms, 0, sta_connect_timeout_timer_cb, NULL);
    }

    mgos_wifi_disconnect();
    bool result = mgos_wifi_setup_sta(sp_test_sta_vals);

    if ( result ){
        mgos_event_trigger(MGOS_CAPTIVE_PORTAL_WIFI_SETUP_TEST_START, sp_test_sta_vals);
        add_event_handlers();
    }

    return true;
}

bool mgos_captive_portal_wifi_setup_test_ent(char *ssid, char *pass, char* user, wifi_setup_test_cb_t cb, void *userdata ){
    if (mgos_conf_str_empty(ssid) || mgos_conf_str_empty(user)){
        return false;
    }

    if (sp_test_sta_vals == NULL){
        // Allocate memory to store sta values in
        sp_test_sta_vals = (struct mgos_config_wifi_sta *)calloc(1, sizeof(*sp_test_sta_vals));
    }

    sp_test_sta_vals->enable = 1; // Same as (*test_sta_vals).enable
    sp_test_sta_vals->ssid = ssid;
    sp_test_sta_vals->pass = pass;
    sp_test_sta_vals->user = user;
    sp_test_sta_vals->anon_identity = user;
    sp_test_sta_vals->ca_cert = "";
	sp_test_sta_vals->protocol = mgos_sys_config_get_wifi_sta_protocol();
    
    LOG(LL_INFO, ("Captive Portal WiFi Setup Testing SSID: %s PASS: %s USER: %s", ssid, pass, user));

    return wifi_setup_test_start(cb, userdata);
}

bool mgos_captive_portal_wifi_setup_test(char *ssid, char *pass, wifi_setup_test_cb_t cb, void *userdata ){

    if (mgos_conf_str_empty(ssid)){
        return false;
    }

    if (sp_test_sta_vals == NULL){
        // Allocate memory to store sta values in
        sp_test_sta_vals = (struct mgos_config_wifi_sta *)calloc(1, sizeof(*sp_test_sta_vals));
    }

    sp_test_sta_vals->enable = 1; // Same as (*test_sta_vals).enable
    sp_test_sta_vals->ssid = ssid;
    sp_test_sta_vals->pass = pass;
    // Set these empty just in case of previous ENTERPRISE network test
    sp_test_sta_vals->user = "";
    sp_test_sta_vals->anon_identity = "";
    sp_test_sta_vals->ca_cert = "";
	sp_test_sta_vals->protocol = mgos_sys_config_get_wifi_sta_protocol();
    
    LOG(LL_INFO, ("Captive Portal WiFi Setup Testing SSID: %s PASS: %s", ssid, pass));

    return wifi_setup_test_start(cb, userdata);
}

bool mgos_captive_portal_wifi_setup_init(void){
    mgos_event_register_base(MGOS_CAPTIVE_PORTAL_WIFI_SETUP_EV_BASE, "Captive Portal WiFi Setup");
    return true;
}