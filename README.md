![loghog](https://github.com/unixbox-net/loghog/assets/104218206/351322c5-0962-427c-bb4c-2eb3ac3244c1)

# How to Install
The inital release has been prepackageed for dnf/yum, however there source is there.. fill your boots.

```bash
git clone https://github.com/unixbox-net/loghog.git
cd loghog
dnf install -y loghog-1.0.0-1.el8.x86_64.rpm
````
or
```bash
clang loghog.c -o loghog -lreadline -ljson-c
```

## Purpose
LogHOG is a **FAST** and comprehensive log search tool designed to simplify log analysis. It offers two modes for reviewing and analyzing logs:

### Modes
- **TAIL MODE:**  
  Logs are automatically stitched together by timestamp, making events easy to follow in real-time.  
  Press `CTRL+C` to quit.

- **LESS MODE:**  
  Buffers from TAIL mode are sent directly to **less**, a powerful text-editing tool that allows for in-depth review, searches, and real-time log analysis.  
  Press `h` for help or `q` to quit.

## Features
- **Effortless Log Stitching:** Logs are automatically sorted by timestamp.
- **Regex Support:** Powerful regular expressions for filtering.
- **Custom Log Paths:** Tailor log paths based on your requirements.
- **JSON Export:** Export filtered logs to JSON for further analysis.

## Speed
Not only is LogHOG efficient and fast it makes most common system problems.  "plain" removing the mystisim of trying to "stitch logs together" and elminating the need for 
multiple windows / terminals. 

all in a package format that anyone can use/enjoy
