## Mandatory Part

**Program name:** ircserv  
**Turn in files:** Makefile, *.{h, hpp}, *.cpp, *.tpp, *.ipp, an optional configuration file  
**Makefile:** NAME, all, clean, fclean, re  
**Arguments:**  
- `port`: The listening port  
- `password`: The connection password  

**External functions:**  
- Everything in C++ 98.  
- socket, close, setsockopt, getsockname, getprotobyname, gethostbyname, getaddrinfo, freeaddrinfo, bind, connect, listen, accept, htons, htonl, ntohs, ntohl, inet_addr, inet_ntoa, send, recv, signal, sigaction, lseek, fstat, fcntl, poll (or equivalent)

**Libft authorized:** n/a  

**Description:**  
An IRC server in C++ 98.  
- You have to develop an IRC server in C++ 98.  
- You mustn’t develop a client.  
- You mustn’t handle server-to-server communication.  

**Execution:**  
Your executable will be run as follows:  
`./ircserv <port> <password>`  

- **port**: The port number on which your IRC server will be listening to for incoming IRC connections.  
- **password**: The connection password. It will be needed by any IRC client that tries to connect to your server.  

Even if `poll()` is mentioned in the subject and the evaluation scale, you can use any equivalent such as `select()`, `kqueue()`, or `epoll()`.

## III.1 Requirements

- The server must be capable of handling multiple clients at the same time and never hang.  
- Forking is not allowed. All I/O operations must be non-blocking.  
- Only **1 poll()** (or equivalent) can be used for handling all these operations (read, write, listen, etc.).  

  - Non-blocking file descriptors must be used.  
  - It is possible to use `read/recv` or `write/send` functions without `poll()` (or equivalent), but doing so would consume more system resources.  
  - If `read/recv` or `write/send` is used on any file descriptor without `poll()` (or equivalent), the grade will be **0**.

- Several IRC clients exist. You have to choose one of them as a reference.  
  - Your reference client will be used during the evaluation process.  
  - It must be able to connect to your server without encountering any error.  

- Communication between the client and server must be done via TCP/IP (v4 or v6).  

- Using your reference client with your server must be similar to using it with any official IRC server.  
  - You only have to implement the following features:  
    - Authenticate, set a nickname, a username, join a channel, and send/receive private messages using the reference client.  
    - All messages sent from one client to a channel must be forwarded to every other client in that channel.  
    - Operators and regular users must be distinguished.  
    - Implement commands for channel operators:  
      - **KICK** - Eject a client from the channel.  
      - **INVITE** - Invite a client to a channel.  
      - **TOPIC** - Change or view the channel topic.  
      - **MODE** - Change the channel’s mode:  
        - `i`: Set/remove invite-only channel.  
        - `t`: Restrict the `TOPIC` command to channel operators.  
        - `k`: Set/remove the channel key (password).  
        - `o`: Give/take channel operator privilege.  
        - `l`: Set/remove the user limit for the channel.  

- Of course, you are expected to write clean code.