CWD=`pwd`
BIN='/home/jcdazeredo/codes/sisop/dropbox-sisop2/bin'
FILE1='kubespray-2.10.3.zip'
FILE2='inf01043trabalhofinal.zip'

# Criando pastas testes
mkdir $CWD/test/server1
mkdir $CWD/test/server2
mkdir $CWD/test/server3
mkdir $CWD/test/client-session1/
mkdir $CWD/test/client-session2/

# Remove files from test
rm $CWD/test/server1/$FILE1
rm $CWD/test/server2/$FILE1
rm $CWD/test/server3/$FILE1
rm $CWD/test/client-session1/$FILE1
rm $CWD/test/client-session2/$FILE1
rm $CWD/test/server1/$FILE2
rm $CWD/test/server2/$FILE2
rm $CWD/test/server3/$FILE2
rm $CWD/test/client-session1/$FILE2
rm $CWD/test/client-session2/$FILE2

clear
echo 'Arquivos testes removidos'

echo ''

echo 'Copiando binarios para pastas testes'

# Copy bin to test
cp $BIN/server $CWD/test/server1/server1
cp $BIN/server $CWD/test/server2/server2
cp $BIN/server $CWD/test/server3/server3
cp $BIN/client $CWD/test/client-session1
cp $BIN/client $CWD/test/client-session2

echo ''

# Rodando Server1 Primario
echo 'Rodando Server1 Primario'
cd $CWD/test/server1
nohup ./server1 5010 1 localhost 5009 localhost 5008 &
sleep 2

# Rodando Server2 Backup
echo 'Rodando Server2 Backup'
cd $CWD/test/server2
nohup ./server2 5009 0 localhost 5010 localhost 5008 &
sleep 2

# Rodando Server3 Backup
echo 'Rodando Server3 Backup'
cd $CWD/test/server3
nohup ./server3 5008 0 localhost 5010 localhost 5009 &
sleep 2

# Rodando Client-session1
echo 'Rodando Client-session1'
cd $CWD/test/client-session1
nohup ./client user1 localhost 5010 6010 &
sleep 2

# Rodando Client-session2
echo 'Rodando Client-session2'
cd $CWD/test/client-session2
nohup ./client user1 localhost 5010 6009 &
sleep 2

echo ''
echo 'Copiando '$FILE1' para client-session 1'

cd $CWD
cp ./test/$FILE1 $CWD/test/client-session1/sync_dir_user1/

for i in `seq 30`
do
    if [ ! -f $CWD/test/client-session2/sync_dir_user1/$FILE1 ]
    then
        sleep 1
    fi
done

echo 'Removendo '$FILE1' de client-session 2'
cd $CWD/test/client-session2/sync_dir_user1/
rm $FILE1
sleep 3

echo ''
echo 'Matando Server1'

killall server1
sleep 5

echo ''
echo 'Copiando '$FILE2' para client-session 2'

cd $CWD
cp ./test/$FILE2 $CWD/test/client-session2/sync_dir_user1/

for i in `seq 30`
do
    if [ ! -f $CWD/test/client-session1/sync_dir_user1/$FILE2 ]
    then
        sleep 1
    fi
done

echo ''

echo 'Removendo '$FILE2' de client-session 1'
cd $CWD/test/client-session1/sync_dir_user1/
rm $FILE2
sleep 3

cd $CWD

test -f $CWD/test/client-session1/sync_dir_user1/$FILE1 || echo '$CWD/test/client-session1/sync_dir_user1/$FILE1 foi excluido'
test -f $CWD/test/server1/sync_dir_user1/$FILE1 || echo '$CWD/test/server1/sync_dir_user1/$FILE2 foi excluido'
test -f $CWD/test/server2/sync_dir_user1/$FILE1 || echo '$CWD/test/server2/sync_dir_user1/$FILE2 foi excluido'
test -f $CWD/test/server3/sync_dir_user1/$FILE1 || echo '$CWD/test/server3/sync_dir_user1/$FILE2 foi excluido'
test -f $CWD/test/client-session1/sync_dir_user1/$FILE2 || echo '$CWD/test/client-session1/sync_dir_user1/$FILE2 foi excluido'
test -f $CWD/test/client-session2/sync_dir_user1/$FILE2 || echo '$CWD/test/client-session2/sync_dir_user1/$FILE2 foi excluido'
test -f $CWD/test/server1/sync_dir_user1/$FILE2 || echo '$CWD/test/server1/sync_dir_user1/$FILE2 foi excluido'
test -f $CWD/test/server2/sync_dir_user1/$FILE2 || echo '$CWD/test/server2/sync_dir_user1/$FILE2 foi excluido'
test -f $CWD/test/server3/sync_dir_user1/$FILE2 || echo '$CWD/test/server3/sync_dir_user1/$FILE2 foi excluido'

test -f $CWD/test/client-session1/sync_dir_user1/$FILE1 && echo '$CWD/test/client-session1/sync_dir_user1/$FILE1 existe'
test -f $CWD/test/server1/sync_dir_user1/$FILE1 && echo '$CWD/test/server1/sync_dir_user1/$FILE2 existe'
test -f $CWD/test/server2/sync_dir_user1/$FILE1 && echo '$CWD/test/server2/sync_dir_user1/$FILE2 existe'
test -f $CWD/test/server3/sync_dir_user1/$FILE1 && echo '$CWD/test/server3/sync_dir_user1/$FILE2 existe'
test -f $CWD/test/client-session1/sync_dir_user1/$FILE2 && echo '$CWD/test/client-session1/sync_dir_user1/$FILE2 existe'
test -f $CWD/test/client-session2/sync_dir_user1/$FILE2 && echo '$CWD/test/client-session2/sync_dir_user1/$FILE2 existe'
test -f $CWD/test/server1/sync_dir_user1/$FILE2 && echo '$CWD/test/server1/sync_dir_user1/$FILE2 existe'
test -f $CWD/test/server2/sync_dir_user1/$FILE2 && echo '$CWD/test/server2/sync_dir_user1/$FILE2 existe'
test -f $CWD/test/server3/sync_dir_user1/$FILE2 && echo '$CWD/test/server3/sync_dir_user1/$FILE2 existe'

killall server2 server3 client
exit