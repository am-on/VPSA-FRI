# Naloga 8

Napišite program v katerem bodo računalniki ciklično dopolnjevali sporočilo

– Proces 0 pošlje svoj rank procesu 1
– Proces 1 sprejetemu sporočilu doda svoj rank in novo sporočilo pošlje procesu 2
– …
– …
– …
– Proces n-1 sprejetemu sporočilu doda svoj rank in novo sporočilo pošlje procesu 0
– Proces 0 sprejetemu sporočilu doda svoj rank ter izpiše končno sporočilo
– Primer izpisa, ko so vpleteni štirje procesi: `0–1–2–3–0`

> compile and run:
> ```mpicc -o solution solution.c && mpirun -np 15 solution```