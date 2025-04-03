import socket
import time

def send_cmd(sock, cmd, wait=0.5):
    print(f">>> {cmd.strip()}")
    sock.send((cmd + "\r\n").encode())
    time.sleep(wait)
    try:
        response = sock.recv(4096).decode(errors='ignore')
        if response.strip():
            print(response)
        return response
    except socket.timeout:
        print("⏳ No response (yet)")
        return ""

def create_client(nick, username, password, port=6667, host="127.0.0.1"):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    s.settimeout(1.0)
    send_cmd(s, f"CAP LS")
    send_cmd(s, f"PASS {password}")
    send_cmd(s, f"NICK {nick}")
    send_cmd(s, f"USER {username} 0 * :{username} Realname")
    return s

def test_basic_flow(password="1111", port=6667):
    print("=== Connecting clients ===")
    c1 = create_client("alice", "alice", password, port)
    c2 = create_client("bob", "bob", password, port)

    print("=== Channel commands ===")
    send_cmd(c1, "JOIN #test")
    send_cmd(c2, "JOIN #test")
    send_cmd(c1, "NAMES #test")
    send_cmd(c1, "WHO #test")
    send_cmd(c1, "TOPIC #test :This is our channel")
    send_cmd(c2, "TOPIC #test")

    print("=== Messaging ===")
    send_cmd(c1, "PRIVMSG #test :Hello from Alice")
    send_cmd(c2, "NOTICE #test :Hi Alice, this is Bob")

    print("=== Operator test ===")
    send_cmd(c1, "MODE #test +o bob")
    send_cmd(c2, "MODE #test -o alice")

    print("=== WHOIS test ===")
    send_cmd(c1, "WHOIS bob")
    send_cmd(c1, "WHOIS")  # edge case

    print("=== INVITE + KICK + PART ===")
    send_cmd(c2, "INVITE alice #test")  # should fail if already joined
    send_cmd(c2, "KICK #test alice :Bye")
    send_cmd(c1, "PART #test")
    send_cmd(c1, "JOIN #test")

    print("=== Misc protocol ===")
    send_cmd(c1, "PING :1234")
    send_cmd(c2, "PONG :1234")
    send_cmd(c1, "QUIT :Leaving")

    print("=== Edge case errors ===")
    send_cmd(c2, "NICK")
    send_cmd(c2, "USER")
    send_cmd(c2, "JOIN")
    send_cmd(c2, "MODE")
    send_cmd(c2, "KICK")
    send_cmd(c2, "PRIVMSG")
    send_cmd(c2, "WHOIS nosuchuser")

    print("=== Cleanup ===")
    c1.close()
    c2.close()
    print("=== Done ===")

if __name__ == "__main__":
    test_basic_flow()