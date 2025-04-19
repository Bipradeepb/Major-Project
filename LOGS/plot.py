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

# --------- Sampling helper ---------
def sample_entries(entries, max_points=80):
    if len(entries) <= max_points:
        return entries
    step = len(entries) // max_points
    sampled = entries[::step]
    if sampled[-1] != entries[-1]:
        sampled.append(entries[-1])  # ensure we always include the last point
    return sampled

# Sample entries for smoother plots
progress_entries = sample_entries(progress_entries)

# Prepare x and y for plotting
x_seconds = [entry[0] for entry in progress_entries]
y_progress = [entry[1] for entry in progress_entries]

# --------- Determine time unit and scale ---------
total_duration = x_seconds[-1] if x_seconds else 0
if total_duration < 120:
    time_unit = 'seconds'
    x_scaled = x_seconds
    x_label = "Time (seconds since start)"
    step = 10 if total_duration > 60 else 5
elif total_duration < 7200:
    time_unit = 'minutes'
    x_scaled = [t / 60 for t in x_seconds]
    x_label = "Time (minutes since start)"
    step = 1 if total_duration < 1800 else 2
else:
    time_unit = 'hours'
    x_scaled = [t / 3600 for t in x_seconds]
    x_label = "Time (hours since start)"
    step = 0.5 if total_duration < 18000 else 1

xticks = np.arange(0, x_scaled[-1] + step, step)

# --------- PLOT 1: Progress vs Time ---------
plt.figure(figsize=(12, 6))
plt.plot(x_scaled, y_progress, marker='o', linestyle='-', color='blue')
plt.title("Data Transfer Progress vs Time")
plt.xlabel(x_label)
plt.ylabel("Data Transfer Progress (%)")
plt.grid(True)
plt.xticks(xticks, rotation=45)
plt.yticks(range(0, 101, 10))

# Annotate file info
plt.text(0.95, 0.05, f"{filename}, {filesize_str}",
         ha='right', va='bottom', transform=plt.gca().transAxes,
         fontsize=9, color='gray')

plt.tight_layout()
progress_plot_path = os.path.join(analysis_dir, f"transfer_progress_plot_{filename.replace('.', '_')}_{filesize_str}.png")
plt.savefig(progress_plot_path)
plt.close()

# --------- PLOT 2: Transfer Rate ---------
if filesize_kb > 0 and total_duration > 0:
    if time_unit == 'seconds':
        rate_y = [(filesize_kb * (p / 100)) / t if t != 0 else 0 for t, p in zip(x_seconds, y_progress)]
        rate_label = "Transfer Rate (KB/sec)"
    elif time_unit == 'minutes':
        rate_y = [(filesize_kb * (p / 100)) / (t / 60) if t != 0 else 0 for t, p in zip(x_seconds, y_progress)]
        rate_label = "Transfer Rate (KB/min)"
    else:
        rate_y = [(filesize_kb * (p / 100)) / (t / 3600) if t != 0 else 0 for t, p in zip(x_seconds, y_progress)]
        rate_label = "Transfer Rate (KB/hr)"

    plt.figure(figsize=(12, 6))
    plt.plot(x_scaled, rate_y, marker='o', linestyle='-', color='green')
    plt.title("Transfer Rate vs Time")
    plt.xlabel(x_label)
    plt.ylabel(rate_label)
    plt.grid(True)
    plt.xticks(xticks, rotation=45)

    # Annotate file info
    plt.text(0.95, 0.05, f"{filename}, {filesize_str}",
             ha='right', va='bottom', transform=plt.gca().transAxes,
             fontsize=9, color='gray')

    plt.tight_layout()
    rate_plot_path = os.path.join(analysis_dir, f"transfer_rate_plot_{filename.replace('.', '_')}_{filesize_str}.png")
    plt.savefig(rate_plot_path)
    plt.close()

print("✅ Plots saved in:", analysis_dir)

