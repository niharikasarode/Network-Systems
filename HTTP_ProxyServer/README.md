#HTTP PROXY SERVER

----------------------------------------------------------------------------
command line arguments: ./webproxy <port num> <timeout in seconds>

Need to configure port num in Browser Preferences 


-----------------------------------------------------------------------------

The webproxy program runs a proxy server which acts as an intermediate between
the client and the contacted server. It provides various advantages as privacy,
and performance through caching.

In this program, the server implements the following :

*It attempts to solve only GET request. All other requests are rejected.

*It checks for blocked sites being accessed and if accessed, straightaway 
  sends a forbidden error back to the client.

* Multi-threaded Proxy : The proxy handles multiple clients through forking.
 
*Caching : whenever a message is received, the md5hash of the url is evaluated
           and checked against the md5list local file (./cache/md5list). If the 
           md5 of the requested URL exists, then it is checked if the cached copy
           has timed out. If the url exists and not timed out, then the proxy sends
           saved cached copy instead of accessing the server

        : Then it is checked if a local cached copy which has timedout exists,if yes
          then it deletes it and saves a new copy after connecting with the server.

