#!/bin/bash

echo "Content-Type: text/html"
echo "Location: /index.html"
echo "Status: 302 Found"
echo ""

#mac
# osascript -e 'tell application "Terminal" to do script "Documents/webserv/demo/var/www/html/projects/2048_mac"'

#linux
export DISPLAY=:0
#gnome-terminal -- bash -c "./var/www/html/projects/2048; exec bash" &
gnome-terminal  -- bash -c "./var/www/html/projects/2048; exec bash" 
#--disable-factory

#nohup ./var/cgi-bin/2048 > /dev/null 2>&1 &
exit 0
