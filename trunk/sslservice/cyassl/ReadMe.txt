Add here the content of cyassl-0.9.8.zip
http://www.yassl.com/download.html

Change line 1716 in src/cyassl_int.c from:
if (ssl->options.connReset || ssl->options.isClosed)
to: 
if (ssl->options.isClosed)
