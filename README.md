# DIY Maimai Touch Screen

This project is a modification from the project shared by [Sucareto](https://github.com/Sucareto/Mai2Touch) (the version we based on is by [HelloworldDk ↗](https://github.com/HelloworldDk/dkmaiproj/blob/main/newmap-llvermtn202212271340/newmap-llvermtn202212271340.ino)) and aims to improve the stability and usability of the touch screen.

## Getting Started

Before you proceed, we strongly recommend you to read the [original repository ↗](https://github.com/HelloworldDk/dkmaiproj) for setup guide and understanding the basis of this project.

## Project Improvements

The improments upon the original project are as follows:

1. **Isolated MPR121 Parameters**: We've isolated key MPR121 parameters for easy modification. This will allow users to customize the sensitivity and responsiveness of the touch screen to meet their specific needs.

2. **Utilized .touched() Method**: We've switched from manually calculating the threshold using raw data to using the .touched() method. This not only improves the speed of the touch screen but also increases its stability by leveraging MPR121's debounce functionality.
