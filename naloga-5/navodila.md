# Naloga 5

- Paralelizirate algoritem za določanje vidnih odsekov črt s pomočjo OpenMP
- Višino in število vidnih odsekov izračunajte in izpišite rezultat

```
Naključno generiraj N črt različnih višin

k[0]=y[0]/x[0]
vidnaVisina=y[0]
steviloVidnih=1

for i=1 to N do
    k[i]=max(k[i-1],y[i]/x[i])
    if k[i] > k[i-1] then
        vidnaVisina = vidnaVisina + y[i]-k[i-1] * x[i]
        steviloVidnih = steviloVidnih + 1
    end
end
```
