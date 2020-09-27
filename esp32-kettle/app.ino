#include <WiFi.h>
#include "wifi_creds.h"

#define basePin 16
WiFiServer server(80);

const char *ok_no_content = "HTTP/1.1 204 No Content";
const char *ok_content = "HTTP/1.1 200 OK";
const char *err_not_found = "HTTP/1.1 404 Not Found";
bool kettleIsActive = false;


void setup() {
    Serial.begin(115200);
    pinMode(basePin, OUTPUT);

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

String handle_requests(String line, bool *kettleIsActive) {
    String returnCode = err_not_found;
    if (line == "PUT /coffee HTTP/1.1") {
        digitalWrite(basePin, HIGH);
        *kettleIsActive = true;
        returnCode = ok_no_content;
    } else if (line == "PUT /stop HTTP/1.1") {
        digitalWrite(basePin, LOW);
        *kettleIsActive = false;
        returnCode = ok_no_content;
    } else if (line == "GET /status HTTP/1.1") {
        returnCode = ok_content;
    }

    return returnCode;
}

void loop(){
    WiFiClient client = server.available();   // listen for incoming clients

    if (client) {
        Serial.println("New Client.");
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
                            char buff[20];
                            sprintf(buff, "{\"kettleStatus\": %d}", kettleIsActive);
                            client.println(buff);
                            client.println();
                        }
                        break;
                    } else {                                            // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                } else if (c != '\r') {                                 // if you got anything else but a carriage return character,
                    currentLine += c;                                   // add it to the end of the currentLine
                }
                // check if we have a HTTP req
                if (currentLine.endsWith("HTTP/1.1")) {
                    if (currentLine.startsWith("PUT /") || currentLine.startsWith("GET /")) {
                        returnCode = handle_requests(currentLine, &kettleIsActive);
                    }
                }
            }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
    }
}
