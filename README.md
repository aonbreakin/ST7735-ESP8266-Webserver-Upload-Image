# ST7735-ESP8266-Webserver-Upload-Image
Based on my research, I'll provide you with a complete guide for setting up an ESP8266 with ST7735S display for image uploading and viewing. Let me break this down into clear sections:

Hardware Requirements:
ESP8266 board (NodeMCU or Wemos D1 Mini recommended)
ST7735S TFT Display (typical resolution 128x160 pixels)
Connecting wires
Wiring Connection (ESP8266 to ST7735S):

ST7735S Pin -> ESP8266 (NodeMCU/Wemos D1 Mini)
VCC        -> 3.3V (IMPORTANT: Use 3.3V, not 5V)
GND        -> GND
SCL/SCK    -> D5 (GPIO14)
SDA/MOSI   -> D7 (GPIO13)
RES/RST    -> D4 (GPIO2)
DC/A0      -> D2 (GPIO4)
CS         -> D8 (GPIO15)
BL         -> 3.3V (or GPIO pin if you want backlight control)

Required Libraries (with latest stable versions):


1. ESP8266 Arduino Core (v3.1.2)
   - Board Manager URL: https://arduino.esp8266.com/stable/package_esp8266com_index.json

2. Adafruit ST7735 Library (v1.10.7)
   - GitHub: https://github.com/adafruit/Adafruit-ST7735-Library
   - Install via Library Manager: "Adafruit ST7735 and ST7789 Library"

3. Adafruit GFX Library (required dependency)
   - Install via Library Manager: "Adafruit GFX Library"

4. TJpg_Decoder (for JPEG decoding)
   - GitHub: https://github.com/Bodmer/TJpg_Decoder
   - Download and add as ZIP library
  
   Arduino IDE Settings:
Board: "NodeMCU 1.0 (ESP-12E Module)" or "LOLIN(WEMOS) D1 R2 & mini"
Flash Size: "4MB (FS:2MB OTA:~1019KB)"
CPU Frequency: "80 MHz"
Upload Speed: "115200" (or higher if stable)

Features of the provided code:
Creates an Access Point (AP) named "ESP-ST7735"
Serves a web interface at 192.168.4.1
Supports upload of any image format (auto-converts in browser)
Automatically resizes images to ST7735S resolution
Converts uploads to JPEG format for efficient storage
Uses LittleFS for file storage
Supports common image formats (JPEG, PNG, GIF) through browser-side conversion
Here's a simplified version of the main code showing the key setup parts:

Code: main.ino

To use this setup:

Install all required libraries through Arduino IDE Library Manager
Copy and upload the code to your ESP8266
Connect to the "ESP-ST7735" WiFi network
Open 192.168.4.1 in your browser
Upload any image through the web interface
Important Notes:

The ST7735S runs on 3.3V logic - never connect it to 5V
Make sure you have enough flash space (4MB recommended)
The web interface automatically handles image conversion and resizing
Images are stored in LittleFS, so you need to upload the filesystem image or format it first time
Would you like me to provide:

The complete web interface code with the image conversion JavaScript?
The JPEG decoder integration code?
A version with multiple image storage and selection?
