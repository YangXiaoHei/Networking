
#define DATALEN (3 << 20)
char data[DATALEN];

int main(int argc, char *argv[])
{
    if (argc != 6) {
        logx("usage : %s <hostname/ip> <service/port> <nchild> <nloop> <nbytes>", argv[0]);
        exit(1);
    }  

    int nchild = atoi(argv[3]);
    int nbytes = atoi(argv[5]);
    int nloop = atoi(argv[4]);

    pid_t pid;
    for (int i = 0; i < nchild; i++) {
        if ((pid = fork()) < 0) {
            logx("fork error!");
            exit(1);
        } else if (pid == 0) {
            int fd = -1;
            if ((fd = tcp_connect(argv[1], argv[2])) < 0) {
                logx("tcp_connect error!");
                exit(1);
            }
            for (int j = 0; j < nloop; j++) {
                
            }
        }

    }


}

