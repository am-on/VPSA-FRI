# Naloga 3

Paralelizirajte skalarni produkt dveh vektorjev dolžine M med N niti

- M elementov razdelimo med N niti, vsaka nit najprej izračuna M/N 
  produktov soležnih elementov vektorjev
- Vsaka izmed N niti sešteje svoje lokalne produkte
  - N/2 niti izvede nadaljnje seštevanje
  - N/4 niti izvede nadaljnje seštevanje, itd.; vse dokler ni število 
    aktivnih niti enako 1. 

*Predpostavite lahko, da sta M in N potenci števila 2, M>N*



> Zaženi rešitev:
> ```bash
> gcc -o solution -pthread solution.cpp && ./solution 
> ```

