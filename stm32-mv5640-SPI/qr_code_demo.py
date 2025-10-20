# QR Code Detection Demo
# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# QR Code Recognition Example
#
# Welcome to the OpenMV IDE! Click on the green run arrow button below to run the script!

import sensor
import image
import time
import json
from pyb import UART

# Initialize the camera sensor
sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565
sensor.set_framesize(sensor.QVGA)    # Set frame size to QVGA (320x240) for better QR detection
sensor.skip_frames(time=2000)        # Wait for settings take effect.
clock = time.clock()                 # Create a clock object to track the FPS.

# Initialize UART for serial communication
uart = UART(3, 115200)  # Use UART 3 with baudrate 115200

# Flag to control whether to continue scanning
scanning = True

print("QR Code detection demo started. Point the camera at a QR code.")
print("Press 's' in the OpenMV IDE terminal to toggle scanning on/off.")

while True:
    clock.tick()             # Update the FPS clock.
    img = sensor.snapshot()  # Take a picture and return the image.
    
    # Check for user input to toggle scanning
    if uart.any():
        char = uart.read(1)
        if char == b's':
            scanning = not scanning
            print("Scanning:", "ON" if scanning else "OFF")
    
    # Only process QR codes if scanning is enabled
    if scanning:
        # Find QR codes in the image
        codes = img.find_qrcodes()
        
        # Process detected QR codes
        if codes:
            for code in codes:
                # Draw rectangle around the QR code
                img.draw_rectangle(code.rect(), color=(255, 0, 0))
                
                # Calculate center point from rect (x, y, w, h)
                rect = code.rect()
                cx = rect[0] + rect[2] // 2
                cy = rect[1] + rect[3] // 2
                
                # Draw cross at the center of the QR code
                img.draw_cross(cx, cy, color=(0, 255, 0))
                
                # Get the payload (data) of the QR code
                qr_data = code.payload()
                
                # Print the QR code data to the console
                print("QR Code Data:", qr_data)
                
                # Pause scanning after detecting a QR code
                scanning = False
                print("QR Code detected. Scanning paused. Press 's' to resume scanning.")
                
                # Send QR code data via UART
                try:
                    # Create JSON data structure
                    data = {
                        "type": "qrcode",
                        "data": qr_data,
                        "x": cx,
                        "y": cy
                    }
                    
                    # Convert to JSON and send via UART
                    json_data = json.dumps(data)
                    uart.write(json_data + '\n')
                except Exception as e:
                    print("UART Error:", e)
        else:
            # If no QR codes are detected
            print("No QR codes detected")
    
    # Print FPS to console
        print("FPS:", clock.fps())
    
    # Small delay to prevent overwhelming the serial output
    time.sleep_ms(100)