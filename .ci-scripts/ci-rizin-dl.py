import urllib.request
import json
import sys
import os

file_name = sys.argv[1]
latest = "master" if len(sys.argv) < 2 else sys.argv[2]

_, file_extension = os.path.splitext(file_name)
out_file = f"rizin{file_extension}"
print(file_name, out_file)

if latest != "dev":
    # master branch always build against latest release of rizin
    tags = None
    with urllib.request.urlopen('https://api.github.com/repos/rizinorg/rizin/tags?per_page=1') as f:
        tags = json.load(f)
    latest = tags[0]['name']
    url = f"https://github.com/rizinorg/rizin/releases/download/{latest}/{file_name}"
    url = url.format(version=latest)
else:
    # dev branch always build against latest commit of rizin
    url = "https://github.com/rizinorg/rizin/archive/refs/heads/dev.zip"

print(f"Using rizin branch: {latest}")
print(f"{url} as {out_file}")

urllib.request.urlretrieve(url, out_file)
