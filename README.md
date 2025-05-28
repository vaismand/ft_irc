
# ft_irc – Internet Relay Chat Server (C++98)

This project is a custom-built IRC server implemented in C++98 as part of the 42 School curriculum. It aims to deepen understanding of low-level network programming and the IRC protocol.

## 🚀 Project Summary

**ft_irc** is a standalone IRC server that communicates with standard IRC clients over TCP/IP using the IRC protocol. It handles multiple client connections simultaneously using a single `poll()` (or equivalent) loop and offers core IRC functionality, including user authentication, private messaging, channel management, and operator privileges.

## 🔧 Features

### ✅ Mandatory Features
- TCP-based IRC server listening on a user-defined port
- Non-blocking I/O using `poll()` (or `select()`)
- Client authentication with password
- Nickname and username management
- Channel creation and joining
- Real-time private and public messaging
- Operator role system with support for:
  - `KICK` – Remove a user from a channel
  - `INVITE` – Invite user to a channel
  - `TOPIC` – Set/view channel topic
  - `MODE` – Manage channel modes:
    - `i` – Invite-only
    - `t` – Topic change restrictions
    - `k` – Channel key/password
    - `o` – Operator privileges
    - `l` – User limit

### 💡 Bonus Features (if implemented)
- IRC Bot with automated responses

## ⚙️ Usage

### Compile
```bash
make
```

### Run the server
```bash
./ircserv <port> <password>
```
- `port`: Port number to listen on
- `password`: Required password for clients to connect

### Connect via IRC client
You can use any IRC client (e.g., `irssi`, `weechat`, or `HexChat`) to connect:
```bash
/connect 127.0.0.1 <port> <password>
```

## 🧪 Testing

To test manually using `netcat`:
```bash
nc -C 127.0.0.1 6667
```
Send partial commands with `Ctrl+D` to test packet aggregation:
```
com^Dman^Dd
```

## 🛠 Technologies

- Language: **C++98**
- Network: **BSD sockets**, **poll()**
- Platform: Linux / macOS

## 📁 Project Structure

```
.
├── src/                # Source files
├── include/            # Header files
├── Makefile            # Build instructions
└── README.md           # Project documentation
```

## 📌 Constraints

- Only one `poll()` (or equivalent) instance
- No `fork()` or blocking I/O
- No external libraries or Boost
- Pure C++98 compliance with `-Wall -Wextra -Werror`

## 👨‍💻 Authors

This project was developed by a group of 3 students from [42Vienna].

- vaismand
- RubenPin90
- Poddani

## 📚 Resources

- [Modern IRC Protocol Documentation](https://modern.ircdocs.horse/)

## 📄 License

This project is for educational purposes and does not include a specific license.