# AshEmu

A lightweight World of Warcraft 2.4.3 (TBC) server emulator written in C17.

## Features

- **Auth Server** - SRP6 authentication protocol implementation
- **World Server** - Basic world server with character creation and login
- **SQLite Database** - Lightweight persistence for accounts and characters
- **Cross-Platform** - Builds on Windows and Linux

## Requirements

### Build Dependencies
- CMake 3.16+
- C17 compatible compiler (MSVC, GCC, Clang)
- OpenSSL 1.1+ (for cryptographic functions)

### Runtime
- WoW Client 2.4.3 (8606)

## Building

### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Linux

```bash
mkdir build
cd build
cmake ..
make
```

## Running

### Combined Launcher (Auth + World)
```bash
./ashemu
```

### Standalone Servers
```bash
./ashemu_auth   # Auth server only (port 3724)
./ashemu_world  # World server only (port 8085)
```

## Client Configuration

Set your `realmlist.wtf` to:
```
set realmlist 127.0.0.1
```

## Default Ports

| Server | Port |
|--------|------|
| Auth   | 3724 |
| World  | 8085 |

## Project Structure

```
AshEmu/
├── common/          # Shared library (networking, crypto, packets)
├── database/        # SQLite database layer
├── auth/            # Authentication server
├── world/           # World server
├── launcher/        # Combined launcher
└── third_party/     # Third-party dependencies (SQLite)
```

## Auto-Account Creation

New accounts are automatically created on first login attempt. The password defaults to the username.

## License

MIT License - See [LICENSE](LICENSE) for details.

## Disclaimer

This project is for educational purposes only. World of Warcraft is a trademark of Blizzard Entertainment, Inc. This project is not affiliated with or endorsed by Blizzard Entertainment.
