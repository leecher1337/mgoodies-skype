struct SSL_INTERFACE
{
	int cbSize;
	void*(*TLSv1_client_method) ();
	void*(*SSLv3_client_method) ();
	void*(*SSL_CTX_new) (void *method);
	void (*SSL_CTX_free) (void *ctx);
	void*(*SSL_new) (void *ctx);
	void (*SSL_free) (void *ssl);
	int  (*SSL_set_fd) (void *ssl, int fd);
	int  (*SSL_connect) (void *ssl);
	int  (*SSL_read) (void *ssl, void *buffer, int bufsize);
	int  (*SSL_write) (void *ssl, void *buffer, int bufsize);
};

#define MS_SYSTEM_GET_SSLI	"Miranda/System/GetSSLI"

static __inline int mir_getSSLI( struct SSL_INTERFACE* dest )
{
	dest->cbSize = sizeof(*dest);
	return CallService( MS_SYSTEM_GET_SSLI, 0, (LPARAM)dest );
}

extern struct SSL_INTERFACE ssli;

