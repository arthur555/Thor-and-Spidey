Project - README
================

Members
-------

- Logan Yokum (lyokum@nd.edu)

Demonstration
-------------

https://docs.google.com/presentation/d/1FCmiINX38Hws6R8kLgJhif5tlpsjffo-HjYdf1imUUA/edit?usp=sharing

Errata
------

Some memory leaks in single and forking modes (particularly in forking mode). It seems like the
request structure is not being freed correctly.

Contributions
-------------

Logan Yokum (lyokum): bug testing and making tests pass, socket.c, requests.c, handler.c
