# Make service:

1. build and copy ./test to /home/pi/test
2. create file spider\_bot.service in /home/pi/test
3. copy calibration file links.bin to /home/pi/test
4. copy file spider\_bot.service to /etc/systemd/system/: `sudo cp ./spider_bot.service /etc/systemd/system/` 
5. start service: `sudo systemctl start spider_bot.service`
6. stop service: `sudo systemctl stop spider_bot.service`
7. show sevice log: `sudo journalctl -u spider_bot.service`
8. for start automatically on reboot by using this command: `sudo systemctl enable spider_bot.service`

