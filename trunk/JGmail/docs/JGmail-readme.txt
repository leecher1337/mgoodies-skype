JGmail is an enhancement of the existing JabberG plugin for Miranda-IM that 
aim better support of the specific XMPP extensions of the Google Talk service.

JGmail is announced and discussed in dedicated “official” thread in Miranda’s 
forum located at http://forums.miranda-im.org/showthread.php?t=5730

Below is a short description of JGmail features cropped from the forum:

Done:
    * JGmail specific options page
    * Implemented popup timeouts and colours in options page
    * Option to synchronize PC clock to Google server
    * Direct the default browser to show the received e-mail on popup click
    * Special sound for new e-mail notification
    * Icons library manager (IcoLib) support
    * Option to log in the history of virtual contact
    * Option to enable/disable chat logs on server
    * “stream version='1.0'” compliant
    * sasl authentication mechanisms – PLAIN and X-GOOGLE-TOKEN
    * Enabled connection to port 443 (useful with restrictive proxies)
    * For https connection needed for X-GOOGLE-TOKEN WinInet will be used 
if OpenSSL is not installed
    * Delayed automatic re-request of the mailbox if last request failed
    * All options dialogs are collected in a single tabbed one
    * Show/Hide Expert settings now works

Planed:
    * Configurable url/program on popup click
    * Visit GMail of fake contact double click

Notes:
    * The non-StaticSSL bulds require OpenSSL libraries for secure connections
    * There are many sites to download OpenSSL dlls. I use these
    * StaticSSL build are suitable for users who don't use OpenSSL for other 
plugins/programs
    * New e-mail notifications do not work with SSL. Use StartTLS or no 
encryption (Thanks "andrewabc" for the assistance to find that out)
    * Screenshots of advisable configurations are available for Win NT/2K/XP 
and Win 9x/ME.
    * It is known Single threaded PopUps make problems. To avoid them enable 
"Use Multiple Threads" PopUps advanced option or use YAPP instead of PopUp(+).

JGmail is based on JabberG by George Hazan and others.
JGmail is under GPL.

yb(at)saaplugin(dot)no-ip(dot)info
