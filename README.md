# bridev

This is a simple application that reads and writes brightness settings.

I personally use this as a suid program, so security is a big concern.

## Usage

With only a device name; the program prints the current brightness setting for that device.

```
$ bridev intel_backlight
3125
```

With an added numerical argument the program will change the brightness, and print the before and after brightness settings.

```
$ bridev intel_backlight 50
3125 -> 1562
```

I have a `bri` script in my `PATH` with this contents:

```
#!/bin/sh

bridev intel_backlight "$1"
```

This lets me simply type `bri 30` to change my brightness.
