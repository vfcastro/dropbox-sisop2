CWD=`pwd`
FILE=kubespray-2.10.3.zip

cd $CWD/test/server
nohup ./server 5000 &
sleep 1

cd $CWD/test/client-session1
nohup ./client user1 localhost 5000 &

cd $CWD/test/client-session2
nohup ./client user1 localhost 5000 &

sleep 3
cd $CWD
cp ./test/$FILE $CWD/test/client-session1/sync_dir_user1/

for i in `seq 90`
do
    if [ ! -f $CWD/test/client-session2/sync_dir_user1/$FILE ]
    then
        sleep 1
    fi
done

md5sum $CWD/test/$FILE
md5sum $CWD/test/client-session1/sync_dir_user1/$FILE
md5sum $CWD/test/client-session2/sync_dir_user1/$FILE

killall server client
exit