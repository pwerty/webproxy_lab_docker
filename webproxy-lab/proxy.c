#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void task(int fd);
void serve_page(int fd);
void valueSelection(char *uri, char* link, char *cgiargs);

int main(int argc, char **argv)
{
  int listenfd, connfd, clientfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  rio_t rio;

  if(argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);

  // 이건 tiny에 연결해야해서.. tiny가 12321로 고정되어야한다.
  clientfd = Open_clientfd("localhost", "12321");
  Rio_readinitb(&rio, clientfd);

  while (1)
  {
    clientlen == sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    task(connfd);
    Close(connfd);
  }


  return 0;
}

void task(int fd)
{
  // fd를 위한 출력 값을 만들어야한다.
  // 즉 여기서 tiny에 값을 보내서, 다시 받아오고, 그걸 다시 뿌려주게 된다.
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiagrgs[MAXLINE];
  rio_t rio;


  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request Headers\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  if(strcasecmp(method, "GET"))
  {   // method의 내용이 GET이 아닌 경우 이 코드를 실행합니다 :
      // 501, GET이 아닌 요청에 대해서 준비되지 않았습니다!
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method!");
    return;
  }
}

void serve_page(int fd)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF], *mallocCh;

  
}

void valueSelection(char *uri, char* link, char *cgiargs)
{
  char *ptr;

  // uri 내용에 cgi-bin 문자열이 없다면
  if (!strstr(uri, "cgi-bin")) // 정적 컨텐츠쪽의 분기를 실행합니다.
  {
    strcpy(cgiargs, "");
    strcpy(link, ".");
    strcat(link, uri);

    if(uri[strlen(uri) - 1] == '/')
      strcat(link, "home.html");
      return 1;
  }
  /*
  else // cgi-bin 문자열을 확인한 분기점입니다. 동적 컨텐츠쪽의 분기를 실행합니다.
  {
    printf("detected!!\n");
    ptr = index(uri, '?');
    if (ptr)
    {
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0';
    }
    else
      strcpy(cgiargs, "");
    strcpy(link, ".");
    strcat(filename, uri);
    return 0;
  }
    
}
  */
}

// 헤더값 필수값을 수동으로 삽입하라고?
// 이건 tiny에서 가져오면 되지 않나

// proxy 입장에서 요청해서 받아오는 걸 할 수 있어야함
// 아 그럼 내가 서버이자 클라가 되어야하는거구나

// 그럼 내가 서버의 입장으로써 받아온 요청값에 따라 그 값에 기반해서 클라이언트가 되어서 요청하는 사람이 되어야하는구나

/*

그렇다면 전체적 루틴이

내가 연결을 받는다
  연결 받은 곳에서 원하는 내용을 받아본다
    해당 내용을 인자로써 서버에 요청한다 : 물론 그 전에 가고자하는 서버랑 연결되어야겠지
    서버가 보내는걸 받는다
    그 내용을 그대로 클라에게 던진다

  이게 기초적인 흐름이군요
*/


// tiny 서버를 열어서 tiny와 proxy를 이어주고, 클라이언트 입장에서는 proxy:proxyport로 연결을 유도해야함
// 클라이언트의 요청을 tiny에게 요청하는 내용으로 echo-client 내용을 기반으로 구현하기
// 그리고 proxy의 serve_ 에서 tiny 내용에 있는 걸로 뱉어주는 형태로 완성