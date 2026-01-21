#ifndef ESSENTIA_H
#define ESSENTIA_H

#include "esphome.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

const int NUM_ZONES = 6;

class Essentia;

struct Zone
{
    int zone = 0;
    string power = "ON";
    string source = "1";
    string group = "1";
    string volume = "0";
    string bass = "0";
    string treble = "0";
    string vrst = "0";
    bool mute = false;
};

class Essentia : public Component, public UARTDevice {

    Zone zones[NUM_ZONES];
    vector<string> cmd_queue;

    public:

        Essentia(UARTComponent *parent) : UARTDevice(parent) {}

        void setup() override
        {
            // Clear any stale commands from previous session
            cmd_queue.clear();
            // Request initial state from all zones
            refresh_state();
        }

        void loop() override
        {
            const int max_line_length = 80;
            static char buffer[max_line_length];
            while (available())
            {
                if (readline(read(), buffer, max_line_length) > 0)
                {
                    ESP_LOGD("essentia", "Received: %s", buffer);
                    parse_response(buffer);
                }
            }
        }

        int readline(int readch, char *buffer, int len)
        {
            static int pos = 0;
            int rpos;

            if (readch > 0)
            {
                switch (readch)
                {
                case '\n': // Ignore new-lines
                    break;
                case '\r': // Return on CR
                    rpos = pos;
                    pos = 0; // Reset position index ready for next time
                    return rpos;
                default:
                    if (pos < len - 1)
                    {
                        buffer[pos++] = readch;
                        buffer[pos] = 0;
                    }
                }
            }
            // No end of line has been found, so return -1.
            return -1;
        }

        void queue_cmd(string command){
            cmd_queue.push_back(command);
        }

        void process_queue(){
            if(!cmd_queue.empty()){
                std::string cmd = cmd_queue.front();
                std::vector<uint8_t> vec(cmd.begin(), cmd.end());
                cmd_queue.erase(cmd_queue.begin());
                write_array(vec);
            }
        }

        int get_is_queue_empty(){
            return cmd_queue.empty();
        }

        Zone get_zone(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) {
                ESP_LOGW("essentia", "Invalid zone_id: %d", zone_id);
                return Zone();
            }
            return zones[zone_id - 1];
        }

        bool get_zone_power(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return false;
            return zones[zone_id - 1].power == "ON";
        }

        void set_zone_on(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            queue_cmd("*Z0" + to_string(zone_id) + "ON\r");
        }

        void set_zone_off(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            queue_cmd("*Z0" + to_string(zone_id) + "OFF\r");
        }

        float get_zone_source(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return 1.0f;
            if (zones[zone_id - 1].source.empty()) return 1.0f;
            return stof(zones[zone_id - 1].source);
        }

        void set_zone_source(int zone_id, float v){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            int source = (int)v;
            if (source < 1 || source > 6) return;
            queue_cmd("*Z0" + to_string(zone_id) + "SRC" + to_string(source) + "\r");
        }

        float get_zone_group(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return 1.0f;
            if (zones[zone_id - 1].group.empty()) return 1.0f;
            return stof(zones[zone_id - 1].group);
        }

        void set_zone_group(int zone_id, float v){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            int group = (int)v;
            if (group < 1 || group > 6) return;
            queue_cmd("*Z0" + to_string(zone_id) + "GRP" + to_string(group) + "\r");
        }

        float get_zone_volume(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return -62.0f;
            if (zones[zone_id - 1].volume.empty()) return -62.0f;
            return stof(zones[zone_id - 1].volume);
        }

        void set_zone_volume(int zone_id, float v){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            if (v < -78 || v > 0) return;
            queue_cmd("*Z0" + to_string(zone_id) + "VOL" + format_volume(v) + "\r");
        }

        float get_zone_bass(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return 0.0f;
            if (zones[zone_id - 1].bass.empty()) return 0.0f;
            return stof(zones[zone_id - 1].bass);
        }

        void set_zone_bass(int zone_id, float v){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            if (v < -8 || v > 8) return;
            queue_cmd("*Z0" + to_string(zone_id) + "BASS" + format_level(v) + "\r");
        }

        float get_zone_treble(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return 0.0f;
            if (zones[zone_id - 1].treble.empty()) return 0.0f;
            return stof(zones[zone_id - 1].treble);
        }

        void set_zone_treble(int zone_id, float v){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            if (v < -8 || v > 8) return;
            queue_cmd("*Z0" + to_string(zone_id) + "TREB" + format_level(v) + "\r");
        }

        bool get_zone_mute(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return false;
            return zones[zone_id - 1].mute;
        }

        void set_zone_mute(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            queue_cmd("*Z0" + to_string(zone_id) + "MTON\r");
        }

        void set_zone_unmute(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            queue_cmd("*Z0" + to_string(zone_id) + "MTOFF\r");
        }

        float get_zone_vrst(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return 0.0f;
            if (zones[zone_id - 1].vrst.empty()) return 0.0f;
            return stof(zones[zone_id - 1].vrst);
        }

        void send_connect_sr(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            queue_cmd("*Z0" + to_string(zone_id) + "CONSR\r");
        }

        void send_zone_sr(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            queue_cmd("*Z0" + to_string(zone_id) + "SETSR\r");
        }

        bool get_all_on(){
            bool all_on = true;
            for (const Zone& z : zones){
                if(z.power == "OFF"){
                    all_on = false;
                    break;
                }
            }
            return all_on;
        }

        void set_all_on(){
             for (int i=1;i<=NUM_ZONES;i++){
                set_zone_on(i);
             }
        }

        void set_all_off(){
            for (int i=1;i<=NUM_ZONES;i++){
                set_zone_off(i);
            }
        }

        bool get_all_mute(){
            bool all_mute = true;
            for (const Zone& z : zones){
                if(z.power == "ON" && !(z.mute)){
                    all_mute = false;
                    break;
                }
            }
            return all_mute;
        }

        void set_all_mute(){
            for (int i=1;i<=NUM_ZONES;i++){
                set_zone_mute(i);
            }
        }

        void set_all_unmute(){
            for (int i=1;i<=NUM_ZONES;i++){
                set_zone_unmute(i);
            }
        }

        void set_all_volume(int volume){
            for (int i=1;i<=NUM_ZONES;i++){
                set_zone_volume(i, volume);
            }
        }

        void refresh_zone_state(int zone_id){
            if (zone_id < 1 || zone_id > NUM_ZONES) return;
            send_connect_sr(zone_id);
            send_zone_sr(zone_id);
        }

        void refresh_state(){
            for (int i=1;i<=NUM_ZONES;i++){
                refresh_zone_state(i);
            }
        }

        void parse_response(string response){
            char message_type = response[1];

            if(message_type == 'Z'){
                char sr_type = response[4];

                switch (sr_type) {
                    case 'P':
                        parse_connect_sr(response);
                        break;
                    case 'O':
                        parse_zone_sr(response);
                        break;
                }
            }
        }

    private:

        void parse_connect_sr(string response)
        {
            vector<string> parts = split(response, ",");

            int zone_id = stoi(parts[0].substr(3, 1)) - 1;

            Zone *zone = &zones[zone_id];

            zone->power = parts[0].substr(7, 3);
            zone->source = parts[1].substr(3,1);
            zone->group = parts[2].substr(3,1);

            ESP_LOGD("essentia", "Parsed CONSR for zone %d: power=%s, source=%s, group=%s",
                     zone_id + 1, zone->power.c_str(), zone->source.c_str(), zone->group.c_str());

            string v = parts[3].substr(3, 3);

            if((v == "-MT" || v == "-XT")){
                zone->mute = true;
            }else{
                zone->volume = v;
                zone->mute = false;
            }
        }

        void parse_zone_sr(string response)
        {
            vector<string> parts = split(response, ",");

            int zone_id = stoi(parts[0].substr(3, 1)) - 1;

            Zone *zone = &zones[zone_id];

            zone->bass = parts[1].substr(4, 3);
            zone->treble = parts[2].substr(4, 3);
            zone->group = parts[3].substr(3, 1);
            zone->vrst = parts[4].substr(4, 1);
        }

        vector<string> split(string str, string delimiter)
        {
            stringstream ss(str);
            vector<string> result;

            while (ss.good())
            {
                string substr;
                getline(ss, substr, ',');
                result.push_back(substr);
            }
            return result;
        }

        string format_volume(float x){
            // Volume range: -78 to 0, formatted as 2-digit string
            int vol = (int)(-x);  // Convert to positive integer
            if (vol < 0) vol = 0;
            if (vol > 78) vol = 78;

            stringstream ss;
            ss << setfill('0') << setw(2) << vol;
            return ss.str();
        }

        string format_level(float x){
            // Bass/Treble range: -8 to +8, formatted as +/-0N
            int level = (int)x;
            if (level < -8) level = -8;
            if (level > 8) level = 8;

            stringstream ss;
            if (level < 0) {
                ss << "-0" << (-level);
            } else {
                ss << "+0" << level;
            }
            return ss.str();
        }

};

#endif
