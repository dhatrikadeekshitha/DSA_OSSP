
#!/bin/bash

echo "All Running Processes:"
ps -ef

echo
echo "Top 10 Processes by CPU Usage:"
ps -eo pid,comm,%cpu,%mem --sort=-%cpu | head -n 11

echo
echo "Live Process Monitoring:"
top
