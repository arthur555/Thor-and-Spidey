Project - README
================

Members
-------

- Logan Yokum (lyokum@nd.edu)

Demonstration
-------------
Forking
| process| request| uri                  | latency    | throughput    |
|--------|--------|----------------------|------------|---------------|
|       1|       2| /html/1k.txt         |     0.54222|         0.0018M|
|       1|       3| /html/1k.txt         |     0.54239|         0.0018M|
|       2|       2| /html/1k.txt         |     0.54248|         0.0018M|
|       2|       3| /html/1k.txt         |     0.54242|         0.0018M|
|       3|       2| /html/1k.txt         |     0.54242|         0.0018M|
|       3|       3| /html/1k.txt         |      0.5425|         0.0018M|
|       4|       2| /html/1k.txt         |     0.54242|         0.0018M|
|       4|       3| /html/1k.txt         |     0.66271|        0.00147M|
|       1|       2| /html/1M.txt         |     9.43735|        0.10596M|
|       1|       3| /html/1M.txt         |    13.94413|        0.07171M|
|       2|       2| /html/1M.txt         |    16.76973|        0.05963M|
|       2|       3| /html/1M.txt         |    24.46417|        0.04088M|
|       3|       2| /html/1M.txt         |    21.45736|         0.0466M|
|       3|       3| /html/1M.txt         |    32.39761|        0.03087M|
|       4|       2| /html/1M.txt         |    27.16702|        0.03681M|
|       4|       3| /html/1M.txt         |    40.32931|         0.0248M|
| /html/1G.txt         | inf        | inf           |
| /html/1G.txt         | inf        | inf           |
| /html/1G.txt         | inf        | inf           |
...
|       1|       2| /html/index.html     |     0.54226|        0.00166M|
|       1|       3| /html/index.html     |     0.66254|        0.00136M|
|       2|       2| /html/index.html     |     0.54205|        0.00166M|
|       2|       3| /html/index.html     |     0.66251|        0.00136M|
|       3|       2| /html/index.html     |     0.66256|        0.00136M|
|       3|       3| /html/index.html     |     0.66257|        0.00136M|
|       4|       2| /html/index.html     |     0.66262|        0.00136M|
|       4|       3| /html/index.html     |     0.66275|        0.00136M|
|       1|       2| /scripts/cowsay.sh   |     0.72264|        0.00117M|
|       1|       3| /scripts/cowsay.sh   |     0.84295|          0.001M|
|       2|       2| /scripts/cowsay.sh   |     0.72268|        0.00117M|
|       2|       3| /scripts/cowsay.sh   |     0.84297|          0.001M|
|       3|       2| /scripts/cowsay.sh   |     0.72266|        0.00117M|
|       3|       3| /scripts/cowsay.sh   |     0.96306|        0.00088M|
|       4|       2| /scripts/cowsay.sh   |     0.84299|          0.001M|
|       4|       3| /scripts/cowsay.sh   |     0.96313|        0.00088M|
|       1|       2| /scripts/env.sh      |     0.54239|        0.00015M|
|       1|       3| /scripts/env.sh      |     0.66252|        0.00012M|
|       2|       2| /scripts/env.sh      |     0.54242|        0.00015M|
|       2|       3| /scripts/env.sh      |     0.66273|        0.00012M|
|       3|       2| /scripts/env.sh      |     0.54239|        0.00015M|
|       3|       3| /scripts/env.sh      |     0.66252|        0.00012M|
|       4|       2| /scripts/env.sh      |     0.66271|        0.00012M|
|       4|       3| /scripts/env.sh      |     0.66267|        0.00012M|
|       1|       2| /text/hackers.txt    |     0.72261|        0.00493M|
|       1|       3| /text/hackers.txt    |     0.72223|        0.00494M|
|       2|       2| /text/hackers.txt    |     0.72279|        0.00493M|
|       2|       3| /text/hackers.txt    |     0.84287|        0.00423M|
|       3|       2| /text/hackers.txt    |     0.72269|        0.00493M|
|       3|       3| /text/hackers.txt    |     0.84298|        0.00423M|
|       4|       2| /text/hackers.txt    |     0.72265|        0.00493M|
|       4|       3| /text/hackers.txt    |     0.96315|         0.0037M|
|       1|       2| /text/lyrics.txt     |     0.54232|        0.00164M|
|       1|       3| /text/lyrics.txt     |     0.66263|        0.00135M|
|       2|       2| /text/lyrics.txt     |     0.66262|        0.00135M|
|       2|       3| /text/lyrics.txt     |      0.5424|        0.00164M|
|       3|       2| /text/lyrics.txt     |     0.54237|        0.00164M|
|       3|       3| /text/lyrics.txt     |     0.66267|        0.00135M|
|       4|       2| /text/lyrics.txt     |     0.66261|        0.00135M|
|       4|       3| /text/lyrics.txt     |      0.6627|        0.00135M|

Single
| process| request| uri                  | latency    | throughput    |
|--------|--------|----------------------|------------|---------------|
|       1|       2| /html/1k.txt         |     0.54221|         0.0018M|
|       1|       3| /html/1k.txt         |     0.54229|         0.0018M|
|       2|       2| /html/1k.txt         |     0.54253|         0.0018M|
|       2|       3| /html/1k.txt         |      0.6626|        0.00147M|
|       3|       2| /html/1k.txt         |     0.66257|        0.00147M|
|       3|       3| /html/1k.txt         |     0.72268|        0.00135M|
|       4|       2| /html/1k.txt         |     0.72239|        0.00135M|
|       4|       3| /html/1k.txt         |     0.84305|        0.00116M|
|       1|       2| /html/1M.txt         |    46.52078|         0.0215M|
|       1|       3| /html/1M.txt         |         inf|             inf|

| process| request| uri                  | latency    | throughput    |
|--------|--------|----------------------|------------|---------------|
|       1|       2| /html/index.html     |     0.54228|        0.00166M|
|       1|       3| /html/index.html     |     0.54239|        0.00166M|
|       2|       2| /html/index.html     |     0.66268|        0.00136M|
|       2|       3| /html/index.html     |     0.66247|        0.00136M|
|       3|       2| /html/index.html     |     0.66222|        0.00136M|
|       3|       3| /html/index.html     |     0.72289|        0.00124M|
|       4|       2| /html/index.html     |     0.72276|        0.00124M|
|       4|       3| /html/index.html     |     0.84305|        0.00107M|
|       1|       2| /scripts/cowsay.sh   |     0.66255|        0.00127M|
|       1|       3| /scripts/cowsay.sh   |      0.8429|          0.001M|
|       2|       2| /scripts/cowsay.sh   |     0.84284|          0.001M|
|       2|       3| /scripts/cowsay.sh   |     0.96335|        0.00088M|
|       3|       2| /scripts/cowsay.sh   |     1.14335|        0.00074M|
|       3|       3| /scripts/cowsay.sh   |     1.26367|        0.00067M|
|       4|       2| /scripts/cowsay.sh   |     1.26363|        0.00067M|
|       4|       3| /scripts/cowsay.sh   |      1.6243|        0.00052M|
|       1|       2| /scripts/env.sh      |     0.54235|        0.00015M|
|       1|       3| /scripts/env.sh      |     0.54248|        0.00015M|
|       2|       2| /scripts/env.sh      |     0.66262|        0.00012M|
|       2|       3| /scripts/env.sh      |     0.66259|        0.00012M|
|       3|       2| /scripts/env.sh      |     0.66258|        0.00012M|
|       3|       3| /scripts/env.sh      |     0.72268|        0.00011M|
|       4|       2| /scripts/env.sh      |     0.66271|        0.00012M|
|       4|       3| /scripts/env.sh      |     0.72278|        0.00011M|
|       1|       2| /text/hackers.txt    |     0.66255|        0.00538M|

https://docs.google.com/presentation/d/1FCmiINX38Hws6R8kLgJhif5tlpsjffo-HjYdf1imUUA/edit?usp=sharing

Errata
------

Some memory leaks in single and forking modes (particularly in forking mode). It seems like the
request structure is not being freed correctly.

Contributions
-------------

Logan Yokum (lyokum): bug testing and making tests pass, socket.c, requests.c, handler.c
