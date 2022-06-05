#!/bin/bash

LOCALDIR=./data
REMOTEDIR=${1:-./datadump}

while inotifywait -r -e modify,create,delete,move $LOCALDIR
do
    rsync -avPRr $LOCALDIR sst@stu:~/tardigrade/SMRT/remotedata/${REMOTEDIR}
done
