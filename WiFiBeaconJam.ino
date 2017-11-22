/*
 * OLIMEX MOD-WIFI-ESP8266(-DEV) board
 * this will send random beacon frames and
 * randomized probe requests.
 * It is probably too random, as chances are very small to randomly create the same frame twice 
 * to add proper noise to tracking systems....
 */
#include "ouis.h"
#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

String characters = "1234567890qwertyuiopasdfghjkklzxcvbnm QWERTYUIOPASDFGHJKLZXCVBNM_";
byte channel;

// Beacon Packet buffer
                            /*Frame Control, Duration*/
uint8_t beaconpacket[128] = { 0x80, 0x00, 0x00, 0x00,         /*0x80 denotes beacon frame*/
                /*4*/   0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   /*DA stays a broadcase*/
                /*10*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,   /*SA this will be changed to something generated*/
                /*16*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,   /*BSS ID, changes to be the same as SA*/
                /*22*/  0xc0, 0x6c,                           /*Seq Control*/
                /*24*/  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, /*Timestamp generate better, can be used to fingerprint*/ 
                /*32*/  0x64, 0x00,                           /*Beacon Interval can be used to fingerprint, check default values*/
                /*34*/  0x01, 0x04,                           /*Capability*/
                /* SSID */
                /*36*/  0x00, 0x06, 0x72, 0x72, 0x72, 0x72, 0x72, 0x72,  /*SSid: 0, length(6), Here 6 times r (hex:72)*/
                        0x01, 0x08, 0x82, 0x84,               /* IBSS parameter set ?*/
                        0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, 0x03, 0x01, /*channel switch + erp ?*/
                /*56*/  0x04};                                /*channel*/

uint8_t afterSSid[13] =  {0x01, 0x08, 0x82, 0x84,
                          0x8b, 0x96, 0x24, 0x30, 
                          0x48, 0x6c, 0x03, 0x01,
                          0x04};
int length_afterSSid = 13;

// Probe request buffer
                              /*Frame Control, Duration*/
uint8_t probereqpacket[128] = { 0x40, 0x00, 0x00, 0x00,       /*0x40 denotes probe request*/
                /*4*/   0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   /*DA is a broadcast frame*/ 
                /*10*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,   /*SA will be generated to be something*/
                /*16*/  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   /*BSS ID should be a broadcast*/
                /*22*/  0x00, 0x00, 0x00, 0x00, 0x01, 0x08,   /*copied from a picture from https://www.aircrack-ng.org/doku.php?id=wpa_capture
                /*28*/  0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12,
                /*34*/  0x18, 0x24, 0x32, 0x04, 0x30, 0x48, 
                /*40*/  0x60, 0x6c};


void setup() {
  Serial.begin(115200);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  
  //Serial.println("Hello");
  
  delay(500);
  wifi_set_opmode(STATION_MODE);
  Serial.println(F("switched mode to station"));
  wifi_promiscuous_enable(1); 
  Serial.println(F("now its i am operating in promiscious mode"));
}



void loop() {
    /* Randomize channel */
    channel = random(1,12); 
    wifi_set_channel(channel);

    int oui = random(1974);
    /* Randomize SRC and BSSID MAC among existing ouis -> check which ouis are actually used by smartphones, 
    * to follow expected vendor statisctics, 
    * right now, this is probably detectable, as frames from un-typical devices are sent
     */
    beaconpacket[10] = beaconpacket[16] = ouis[oui][0];
    beaconpacket[11] = beaconpacket[17] = ouis[oui][1];
    beaconpacket[12] = beaconpacket[18] = ouis[oui][2];
    beaconpacket[13] = beaconpacket[19] = random(256);
    beaconpacket[14] = beaconpacket[20] = random(256);
    beaconpacket[15] = beaconpacket[21] = random(256);

    /* randomize length of ssid, (min 5 character, max. 20) 
    * this should follow an exponential distribution, as ssids tend to be short
    */
    int l = random(5,20);
    beaconpacket[37]=char(l);  /*is this the right way to set one byte?*/
    for(int i=0;i<l;i++){
      beaconpacket[38+i] = characters[random(65)];
    }

    /*fill the packet with the leftovers...*/
    for(int i=0;i<length_afterSSid;i++){
        beaconpacket[38+l+i] = afterSSid[i];
    }
    beaconpacket[38+l+length_afterSSid-1] = channel;

    int packetlength= 38+l+length_afterSSid;

    /*send fake beacon frame*/
    wifi_send_pkt_freedom(beaconpacket, packetlength, 0);
    wifi_send_pkt_freedom(beaconpacket, packetlength, 0);
    wifi_send_pkt_freedom(beaconpacket, packetlength, 0);
    delay(1);

    /* send the probe request from a different channel*/
    channel = random(1,12); 
    wifi_set_channel(channel);

    oui = random(1974);
    probereqpacket[10] = ouis[oui][0];
    probereqpacket[11] = ouis[oui][1];
    probereqpacket[12] = ouis[oui][2];
    probereqpacket[13] = random(256);
    probereqpacket[14] = random(256);
    probereqpacket[15] = random(256);

    /*send fake probe request*/
    wifi_send_pkt_freedom(probereqpacket, 42, 0);
    wifi_send_pkt_freedom(probereqpacket, 42, 0);
    wifi_send_pkt_freedom(probereqpacket, 42, 0);
    delay(1);
}

