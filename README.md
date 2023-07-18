# DIY Maimai Touch Screen

Chinese version of README.md is available at [中文简介 ↗](README_zh.md)

This project is a modification from the project shared by [Sucareto ↗](https://github.com/Sucareto/Mai2Touch) (the version we based on is by [HelloworldDk ↗](https://github.com/HelloworldDk/dkmaiproj/blob/main/newmap-llvermtn202212271340/newmap-llvermtn202212271340.ino)) and aims to improve the stability and usability of the touch screen.

## FOR ANYONE MIGRATING FROM newmap-llvermtn202212271340.ino, PLEASE READ THE MIGRATION GUIDE**

## Getting Started

Before you proceed, we strongly recommend you to read the [original repository ↗](https://github.com/HelloworldDk/dkmaiproj) for setup guide and understanding the basis of this project.

## Project Improvements

The improments upon the original project are as follows:

1. **Isolated MPR121 Parameters**: We've isolated key MPR121 parameters for easy modification. This will allow users to customize the sensitivity and responsiveness of the touch screen to meet their specific needs.

2. **Utilized .touched() Method**: We've switched from manually calculating the threshold using raw data to using the .touched() method. This not only improves the speed of the touch screen but also increases its stability by leveraging MPR121's debounce functionality.
3. **Enabling Charge Discharge Time search by default**: MPR121 is able to search for the optimal charge and discharge time for each electrode. dk's version disabled this feature for some reason. We found enabling this feature results in a more stable touch screen.

## Migration Guide

### migrating from newmap-llvermtn202212271340.ino

1. User should define the threadhold in config.h instead of the main program.
2. The first column of touchmap is now 0-indexed instead of 1-indexed. User should subtract 1 from the first column of touchmap.
3. The third column of touchmap is now an offset instead of the actual threashold. User should change all the values to 0 first and then adjust the offset. The acceptable range of the offset is -128 to 127.
4. The definition of number of electrodes of each MPR121 is now at `config.h`. User should not change the code in the ino file.

### migrating from Mai2TouchOK or other versions

Please read the migration guide from newmap-llvermtn202212271340.ino first and then read the comments in the code to decide how to migrate.
