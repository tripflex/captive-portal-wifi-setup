# Mongoose OS WiFi Setup

[![Gitter](https://badges.gitter.im/cesanta/mongoose-os.svg)](https://gitter.im/cesanta/mongoose-os?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

- [Mongoose OS WiFi Setup](#mongoose-os-wifi-setup)
  - [Captive Portal Stack](#captive-portal-stack)
  - [Author](#author)
  - [Features](#features)
  - [Settings](#settings)
      - [`cportal.setup.copy` Setting](#cportalsetupcopy-setting)
      - [`cportal.setup.disable` Setting](#cportalsetupdisable-setting)
      - [`cportal.redirect_file` Setting](#cportalredirect_file-setting)
  - [Installation/Usage](#installationusage)
    - [Full Captive Portal Stack](#full-captive-portal-stack)
    - [Only this library](#only-this-library)
    - [Use specific branch of library](#use-specific-branch-of-library)
  - [Required Libraries](#required-libraries)
  - [How it works](#how-it-works)
  - [Events](#events)
    - [Handling/Triggering Events in mJS](#handlingtriggering-events-in-mjs)
  - [Available Functions/Methods](#available-functionsmethods)
    - [C Functions](#c-functions)
    - [Usage in mJS](#usage-in-mjs)
  - [Changelog](#changelog)
  - [License](#license)

This library is for testing, saving, and setting up Mongoose OS device's WiFi.  The main feature of this library is the ability to test WiFi credentials, and then save or update them in the configuration.

## Captive Portal Stack

This is the **WiFi Setup** library from the [Captive Portal WiFi Full Stack](https://github.com/tripflex/captive-portal-wifi-stack), a full stack (frontend web ui & backend handling) library for implementing a full Captive Portal WiFi with Mongoose OS

## Author
Myles McNamara ( https://smyl.es )

## Features
- Test WiFi SSID and Passwords
- Automatically save SSID and Password after validating SSID and Password
- Automatically disable AP after successful WiFi test
- Automatically reboot the device after saving and successful test
- Set which STA index to save configuration to `wifi.sta` `wifi.sta1` or `wifi.sta2`
- Set custom timeout for testing connection and credentials (`30` seconds by default `cportal.setup.timeout`)
- Mongoose OS successful, failed, and started test events
- Callback for failed/successful test for use in `C` or `mJS`
- Disable Captive Portal setting after successful test
- Support for testing Enterprise WPA2 Networks

## Settings
Check the `mos.yml` file for latest settings, all settings listed below are defaults

```yaml
  - [ "cportal.setup.copy", "i", 0, {title: "Copy SSID and Password to this STA ID after succesful test ( 0 - wifi.sta | 1 - wifi.sta1 | 2 - wifi.sta2 )"}]
  - [ "cportal.setup.timeout", "i", 30, {title: "Timeout, in seconds, before considering a WiFi connection test as failed"}]
  - [ "cportal.setup.disable", "i", 2, {title: "Action to perform after successful test and copy/save values -- 0 - do nothing, 1 - Disable AP (wifi.ap.enable), 2 - Disable AP and Captive Portal (cportal.enable)"}]
  - [ "cportal.setup.reboot", "i", 0, {title: "0 to disable, or value (in seconds) to wait and then reboot device, after successful test (and copy/save values)"}]

```
#### `cportal.setup.copy` Setting
Set this equal to the STA ID you want to save the values to (if you want to save them after successful test).  Use `-1` to disable saving after successful test.

#### `cportal.setup.disable` Setting
Action to perform after successful test and copy/save values (if enabled)
 - `0` - Do Nothing
 - `1` - Disable AP `wifi.ap.enable`
 - `2` - Disable AP `wifi.ap.enable` and Captive Portal `cportal.enable`

#### `cportal.redirect_file` Setting
This setting if for if you want to use your own custom HTML file as the "redirect" file sent that includes a `meta` refresh tag.  This setting is optional, and when not defined, a dynamically generated response will be sent, that looks similar to this:

## Installation/Usage

### Full Captive Portal Stack
If you want all of the features this library was built for, you should install the [Captive Portal WiFi Stack](https://github.com/tripflex/captive-portal-wifi-stack) library instead of just this one:

Add this lib your `mos.yml` file under `libs:`

```yaml
  - origin: https://github.com/tripflex/captive-portal-wifi-stack
```

### Only this library

Add this lib your `mos.yml` file under `libs:`

```yaml
  - origin: https://github.com/tripflex/captive-portal-wifi-setup
```

### Use specific branch of library
To use a specific branch of this library (as example, `dev`), you need to specify the `version` below the library

```yaml
  - origin: https://github.com/tripflex/captive-portal-wifi-setup
   version: dev
```

## Required Libraries
*These libraries are already defined as dependencies of this library, and is just here for reference (you're probably already using these anyways)*
- [wifi](https://github.com/mongoose-os-libs/wifi)
  
## How it works
This library adds `C` functions you can use to test wifi credentials (see below).  For a complete solution, use the Captive Portal WiFi Stack which adds easy methods for calling this (web ui, etc).

## Events

```C
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
```

### Handling/Triggering Events in mJS
You can add event handling in mJS by adding this to one of your `.js` files:

```javascript
let CaptivePortalWiFiSetup = {
    START: Event.baseNumber("CPS"),
    SUCCESS: CaptivePortalWiFiSetup.START + 1,
    FAILED: CaptivePortalWiFiSetup.START + 2
};

function CaptivePortalWiFiEvent(ev, evdata, arg) {
    if (ev === CaptivePortalWiFiSetup.START) {
      // Test started
    } else if (ev === CaptivePortalWiFiSetup.SUCCESS) {
      // Successful test
    } else if (ev === CaptivePortalWiFiSetup.FAILED) {
      // Failed test
    }
}

Event.addGroupHandler(CaptivePortalWiFiSetup.START, CaptivePortalWiFiEvent, null);
```

## Available Functions/Methods

### C Functions
```C
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
```
```C
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
```

The `mgos_captive_portal_wifi_setup_test` will return `true` if the test was started, `false` if it was not

The `wifi_setup_test_cb_t` is for a callback (optional) after a successful/failed wifi test.
```C
typedef void (*wifi_setup_test_cb_t)(bool result, char *ssid, char* password, void *userdata);
```

It will return the result `false` for failed `true` for success, the tested SSID, Password, and any userdata if you passed it when originally calling the function.

### Usage in mJS
To keep library size to a minimum, no mjs file is included with this library, but you can easily call it using the built in **ffi** for mjs, like this:
```javascript
let testWiFi = ffi('bool mgos_captive_portal_wifi_setup_test(char*,char*,void(*)(bool,char*,char*,userdata),userdata)')
testWiFi( "My SSID", "somePassword", function( success, ssid, pass, userdata){
  print( 'Test Completed!');
}, NULL );
```

## Changelog

**1.0.1** (March 10, 2019) 
 - Added support for Enterprise Networks
 - Updated event base to `CPS`

**1.0.0** (March 9, 2019)
 - Initial release

## License
Apache 2.0