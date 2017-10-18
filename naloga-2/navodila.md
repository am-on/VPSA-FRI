# Naloga 2

Nadgradite strežnik tako, da bo zmogel oskrbeti več klientov hkrati.

- Za vsakega klienta, ki se poveže na strežnik, naj se ustvari
  nova nit, ki ga oskrbuje, dokler klient ne zapre povezave.

Uporabite knjižico pthreads.



### Dodatno

Več-nitni strežnik vsebuje pomankljivosti:
- Nepridiprav lahko odpre ogromno število povezav, kar povzroči  stvaritev ogromnega števila niti.

- Upočasni se/odpove lahko celoten sistem.

   ​


Razmislite in popravite strežnik tako, da bo število klientov, ki jih lahko hkrati streže omejeno.

- V primeru, da se poskusi povezati več klientov kot je dovoljenih niti,  naj strežnik presežene povezave zavrne.

