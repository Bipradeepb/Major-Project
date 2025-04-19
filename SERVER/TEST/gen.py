# Python script to generate a 100KB file
file_name = "100KB_file.txt"

# 100KB = 102400 bytes. Generate dummy content to reach 100KB.
content = "A" * 102400

# Write to file
with open(file_name, "w") as f:
    f.write(content)

print(f"File '{file_name}' of 100KB created successfully.")

