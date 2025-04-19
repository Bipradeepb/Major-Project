import os
import re
from datetime import datetime
import matplotlib.pyplot as plt
import numpy as np

log_file_path = "logs.txt"
analysis_dir = "ANALYSIS"

# Ensure output directory exists
os.makedirs(analysis_dir, exist_ok=True)

# Regex patterns
timestamp_pattern = r"\[(.*?)\]"
progress_pattern = r"File transfer progress:(\d+\.\d+)"
start_pattern = r"Client IP set to:"
final_ack_pattern = r"Final Ack has been received"
filename_pattern = r"Filename\s*=\s*(\S+)"
filesize_pattern1 = r"Filesize\s*=\s*([\d\.]+)([kK][bB])"
filesize_pattern2 = r"Transfer of\s+([\d\.]+)\s*([kK][bB])\s+complete"

# Variables to track
start_time = None
end_time = None
progress_entries = []
filename = "unknown_file"
filesize_kb = None

# Read and parse the log
with open(log_file_path, 'r') as file:
    for line in file:
        timestamp_match = re.search(timestamp_pattern, line)
        if not timestamp_match:
            continue
        log_time = datetime.strptime(timestamp_match.group(1), "%Y-%m-%d %H:%M:%S")

        if start_pattern in line:
            start_time = log_time

        if start_time:
            seconds_since_start = (log_time - start_time).total_seconds()

            progress_match = re.search(progress_pattern, line)
            if progress_match:
                progress = float(progress_match.group(1))
                progress_entries.append((seconds_since_start, progress))

            if final_ack_pattern in line:
                end_time = log_time
                progress_entries.append(((end_time - start_time).total_seconds(), 100.0))

            filename_match = re.search(filename_pattern, line)
            if filename_match:
                filename = filename_match.group(1)

            size_match1 = re.search(filesize_pattern1, line)
            size_match2 = re.search(filesize_pattern2, line)
            if size_match1:
                filesize_kb = float(size_match1.group(1))
            elif size_match2:
                filesize_kb = float(size_match2.group(1))

# Fallback if filesize is unknown
filesize_kb = filesize_kb or 0.0
filesize_str = f"{filesize_kb:.2f}KB"

# Prepare x and y for plotting
x_seconds = [entry[0] for entry in progress_entries]
y_progress = [entry[1] for entry in progress_entries]

# --------- PLOT 1: Progress vs Time ---------
plt.figure(figsize=(12, 6))
plt.plot(x_seconds, y_progress, marker='o', linestyle='-', color='blue')
plt.title("Data Transfer Progress vs Time")
plt.xlabel("Time (seconds since start)")
plt.ylabel("Data Transfer Progress (%)")
plt.grid(True)

# Dynamically scale x-axis ticks
total_duration = x_seconds[-1] if x_seconds else 0
if total_duration <= 10:
    step = 1
elif total_duration <= 30:
    step = 2
elif total_duration <= 60:
    step = 5
elif total_duration <= 120:
    step = 10
elif total_duration <= 300:
    step = 30
else:
    step = 60

xticks = np.arange(0, total_duration + 1, step)
plt.xticks(xticks)
plt.yticks(range(0, 101, 10))

# Annotate file info
plt.text(0.95, 0.05, f"{filename}, {filesize_str}",
         ha='right', va='bottom', transform=plt.gca().transAxes,
         fontsize=9, color='gray')

plt.tight_layout()
progress_plot_path = os.path.join(analysis_dir, f"transfer_progress_plot_{filename.replace('.', '_')}_{filesize_str}.png")
plt.savefig(progress_plot_path)
plt.close()

# --------- PLOT 2: Transfer Rate (KB/s) ---------
if filesize_kb > 0 and total_duration > 0:
    rate_y = [(filesize_kb * (p / 100)) / t if t != 0 else 0 for t, p in zip(x_seconds, y_progress)]
    plt.figure(figsize=(12, 6))
    plt.plot(x_seconds, rate_y, marker='o', linestyle='-', color='green')
    plt.title("Transfer Rate vs Time")
    plt.xlabel("Time (seconds since start)")
    plt.ylabel("Transfer Rate (KB/sec)")
    plt.grid(True)

    plt.xticks(xticks)

    # Annotate file info
    plt.text(0.95, 0.05, f"{filename}, {filesize_str}",
             ha='right', va='bottom', transform=plt.gca().transAxes,
             fontsize=9, color='gray')

    plt.tight_layout()
    rate_plot_path = os.path.join(analysis_dir, f"transfer_rate_plot_{filename.replace('.', '_')}_{filesize_str}.png")
    plt.savefig(rate_plot_path)
    plt.close()

print("✅ Plots saved in:", analysis_dir)

