# ST7735-ESP8266-Webserver-Upload-Image
The sketch below is a complete working starting point. It implements the AP, the upload handler, saves to LittleFS, and contains the client-side JS that converts any image to JPEG sized to the display. The JPEG decoding call is shown as a small function you should wire to your chosen JPEG library (TJpg_Decoder or JPEGDecoder).
