# system-for-coCommunication-system-for-internal-networkmmunication
//Linux环境下
gcc client.c -o client -pthread
gcc server.c -o server -pthread

./client <服务器IP> <端口号>
./server <端口号>
