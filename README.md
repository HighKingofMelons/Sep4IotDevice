# Sep4IotDevice

## Requirments
This project is developed only with support for Arduino Mega2560 programmed using an Atmel-ICE
Additionally a custom header is required, see [ihavn/IoT_Semester_project](https://github.com/ihavn/IoT_Semester_project)

## Cloning Project
To clone the project, as well as submodules, run the following command:

`git clone https://github.com/HighKingofMelons/Sep4IotDevice --recurse-submodules`

## Running on Microchip Studio
* Open project file (`Sep4IotDevice.cproj`) in Microchip Studio

* Select Programmmer/Debugger e.g. Atmel-ICE

* Build then Upload and Run

## Running using PlatformIO
* Install `PlatformIO` VSCode extension

* Open project folder

* Select appropriate env according to the computer of which is used to program the device.
E.g. if the programmer (Atmel-ICE) is plugged into a windows machine, use windows. By default Windows is selected.

* Press Upload/Remote Upload