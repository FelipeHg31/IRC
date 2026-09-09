*This project has been created as part of the 42 curriculum by dmaestro, juan-her and lde-medi*

# IRC

## Description

This project is an **Internet Relay Chat (IRC) server** written in C++98 from scratch, built as part of the 42 school curriculum. It implements a real, non-blocking TCP server using the POSIX `poll()` system call so that a single-threaded event loop can serve many clients concurrently without blocking on I/O.

The goal of the project is to reproduce a functional subset of a classic IRC server, closely following the behaviour described in the original IRC RFCs (1459, 2812, 2813) so that any standard IRC client (e.g. `irssi`, `weechat`, `netcat` with raw messages) can connect to it.

### Features

- **Concurrent, non-blocking server** based on `poll()` with a `1000ms` polling timeout.
- **Full registration flow**: `PASS`, `NICK`, `USER`, `CAP` and `QUIT`, with proper numeric replies and duplicate-nick rejection.
- **Channel management**: `JOIN`, `PART`, `KICK`, `TOPIC`, `INVITE`, `NAMES`, `WHO`, `LIST`, `PRIVMSG`, `NOTICE`, `PING`.
- **Channel modes**: `+i` (invite-only), `+t` (topic restricted), `+k` (password), `+l` (user limit), `+o` (operator privileges).
- **Case-insensitive nick/channel matching** following the IRC rfc1459 casemapping (`[{]}\|~^`).
- **A built-in bot ("KimJongBot")** that lets channel members run *democratic moderation*: anyone can start a vote to `KICK`, `PROMOTE` (`+o`) or `DEMOTE` (`-o`) another member, and the action is executed if a majority of channel members vote `YES` within 30 seconds.

### Bot usage (KIM)

The bot is a virtual client that is automatically an operator of every channel.

```
KIM HELP                        Show bot commands
KIM KICK    #channel <nick>     Start a vote to kick <nick>
KIM PROMOTE #channel <nick>     Start a vote to give +o to <nick>
KIM DEMOTE  #channel <nick>     Start a vote to remove -o from <nick>
KIM YES     #channel            Vote YES on the active vote
KIM NO      #channel            Vote NO on the active vote
```

A vote passes when more than half of the channel members vote `YES`. Votes follow users across nickname changes, and a vote is cancelled if the channel empties before it finishes.

## Instructions

### Requirements

- A Unix-like operating system (Linux or macOS).
- A C++ compiler with C++98 support (the project builds with `c++`/`clang++` on Linux/macOS).
- GNU `make`.

### Compilation

```sh
make            # build the ircserv binary
make re         # force a full rebuild
make clean      # remove object files and dependencies
make fclean     # remove object files and the binary
```

The `Makefile` compiles with `-Wall -Wextra -Werror -std=c++98 -g3`, so the build is strict and warning-free.

### Execution

```sh
./ircserv <port> <password>
```

- `<port>` — the listening port (must be between 1024 and 49151).
- `<password>` — the server password; every client must send it with `PASS` before registering.

Example:

```sh
./ircserv 6667 supersecret
```

Then connect with any IRC client or raw socket:

```
PASS supersecret
NICK alice
USER alice 0 * Alice Realname
JOIN #test
PRIVMSG #test :Hello everyone!
```

The server can be stopped gracefully with `Ctrl+C` (SIGINT) and is safe to restart immediately thanks to `SO_REUSEADDR`.

## Resources

### IRC protocol references

- **RFC 1459** — *Internet Relay Chat Protocol* — https://datatracker.ietf.org/doc/html/rfc1459
- **RFC 2810** — *Internet Relay Chat: Architecture* — https://datatracker.ietf.org/doc/html/rfc2810
- **RFC 2811** — *Internet Relay Chat: Channel Management* — https://datatracker.ietf.org/doc/html/rfc2811
- **RFC 2812** — *Internet Relay Chat: Client Protocol* — https://datatracker.ietf.org/doc/html/rfc2812
- **RFC 2813** — *Internet Relay Chat: Server Protocol* — https://datatracker.ietf.org/doc/html/rfc2813
- **modern.ircdocs.horse** — *Modern IRC Client Protocol* (community standard, very useful for numeric replies) — https://modern.ircdocs.horse/
- **DEFCON / architecture references**
  - `man poll(2)` — event-driven I/O multiplexing.
  - Beej's Guide to Network Programming — https://beej.us/guide/bgnet/

### How AI was used

AI (an interactive coding assistant) was used during development for the following tasks:

- **Debugging**: analysing server-side crashes, memory leaks and protocol errors found through valgrind and gdb; the valgrind runs and leak reports were used to validate that all heap allocations are freed.
- **Documentation**: Explaining concepts and useful options for coding.

