# Application information

__This example only supports the EU868 frequency plan.__ Please use https://github.com/jcaridadhdez/RAK811-tracker for other frequency plans.

This application sends GPS coordinates in the compact TTN Mapper format, as fast as is allowed by the LoRaWAN duty cycle regulations. At this rate you will break the TTN fair use policy if you keep on using this device for more than an hour. It is however ideal if you want to check if your gateway is working and to measure its coverage.

To use this sketch to contribute to TTN Mapper, create a dedicated mapping application on the TTN Console. Create a device, then in its settings switch it to ABP and disable frame counter checks.

After you programmed the device and you see data arriving on the TTN Console, you can add a payload decoder function. Use the code inside `decoder.js` for this. If you see that the coordinates from you devices under the Data tab and it is decoded sucessfully, you can simply enable the TTN Mapper integration.

# Programming your RAK811

Install Platormio for either Microsoft Visual Studio Code, or Atom.

## Add new board definition

In Platformio go to Platforms, Embedded and search for `ST STM32` and install it.

After you installed the STM32 board definitions you have to add the RAK811 customisation. Under Platforms, Installed, next to ST STM32, click on reveal to open the board definitions in your file browser. You should see a folder names `ststm32`, open it, then open the `boards` folder. Copy the `rak811.json` file that is included with this repository into this `boards` folder.


## Setting Up

Open the RAK-811 project in Platformio. On the left in the file browser you should see a `src` folder. Expand it, and then double click on the `mapper.c` file.

At the top of the file change the `DevAddr`, `NwkSKey` and `AppSKey` to the ones you obtained form the TTN Console. When copying them from the console make sure the format is correct, ie. MSB with braces.


## Compile

At the bottom of the Platformio window, there are a couple of small icons that will show a tooltip when you hover above them with your mouse. Find and click on the one that says "New Terminal". A small window should open at the bottom of the screen where you can type in commands. Type in the following to compile the software:

    pio run

## Program your RAK811

In the corner of the RAK811 GPS board there is a header pin you can set to two positions. The position closest to the corner is for porgramming mode, and the other position is normal mode in which your code executes. After changing the jumper position you have to either press the reset bottun next to the USB port, or power cycle the board.

![alt text](https://raw.githubusercontent.com/username/projectname/branch/path/to/img.png)

Switch the jumper to the programming position and reset the board. Now in the Platformio terminal we used in the previous step, run the following command:

    stm32flash -e 254 -w .pioenvs/rak811/firmware.bin /dev/ttyUSB0

The last part of that command is the name of the port to which the RAK module is connected on your computer. On Windows it will likely be something like COM3. On Ubuntu you can have to install the `stm32flash` via APT. Depending on your platform there will be different methods to obtain this utility. A copy of it is also included in the `stm32flash_src` folder in this repository.

# Acknowledgements
This code is possible thanks to the previous work done by @Ebiroll to program the RAK811 from Platformio. You can find his work here: https://github.com/Ebiroll/RAK811_BreakBoard

This code has been adapted to US915 band by @RussNelson https://github.com/RussNelson 

This work is carried out within the project http://openmaker.eu/2018/02/05/fenps/ financed by the EU's HORIZON 2020 programme.
