# LogHOG: The NO Nonsense Log Search Tool

![LogHOG ASCII Art](https://i.imgur.com/JFrtbLj.png)

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
LogHOG uses efficient log collection and filtering methods, allowing you to swiftly process vast amounts of log data without breaking a sweat.

## Menu Overview
```ansi
[A] Authentication (Tail)
[Cyan]Identify events such as:[/Cyan]
[Blue]'authentication failed'[/Blue]
[Blue]'permission denied'[/Blue]
[Blue]'invalid user'[/Blue]
[Blue]'unauthorized access'[/Blue]
[Blue]'SQL injection detected'[/Blue]
[Blue]'cross-site scripting attempt'[/Blue]
[Blue]'directory traversal attack'[/Blue]
[Blue]'and more...'[/Blue]

[E] Errors (Tail)
[Cyan]Filters logs for error-related events:[/Cyan]
[Blue]'error'[/Blue]
[Blue]'failure'[/Blue]
[Blue]'critical'[/Blue]
[Blue]'socket timeout'[/Blue]
[Blue]'network reset'[/Blue]
[Blue]'DNS resolution failure'[/Blue]
[Blue]'permission denied'[/Blue]

[L] Live (LIVE, timestamp order)
[Cyan]Displays every log in real-time, sorted by timestamp.[/Cyan]

[N] Network Protocol Filter (Tail)
[Cyan]Filters logs by protocol:[/Cyan]
[Blue]'http://'[/Blue]
[Blue]'https://'[/Blue]
[Blue]'ftp://'[/Blue]
[Blue]'ssh://'[/Blue]
[Blue]'telnet://'[/Blue]
[Blue]'smtp://'[/Blue]
[Blue]'sftp://'[/Blue]

[R] Regex (Tail)
[Cyan]Search EVERYTHING using standard regular expressions:[/Cyan]
[Blue]'error|failure'[/Blue]
[Blue]'\\bREGEX_PATTERN\\b'[/Blue]
[Blue]'\\b(?:[0-9]{1,3}\\.){3}[0-9]{1,3}\\b'[/Blue]
[Blue]'(authentication|permission|invalid user)'[/Blue]
[Blue]'(DoS|DDoS attack)'[/Blue]
[Blue]'brute-?force|directory traversal'[/Blue]
[Blue]'(SQL injection|cross-site scripting)'[/Blue]
[Blue]'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$'[/Blue]
[Blue]'(GET|POST|PUT|DELETE|PATCH) /[a-zA-Z0-9/_-]*'[/Blue]
[Blue]'cron.*\\((root|admin)\\)'[/Blue]

[I] IP (Log Search)
[Cyan]Filters logs by IP, ranges, and regular expressions:[/Cyan]
[Blue]'\\b(?:[0-9]{1,3}\\.){3}[0-9]{1,3}\\b'[/Blue]
[Blue]'(192\\.168\\.[0-9]+\\.[0-9]+)'[/Blue]
[Blue]'\\b(?:[A-Fa-f0-9]{1,4}:){7}[A-Fa-f0-9]{1,4}\\b'[/Blue]
[Blue]'(::|(?:[A-Fa-f0-9]{1,4}:){1,7}:)'[/Blue]
[Blue]'(?::[A-Fa-f0-9]{1,4}){1,7}'[/Blue]
[Blue]'192\\.168\\.\\d{1,3}\\.\\d{1,3}'[/Blue]

[S] Set (Log Paths)
[Cyan]Allows setting custom log paths:[/Cyan]
[Blue]'/var/log /opt/logs'[/Blue]
[Blue]'/var/lib/docker /var/log/nginx'[/Blue]
[Blue]'/usr/local/logs /home/user/logs'[/Blue]

[J] JSON (Export tool)
[Cyan]Exports filtered logs to a JSON file:[/Cyan]
[Magenta]'log_search_results.json'[/Magenta]
[Blue]'jq ".[] | .log_entry" log_search_results.json'[/Blue]

[H] Help
[Cyan]Displays this Help.[/Cyan]

[Q] Quit
