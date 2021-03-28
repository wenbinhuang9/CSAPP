#include <stdio.h>
#include "csapp.h"
#include "assert.h"


//todo review the code and summary about the code writing , writing the blog 
//after doing this, start testing the code. 
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

typedef struct Request {
    char uri[MAXLINE];
    char method[128];
    char hostname[256];
    char path[256]; 
    unsigned int port; 
} Request;

typedef struct Header{
    char raw[256];
} Header; 

// todo review code. 
struct CacheObj{
    char* uri;
    char* object;
    struct CacheObj* prev;
    struct CacheObj* next;
};

struct Cache{
    int size;
    int capacity;
    struct CacheObj* head; 
};

sem_t mutex;
struct  Cache* cache;

struct CacheObj* cache_read (char* uri); // thread safe
void cache_write(char* uri, char* object); // thread safe 
void init_cache();
void insert_to_head(struct CacheObj* obj);// thead safe
void cache_remove(struct CacheObj* obj); // thread safe
struct CacheObj* find(char* url);

struct CacheObj* findlast() {
    //the last element is the evicted element 
    struct CacheObj* cur = cache->head->next;
    if (cur == NULL) {
        return NULL;
    }

    while(cur->next !=NULL) {
        cur = cur->next;
    }

    return cur; 
}

void cache_remove(struct CacheObj* obj) {
    struct CacheObj* prev = obj->prev;
    struct CacheObj* next = obj->next;

    prev->next = next;

    if(next != NULL) {
        next->prev = prev;
    }
    obj->prev = NULL;
    obj->next = NULL;
}

void insert_to_head(struct  CacheObj* obj)
{
    struct CacheObj* head = cache->head;

    struct CacheObj* head_next = head->next;

    head->next = obj;
    obj->next = head_next;

    obj->prev = head;

    if(head_next != NULL) {
        head_next->prev = obj;
    }
}

struct CacheObj* find(char* url) {
    struct CacheObj* head = cache->head;
    struct CacheObj* cur;

    for(cur = head->next; cur != NULL; cur = cur ->next) {
        if (strcmp(url, cur->uri) == 0) {
            return cur;
        }
    }

    return NULL;
}

struct CacheObj* cache_read(char* url) {
    struct CacheObj* obj = find(url);
    if (obj == NULL) {
        return NULL;
    }

    P(&mutex);
    cache_remove(obj);
    insert_to_head(obj);
    V(&mutex);
    
    return obj;
}

void cache_write(char* uri, char* object) {
    struct CacheObj* obj = (struct CacheObj*)malloc(sizeof(struct CacheObj));

    obj->uri = (char*) Malloc(sizeof(char) * 128);
    obj->object =(char* ) Malloc(sizeof(char) * MAX_OBJECT_SIZE);
    strcpy(obj->uri, uri);
    strcpy(obj->object, object);

    obj->prev = NULL;
    obj -> next = NULL;

    P(&mutex); 
    if(cache->size >= cache->capacity) {
        //find last 
        struct CacheObj* evicted_obj = findlast();

        cache_remove(evicted_obj);
        Free(evicted_obj->uri);
        Free(evicted_obj-> object);
        Free(evicted_obj);

        cache->size -= 1;
    }

    insert_to_head(obj);
    cache -> size += 1;
    V(&mutex);
}

void init_cache() {
    cache = (struct Cache*)Malloc(sizeof(struct Cache));
    cache->size = 0;
    cache->capacity = 10;

    cache->head = (struct CacheObj*) Malloc(sizeof(struct CacheObj));

    cache->head->next = NULL;
    cache->head->prev = NULL;

    Sem_init(&mutex, 0, 1);
}
void response_success(int fd) {
    char buf[MAXBUF];
    sprintf(buf, "HTTP/1.0 200 OK\r\n");

    rio_writen(fd, buf, strlen(buf));
}

// return client_id
int send_servers(Request* request, Header* headers, int headernum) {
    printf("start send to servers, hostname is %s, port is %d\n", request->hostname, request->port);
    rio_t rio;

    int port = request -> port;
    char port_str[20];

    sprintf(port_str, "%d", port);

    // get socket 
    int client_fd = Open_clientfd(request->hostname, port_str);
    char buf[MAXLINE];
    char* buf_head;
    sprintf(buf, "%s %s %s\r\n", "GET", request->path, "HTTP/1.0");
    
    // send the request 
    Rio_writen(client_fd, buf, MAXLINE);
    printf("send request to server\n");
    // send headers 
    buf_head = buf + strlen(buf);
    for (int i = 0; i < headernum; i ++) {
        sprintf(buf_head, "%s", headers[i].raw);
        buf_head = buf + strlen(buf);
    }
    sprintf(buf_head, "\r\n");
    printf("%s", buf);
    // http headers must send once. 
    Rio_writen(client_fd, buf, MAXLINE);

    printf("end of sending to server\n");
    return client_fd;
}



int parse_uri(char* uri, Request * req) {
    char* p; 

    if(strncasecmp(uri, "http://", 7) != 0 ) {
        fprintf(stderr, "invalid url\n");
        exit(0);
    }
    
    strcpy(req->hostname, uri + 7);
    p = strpbrk(req->hostname, ":/\r\n\0");
    printf("*p is %c\n", *p);
    assert(p != 0);
    //parse port
    req->port = 80;
    if(*p == ':') {
        req->port = atoi(p + 1);
    }
    // truncate  to get the hostname here 
    *p = '\0';

    printf("hostname is %s\n", req->hostname);

    // parse path name
    p = strchr(uri + 7, '/');
    if (p == NULL) {
        req->path[0] = '\0';
    }else{
        strcpy(req->path, p);
    }

    return 0;
}


void parse_request( int fd, Request* req, Header* headers, int* headernum ) {
    rio_t rio;
    printf("start parse request\n");

    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE],version[MAXLINE];
    rio_readinitb(&rio, fd);
    if(!Rio_readlineb(&rio, buf, MAXLINE)) {
        return;
    }

    sscanf(buf, "%s %s %s", method, uri, version);

    printf("method is %s, uri is %s, version is %s \n", method, uri, version);
    
    strcpy(req->uri, uri);
    parse_uri(uri, req);
    
    *headernum = 0;
    while (rio_readlineb(&rio, buf, MAXLINE) > 2)  
    {
        if(strstr(buf, "Proxy-Connection")) {
            strcpy(buf, "Proxy-Connection: close\r\n");
        }else if(strstr(buf,  "Connection")) {
            strcpy(buf, "Connection: close\r\n");
        }else if(strstr(buf, "User-Agent")) {
            strcpy(buf, user_agent_hdr);
        }
        strcpy(headers[*headernum].raw, buf);
        *headernum = *headernum + 1;
    }

    printf("end of parse request\n");
    return; 
}

void doit(int fd) {
    printf("start do it\n");
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio;

    Request req; 
    Header headers[100];
    int headernum = 0; 
    parse_request(fd, &req, headers, &headernum);
    
    struct CacheObj* obj = cache_read(req.uri);
    if( obj != NULL) {
        printf("url: %s, hit cache\n", obj->uri);

        printf("hitted cache is \n %s", obj->object);
        Rio_writen(fd, obj->object, strlen(obj->object));
        return;
    }
    printf("request is hostname %s, path %s, port %d, headernum is %d\n", req.hostname, req.path, req.port, headernum);
    int client_fd = send_servers(&req, headers, headernum);

    rio_t client_rio; 
    Rio_readinitb(&client_rio, client_fd);

    printf("start to write back to client\n");
    size_t n;
    char cach_buf[MAX_OBJECT_SIZE];
    char*  head = cach_buf;
    int total_size = 0;
    while((n = Rio_readlineb(&client_rio, buf, MAXLINE)) > 0) {
        Rio_writen(fd, buf, n);
        printf("%s",buf );
        // write into cach_buf 
        strcpy(head, buf);
        head += strlen(buf);
        total_size += strlen(buf);
    }
    // cache it 
    printf("total size is %d\n", total_size);

    if(total_size < MAX_OBJECT_SIZE) {
        cache_write(req.uri, cach_buf);
    }
    printf("end of write back to client\n");

    printf("end of do it\n");
    Close(client_fd);
}


void *thread(void *vargp) {
    int connfd = *((int*)vargp);
    Pthread_detach(pthread_self()); //将线程分离，让这个线程计数结束后自己回收资源
    Free(vargp);
    doit(connfd);
    Close(connfd);
    return NULL;
}


int main(int argc, char **argv)
{
    int listen_fd;
    int* conn_fd;
    socklen_t clientlen;

    struct sockaddr_storage clientaddr;
    char hostname[MAXLINE], port_str[MAXLINE];
    // create a listen port 
    if( argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listen_fd = Open_listenfd(argv[1]);

    printf("the proxy starts listen on port %s\n", argv[1]);

    pthread_t tid;

    init_cache();
    while(1) {  
        printf("get into loop\n");
        clientlen = sizeof(clientaddr);
        conn_fd = Malloc(sizeof(int));
        *conn_fd = Accept(listen_fd, (SA*)&clientaddr, &clientlen);

        printf(" listen_fd is %d, conn_fd is %d\n",listen_fd ,conn_fd);
        
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port_str, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port_str);  
        Pthread_create(&tid, NULL, thread, (void* )conn_fd);
    }

    return 0;
}
