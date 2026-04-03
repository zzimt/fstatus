# fstatus

Simple status bar.

## Table of Contents

- [Dependencies](#dependencies)
- [Usage](#usage)
    - [Building](#building)
    - [Installing](#installing)
    - [Configuration](#configuration)
- [License](#license)

## Dependencies

- `gcc`
- `make`
- `pkg-config`
- `X11`

## Usage

### Building

```bash
make
```

### Installing

```bash
sudo make install
```

### Running

```bash
# Run fstatus
fstatus

# Send update request to running fstatus process
fstatus -r

# Kill running fstatus process
fstatus -k
```

### Configuration

`fstatus` is configured by changing `src/config.h` and rebuilding the application.

See `src/modules` directory for individual module configuration values and defaults.

## License

MIT License. See [LICENSE](LICENSE) for details.
