/**
 * 编译：gcc -o TLS_client TLS_client.c -lssl  -lcrypto
 * 编译会报错：TLSv1_2_method is deprecated
 * 未解决
*/
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <string.h>

#define CHK_SSL(err) if ((err) < 1){ ERR_print_errors_fp(stderr); exit(2);}

// TLS initialization
SSL* setupTLSClient(const char * hostname){
    // openssl version 1.1.1
    // no longer need as of version 1.1.0
    // SSL_library_init();
    // SSL_load_error_strings();

    // SSL contest init
    SSL_METHOD *meth = (SSL_METHOD*)TLSv1_2_method();
    SSL_CTX *ctx = SSL_CTX_new(meth);
    SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER,NULL);
    SSL_CTX_load_verify_locations(ctx,NULL,"./cert");

    SSL *ssl = SSL_new(ctx);

    X509_VERIFY_PARAM * vpm = SSL_get0_param(ssl);
    X509_VERIFY_PARAM_set1_host(vpm,hostname,0);

    return ssl;
}

int setupTCPClient(const char* hostname,int port){
    struct sockaddr_in server_addr;

    struct hostent* hp = gethostbyname(hostname);

    int sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    memset(&server_addr,'\0',sizeof(server_addr));
    memcpy(&(server_addr.sin_addr.s_addr),hp->h_addr_list[0],hp->h_length);
    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;

    connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));

    return sockfd;
}

int main(int argc,char *argv[]){
    char *hostname = "example.com";
    int port = 443;

    if(argc > 1) hostname = argv[1];
    if(argc > 2) port = atoi(argv[2]);

    SSL *ssl = setupTLSClient(hostname);
    int sockfd = setupTCPClient(hostname,port);

    SSL_set_fd(ssl,sockfd);
    int err = SSL_connect(ssl);CHK_SSL(err);
    printf("SSL connection is successfully\n");
    printf("SSL connection using %s\n",SSL_get_cipher(ssl));

    char buf[9000];
    char sendBuf[200];

    sprintf(sendBuf,"GET / HTTP/1.1\nHost:%s\n\n",hostname);
    SSL_write(ssl,sendBuf,strlen(sendBuf));

    int len;
    do{
        len = SSL_read(ssl,buf,sizeof(buf)-1);
        buf[len] = '\0';
        printf("%s\n",buf);
    }while(len > 0);
}