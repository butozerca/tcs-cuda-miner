Praca wykonywana przez program przypomina pracę minerów Bitcoinów:

- Pobiera hash (u nas z pliku input.txt)
- Szuka dla niego takiej liczby(zwanej `nonce`), że sha256(sha256("$hash + $nonce")) ma pewną ilość zer(w bitach) na początku.
- Kończy działanie po znalezieniu takiej liczby.

Nasz program wykonuję tę pracę na CPU i na GPU, porównując ich wydajność.

Aby wykonać program należy wpisać w konsoli
`make` (jeśli natrafisz na błąd przy kompilacji pliku `real.cpp` to się nie zniechęcaj)
`./main [difficulty [nonce_min nonce_max]]`

Można opcjonalnie podać minimalną ilość zer wymaganą w hashu oraz zakres `nonce` jaki chcemy przeszukać.
W kodzie wpisane są rozsądne domyślne wartości, także nie trzeba ich podawać (przykład wykonuje się ~1 min na miraclu).

W pliku `input.txt` znajdują się frazy dla których program spróbuje policzyć hashe - można coś tam dodać, ale trzeba pamiętać, aby były one 76 znakowe.



Żeby skompilować real-cpu i real-gpu potrzebne są biblioteki cpp-netlib, libblkmaker i libjansson
W ubuntu można je zainstalować poleceniem sudo apt-get install libblkmaker-0.1-dev libcppnetlib-dev libjansson-dev
