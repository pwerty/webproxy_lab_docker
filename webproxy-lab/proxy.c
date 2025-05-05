#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
// Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/135.0.0.0 Safari/537.36

void task(int fd, int toServer_fd);
void serve_page(int fd, char *contents);
void valueSelection(char *uri, char* link, char *cgiargs);
void read_requesthdrs(rio_t *rp);
void get_filetype(char *filename, char *filetype);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);


int main(int argc, char **argv)
{
  int listenfd, connfd, clientfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  rio_t rio, _rio;

  if(argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  // 프록시에게서 들어오는 연결 리스닝 시도.
  listenfd = Open_listenfd(argv[1]);
  // Tiny에 연결 시도.
 // clientfd = Open_clientfd("localhost", "12321");
  // 프록시와 Tiny 간의 버퍼 초기화.
  Rio_readinitb(&_rio, clientfd);

  while (1)
  {
    clientlen = sizeof(clientaddr);
    // 리스닝에서 연결 허락 되어 최종 연결된 클라이언트 발생.
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    // 할 일 실행
    task(connfd, clientfd);
    Close(connfd);
  }

  Close(clientfd);
  exit(0);
}

void task(int fd, int toServer_fd)
{
  // fd를 위한 출력 값을 만들어야한다.
  // 즉 여기서 tiny에 값을 보내서, 다시 받아오고, 그걸 다시 뿌려주게 된다.
  struct stat sbuf;
  char buf[MAXLINE], _buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char newHeader[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;


  Rio_readinitb(&rio, fd);

  // 프록시랑 연결된 클라이언트로부터 요청 내용을 긁어온다.
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request Headers?!\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  sprintf(newHeader, "%s %s HTTP/1.0\r\n", method, uri);
  sprintf(newHeader, "%sHost: localhost\r\n", newHeader);
  sprintf(newHeader, "%sServer: Tiny Web Server\r\n", newHeader);
  sprintf(newHeader, "%sUser-Agent: %s\r\n",newHeader, user_agent_hdr);
  sprintf(newHeader, "%sConnection: close\r\n", newHeader);
  sprintf(newHeader, "%sProxy-Connection: close\r\n\r\n", newHeader);


  if(strcasecmp(method, "GET"))
  { // method의 내용이 GET이 아닌 경우 이 코드를 실행
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method!");
    return;
  }
  read_requesthdrs(&rio);
  valueSelection(uri, filename, cgiargs);

  if (stat(filename, &sbuf) < 0)
  { // 파일이 존재하지 않는 경우 중단
    clienterror(fd, filename, "404", "Not Found", "Tiny couldn't find this file.");
    return;
  } 

  Rio_writen(toServer_fd, *newHeader, MAXLINE);
  Rio_readn(toServer_fd, *newHeader, MAXLINE);
  //serve_page(fd, *_buf);
}

void serve_page(int fd, char *contents)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF], *mallocCh;

    printf("Response headers:\n");
    printf("%s", buf);
  
    // filename을 읽기 모드로 열어서 파일 디스크립터를 확인한다.
    //srcfd = Open(filename, O_RDONLY, 0);
    // 요청된 파일을 가상 메모리 영역에 매핑한다.
    // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
   //srcp = Malloc(filesize);
    //Rio_readn(srcfd, srcp, filesize);
    
  
    // 더 이상 파일 디스크립터를 쓸 필요가 없으니 Close한다.
    // 이를 안 닫으면 메모리 누수가 난다.
    Close(srcfd);
    
    // srcp에서 시작하는 filesize 바이트가 클라이언트의 연결 디스크립터로 복사되도록 한다.
   // Rio_writen(fd, srcp, filesize);
  
    // 매핑된 가상 메모리 영역을 해제한다.
    Free(srcp);
    //Munmap(srcp, filesize);

  
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


void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  while(strcmp(buf, "\r\n"))
  {
    Rio_readlineb(rp, buf, MAXLINE);
    //printf("%s", buf);
  }
  return;
}


void get_filetype(char *filename, char *filetype)
{

  if(strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else if (strstr(filename, ".mpg"))
    strcpy(filetype, "video/mpeg");
  else
    strcpy(filetype, "text/plain");
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