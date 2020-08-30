# delete the invalid jpg file by header

import os

counter_all = 0
counter_deleted = 0
for dir, _, fileList in os.walk(os.path.curdir):
    for file in fileList:
        path = os.path.join(dir, file)
        print(path)
        if not file.endswith(".jpg"):
            continue
        counter_all += 1
        with open(path, "rb") as openedFile:
            if openedFile.read(2) != b'\xff\xd8':
                os.remove(path)
                counter_deleted += 1

print(counter_all)
print(counter_deleted)