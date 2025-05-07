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

void *task(void *fd);
void realTask(int client_fd);
void format_http_header(char *client_rio, char *path, char *hostname, char *other_header);
void read_requesthdrs(rio_t *rp, char *host_header, char *other_header);
void get_filetype(char *filename, char *filetype);


int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  if(argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1)
  {
    int *connfd_ptr = Malloc(sizeof(int));
    *connfd_ptr = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    pthread_t pthr;
    pthread_create(&pthr, NULL, task, connfd_ptr);
  } 
}



void realTask(int client_fd)
{
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char hostHeader[MAXLINE], otherHeader[MAXLINE];
  char hostname[MAXLINE], path[MAXLINE], port[MAXLINE];
  char request_buf[MAXLINE], response_buf[MAXLINE];
  int server_fd, flag;
  ssize_t n;
  rio_t client_rio, server_rio;

  Rio_readinitb(&client_rio, client_fd);
  Rio_readlineb(&client_rio, buf, MAXLINE);
  printf("Request Headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  

  if(strcasecmp(method, "GET") != 0 && strcasecmp(method, "HEAD") != 0){ //strcasecmp는 두 함수가 동일하면 리턴 0, 다르면 1이다.
    return;
  }

 parseURI(uri, hostname, port, path);


    read_requesthdrs(&client_rio, hostHeader, otherHeader); // read HTTP request headers
    // HTTP 1.1->HTTP 1.0으로 변경 
    format_http_header(request_buf, path, hostname, otherHeader);

    server_fd = Open_clientfd(hostname, port);
    Rio_writen(server_fd, request_buf, strlen(request_buf));

   Rio_readinitb(&server_rio, server_fd);
    // 여기서 클라이언트에 최종 전달한다.
   while ((n = Rio_readnb(&server_rio, response_buf, MAXBUF)) > 0)
   {
     Rio_writen(client_fd, response_buf, n);
   }


  Close(server_fd);
  // Close(server_fd);
  return NULL;
}

void read_requesthdrs(rio_t *rp, char *host_header, char *other_header){
    char buf[MAXLINE];
  
    host_header[0]='\0';
    other_header[0]='\0';
  
    while(Rio_readlineb(rp, buf, MAXLINE) > 0 && strcmp(buf, "\r\n")){
      //buf에 남은 자리가 있고, buf가 "\r\n"으로 마지막에 도달하지 않았으면 계속 읽음
      if(!strncasecmp(buf, "Host:",5)){
        //Host만 따로 저장해주면 되서? 
        strcpy(host_header, buf);
      }
      else if(!strncasecmp(buf, "User-Agent:", 11)||!strncasecmp(buf, "Connection:", 11)||!strncasecmp(buf, "Proxy-Connection:", 17)){
        //얘들은 버림
        continue;
      }else{
        //그 외의 헤더는 다른곳에 저장해둠 
        strcat(other_header, buf);
      }
    }
  }

void format_http_header(char *client_rio, char *path, char *hostname, char *other_header){
    sprintf( client_rio,
    "GET %s HTTP/1.0\r\n"
    "Host: %s\r\n"
    "%s"
    "Connection: close\r\n"
    "Proxy-Connection: close\r\n"
    "%s"
    "\r\n",
    path, hostname, user_agent_hdr, other_header);
  }
  

void parseURI(char *uri, char *hostname, char *port, char *path)
{
    char *hostBegin, *hostEnd, *portBegin, *pathBegin;
    int hostLen, portLen, pathLen;

    hostBegin = strstr(uri, "//");
    if (hostBegin != NULL)
        hostBegin += 2;
    else
        hostBegin = (char *)uri;
    // hostbegin, 즉 본격적인 위치를 확인해야함. //의 시작 위치니 //를 스킵한 위치로 던져주는 것
    
    pathBegin = strchr(hostBegin, '/');
    if(pathBegin != NULL)
    {
        hostEnd = pathBegin;
        pathLen = strlen(pathBegin);
        strncpy(path, pathBegin, pathLen);
        path[pathLen] = '\0';
    }
    else
    {
        hostEnd = hostBegin + strlen(hostBegin);
        path[0] = '\0';
    }
    // http://www.naver.com/ << 즉, hostbegin은 //를 이미 넘어간 상태에서 다음 /를 찾는 것이다.

    portBegin = strchr(hostBegin, ':');
    if (portBegin != NULL && portBegin < hostEnd)
    {
        hostLen = portBegin - hostBegin;
        strncpy(hostname, hostBegin, hostLen);
        hostname[hostLen] = '\0';

        portBegin++;
        portLen = hostEnd - portBegin;
        strncpy(port, portBegin, portLen);
        port[portLen] = '\0';
    }
    else
    {
        hostLen = hostEnd - hostBegin;
        strncpy(hostname, hostBegin, hostLen);
        hostname[hostLen] = '\0';
        port[0] = '\0';
    }
    // 포트 번호 탐색.
}

void *task(void *connfd)
{
    int fd = *((int *)connfd);
    free(connfd);
    Pthread_detach(pthread_self());
    realTask(fd);
    Close(fd);
}
// 1차 구현 완료
// 2차 목표, 멀티쓰레딩 프로젝트
  // 뭐가 어떻게 흘러가느냐?
  // 클라이언트의 연결 발생에 대해 디스크립터를 쥐어주고 새로운 쓰레드를 만든다.
    // 해당 쓰레드는 할 일을 하는데, 할 일을 할때 써야할 rio는 공통된 rio를 사용하게 끔한다.
