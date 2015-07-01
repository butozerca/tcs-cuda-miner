Praca wykonywana przez nasz program przypomina pracę minerów `Bitcoin`:

- Pobiera hash (u nas z pliku `input.txt`)
- Szuka dla niego takiej liczby(zwanej `nonce`), że `sha256(sha256("$hash + $nonce"))` ma pewną ilość zer(w bitach) na początku.
- Zwraca tę liczbę.

`difficulty` - pożądana liczba zer wiodących w hashu.
`nonce_min, nonce_max` - końce przedziału dla wartości `nonce`.
W kodzie wpisane są rozsądne domyślne wartości, także nie trzeba ich podawać (przykład wykonuje się ~1 min na `miracle`).

Nasz program wykonuje się na CPU i na GPU, porównując ich wydajność.

Aby wykonać program należy wpisać w konsoli
`make` 
`./main [difficulty [nonce_min nonce_max]]`

W pliku `input.txt` znajdują się frazy dla których program spróbuje policzyć hashe - można coś tam dodać, ale trzeba pamiętać, aby były one 76 znakowe.
