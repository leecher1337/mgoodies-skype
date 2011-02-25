What is the SkypeProxy for imo.im
----------------------------------
Back in 2005 when I developed the Skype plugin for Miranda IM, I also developed 
a very simple protocol to tunnel the Skype API protocol via a socket.
The reason was, that some users wanted the possibility to run Skype on a
machine different to the machine they are using Miranda IM on.
So to tunnel the Skype API remotely over the network SkypeProxy was built and
was shipped with every Skype plugin build.

The Skype API plugin for Miranda got more and more functions as the API evolved 
and finally as I didn't have enough time to continue development, I handed over
development to user "tweety" who did the later versions.
The SkypeProxy-feature is still present in the current versions, as it is very
simple. It just wraps the communiction layer with the Skype API, therefore 
this concept can also be adopted by other Skype-Plugins of other IMs.

Ever since the first version of Skype plugin, people complained that they had to
install the Skype client on their machine in order to get it to work.
The Skype client was always bloated but it got even worse throughout the years.
I abandoned Skype some time ago, as this application is just consuming far too 
many system ressources. However, there are some people that are only using Skype
and I need a way to communicate with them. I'm just interested in the chat
feature, so I only need to send and receive messages from other chat users.
So what can we do about it?

There were some attempts to reverse-engineer the Skype protocol, but nobody 
succeeded so far as the protocol relies heavily on cryptography, is proprietary
closed source and the Skype client itself is heaviely protected agains all
kinds of reverse-engineering (decrypts code on the fly, uses anti-debugging 
tricks, etc.). 

However some users recently discovered, that there is a new Web 2.0 service that
makes it possible to do Instant messaging via the web browser using the Ajax
framework. The name of the service is imo.im
imo.im somehow managed to provid basic connectivity to the Skype network and
the nice thing about their service is, that you don't need to register, you
can just use it out of the box.
However most people are not so excited about IM in a web interface, there are
many features that an IM application provides and that are not so convenient in
a webinterface. But as their service is a webservice using jSON calls,
it is relatively easy to talk to their application server and so a Miranda IM
user can take advantage of this for providing basic connectivity to the Skype
network for instant messaging without having to install the bloated 
Skype client application.
Of course, only instant messaging is possible, no other features that would
require special interfaces. But I think for most users like me that just
want to stay in contact with other Skype users via chat these functions
will be enough.

So as mentioned above the most generic approach to link imo.im services
with an Instant messanger would be to implement this as a wrapper between
imo.im and the Skype API. This way, we don't need to implement an extra
plugin and it is easily adaptable to Skype plugins of other IMs, as they
just need the implement a few lines of code for the SkypeProxy protocol in
their communication layer to the Skype API.
Of course, the Skype plugins have to do correct error checking as only
a very, very small subset of the Skype API is implemented to allow very
basic messaging. The plugins mustn't assume that an API call always returns
something useful on every call.

This plugin tries to provide a very basic layer in order to accomplish this
goal. It runs on Win32 as well as on Unix, so it's a cross-platform layer that
you can also install on your personal Server. By design, it would be even
able to manage multiple users at once, however this feature is currently not
implemented in the SkypeProxy protocol. Depeding on the user's needs, this
can be implemented without problems, most of the code for this is already there.
Feedback is apreciated, as I would like to know if I should continue
developing this application.

How does it work
----------------

Is described in the preamble, this is just a drop-in replacement for the
communication end point of the Skype API.
The classical communication model for Skype API is:

[ Skype servers ] <--> [ Skype App ] <--> [ Plugin ]

The SkypeProxy communication model is:

[ Skype servers ] <--> [ Skype App ] <--> [ SkypeProxy ] <--> [ Plugin ]

Now with this Dop-in replacement the communication model is:

[ Skype servers ] <--> [ imo.im ] <--> [ SkypeProxy ] <--> [ Plugin ]

The application communicates via a socket connection on Port 1402, like
SkypeProxy does. It uses the imo.im services to connect to the Skype network.


How to compile
--------------
On Unix type systems, just extract this package and type "make".
After compiling, you will find imo2sproxy in the bin/ directory

On Windows systems, you will find Visual C 6 project files in the
msvc/ subdirectory.
You can also just build with nmake using the imoskype.mak makefile.

Requirements
------------
On Linux, libcurl Libraries and the pthread library for threading
are needed in order to get this to work. 
You can download CURL on http://curl.haxx.se/download.html
To install, just use libtool as usual.

On Windows, WININET is required which should be shipped with
Internet Explorer. So on an average Windows System, you shouldn't
need to install any additional libraries.
Please note, that libcurl IS NO LONGER NEEDED on Windows.

How to use the commandline version
----------------------------------
Find the precompiled binaries in the bin/ subdirectory.

imo2sproxy [-d] [-v [-l <Logfile>]] [-t] [-i][-h <Bind to IP>]
           [-p <Port>] <Username> <Password>

-v      - Verbose mode, log commands to console
-l      - Set logfile to redirect verbose log to.
-d      - Daemonize (detach from console)
-i      - Use interactive mode (starts imo.im flash app upon call)
-t      - Ignore server timestamp and use current time for messages
-h      - Bind to a specific IP, not to all interfaces (default)
-p      - Bind to another port (default: 1401)

As there is currently only a single-user implementation of this application, you
 need to start imo2sproxy with your Skype username and password as parameters.
 
For example:
imo2sproxy -d -h 127.0.0.1 myuser mypass

As soon as the imo2sproxy is running, setup your Skype Plugin to use the 
SkypeProxy at the machine you are running imo2sproxy on.
If you are running imo2sproxy locally, enter 127.0.0.1 as IP address.
After setting the SkypeProxy, you have to restart Miranda IM.

If all works well, you should now be able to use Skype via imo.im.
If it doesn't work, you can use the -v parameter to see what's going on and
if the connection works.

If it all works well, you can also install imo2sproxy on your machine
as a service using srvany.

How to use the Miranda plugin
-----------------------------
Copy the imoproxy.dll file to your Miranda Plugins-directory.
As the name starts with i and the the Skype plugin name starts with s,
the imoproxy.dll module is loaded prior to the Skype-Plugin DLL which
is important to work properly.

Configure the plugin DLL in the Options dialog Network/Skype Imoproxy.
Bind to address 127.0.0.1, Port 1401
Make sure that the Skype Proxy settings in your Skype plugin are the
same. The Skypeproxy-Plugin tries to take care of that.
Enter your Username and Password, set the appropriate options and press
OK. The Skypeproxy-plugin restarts itself. Try to reconnect Skype plugin
and see if it works.
The plugin is still experimental and may contain some bugs or stability
problems, so use with care. It may be harder to hunt down bugs with this
Plugin as its running as Miranda plugin and the only logging facility is
a logfile. So if you want to hunt down bugs, you may be better off with
using the standalone version, however the plugin is a start to make the
use of imo2sproxy easier. Feel free to improve the plugin.

Module structure
----------------

For the structure of the code-modules, please refer to the 
structure.txt document.

SkypeProxy protocol
-------------------

Plase download the sourcecode of skypeproxy.c of Miranda IM Skype plugin, it's 
all well documented in the .c file. Basically the protocol just consists of:

[UCHAR: number of bytes to send/receive][<char> Data]

for every line sent or received.

Contact
-------
Feel free to contact me regarding this project or if you wish to implement
SkypeProxy protocol in your own Skype-plugin implementation.

leecher@dose.0wnz.at


Vienna, 10/07/2009
