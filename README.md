Tutorial: 
https://reactive.so/post/42-a-comprehensive-guide-to-ft_irc/
https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9

IRC Documentation:
https://modern.ircdocs.horse/

IRSSI client:
`sudo apt install irssi`
`irssi`
`/connect localhost <port> <pass>`

Or
`openssl s_client -connect irc.libera.chat:6697`

NC client:
`nc localhost <port>`
`PASS <pass>`
`USER <username> 0 * <realname>`

Check connections:
`netstat -an | grep <port>`
Check Suspend
`CTRL-Z` 
`fg`

Supported Commands: CAP,PASS,NICK,USER,NAMES,WHO,WHOIS,PRIVMSG,NOTICE,MODE,TOPIC,KICK,PART,JOIN,PING,PONG,INVITE,QUIT