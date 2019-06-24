CWD=`pwd`
FILE1=kubespray-2.10.3.zip
FILE2=inf01043trabalhofinal.zip

cd $CWD/test/server1
nohup ./server1 5000 1 localhost 4999 localhost 4998 &
sleep 2

cd $CWD/test/server2
nohup ./server2 4999 0 localhost 5000 localhost 4998 &
sleep 2

cd $CWD/test/server3
nohup ./server3 4998 0 localhost 5000 localhost 4999 &
sleep 2

cd $CWD/test/client-session1
nohup ./client user1 localhost 5000 6000 &
sleep 2

cd $CWD/test/client-session2
nohup ./client user1 localhost 5000 6001 &
sleep 2

cd $CWD
cp ./test/$FILE1 $CWD/test/client-session1/sync_dir_user1/

for i in `seq 30`
do
    if [ ! -f $CWD/test/client-session2/sync_dir_user1/$FILE1 ]
    then
        sleep 1
    fi
done

killall server1
sleep 5

cd $CWD
cp ./test/$FILE2 $CWD/test/client-session2/sync_dir_user1/

for i in `seq 30`
do
    if [ ! -f $CWD/test/client-session1/sync_dir_user1/$FILE2 ]
    then
        sleep 1
    fi
done

md5sum $CWD/test/$FILE1
md5sum $CWD/test/client-session1/sync_dir_user1/$FILE1
md5sum $CWD/test/client-session2/sync_dir_user1/$FILE1
md5sum $CWD/test/server1/sync_dir_user1/$FILE1
md5sum $CWD/test/server2/sync_dir_user1/$FILE1
md5sum $CWD/test/server3/sync_dir_user1/$FILE1
md5sum $CWD/test/$FILE2
md5sum $CWD/test/client-session1/sync_dir_user1/$FILE2
md5sum $CWD/test/client-session2/sync_dir_user1/$FILE2
md5sum $CWD/test/server2/sync_dir_user1/$FILE2
md5sum $CWD/test/server3/sync_dir_user1/$FILE2

killall server2 server3 client
exit