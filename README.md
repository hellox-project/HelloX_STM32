HelloX operating system is a open source project dedicated to M2M(or IoT,Internet of Things) application,it include a compact kernel,some auxillary applications and development environment.
Any one can contribute it,and any contribution will be recorded in authors.txt file under the same directory as README file.
The source code and documents under this repository is specific for STM32 chipset,you can download the whole directory and open&compile it with MDK,then load it into your STM32 based board,it should work.Use serial port to monitor and control the operating system.
Please note the RAM of your board must be larger than 64K,otherwise please cut some function modules by editing config.h file under HXOS/config directory to shrink the binary size.
Any questions or problems please contact garryxin@gmail.com,or QQ/WeChat:89007638
