# download faces in PubFig
# put https://www.cs.columbia.edu/CAVE/databases/pubfig/download/dev_urls.txt in the same folder first

import re,requests,os
from pathlib import Path

skipped_lines=0

if not Path("samples").is_dir():
    os.mkdir("samples")

file = open("dev_urls.txt")
for _ in range(skipped_lines):
    file.readline()
proxy={"nothing":"there"}
handledList = []
failedTimes=0
downloadedFiles=0
for item in file.readlines():
    info = item.split("\t")
    if (info[0] not in handledList):
        if not Path(info[0]).is_dir():
            os.mkdir(info[0])

    if(Path(info[0]+"/"+info[1]+".jpg").exists()):
        continue

    try:
        data=requests.get(info[2],proxies=proxy,timeout=15)
    except :
        print("failed to download "+info[0]+"'s "+info[1]+" image!")
        failedTimes+=1
        continue
    downloadedFiles+=1
    if info[0] not in handledList:
        handledList.append(info[0])
        if not Path("samples/"+info[0]+".jpg").is_file():
            with open("samples/"+info[0]+".jpg","wb") as file:
                file.write(data.content)
                file.close()
    with open (info[0]+"/"+info[1]+".jpg","wb") as file:
        file.write(data.content)
        file.close()
print("peoples:",len(handledList))
print("images:",downloadedFiles)
print("failed:",failedTimes)
