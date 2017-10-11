# Naloga 1

Napiši program, ki vsebuje naslednje funkcije:

- ### Random

```c++
  double * Random(int n);
```

Funkcija naj dinamično ustvari in vrne kazalec na polje n naključnih realnih števil med 0 in 1 



- ### Matrix

 ```c++
  double ** Matrix (double *A, int n, int r);
 ```

Funkcija naj dinamično ustvari matriko z r vrsticami in vanjo prepiše vrednosti iz polja A. Nato naj vrne kazalec na matriko. _(Opomba: število stolpcev mora funkcija določiti na podlagi števil n in r. Če je števil premalo dopolnite matriko z ničlami!)_

  ​

- ### Max

 ```c++
  double * Max(double *A, int n); 
 ```

Funkcija naj vrne kazalec na največjo vrednost v polju A.



- Vse tri funkcije uporabite v glavnem programu, tako da polje, ki ga ustvari funkcija Random kot argument podate preostalima dvema funkcijama in izpišete rezultate.  
- Izmerite čas v sekundah, ki ga potrebuje funkcija Random, da ustvari in napolni polje števil in ga izpišite!




### Primer izpisa

```
Vhod Vnesi n: 8 
Vnesi r: 2 

Izhod 1D: 
0.02 0.03 0.23 0.89 0.10 0.56 0.99 0.73 
2D: 
0.02 0.03 0.23 0.89 
0.10 0.56 0.99 0.73 

Najvecja vrednost: 0.99 na naslovu: 0012FEDC. 
Cas generiranja nakljucnih stevil: 0.01 s.
```




> Pomagajte si s spletno stranjo, kjer najdete vse potrebne 1 funkcije: http://www.acm.uiuc.edu/webmonkeys/book/c_guide/