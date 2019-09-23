gcc chatclient.c my_recv.c -lpthread -o client;
gcc chatserver.c -lmysqlclient -o server;

