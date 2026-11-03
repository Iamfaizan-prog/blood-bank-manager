# Blood Bank Management System

A local, single-computer project with a C99 backend, independent terminal interface, command-line adapter, Python standard-library HTTP bridge, and vanilla HTML/CSS/JavaScript frontend.

## Requirements
- GCC
- GNU Make
- Python 3
- A modern browser

## Build
Open a terminal in this folder and run:

```sh
make
```

## Run the terminal application

```sh
make run
```

The TUI does not require Python or the web frontend.

## Run the local website
In a second terminal, from this same folder:

```sh
make web
```

Open `http://127.0.0.1:8000` in your browser. The bridge binds only to `127.0.0.1`, so it is intended for this computer, not a public network.

## Data
Binary data is stored in `data/donors.dat` and `data/inventory.dat`. Run the programs from the project root so these relative paths resolve correctly. Back up the `data` folder before replacing or changing the project.

## Architecture
- `src/core`: donor operations, inventory operations, validation, and binary persistence.
- `src/tui`: terminal menus and prompts. Calls the C core directly.
- `src/cli`: non-interactive command interface used by the Python bridge.
- `bridge/server.py`: local HTTP server and static frontend server. It does not implement donor or inventory rules.
- `frontend`: browser interface.

## CLI commands
The CLI is normally called by the bridge. Examples:

```sh
./bin/bb_cli list
./bin/bb_cli inventory
./bin/bb_cli get 1001
./bin/bb_cli stock-add O+ 3
./bin/bb_cli stock-deduct O+ 1
```

## Notes
- The system is designed for a single local user. Do not run simultaneous writers against the binary data files.
- This educational project is not a clinical donor-screening system. Real donation eligibility must be decided by qualified medical personnel and applicable local policy.
- The replacement project has its own binary data files. Do not copy old binary files into it unless their structure and field layout have been verified.
