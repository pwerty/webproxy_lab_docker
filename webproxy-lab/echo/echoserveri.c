#include "csapp.h"

void echo(int connfd);
int Open_listenfd(char *port);

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        printf("server received %d byted\n", (int)n);
        Rio_writen(connfd, buf, n);
    }
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2)
    {
        // 실행 시도 시 갯수가 안맞으면 안내 후 종료.
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // 포트번호를 매개변수로 Open_listenfd를 호출한다.
    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        echo(connfd);
        Close(connfd);
    }
}

int Open_listenfd(char *port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    // memset 함수로 hints 구조체를 완전히 초기화한다.
    memset(&hints, 0, sizeof(struct addrinfo));

    // hints에 대한 상세 내용을 정의한다.
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;

    Getaddrinfo(NULL, port, &hints, &listp);
    // hints를 필터로, port가 반영 된 내용을 listp에 가져온다.



    
    // listp 내의 모든 아이템에 대해 시도한다.
    for (p = listp; p; p->ai_next)
    {
        // 소켓 디스크립터를 생성합니다.
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; // 음수가 나오면 소켓 생성 실패로, 다음 주소에 대해 진행한다.

        // Setsockopt를 사용해 SO_REUSEADDR을 설정함으로써, 서버 프로그램을 재실행할 때 이전 연결에서 사용된 주소와 포트를 바로 재사용할 수 있도록 합니다.
        // 만약 이전 연결이 TIME_WAIT 상태에 있어도 주소 재바인딩이 가능하게 됩니다.
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

        // 디스크립터가 생성
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; // 성공
        Close(listenfd); // 바인드 실패, 다음꺼 시도
    }

    // 정리!!
    Freeaddrinfo(listp);
    if (!p) // 작동 되는게 없었다면
        return -1;
    
    // Make it a listening socket ready to accept connection requests
    if (listen(listenfd, LISTENQ) < 0)
    {
        Close(listenfd);
        return -1;
    }
    return listenfd;
}