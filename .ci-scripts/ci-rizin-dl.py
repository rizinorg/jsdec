import urllib.request
import json
import sys
import os

out_file = "rizin.zip"

latest = "master" if len(sys.argv) < 1 else sys.argv[1]

if latest != "dev":
    # master branch always build against latest release of rizin
    tags = None
    with urllib.request.urlopen('https://api.github.com/repos/rizinorg/rizin/tags?per_page=1') as f:
        tags = json.load(f)
    latest = tags[0]['name']
    url = f"https://github.com/rizinorg/rizin/archive/refs/tags/{latest}.zip"
else:
    # dev branch always build against latest commit of rizin
    url = "https://github.com/rizinorg/rizin/archive/refs/heads/dev.zip"

print(f"Using rizin branch: {latest}")
print(f"{url} as {out_file}")

urllib.request.urlretrieve(url, out_file)
