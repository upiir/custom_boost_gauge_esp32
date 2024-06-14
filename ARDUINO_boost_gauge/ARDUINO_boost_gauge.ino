// Turbo boost gauge using the round LCD display from Waveshare with ESP32S3 and a turbo boost sensor

// created by upir, 2024
// youtube channel: https://www.youtube.com/upir_upir

// YouTube video: https://youtu.be/cZTx7T9uwA4
// Source files: https://github.com/upiir/custom_boost_gauge_esp32

// Links from the video:
// Do you like this sketch/tutorial/video? You can buy me a coffee ☕: https://www.buymeacoffee.com/upir
// Boost pressure sensor: https://s.click.aliexpress.com/e/_DBD9ak3
// Display with enclosure: https://s.click.aliexpress.com/e/_DkQiwQf
// Display without enclosure: https://s.click.aliexpress.com/e/_DEe0YJv
// Display documentation: https://www.waveshare.com/wiki/ESP32-S3-LCD-1.28
// Reddit post: https://www.reddit.com/r/arduino/comments/1bsl0l5/waveshare_esp32s3_128in_lcd_display_with_wifi/
// Online image converter (to C code): http://www.rinkydinkelectronics.com/_t_doimageconverter565.php
// Adapter board (1.27mm to 2.54mm Pitch): https://s.click.aliexpress.com/e/_Dc74hqb
// Image2cpp (convert array to image): https://javl.github.io/image2cpp/
// Photopea (online graphics editor like Photoshop): https://www.photopea.com/

// Related videos:
// Boost pressure Arduino + OLED: https://youtu.be/JXmw1xOlBdk
// Pitch Roll Indicator: https://youtu.be/S_ppdbb0poQ
// Custom Shifter Knob with Display: https://www.youtube.com/playlist?list=PLjQRaMdk7pBb6r6xglZb92DGyWJTgBVaV

#include <TFT_eSPI.h>       // Library for drawing on the display
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// uncomment only one of the header files to choose between two color themes - VIOLET or RED
#include "boost_gauge_images_RED.h" // all 61 images for boost gauge graphics
//#include "boost_gauge_images_VIOLET.h" // all 61 images for boost gauge graphics

// set the mode
int mode = 1; // 0 = ping pong animation, 1 = get value from boost pressure sensor

int frame = 0; // current frame
int frame_inc = 1; // frame increment

void setup()
{
  Serial.begin(115200); // start serial connection in case we want to print/debug some information
  tft.begin(); // initialize the display
  //tft.setRotation(1);	// rotation of the display set to landscape
  tft.setRotation(4); // in this rotation, the USB port is on the "bottom" of the screen
  tft.fillScreen(TFT_BLACK); // fill display with the black color
  tft.setSwapBytes(true); // Swap the colour byte order when rendering

  pinMode(4, INPUT); // GPIO4 as analog input 
  analogReadResolution(12); //set the resolution of analog input to 12 bits (0-4096)
}

float psi_older[20]; // store a few older readings for smoother readout

void loop()
{

  if (mode == 0) { // 0 = ping pong animation

    // play the animation - ping-pong - going from min to max and than back from max to min
    frame = frame + frame_inc;
    if ((frame > 60) && (frame_inc > 0)) {
      frame = 60;
      frame_inc = -1;
    } else if ((frame < 0) && (frame_inc < 0)) {
      frame = 0;
      frame_inc = 1;
    } 

    tft.pushImage(0, 0, 240, 240, epd_bitmap_allArray[frame]); // draw the fullscreen boost gauge image 

  } else if (mode == 1) { // 1 = get value from boost pressure sensor

    int pot_value = analogRead(4); // read analog in value from the boost pressure sensor 0-4096 

    float calculated_bar_pressure = (pot_value - 587.0) / 631.0; // calculate BAR pressure based on the voltage, this calculation was done based on measuring analog gauge
    float calculated_psi_pressure = calculated_bar_pressure * 14.504; // calculate PSI pressure from BAR pressure

    // store a few older measurements
    for (int i = 0; i < 9; i++) {
      psi_older[i] = psi_older[i+1];
    }
    psi_older[9] = calculated_psi_pressure;

    float psi_combined = 0;
    for (int i = 0; i < 10; i++) {
      psi_combined = psi_combined + psi_older[i];
    }
    psi_combined = psi_combined / 10; // final pressure value is the average of last 10 readings

    // set the image to be displayed based on the PSI pressure,
    // we have 60 images, and the PSI pressure should be in the range -15 .. 45, i.e., also 60 steps
    int image_pressure = round(psi_combined) + 15;
    image_pressure = constrain(image_pressure, 0, 60); // restrict the images to 0-60 in case the PSI pressure is outside the range of -15..45

    tft.pushImage(0, 0, 240, 240, epd_bitmap_allArray[image_pressure]); // draw the fullscreen boost gauge image 

  }

}
