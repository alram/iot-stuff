#include <WiFi.h>
#include "wifi_creds.h"

#define MONEPLUS 16
#define MONEMINUS 17

WiFiServer server(80);

const char *ok_no_content = "HTTP/1.1 204 No Content";
const char *ok_content    = "HTTP/1.1 200 OK";
const char *err_not_found = "HTTP/1.1 404 Not Found";
const char *bad_request   = "HTTP/1.1 400 Bad Request";

// 0: off - down
// 1: on  - up
bool deskStatus = 0;

void setup() {
    Serial.begin(115200);
    pinMode(MONEPLUS, OUTPUT);
    pinMode(MONEMINUS, OUTPUT);
    digitalWrite(MONEPLUS, HIGH);

    // When starting, we want to set the desk
    // all the way down to initialize it as "off"
    digitalWrite(MONEMINUS, LOW);
    delay(6000);
    digitalWrite(MONEMINUS, HIGH);

    Serial.print("Connecting to: ");
    Serial.println(ssid);

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP Address :");
    Serial.println(WiFi.localIP());
    server.begin();
}

void move(int direction, int timeS) {
    digitalWrite(direction, LOW);
    delay((timeS * 1000) );
    digitalWrite(direction, HIGH);
    if (direction == MONEPLUS) {
        deskStatus = 1;
    } else {
        deskStatus = 0;
    }
}

int getTimeSeconds(String line) {
    int found = 0;
    char separator = '/';
    int index = 2;
    int time = 0;

    int strIndex[] = { 0, -1 };
    int maxIndex = line.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (line.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }

    if (found > index) {
        time = line.substring(strIndex[0], strIndex[1]).toInt();
    }

    return time;
}

String handle_requests(String line) {
    String returnCode = err_not_found;
    int direction = -1;
    bool should_move = false;

    if (line.startsWith("PUT /up/")) {
        should_move = !deskStatus;
        direction = MONEPLUS;
    } else if (line.startsWith("PUT /down/")) {
        should_move = deskStatus;
        direction = MONEMINUS;
    } else {
        return err_not_found;
    }
    int time = getTimeSeconds(line);
    if (time < 1 || time > 20) {
        return bad_request;
    }

    if (should_move) {
        move(direction, time);
    }
    return ok_no_content;
}

void loop(){
    WiFiClient client = server.available();   // listen for incoming clients

    if (client) {
        Serial.println("New Client.");
        bool processing = 0;
        String currentLine = "";                // make a String to hold incoming data from the client
        String returnCode = err_not_found;
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();             // read a byte, then
                Serial.write(c);                    // print it out the serial monitor
                if (c == '\n') {                    // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {
                        client.println(returnCode);
                        client.println("Content-type: application/json");
                        client.println("Connection: close");
                        client.println();
                        if (returnCode == ok_content) {
                            client.println(deskStatus);
                        }
                        break;
                    } else {                                            // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                } else if (c != '\r') {                                 // if you got anything else but a carriage return character,
                    currentLine += c;                                   // add it to the end of the currentLine
                }
                // check if we have a HTTP req
                if (currentLine.startsWith("PUT /") && currentLine.endsWith("HTTP/1.1") && !processing) {
                    processing = 1;
                    returnCode = handle_requests(currentLine);
                }
                if (currentLine.startsWith("GET /status HTTP/1.1")) {
                    returnCode = ok_content;
                }
            }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
    }
}
