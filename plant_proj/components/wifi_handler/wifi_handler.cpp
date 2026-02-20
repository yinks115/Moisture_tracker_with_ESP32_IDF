#include <freertos/FreeRTOS.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_bit_defs.h>
#include <esp_netif.h> //for networking
#include <esp_wifi.h>
// #include <esp_http_client.h>
// #include <string>

#include "wifi_handler.hpp"
#include "leaf_config.hpp"

static const char* _WIFI_EVENTS_LOGGER = "Wifi Handler *** ";
const int HTTP_BUFF_LEN = 100;
const char* SERVER_URL = "http://";
const char* SUBMIT_READING_ROUTE = "/submit-reading";
const int POST_TIMEOUT = 10000; //ms
const int WIFI_CONNECTION_TIMEOUT = 5000;

static std::string target_post_url;
esp_http_client_config_t http_client_config;
events_data_t wifi_events;

esp_err_t wan_event_handler_setup() {
    ESP_LOGI(_WIFI_EVENTS_LOGGER, "Initializing WAN events handler SetUp");
    ESP_LOGI(_WIFI_EVENTS_LOGGER, "Creating wifi event loop");

    bool critical_error = false;
    esp_err_t result = esp_event_loop_create_default(); //creates an event loop. components can publish and subscribe to events in this loop
    if (result != ESP_OK) {
        switch(result){
            case ESP_ERR_NO_MEM:
                ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! No memory available to create event loop");
                critical_error = true;
                break;
            case ESP_ERR_INVALID_STATE:
                ESP_LOGW(_WIFI_EVENTS_LOGGER, "Warning! Event loop was already created, Continuing ...");
                break;
            case ESP_FAIL:
                ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! Failed to created event loop");
                critical_error = true;
                break;
            default:
                ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! Failed to created event loop for unknown reason");
                critical_error = true;
        }
    } else {
        ESP_LOGI(_WIFI_EVENTS_LOGGER, "Wifi Event loop created successfully...Creating Event Group");
    }

    if (critical_error) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Encountered a critical error when setting up event loop");
        return ESP_FAIL;
    }

    // just for my own understanding so I don't forget. Event groups are used for synchronization among tasks
    //where as event loops are used to invoke an handler (block of code) when an event occurs

    //creating an events group
    wifi_events.wan_event_group = xEventGroupCreate(); //returns a pointer to a set of bits. Each bit acts as flag for some type of info
    wifi_events.GOT_IP_BIT = BIT0; //the lsb of events flags(bits) will correspond to whether or not we got an ip address

    if (wifi_events.wan_event_group == NULL) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! Failed to create Event Group");
        return ESP_FAIL;
    } else {
        ESP_LOGI(_WIFI_EVENTS_LOGGER, "Created Events Group");
    }

    //making subscribers to event (ie event handlers)
    if (esp_event_handler_register(
        WIFI_EVENT,   //the type of event 
        ESP_EVENT_ANY_ID, //the id of the specifc event that falls under the event type. we specified any wifi event
        &wifi_event_handler,
        NULL
    ) == ESP_OK) {
        ESP_LOGI(_WIFI_EVENTS_LOGGER, "Created Wifi event handler");
    } else {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Failed to Create Wifi event handler");
        return ESP_FAIL;
    }

    //ip address is not guranteed to be set once connected to a wifi network so we need to make an event handler
    //specifically for it
    if (esp_event_handler_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &ip_event_handler,
        NULL
    ) == ESP_OK) {
        ESP_LOGI(_WIFI_EVENTS_LOGGER, "Created IP event handler");
    } else {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! Failed to Create IP event handler");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t start_wifi_connection() {
    target_post_url = (std::string(SERVER_URL) + SUBMIT_READING_ROUTE);

    //initializing http client and 
    //registering http handler
    http_client_config = {
        .url = target_post_url.c_str(),
        .method = HTTP_METHOD_POST,
        .timeout_ms = POST_TIMEOUT,
        .event_handler = http_event_handler
    };

    //event handlers setup
    if (wan_event_handler_setup() != ESP_OK) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Critical Error! Failed to set up wifi event group");
        return ESP_FAIL;
    }

    //initializing tcp/ip adapter
    if (esp_netif_init() != ESP_OK){
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Critical Error! Failed to initialize TCP/IP adapter");
        return ESP_FAIL;
    }

    //initializes wifi config
    wifi_init_config_t wifi_config_def = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&wifi_config_def) != ESP_OK) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "CRITICAL ERROR! Unable to initialize wifi");
        return ESP_FAIL;
    }

    //setting the mode of esp_idf's internal wifi handler
    // station mode means esp32 connects to a wifi source and can communicate with devices on the network
    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) { 
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Critical Error! Failed to set wifi mode");
        return ESP_FAIL;
    }

    //verifies that our wifi credentials make sense
    //it's expected that the wifi_credentials variable in leaf_config.hpp has been set
    if (strlen(reinterpret_cast<char*>(wifi_credentials.wifi_ssid)) == 0 || wifi_credentials.wifi_ssid[0] == '\0') {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Critical Error! Invalid Wifi SSID");
        return ESP_FAIL;
    }

    if (strlen(reinterpret_cast<char*>(wifi_credentials.wifi_password)) == 0 || wifi_credentials.wifi_password[0] == '\0') {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Critical Error! Invalid Wifi password");
        return ESP_FAIL;
    }

    //wifi configuration
    //loads wifi name and password into the config object
    wifi_config_t wifi_config_custom;
    memset(&wifi_config_custom, 0, sizeof(wifi_config_custom)); //clear the object 
    memcpy(wifi_config_custom.sta.ssid, wifi_credentials.wifi_ssid, sizeof(wifi_credentials.wifi_ssid));
    memcpy(wifi_config_custom.sta.password, wifi_credentials.wifi_password, sizeof(wifi_credentials.wifi_password));

    if (esp_wifi_set_config(WIFI_IF_STA, &wifi_config_custom) != ESP_OK) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Critical Error! Failed to set wifi configuration");
        return ESP_FAIL;
    }

    //finally starts wifi connection
    if (esp_wifi_start() != ESP_OK) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Critical Error! Failed to start wifi");
        return ESP_FAIL;
    }

    //DHCP client setup
    esp_netif_t* sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_netif == NULL) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Critical Error! Failed to get wifi station interface");
        return ESP_FAIL;
    }
    if (esp_netif_dhcpc_start(sta_netif) != ESP_OK) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Critical Error! Failed to enable DHCP Client");
        return ESP_FAIL;
    }

    //waits until the router gives us an ip address
    //once ip is received the ip handler will be called and it will set the ip bit in the event group
    EventBits_t bits = xEventGroupWaitBits(
        wifi_events.wan_event_group,
        wifi_events.GOT_IP_BIT,
        pdFALSE, //this parameter is set to true if we want to set the bit(s) back to 0 after it was set to 1
        pdFALSE, //this is set to true if we're waiting on multiple bits and we want to wait until they're all set to 1
        pdMS_TO_TICKS(WIFI_CONNECTION_TIMEOUT) // how long we wait for before returning
    );

    //checks if we returned due to a timeout or the bit was set
    if ((bits & wifi_events.GOT_IP_BIT) == 0) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Critical Error! Timedout while waiting to obtain an IP address");
        return ESP_FAIL;
    }

    ESP_LOGI(_WIFI_EVENTS_LOGGER, "Successfully connected to wifi");
    return ESP_OK;
}

esp_err_t stop_wifi_connection() {
    //first we stop the wifi module, then release all the resources allocated by esp_wifi_init

    esp_err_t ret_val = ESP_OK;

    ret_val = esp_wifi_stop();
    if (ret_val != ESP_OK) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error when stopping wifi. wifi wasn't initialized by esp_wifi_init");
        return ret_val;
    }

    ret_val = esp_wifi_deinit();
    if (ret_val != ESP_OK) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error when de-initializing wifi");
        return ret_val;
    }

    return ret_val;
}

esp_err_t post_moisture_reading(const double reading) {

    // convert reading to json
    const std::string payload = _to_json(reading);
    bool err = false;

    /*
    Procedure:
    1. make a http client config (already did in start_wifi func)
    2. initialize an http client object using the http client config. returns a ptr
    3. set the http header, method, and the post field
    4. perform the Post request
    5. free the http client
    */

    esp_http_client_handle_t client = esp_http_client_init(&http_client_config);
    if (client == NULL) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "ERROR! Unable to create HTTP Client");
        err = true;
    }

    // set http header
    if (esp_http_client_set_header(client, "content-type", "application/json") == ESP_OK) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "ERROR! Unable to set HTTP header");
        err = true;
    }

    // set method to POST
    if (esp_http_client_set_method(client, HTTP_METHOD_POST) != ESP_OK) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "ERROR! Unable to set HTTP_METHOD to POST");
        err = true;
    }

    //set post field
    if (esp_http_client_set_post_field(client, payload.c_str(), payload.length()) != ESP_OK) {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "ERROR! Unable set HTTP Post field");
        err = true;
    }

    //performs the request and frees resources
    esp_err_t res = esp_http_client_perform(client);
    if (res == ESP_OK) {
        ESP_LOGI(_WIFI_EVENTS_LOGGER, "Successfully POST moisture reading!");
    } else {
        ESP_LOGE(_WIFI_EVENTS_LOGGER, "ERROR! Unable to POST moisture reading. Error Code=%s", esp_err_to_name(res));

    }

    return err ? ESP_FAIL: ESP_OK;
}

//**************implementation of event handlers

//board will be listening to events through wifi so we use Wifi_event_sta (sta stands for station)
void wifi_event_handler(
    void* arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data //will be NULL
) {
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(_WIFI_EVENTS_LOGGER, "Connecting to WIFI....");
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGW(_WIFI_EVENTS_LOGGER, "ESP32 has been disconnected from Wifi");
            break;
        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(_WIFI_EVENTS_LOGGER, "Connected to WIFI");
            break;
        default:
            ESP_LOGE(_WIFI_EVENTS_LOGGER, "Unknown Event Id: %d", static_cast<int>(event_id));
            break;

    }

}

//after connecting to wifi we'll be assigned an ip address
void ip_event_handler(
    void* arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data //will be NULL
) {
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            //get's ip address that we've been assigned
            esp_netif_ip_info_t ip_info;
            esp_netif_get_ip_info(
                //the handler function returns an object/handler that stores all info related to networking
                esp_netif_get_handle_from_ifkey("WiFi_STA_DEF"), //gets the handlre for the wifi station
                &ip_info //saves info from handler
            );
            char ip_addr[IP4ADDR_STRLEN_MAX];
            esp_ip4addr_ntoa(&ip_info.ip, ip_addr, IP4ADDR_STRLEN_MAX); //copies Ip adrress into the ip_addr var
            ESP_LOGI(_WIFI_EVENTS_LOGGER, "Received the ip adrress-%s", ip_addr);

            //sets the bit corresponding to ip data in the event group
            //allows other components that are subscribed to the event group to know that we've received an ip addr
            xEventGroupSetBits(wifi_events.wan_event_group, wifi_events.GOT_IP_BIT);
            break;
        default:
            ESP_LOGI(
                _WIFI_EVENTS_LOGGER,
                "Failed to get an IP address. Handler might have been activated by the wrong event type. Event id: %d", static_cast<int>(event_id)
            );
            break;
    }
}

esp_err_t http_event_handler(esp_http_client_event_t* event) {
    //buffer that stores the received data. declared static because we want the buffer to 
    //last during the duration of the whole program
    static char* output_buffer = NULL;
    static int buff_data_len = 0; //len of data in the buffer, this is different from the buffer size

    switch (event->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(_WIFI_EVENTS_LOGGER, "http_events_handler: HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(_WIFI_EVENTS_LOGGER, "http_events_handler: HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(_WIFI_EVENTS_LOGGER, "http_events_handler: HTTP_EVENT_HEADERS_SENT"); //sending http header for event
            break;
        case HTTP_EVENT_ON_HEADER: //received http header for an event
            ESP_LOGI(
                _WIFI_EVENTS_LOGGER,
                "http_events_handler: HTTP_EVENT_ON_HEADER, recevied http header. key=%s, value=%s", event->header_key, event->header_value);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(_WIFI_EVENTS_LOGGER, "http_events_handler: HTTP_EVENT_ON_FINISH");
            //clearing the buffer
            buff_data_len = 0;
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(_WIFI_EVENTS_LOGGER, "http_events_handler: HTTP_EVENT_DISCONNECTED");
            buff_data_len = 0;
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            break;
        case HTTP_EVENT_ON_DATA:
        {
            ESP_LOGI(_WIFI_EVENTS_LOGGER, "http_events_handler: HTTP_EVENT_ON_DATA, data_len: %d", event->data_len);

            if (output_buffer == NULL) {
                output_buffer = (char*) malloc(sizeof(char) * HTTP_BUFF_LEN);
                if (output_buffer == NULL){
                    //means we weren't able to allocate memory for the buffer
                    ESP_LOGE(_WIFI_EVENTS_LOGGER, "Error! Unable to allocate memory for http buffer");
                    return ESP_FAIL;
                }
                memset(output_buffer, 0, HTTP_BUFF_LEN);
            }

            //copies data into the buffer

            int rem_space = HTTP_BUFF_LEN - buff_data_len; //space left in our buffer
            int copy_len = _min(event->data_len, rem_space); //returns the smaller value
            if (copy_len > 0) {
                //output_buff+buff_data_len (ptr arithmetic) gets the index of the last valid character in the buff. then we copy the data
                //from the http event to the remainin space
                memcpy(output_buffer+buff_data_len, event->data, copy_len);
                buff_data_len += copy_len;
            }

            //allows us to print what's curr in the buffer
            char* curr_buff = (char*)malloc(buff_data_len+1);
            if (curr_buff != NULL){
                memcpy(curr_buff, output_buffer, buff_data_len);
                curr_buff[buff_data_len] = '\0'; //null char termination
                ESP_LOGI(_WIFI_EVENTS_LOGGER, "http_events_handler: HTTP_EVENT_ON_DATA, buff_len: %d , buff_data: %s", buff_data_len, curr_buff);

                free(curr_buff);
                curr_buff = NULL;
            }
            break;
        }
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(_WIFI_EVENTS_LOGGER, "http_events_handler: HTTP_EVENT_REDIRECT");
            break;
        default:
            ESP_LOGE(_WIFI_EVENTS_LOGGER, "https_events_handler: Unknown event id: %d", event->event_id);
            return ESP_ERR_INVALID_RESPONSE;
    }

    return ESP_OK;
}

const std::string _to_json(const double value) {
    const std::string plant_name = std::string(reinterpret_cast<const char*>(leaf_info.plant_name));

    /*
    {
        "plant name": "<plant's name>",
        "moisture val" "<moisture val"
    }
    */
    const std::string json_str = "{\"plant name\":\"" + plant_name + "\", \"moisture\" :\"" + std::to_string(value) + "}";

    return json_str;
}

int _min(int n1, int n2) {
    return (n1 < n2) ? n1: n2;
}
