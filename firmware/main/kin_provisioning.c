#include "kin_provisioning.h"
#include "board_kinpod_2.06.h"
#include "kin_wifi.h"
#include "kin_ws.h"
#include "kin_avatar.h"
#include "kin_audio.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_heap_caps.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "cJSON.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char *TAG = "KinProv";

static bool s_is_active = false;
static httpd_handle_t s_http_server = NULL;
static TaskHandle_t s_dns_task_handle = NULL;
static int s_dns_sock = -1;
static uint16_t *s_prov_frame = NULL;

#define NVS_NAMESPACE "kin_config"
#define DNS_PORT 53

// Big-Endian RGB565 helper: Swaps bytes so DMA streams High Byte first, Low Byte second
static inline uint16_t rgb565_be(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t val = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return (uint16_t)((val >> 8) | (val << 8));
}

// 5x7 Font for typography
static const uint8_t s_font5x7[59][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!'
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // '$'
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%'
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&'
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '''
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // '('
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ')'
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // '*'
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // '+'
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ','
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-'
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.'
    {0x20, 0x10, 0x08, 0x04, 0x02}, // '/'
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9'
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':'
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ';'
    {0x00, 0x08, 0x14, 0x22, 0x41}, // '<'
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '='
    {0x41, 0x22, 0x14, 0x08, 0x00}, // '>'
    {0x02, 0x01, 0x51, 0x09, 0x06}, // '?'
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // '@'
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E'
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // 'F'
    {0x3E, 0x41, 0x41, 0x51, 0x32}, // 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L'
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R'
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V'
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X'
    {0x03, 0x04, 0x78, 0x04, 0x03}, // 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 'Z'
};

static inline void draw_pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < KIN_LCD_H_RES && y >= 0 && y < KIN_LCD_V_RES) {
        s_prov_frame[y * KIN_LCD_H_RES + x] = color;
    }
}

static void fill_rect(int x, int y, int w, int h, uint16_t color) {
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= KIN_LCD_V_RES) continue;
        int row_offset = j * KIN_LCD_H_RES;
        for (int i = x; i < x + w; i++) {
            if (i < 0 || i >= KIN_LCD_H_RES) continue;
            s_prov_frame[row_offset + i] = color;
        }
    }
}

static void fill_rounded_rect(int x, int y, int w, int h, int r, uint16_t color) {
    fill_rect(x + r, y, w - 2 * r, h, color);
    fill_rect(x, y + r, r, h - 2 * r, color);
    fill_rect(x + w - r, y + r, r, h - 2 * r, color);

    for (int dy = 0; dy < r; dy++) {
        for (int dx = 0; dx < r; dx++) {
            if ((r - dx) * (r - dx) + (r - dy) * (r - dy) <= r * r) {
                draw_pixel(x + dx, y + dy, color);
                draw_pixel(x + w - 1 - dx, y + dy, color);
                draw_pixel(x + dx, y + h - 1 - dy, color);
                draw_pixel(x + w - 1 - dx, y + h - 1 - dy, color);
            }
        }
    }
}

static void draw_rounded_rect_border(int x, int y, int w, int h, int r, uint16_t border_col) {
    fill_rect(x + r, y, w - 2 * r, 2, border_col);
    fill_rect(x + r, y + h - 2, w - 2 * r, 2, border_col);
    fill_rect(x, y + r, 2, h - 2 * r, border_col);
    fill_rect(x + w - 2, y + r, 2, h - 2 * r, border_col);
}

static void draw_string(int x, int y, const char *str, uint16_t color, int scale) {
    while (*str) {
        char c = *str++;
        if (c >= 'a' && c <= 'z') c -= 32;
        if (c >= ' ' && c <= 'Z') {
            const uint8_t *glyph = s_font5x7[c - ' '];
            for (int col = 0; col < 5; col++) {
                uint8_t line = glyph[col];
                for (int row = 0; row < 7; row++) {
                    if (line & (1 << row)) {
                        fill_rect(x + col * scale, y + row * scale, scale, scale, color);
                    }
                }
            }
            x += (5 + 1) * scale;
        } else {
            x += 4 * scale;
        }
    }
}

static int string_width(const char *str, int scale) {
    int len = 0;
    while (*str++) len++;
    if (len == 0) return 0;
    return len * 6 * scale - scale;
}

static void draw_string_centered(int y, const char *str, uint16_t color, int scale) {
    int w = string_width(str, scale);
    int x = (KIN_LCD_H_RES - w) / 2;
    if (x < 10) x = 10;
    draw_string(x, y, str, color, scale);
}

// -----------------------------------------------------------------------------
// NVS Storage Implementation
// -----------------------------------------------------------------------------
esp_err_t kin_provisioning_load_config(char *ssid_out, size_t ssid_len,
                                       char *pass_out, size_t pass_len,
                                       char *ws_url_out, size_t ws_url_len) {
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "No stored Wi-Fi config found in NVS (using defaults)");
        return ret;
    }

    if (ssid_out && ssid_len > 0) {
        size_t required = ssid_len;
        if (nvs_get_str(handle, "wifi_ssid", ssid_out, &required) == ESP_OK) {
            ESP_LOGI(TAG, "Loaded SSID from NVS: '%s'", ssid_out);
        }
    }

    if (pass_out && pass_len > 0) {
        size_t required = pass_len;
        if (nvs_get_str(handle, "wifi_pass", pass_out, &required) == ESP_OK) {
            ESP_LOGI(TAG, "Loaded Wi-Fi Password from NVS");
        }
    }

    if (ws_url_out && ws_url_len > 0) {
        size_t required = ws_url_len;
        if (nvs_get_str(handle, "ws_url", ws_url_out, &required) == ESP_OK) {
            ESP_LOGI(TAG, "Loaded WebSocket URL from NVS: '%s'", ws_url_out);
        }
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t kin_provisioning_save_config(const char *ssid, const char *pass, const char *ws_url) {
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) return ret;

    if (ssid && strlen(ssid) > 0) {
        nvs_set_str(handle, "wifi_ssid", ssid);
    }
    if (pass) {
        nvs_set_str(handle, "wifi_pass", pass);
    }
    if (ws_url && strlen(ws_url) > 0) {
        nvs_set_str(handle, "ws_url", ws_url);
    }

    ret = nvs_commit(handle);
    nvs_close(handle);
    ESP_LOGI(TAG, "Saved new credentials to NVS successfully [OK]");
    return ret;
}

// -----------------------------------------------------------------------------
// AMOLED Screen Rendering (Apple watchOS Style)
// -----------------------------------------------------------------------------
static void render_provisioning_screen(const char *custom_status) {
    if (!s_prov_frame) {
        s_prov_frame = (uint16_t *)heap_caps_malloc(KIN_LCD_H_RES * KIN_LCD_V_RES * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
        if (!s_prov_frame) return;
    }

    memset(s_prov_frame, 0x00, KIN_LCD_H_RES * KIN_LCD_V_RES * sizeof(uint16_t));

    const uint16_t col_card        = rgb565_be(24, 24, 26);     // #18181A Deep obsidian
    const uint16_t col_border      = rgb565_be(48, 48, 52);     // #303034
    const uint16_t col_accent_blue = rgb565_be(10, 132, 255);   // #0A84FF Apple Blue
    const uint16_t col_green       = rgb565_be(48, 209, 88);    // #30D158 Apple Green
    const uint16_t col_white       = 0xFFFF;
    const uint16_t col_gray        = rgb565_be(170, 170, 175);
    const uint16_t col_dim         = rgb565_be(75, 75, 80);
    const uint16_t col_btn_bg      = rgb565_be(38, 38, 42);

    // Grabber
    fill_rounded_rect(185, 12, 40, 4, 2, col_dim);

    // Title & Header (Centered safely)
    draw_string_centered(26, "WI-FI SETUP", col_white, 2);
    draw_string_centered(48, "CONNECT FROM YOUR PHONE", col_accent_blue, 2);

    // Main Guide Card (y=74, h=240, x=20, w=370)
    fill_rounded_rect(20, 74, 370, 240, 14, col_card);
    draw_rounded_rect_border(20, 74, 370, 240, 14, col_border);

    // Step 1
    draw_string(36, 90, "1. CONNECT PHONE TO WI-FI:", col_gray, 2);
    draw_string(36, 114, "SSID: KinPod-Setup", col_accent_blue, 2);

    // Step 2
    draw_string(36, 146, "2. OPEN BROWSER AT:", col_gray, 2);
    draw_string(36, 170, "http://192.168.4.1", col_green, 2);

    // Step 3
    draw_string(36, 202, "3. SELECT NETWORK & SAVE", col_white, 2);

    // Dynamic Live Status
    const char *status_text = custom_status ? custom_status : "WAITING FOR CONNECTION...";
    draw_string_centered(242, status_text, col_accent_blue, 2);

    // Details Card (y=326, h=92, x=20, w=370)
    fill_rounded_rect(20, 326, 370, 92, 14, col_card);
    draw_rounded_rect_border(20, 326, 370, 92, 14, col_border);

    draw_string(36, 342, "NETWORK INFO", col_gray, 2);
    draw_string(36, 364, "OPEN AP (NO PASSWORD)", col_white, 2);
    draw_string(36, 386, "HOTSPOT & WPA2/WPA3", col_gray, 2);

    // Cancel Button (y=432, h=52, x=20, w=370)
    fill_rounded_rect(20, 432, 370, 52, 14, col_btn_bg);
    draw_rounded_rect_border(20, 432, 370, 52, 14, col_border);
    draw_string_centered(448, "^ CANCEL SETUP ^", col_white, 2);

    kin_display_draw_raw_frame((const uint8_t *)s_prov_frame);
}

// -----------------------------------------------------------------------------
// HTTP Server & Captive Portal Web Page
// -----------------------------------------------------------------------------
static const char *s_setup_html =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'>"
"<title>KinPod Setup</title>"
"<style>"
"* { box-sizing: border-box; margin: 0; padding: 0; }"
"body { background: #000; color: #fff; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, sans-serif; padding: 24px 16px; }"
".card { background: #1c1c1e; border: 1px solid #303034; border-radius: 20px; padding: 24px; max-width: 440px; margin: 0 auto; box-shadow: 0 10px 30px rgba(0,0,0,0.6); }"
".header { display: flex; align-items: center; justify-content: space-between; margin-bottom: 6px; }"
"h1 { font-size: 22px; font-weight: 700; color: #fff; }"
".badge { background: #ff5a1f; color: #fff; font-size: 11px; padding: 3px 8px; border-radius: 6px; font-weight: 700; letter-spacing: 0.5px; }"
"p.sub { color: #8e8e93; font-size: 13px; margin-bottom: 18px; line-height: 1.4; }"
".tab-bar { display: flex; background: #2c2c2e; border-radius: 12px; padding: 4px; margin-bottom: 20px; border: 1px solid #3a3a3c; }"
".tab-btn { flex: 1; text-align: center; padding: 10px; font-size: 13px; font-weight: 600; color: #8e8e93; border-radius: 9px; cursor: pointer; transition: 0.2s; user-select: none; }"
".tab-btn.active { background: #0a84ff; color: #fff; box-shadow: 0 2px 8px rgba(10,132,255,0.4); }"
".note-box { background: rgba(10, 132, 255, 0.12); border: 1px solid rgba(10, 132, 255, 0.3); border-radius: 12px; padding: 14px; font-size: 13px; color: #d0e6ff; line-height: 1.5; margin-bottom: 18px; }"
".row-label { display: flex; justify-content: space-between; align-items: center; margin-bottom: 6px; }"
"label { font-size: 11px; color: #8e8e93; font-weight: 700; text-transform: uppercase; letter-spacing: 0.5px; }"
".rescan-btn { font-size: 12px; color: #0a84ff; text-decoration: none; font-weight: 600; cursor: pointer; }"
"select { width: 100%; background: #2c2c2e; border: 1px solid #3a3a3c; border-radius: 12px; color: #fff; font-size: 15px; padding: 13px 14px; margin-bottom: 12px; outline: none; -webkit-appearance: none; background-image: url('data:image/svg+xml;utf8,<svg fill=\"%238e8e93\" height=\"20\" viewBox=\"0 0 24 24\" width=\"20\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M7 10l5 5 5-5z\"/></svg>'); background-repeat: no-repeat; background-position: right 14px center; background-size: 18px; }"
"select:focus { border-color: #0a84ff; background-color: #323236; }"
"input { width: 100%; background: #2c2c2e; border: 1px solid #3a3a3c; border-radius: 12px; color: #fff; font-size: 15px; padding: 13px 14px; margin-bottom: 16px; outline: none; -webkit-appearance: none; }"
"input:focus { border-color: #0a84ff; background: #323236; }"
".hint { font-size: 11px; color: #636366; margin-top: -12px; margin-bottom: 16px; }"
"button.submit-btn { width: 100%; background: #0a84ff; color: #fff; font-size: 16px; font-weight: 600; border: none; border-radius: 12px; padding: 14px; cursor: pointer; transition: 0.2s; -webkit-appearance: none; }"
"button.submit-btn:active { background: #0071e3; transform: scale(0.98); }"
".status { margin-top: 16px; padding: 12px; border-radius: 10px; font-size: 14px; text-align: center; display: none; line-height: 1.4; }"
".success { background: rgba(48, 209, 88, 0.15); color: #30d158; border: 1px solid #30d158; }"
"</style>"
"</head>"
"<body>"
"<div class='card'>"
"  <div class='header'>"
"    <h1>KinPod</h1>"
"    <span class='badge'>SETUP</span>"
"  </div>"
"  <p class='sub'>Choose how you want to connect KinPod to the internet.</p>"
"  <div class='tab-bar'>"
"    <div id='tab_wifi' class='tab-btn active' onclick=\"setTab('wifi')\">📶 Home / Office Wi-Fi</div>"
"    <div id='tab_hotspot' class='tab-btn' onclick=\"setTab('hotspot')\">📱 Phone Hotspot</div>"
"  </div>"
"  <div id='view_wifi'>"
"    <div class='row-label'>"
"      <label for='ssid_select'>AVAILABLE WI-FI NETWORKS</label>"
"      <a class='rescan-btn' onclick='scanNetworks()'>↻ Rescan</a>"
"    </div>"
"    <select id='ssid_select' onchange='onSelectNetwork(this.value)'>"
"      <option value='' disabled selected>Scanning nearby networks...</option>"
"    </select>"
"    <label for='ssid' style='display:block; margin-bottom:6px;'>NETWORK NAME (SSID)</label>"
"    <input type='text' id='ssid' placeholder='Selected network or type manually' autocorrect='off' autocapitalize='off'>"
"    <label for='pass' style='display:block; margin-bottom:6px;'>WI-FI PASSWORD</label>"
"    <input type='password' id='pass' placeholder='Password (blank if open)'>"
"  </div>"
"  <div id='view_hotspot' style='display:none;'>"
"    <div class='note-box'>"
"      <b>📱 Mobile Hotspot Pairing:</b><br>"
"      Connecting to KinPod pauses your phone hotspot. Enter your hotspot details below and tap <b>Connect KinPod</b>. Then open <i>Settings &gt; Personal Hotspot</i> on your phone for 5 seconds so KinPod can pair. Saved permanently in memory!"
"    </div>"
"    <label for='hotspot_ssid' style='display:block; margin-bottom:6px;'>PHONE HOTSPOT NAME (SSID)</label>"
"    <input type='text' id='hotspot_ssid' placeholder='e.g. Fernando iPhone' autocorrect='off' autocapitalize='off'>"
"    <label for='hotspot_pass' style='display:block; margin-bottom:6px;'>HOTSPOT PASSWORD</label>"
"    <input type='password' id='hotspot_pass' placeholder='Your Hotspot password'>"
"  </div>"
"  <label for='ws' style='display:block; margin-bottom:6px;'>KINBRIDGE SERVER URL</label>"
"  <input type='text' id='ws' placeholder='ws://192.168.1.77:8000/ws/kinpod' autocorrect='off' autocapitalize='off'>"
"  <div class='hint'>Use your computer LAN IP or public domain.</div>"
"  <button id='btn' class='submit-btn' onclick='save()'>Connect KinPod</button>"
"  <div id='status' class='status success'>Saved! KinPod is now reconnecting...<br>Re-open Personal Hotspot on phone now.</div>"
"</div>"
"<script>"
"var currentTab = 'wifi';"
"function setTab(tab) {"
"  currentTab = tab;"
"  document.getElementById('tab_wifi').className = tab === 'wifi' ? 'tab-btn active' : 'tab-btn';"
"  document.getElementById('tab_hotspot').className = tab === 'hotspot' ? 'tab-btn active' : 'tab-btn';"
"  document.getElementById('view_wifi').style.display = tab === 'wifi' ? 'block' : 'none';"
"  document.getElementById('view_hotspot').style.display = tab === 'hotspot' ? 'block' : 'none';"
"}"
"function scanNetworks() {"
"  var sel = document.getElementById('ssid_select');"
"  sel.innerHTML = '<option value=\"\" disabled selected>🔍 Scanning nearby networks...</option>';"
"  fetch('/api/scan')"
"    .then(function(r) { return r.json(); })"
"    .then(function(nets) {"
"      sel.innerHTML = '';"
"      if (!nets || nets.length === 0) {"
"        sel.innerHTML = '<option value=\"\" disabled selected>No networks found (click Rescan)</option>';"
"        return;"
"      }"
"      var defOpt = document.createElement('option');"
"      defOpt.value = '';"
"      defOpt.disabled = true;"
"      defOpt.selected = true;"
"      defOpt.innerText = 'Choose network (' + nets.length + ' found)...';"
"      sel.appendChild(defOpt);"
"      nets.forEach(function(net) {"
"        var opt = document.createElement('option');"
"        opt.value = net.ssid;"
"        var lock = net.secure ? '🔒' : '🔓';"
"        var sig = net.rssi >= -65 ? '🟢' : (net.rssi >= -80 ? '🟡' : '🟠');"
"        opt.innerText = lock + ' ' + net.ssid + ' ' + sig;"
"        sel.appendChild(opt);"
"      });"
"      var manOpt = document.createElement('option');"
"      manOpt.value = '__manual__';"
"      manOpt.innerText = '✏️ Enter hidden network manually...';"
"      sel.appendChild(manOpt);"
"    })"
"    .catch(function(e) {"
"      sel.innerHTML = '<option value=\"\" disabled selected>Scan failed (click Rescan)</option>';"
"    });"
"}"
"function onSelectNetwork(val) {"
"  var ssidInput = document.getElementById('ssid');"
"  if (val === '__manual__') {"
"    ssidInput.value = '';"
"    ssidInput.focus();"
"  } else if (val) {"
"    ssidInput.value = val;"
"    document.getElementById('pass').focus();"
"  }"
"}"
"window.onload = function() {"
"  fetch('/api/config')"
"    .then(function(r) { return r.json(); })"
"    .then(function(d) {"
"      if (d.ssid) {"
"        document.getElementById('ssid').value = d.ssid;"
"        document.getElementById('hotspot_ssid').value = d.ssid;"
"      }"
"      if (d.ws) document.getElementById('ws').value = d.ws;"
"    })"
"    .catch(function(e) {});"
"  scanNetworks();"
"};"
"function save() {"
"  var s, p;"
"  if (currentTab === 'hotspot') {"
"    s = document.getElementById('hotspot_ssid').value.trim();"
"    p = document.getElementById('hotspot_pass').value;"
"  } else {"
"    s = document.getElementById('ssid').value.trim();"
"    p = document.getElementById('pass').value;"
"  }"
"  var w = document.getElementById('ws').value.trim();"
"  if(!s){ alert('Please enter or select a Wi-Fi SSID'); return; }"
"  var btn = document.getElementById('btn');"
"  btn.innerText = 'Connecting KinPod...';"
"  btn.disabled = true;"
"  fetch('/api/save', {"
"    method: 'POST',"
"    headers: {'Content-Type': 'application/json'},"
"    body: JSON.stringify({ssid: s, pass: p, ws: w})"
"  }).then(function(r){ return r.json(); }).then(function(d){"
"    document.getElementById('status').style.display = 'block';"
"  }).catch(function(e){"
"    document.getElementById('status').style.display = 'block';"
"  });"
"}"
"</script>"
"</body>"
"</html>";

static esp_err_t http_get_index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, s_setup_html, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t http_get_config_handler(httpd_req_t *req) {
    char ssid[64] = {0};
    char pass[64] = {0};
    char ws_url[256] = {0};
    kin_provisioning_load_config(ssid, sizeof(ssid), pass, sizeof(pass), ws_url, sizeof(ws_url));

    const char *resp_ssid = (strlen(ssid) > 0) ? ssid : "";
    const char *resp_ws   = (strlen(ws_url) > 0) ? ws_url : KIN_DEFAULT_WS_URL;

    char buf[512];
    snprintf(buf, sizeof(buf), "{\"ssid\":\"%s\",\"ws\":\"%s\"}", resp_ssid, resp_ws);

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t http_get_scan_handler(httpd_req_t *req) {
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 120,
        .scan_time.active.max = 250,
    };

    ESP_LOGI(TAG, "Executing Wi-Fi scan for AP dropdown...");
    esp_err_t err = esp_wifi_scan_start(&scan_config, true);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Wi-Fi scan failed or busy: %s", esp_err_to_name(err));
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_send(req, "[]", HTTPD_RESP_USE_STRLEN);
    }

    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    if (ap_count > 30) ap_count = 30;

    wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
    if (!ap_records) {
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_send(req, "[]", HTTPD_RESP_USE_STRLEN);
    }

    esp_wifi_scan_get_ap_records(&ap_count, ap_records);

    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < ap_count; i++) {
        const char *curr_ssid = (const char *)ap_records[i].ssid;
        if (strlen(curr_ssid) == 0) continue; // Skip hidden SSIDs

        // Deduplicate: check if this SSID is already in the list
        bool duplicate = false;
        int arr_size = cJSON_GetArraySize(root);
        for (int j = 0; j < arr_size; j++) {
            cJSON *item = cJSON_GetArrayItem(root, j);
            cJSON *s = cJSON_GetObjectItem(item, "ssid");
            if (s && s->valuestring && strcmp(s->valuestring, curr_ssid) == 0) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;

        cJSON *ap_item = cJSON_CreateObject();
        cJSON_AddStringToObject(ap_item, "ssid", curr_ssid);
        cJSON_AddNumberToObject(ap_item, "rssi", ap_records[i].rssi);
        cJSON_AddBoolToObject(ap_item, "secure", ap_records[i].authmode != WIFI_AUTH_OPEN);
        cJSON_AddItemToArray(root, ap_item);
    }

    free(ap_records);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    httpd_resp_set_type(req, "application/json");
    if (json_str) {
        httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);
        free(json_str);
    } else {
        httpd_resp_send(req, "[]", HTTPD_RESP_USE_STRLEN);
    }

    return ESP_OK;
}

static void stop_prov_worker_task(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(1500));
    kin_provisioning_stop();
    vTaskDelete(NULL);
}

static esp_err_t http_post_save_handler(httpd_req_t *req) {
    char buf[512];
    int ret, remaining = req->content_len;

    if (remaining >= sizeof(buf)) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *ssid_item = cJSON_GetObjectItem(root, "ssid");
    cJSON *pass_item = cJSON_GetObjectItem(root, "pass");
    cJSON *ws_item = cJSON_GetObjectItem(root, "ws");

    const char *new_ssid = (ssid_item && ssid_item->valuestring) ? ssid_item->valuestring : NULL;
    const char *new_pass = (pass_item && pass_item->valuestring) ? pass_item->valuestring : "";
    const char *new_ws = (ws_item && ws_item->valuestring) ? ws_item->valuestring : NULL;

    if (new_ssid) {
        ESP_LOGI(TAG, "Received new config: SSID='%s' WS='%s'", new_ssid, new_ws ? new_ws : "");
        kin_provisioning_save_config(new_ssid, new_pass, new_ws);

        // Update on-screen status to connecting
        render_provisioning_screen("SAVED! RECONNECTING...");
    }

    cJSON_Delete(root);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);

    // Schedule exit from provisioning mode on separate worker task
    xTaskCreate(stop_prov_worker_task, "stop_prov_w", 4096, NULL, 5, NULL);

    return ESP_OK;
}

static esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err) {
    // Captive portal fallback: Redirect all unrecognized requests to http://192.168.4.1/
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// -----------------------------------------------------------------------------
// DNS Captive Portal Server (UDP Port 53)
// -----------------------------------------------------------------------------
static void dns_server_task(void *pvParameters) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(DNS_PORT);

    s_dns_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (s_dns_sock < 0) {
        ESP_LOGE(TAG, "Unable to create DNS socket: errno %d", errno);
        s_dns_task_handle = NULL;
        vTaskDelete(NULL);
        return;
    }

    struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };
    setsockopt(s_dns_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    int err = bind(s_dns_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "DNS socket unable to bind: errno %d", errno);
        close(s_dns_sock);
        s_dns_sock = -1;
        s_dns_task_handle = NULL;
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "DNS Captive Portal Server listening on UDP port 53 [OK]");

    uint8_t rx_buffer[512];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (s_is_active) {
        int len = recvfrom(s_dns_sock, rx_buffer, sizeof(rx_buffer) - 64, 0,
                           (struct sockaddr *)&client_addr, &client_addr_len);
        if (len < 0) {
            continue;
        }

        if (len < 12) {
            continue;
        }

        // DNS Header flags: response, authoritative, recursion desired
        rx_buffer[2] = 0x81;
        rx_buffer[3] = 0x80;

        // Answers = 1
        rx_buffer[6] = 0x00;
        rx_buffer[7] = 0x01;
        // Authority = 0, Additional = 0
        rx_buffer[8] = 0x00;
        rx_buffer[9] = 0x00;
        rx_buffer[10] = 0x00;
        rx_buffer[11] = 0x00;

        // Skip Question Section
        int idx = 12;
        while (idx < len && rx_buffer[idx] != 0) {
            idx += rx_buffer[idx] + 1;
        }
        idx++; // Skip null terminator of QNAME
        idx += 4; // Skip QTYPE (2) and QCLASS (2)

        if (idx > len || idx + 16 > sizeof(rx_buffer)) {
            continue;
        }

        // Append A-record pointing to 192.168.4.1
        rx_buffer[idx++] = 0xC0;
        rx_buffer[idx++] = 0x0C; // Name pointer to offset 12 (QNAME)
        rx_buffer[idx++] = 0x00;
        rx_buffer[idx++] = 0x01; // Type A
        rx_buffer[idx++] = 0x00;
        rx_buffer[idx++] = 0x01; // Class IN
        rx_buffer[idx++] = 0x00;
        rx_buffer[idx++] = 0x00;
        rx_buffer[idx++] = 0x00;
        rx_buffer[idx++] = 0x3C; // TTL 60s
        rx_buffer[idx++] = 0x00;
        rx_buffer[idx++] = 0x04; // Data Length = 4 bytes
        rx_buffer[idx++] = 192;
        rx_buffer[idx++] = 168;
        rx_buffer[idx++] = 4;
        rx_buffer[idx++] = 1;

        sendto(s_dns_sock, rx_buffer, idx, 0, (struct sockaddr *)&client_addr, client_addr_len);
    }

    if (s_dns_sock >= 0) {
        close(s_dns_sock);
        s_dns_sock = -1;
    }
    s_dns_task_handle = NULL;
    ESP_LOGI(TAG, "DNS Captive Server task exited [OK]");
    vTaskDelete(NULL);
}

static esp_err_t start_web_server(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 14;

    esp_err_t ret = httpd_start(&s_http_server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        return ret;
    }

    httpd_uri_t get_index = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = http_get_index_handler,
    };
    httpd_register_uri_handler(s_http_server, &get_index);

    // Captive portal probes for iOS & macOS
    httpd_uri_t get_apple1 = {
        .uri = "/hotspot-detect.html",
        .method = HTTP_GET,
        .handler = http_get_index_handler,
    };
    httpd_register_uri_handler(s_http_server, &get_apple1);

    httpd_uri_t get_apple2 = {
        .uri = "/canonical.html",
        .method = HTTP_GET,
        .handler = http_get_index_handler,
    };
    httpd_register_uri_handler(s_http_server, &get_apple2);

    // Captive portal probes for Android
    httpd_uri_t get_gen204 = {
        .uri = "/generate_204",
        .method = HTTP_GET,
        .handler = http_get_index_handler,
    };
    httpd_register_uri_handler(s_http_server, &get_gen204);

    httpd_uri_t get_gen204_alt = {
        .uri = "/gen_204",
        .method = HTTP_GET,
        .handler = http_get_index_handler,
    };
    httpd_register_uri_handler(s_http_server, &get_gen204_alt);

    // Captive portal probes for Windows
    httpd_uri_t get_ncsi = {
        .uri = "/ncsi.txt",
        .method = HTTP_GET,
        .handler = http_get_index_handler,
    };
    httpd_register_uri_handler(s_http_server, &get_ncsi);

    httpd_uri_t get_connecttest = {
        .uri = "/connecttest.txt",
        .method = HTTP_GET,
        .handler = http_get_index_handler,
    };
    httpd_register_uri_handler(s_http_server, &get_connecttest);

    // REST APIs
    httpd_uri_t get_config = {
        .uri = "/api/config",
        .method = HTTP_GET,
        .handler = http_get_config_handler,
    };
    httpd_register_uri_handler(s_http_server, &get_config);

    httpd_uri_t get_scan = {
        .uri = "/api/scan",
        .method = HTTP_GET,
        .handler = http_get_scan_handler,
    };
    httpd_register_uri_handler(s_http_server, &get_scan);

    httpd_uri_t post_save = {
        .uri = "/api/save",
        .method = HTTP_POST,
        .handler = http_post_save_handler,
    };
    httpd_register_uri_handler(s_http_server, &post_save);

    // Catch-all 404 handler redirects to captive portal home
    httpd_register_err_handler(s_http_server, HTTPD_404_NOT_FOUND, http_404_error_handler);

    ESP_LOGI(TAG, "Provisioning Web Server listening on port 80 [OK]");
    return ESP_OK;
}

// -----------------------------------------------------------------------------
// Public Subsystem API
// -----------------------------------------------------------------------------
esp_err_t kin_provisioning_start(void) {
    if (s_is_active) return ESP_OK;
    s_is_active = true;
    ESP_LOGI(TAG, "Entering Wi-Fi Provisioning Mode (SoftAP '%s')...", KIN_PROV_SOFTAP_SSID);

    // 1. Render setup screen on AMOLED immediately
    render_provisioning_screen(NULL);

    // 2. Disconnect station to prevent channel hopping during setup
    esp_wifi_disconnect();

    // 3. Configure Wi-Fi to APSTA mode
    wifi_config_t ap_config = {
        .ap = {
            .ssid = KIN_PROV_SOFTAP_SSID,
            .ssid_len = strlen(KIN_PROV_SOFTAP_SSID),
            .channel = 1,
            .password = "",
            .max_connection = 4,
            .authmode = WIFI_AUTH_OPEN,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_APSTA);
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();

    // 4. Start Captive DNS Server
    if (s_dns_task_handle == NULL) {
        xTaskCreatePinnedToCore(dns_server_task, "dns_prov", 4096, NULL, 5, &s_dns_task_handle, 0);
    }

    // 5. Start Web Server
    start_web_server();

    return ESP_OK;
}

esp_err_t kin_provisioning_stop(void) {
    if (!s_is_active) return ESP_OK;
    s_is_active = false;
    ESP_LOGI(TAG, "Stopping Provisioning Mode...");

    // Stop HTTP server
    if (s_http_server) {
        httpd_stop(s_http_server);
        s_http_server = NULL;
    }

    // (DNS task exits automatically when s_is_active is false within 1 sec)

    // Switch back to STA mode
    esp_wifi_set_mode(WIFI_MODE_STA);

    // Load any newly configured credentials from NVS
    char ssid[64] = {0};
    char pass[64] = {0};
    char ws_url[128] = {0};
    if (kin_provisioning_load_config(ssid, sizeof(ssid), pass, sizeof(pass), ws_url, sizeof(ws_url)) == ESP_OK) {
        ESP_LOGI(TAG, "Reconnecting Wi-Fi with NVS config: SSID='%s'", ssid);
        kin_wifi_init(ssid, pass);
    } else {
        esp_wifi_connect();
    }

    // Restore active character avatar
    return kin_avatar_set_active(kin_avatar_get_active());
}

bool kin_provisioning_is_active(void) {
    return s_is_active;
}

bool kin_provisioning_handle_touch(uint16_t x, uint16_t y) {
    if (!s_is_active) return false;

    // Check tap on Cancel Button (x=20 to 390, y=424 to 490)
    if (x >= 20 && x <= 390 && y >= 424 && y <= 490) {
        ESP_LOGI(TAG, ">>> TAP: CANCEL PROVISIONING <<<");
        kin_audio_haptic_buzz(25);
        kin_provisioning_stop();
        return true;
    }

    return true; // Consume touches while provisioning screen is up
}
