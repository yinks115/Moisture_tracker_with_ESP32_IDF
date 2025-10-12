#ifndef LEAF_CONFIG_HPP
#define LEAF_CONFIG_HPP

typedef struct {
    unsigned char wifi_ssid[32];
    unsigned char wifi_password[64];
} wifi_credentials_t;
extern wifi_credentials_t wifi_credentials;

typedef struct {
    unsigned char plant_name[16];
} leaf_info_t;
extern leaf_info_t leaf_info;

void initialize_leaf_config(void); //uses the 2 private functions below

void _load_wifi_credentials(void); 
void _load_leaf_info(void);

#endif