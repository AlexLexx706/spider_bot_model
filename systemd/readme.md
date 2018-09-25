Make serive
------------
0. create file spider_bot.service in /home/pi/test
1. copy calibration file links.bin to /home/pi/test
2. copy file spider_bot.service to /etc/systemd/system/: `sudo cp ./spider_bot.service /etc/systemd/system/` 
3. start service: `sudo systemctl start spider_bot.service`
4. stop service: `sudo systemctl stop spider_bot.service`
5. show sevice log: `sudo journalctl -u spider_bot.service`
6. for start automatically on reboot by using this command: `sudo systemctl enable spider_bot.service`