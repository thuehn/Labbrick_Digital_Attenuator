# Labbrick_Digital_Attenuator
Control Tool to use the digital attenuator from Labbrick in your WiFi experiments under Linux.

This is a small tool to use the LabBrick Digital Atenuator from Vaunix under Linux. It is tested and validated with the USB LDA-602 attenuator under Ubuntu 14.04.

![alt tag](https://cloud.githubusercontent.com/assets/1880886/9039179/033e9f86-39fa-11e5-869c-4fd7ee60424e.jpg)

As the Code of Vaunix is not open source it will not be included in this
repository so that everyone who wishes to use this program needs to download
their source and include it into the folder in order to build the tool with the provided makefile.

A typical experimental setup, where the digital anttenuator is connecto to a Laptop via USB looks like:

![alt tag](https://cloud.githubusercontent.com/assets/1880886/9039288/a4e8f3c2-39fa-11e5-8fd9-68f4c43a9418.jpg)

## How to compile ?

1. clone this resository to your local linux machine
2. request the Linux Library from Vaunix Lab brick via Email
3. copy the files: "LDAhid.h LDAhid.c" from Lab Bricks SDK into the src folder
5. ensure that you have the libusb-dev package installed in your system
6. build the tool via "make"
7. use the compiled "attenuator_lab_brick" tool to instruct the digital attenuator in your experiments

## How to use our tool ?

1. start our tool with  "sudo ./attenuator_lab_brick -h" to get a list of supported commands
2. either you set the attenuation level directly or for more complex attenuation patters via a csv file
3. we support rigth now full dB values only, where the minimum is 0dB and the maximum is 63dB

## Example usage with a csv file

The provided attenuation.csv file has the format: first row = step time (to be specified as seconds, milliseconds or microseconds), second row = attenuation in dB.
To read the example attenuation.csv file continiously with milliseconds step time, you can use:
```
sudo ./attenuator_lab_brick ms -r -f attenuation.csv -l att_log.txt > /dev/null
```

For more enhanced usage of our tool within your wireless experimentation look at the "run-experiment.sh" shell script as example.
