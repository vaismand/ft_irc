Tutorial: 
https://reactive.so/post/42-a-comprehensive-guide-to-ft_irc/
https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9
https://github.com/barimehdi77/ft_irc


IRC Documentation:
https://modern.ircdocs.horse/

IRSSI client:
sudo apt install irssi
irssi
/connect localhost <port> <pass>

Server - David:
1. Server is running with port >1024 and any password
2. Server finally connecting to irssi server
3. NC connection

Channel - Ruben
1. Extra functions for channel for inviting/kicking/etc

Parsing - Dani
1. Please start with parsing command in all our Command.cpp function(maybe make a separate function for it)
2. Numeric Replies:
The server is expected to respond with numeric codes (e.g., 001 for welcome, 433 for nickname in use, etc.) - work on this numeric codes please.
