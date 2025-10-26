```cpp
// main.ino
// ESP8266 AP image upload -> ST7735S example (starting point)
// - Serves a web page that converts any image in browser to a JPEG at display resolution,
//   then POSTs to ESP.
// - ESP saves JPEG to LittleFS and then displays it (JPEG decode step uses TJpg_Decoder or JPEGDecoder).
//
// Hardware mapping example (NodeMCU / Wemos D1 mini):
// SCLK -> D5 (GPIO14)
// MOSI -> D7 (GPIO13)
// CS   -> D8 (GPIO15)
// DC   -> D2 (GPIO4)
// RST  -> D4 (GPIO2)
// VCC  -> 3.3V
// GND  -> GND
//
// Libraries needed:
// - Adafruit_GFX
// - Adafruit_ST7735
// - LittleFS (comes with ESP8266 core)
// - TJpg_Decoder (or JPEGDecoder) for decoding JPEGs saved in LittleFS
// - ESP8266WiFi, ESP8266WebServer
//
// NOTE: you MUST install the JPEG decoder library and adapt the showImage() function
// to the decoder API. See comments in showImage() below.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// ---- Pin configuration (adjust if needed) ----
#define TFT_CS   D8   // GPIO15
#define TFT_DC   D2   // GPIO4
#define TFT_RST  D4   // GPIO2

// ST7735 resolution (common): 128 x 160
const uint16_t TFT_WIDTH  = 128;
const uint16_t TFT_HEIGHT = 160;

// Create display object
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Web server on port 80
ESP8266WebServer server(80);

// File where uploaded JPEG will be stored
const char *uploadedFile = "/uploaded.jpg";


// Minimal HTML page (client-side converts images to JPEG at display resolution)
const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>Upload image to ST7735</title>
</head>
<body>
  <h3>Upload image to ST7735</h3>
  <input id="file" type="file" accept="image/*"><br><br>
  <button id="send">Send to ESP</button>
  <div id="status"></div>
<script>
const WIDTH = %TFT_W%;
const HEIGHT = %TFT_H%;
const STATUS = document.getElementById('status');
document.getElementById('send').addEventListener('click', async () => {
  const input = document.getElementById('file');
  if (!input.files || !input.files[0]) { STATUS.innerText = 'Pick a file first'; return; }
  const file = input.files[0];
  STATUS.innerText = 'Loading image...';
  const img = new Image();
  const reader = new FileReader();
  reader.onload = function(e) {
    img.src = e.target.result;
  };
  reader.readAsDataURL(file);
  img.onload = async function() {
    STATUS.innerText = 'Resizing and converting...';
    const canvas = document.createElement('canvas');
    canvas.width = WIDTH;
    canvas.height = HEIGHT;
    const ctx = canvas.getContext('2d');
    // Optionally clear background to black
    ctx.fillStyle = 'black';
    ctx.fillRect(0,0,WIDTH,HEIGHT);
    // Draw the image centered and scaled to fit preserving aspect
    let sx = 0, sy = 0, sWidth = img.width, sHeight = img.height;
    // Fit image to canvas (cover or contain behavior). We'll contain it (letterbox).
    const ratio = Math.min(WIDTH / img.width, HEIGHT / img.height);
    const dw = img.width * ratio;
    const dh = img.height * ratio;
    const dx = (WIDTH - dw) / 2;
    const dy = (HEIGHT - dh) / 2;
    ctx.drawImage(img, 0, 0, img.width, img.height, dx, dy, dw, dh);
    // Convert to JPEG to keep payload small
    canvas.toBlob(async function(blob) {
      STATUS.innerText = 'Uploading...';
      const form = new FormData();
      form.append('image', blob, 'image.jpg');
      try {
        const resp = await fetch('/upload', { method:'POST', body: form });
        const text = await resp.text();
        STATUS.innerText = 'Upload complete: ' + text;
      } catch (err) {
        STATUS.innerText = 'Upload failed: ' + err;
      }
    }, 'image/jpeg', 0.85);
  };
  img.onerror = function() { STATUS.innerText = 'Failed to read image'; };
});
</script>
</body>
</html>
)rawliteral";


// Helper to serve index page with the display resolution inserted
String getIndexPage() {
  String s = index_html;
  s.replace("%TFT_W%", String(TFT_WIDTH));
  s.replace("%TFT_H%", String(TFT_HEIGHT));
  return s;
}


// This function should use your chosen JPEG decoder library to open the file
// at path (LittleFS) and render it to the tft. The exact API depends on the
// decoder library you pick. Below are pointers and a stub you must adapt.
//
// If you use TJpg_Decoder (Bodmer), see:
//  https://github.com/Bodmer/TJpg_Decoder
// TJpg_Decoder examples often use a callback that delivers decoded blocks to a
// draw callback where you can use tft.drawRGBBitmap().
//
// If you use JPEGDecoder (other library), check its examples for reading from
// LittleFS and pushing pixels to Adafruit_GFX.
//
// For now this function is a stub that demonstrates where to plug in the call.
void showImage(const char *path) {
  Serial.printf("Attempting to show image: %s\n", path);
  // Example approach using a decoder that creates an RGB565 buffer per-block:
  // 1) Open file with LittleFS
  // 2) Call decoder to decode the JPEG and, for each decoded rectangle:
  //    - get pointer to uint16_t bitmap (RGB565)
  //    - call tft.drawRGBBitmap(x + offset, y + offset, bitmap, w, h);
  //
  // Example pseudocode (non-compilable):
  //   JpegDec.decodeFsFile(path);
  //   while (JpegDec.read()) {
  //      uint16_t *bmp = JpegDec.pImage;
  //      int x = JpegDec.x;
  //      int y = JpegDec.y;
  //      int w = JpegDec.width;
  //      int h = JpegDec.height;
  //      tft.drawRGBBitmap(x, y, bmp, w, h);
  //   }
  //
  // See your JPEG library docs for exact calls and integrate here.

  // ---- Simple fallback: show stored image size and info ----
  File f = LittleFS.open(path, "r");
  if (!f) {
    Serial.println("File not found");
    return;
  }
  Serial.printf("Uploaded file size: %u bytes\n", (unsigned)f.size());
  f.close();

  // TODO: Replace the following with actual decode+render using TJpg_Decoder or JPEGDecoder.
  // For now, clear screen and show a placeholder rectangle so you can see something.
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Image uploaded.");
  tft.print("File: "); tft.println(path);
  tft.print("Size: "); tft.println(String(LittleFS.open(path,"r").size()));
  // After you hook up TJpg_Decoder, remove this placeholder and decode/render instead.
}


// Handle upload: writes incoming upload to LittleFS and at the end calls showImage()
void handleUpload() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Upload start: %s\n", upload.filename.c_str());
    // delete old file if exists
    if (LittleFS.exists(uploadedFile)) LittleFS.remove(uploadedFile);
    // open file for writing
    File f = LittleFS.open(uploadedFile, "w");
    f.close();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // append data
    File f = LittleFS.open(uploadedFile, "a");
    if (f) {
      f.write(upload.buf, upload.currentSize);
      f.close();
    } else {
      Serial.println("Failed to open file for write");
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.printf("Upload finished: %u bytes\n", (unsigned)upload.totalSize);
    // Call showImage to decode and display
    showImage(uploadedFile);
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting ESP8266 ST7735S image uploader example");

  // LittleFS mount
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
  } else {
    Serial.println("LittleFS mounted");
  }

  // Init TFT
  tft.initR(INITR_BLACKTAB); // use appropriate init for your module
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);

  // Start WiFi AP
  const char *ssid = "ESP-ST7735";
  const char *password = nullptr; // open AP. Set a password if you want.
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Web server routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getIndexPage());
  });

  // upload handling: note on() signature that accepts two handlers: first to finalise, second is upload handler
  server.on("/upload", HTTP_POST, []() {
    // Called after upload is finished
    server.send(200, "text/plain", "OK");
  }, handleUpload);

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}