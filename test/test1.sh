CWD=`pwd`
FILE=kubespray-2.10.3.zip

cd $CWD/test/server1
nohup ./server 5000 1 localhost 4999 localhost 4998 &
sleep 2

cd $CWD/test/server2
nohup ./server 4999 0 localhost 4998 &
sleep 2

cd $CWD/test/server3
nohup ./server 4998 0 localhost 4999 &
sleep 2

cd $CWD/test/client-session1
nohup ./client user1 localhost 5000 &
sleep 2

cd $CWD/test/client-session2
nohup ./client user1 localhost 5000 &
sleep 2

cd $CWD
cp ./test/$FILE $CWD/test/client-session1/sync_dir_user1/

for i in `seq 30`
do
    if [ ! -f $CWD/test/client-session2/sync_dir_user1/$FILE ]
    then
        sleep 1
    fi
done

md5sum $CWD/test/$FILE
md5sum $CWD/test/client-session1/sync_dir_user1/$FILE
md5sum $CWD/test/client-session2/sync_dir_user1/$FILE
md5sum $CWD/test/server1/sync_dir_user1/$FILE
md5sum $CWD/test/server2/sync_dir_user1/$FILE
md5sum $CWD/test/server3/sync_dir_user1/$FILE

killall server client
exit