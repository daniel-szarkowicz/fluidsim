#import "template.typ": *

#show: project.with(
  title: "Folyadék szimuláció",
  authors: (
    (name: "Nagy Áron Artúr", email: "nagyaronartur@edu.bme.hu"),
    (name: "Szarkowicz Dániel", email: "szarkowicz.daniel@edu.bme.hu"),
  ),
  date: "2023. December 5.",
)

#let todo(content) = text(red, content)

= Bevezetés
#todo[Rövid szöveg a folyadék szimulációról.]

#todo[Motiváció]

#todo[Szakirodalmi áttekintés @coding_adventures @sph_tutorial]

= Implementáció

== Szimuláció

#todo[smoothing kernel @coding_adventures @sph_tutorial]

#todo[density @coding_adventures @sph_tutorial]

#todo[pressure force @coding_adventures @sph_tutorial]

#todo[viscosity, near pressure (referencia)]

#todo[
  Továbbiak
  + Boundary force
  + Rigid body interakció
  + Soft body
  + ...
]

== Optimalizáció

A program #todo[nemhivatalos] célja valós idejű folyadékszimuláció volt, így
ahhoz, hogy minél részletesebb szimulálást hajthassunk végre, növelnünk kellett a szimulált részecskék számát. Ennek eléréséhez különböző optimalizálási
stratégiákat kellett implementálni.

_A mérések egy AMD RX Vega 6 mobil integrált videokártyán készültek.
Nagy valószínűséggel modern dedikált videokártyákon a szimulálható részecskék
száma ezeknek a többszöröse lenne._

=== Optimalizáció nélkül GPU-n

A program első verziója nem tartalmazott optimalizációt, ezért a GPU sok felesleges
számítást végzett. Ennek ellenére így is #todo[\~5000] részecskét lehetett
szimulálni valós időben, ami jó kiindulási pont volt.

Pszeudo kód:
```
for részecske_1 in részecskék:
  for részecske_2 in részecskék:
    a részecske_2-höz tartozó paraméterváltozások kiszámítása
    részecske_1 paramétereinek frissítése
```

#todo[kép]

Ezen a kódon elég könnyű gyorsítani, ha kihasználjuk azt, hogy a smoothing
kernel a sugarán kívül mindig 0-val tér vissza, azaz a smoothing radius-nél (sugárnál) távolabb levő részecskék nem
befolyásolhatják egy adott részecskének a paramétereit. Ezzel az egyszerű
javítással már #todo[\~9000] részecskét is tudtunk értelmes gyorsaság mellett szimulálni.

Pszeudo kód:
```
for részecske_1 in részecskék:
  for részecske_2 in részecskék:
    ha távolság(részecske_1, részecske_2) <= smoothing_radius:
      a részecske_2-höz tartozó paraméterváltozások kiszámítása
      részecske_1 paramétereinek frissítése
```

#todo[kép]

=== Cellákra bontás, hash tábla

Az előző részben javított algoritmus még mindig minden részecskét megvizsgál,
csak a nagy részét rögtön eldobja. A további optimalizálások a megvizsgált
részecskék számának csökkentésére fognak fókuszálni.

Gyakran használt megoldás@sph_tutorial, hogy a szimulált teret cellákra bontjuk és minden részecske csak a szomszédos cellákban levő részecskéket vizsgálja, így csak 3x3(x3) cella tartalmát kell megvizsgálni.

Az első megoldás egy vödrös hash szerű adatstruktúrát használ.
Ezzel a megoldással az volt a kihívás, hogy egy cellában a részecskék száma 0-tól a részecskék számáig terjedhet és az OpenGL nem támogat dinamikus memória foglalást. A probléma áthidalására egy megoldást ad az, ha a részecskéket kulcsaik szerint rendezzük, és egy segéd tömbben eltároljuk, hogy az egy kulcshoz tartozó részecskék mettől meddig tartanak. A kulcsokat egy generikus hash függvény@hash segítségével számoljuk ki.

#todo[ábra az adatstruktúráról]

Maga az algoritmus a vödrös rendezésnek egy párhuzamos változata, 3 részből áll:
+ A kulcshoz tartozó részecskék megszámolása és a részecsék kulcson belüli indexének kiszámítása (párhuzamosan, atomi számlálókkal)
+ Kulcshoz tartozó indexek kiszámolása (párhuzamos prefix sum@par_algs)
+ Részecskék áthelyezése a megfelelő helyre

#todo[n és k paraméterek megnevezése]
Ez az algoritmut $O(n+k)$ extra memóriát igényel, de ez nem baj, mert általában
$n >> k$ és a szimuláció eddig is 2 tömböt használt a részecskékhez, szóval
csak a kulcsoknak kell új memóriát foglalni.

Ezután minden részecskéhez már csak a környező cellák részecskéit kell megvizsgálni.

Pszeudo kód:
```
számlálók = [0, 0, ..., 0]                   -- kulcsok száma+1 db
for r in részecskék:
  k = r cellájának a kulcsa
  r kulcson belüli indexe = ++számlálók[k+1] -- atomi inkrementálás

-- prefix sum kiszámítása
indexek[i] = számlálók[0] + számlálók[1] + ... + számlálók[i]

for r in részecskék:
  k = r cellájának a kulcsa
  új_részecskék[indexek[k] + r kulcson belüli indexe] = r

for részecske in részecskék:
  for cella in részecske cellájának környezete:
    for szomszéd in cella részecskéi:
      ha távolság(részecske, szomszéd) <= smoothing_radius:
        a szomszéd paraméterváltozásainak kiszámítása
        részecske paraméterének frissítése
```

Ezzel az algoritmussal már #todo[\~25000] részecske is szimulálható volt érdemi gyorsasággal.

Sajnos mivel a hash függvény nem tökéletes, van amikor különböző celláknak azonos kulcsa van (akkor is ha a kulcsok száma sokkal több, mint a cellák száma), így nem csak a szomszédos cellák részecskéit vizsgáljuk meg, hanem néhány extrát is.

#todo[kép]

#todo[
  harmadik iteráció: domain-specific tökéletes "hash",
  domain-ből kilógó cellák vágása,
  ajánlott kulcs szám
]

#todo[
  További optimalizálási lehetőségek
  + Cella méret és kernel sugár különválasztása
  + Lokálisabb cella rendezés #todo[Hilbert-curve, Z-curve, ...]
  + Ha több a cella kulcs, mint a részecskék, akkor #todo[Bitonic Sort]
  + ...
]

== Vizualizáció

#todo[semmi extra, csak a részecskék különböző paramétereit vizualizálják]

#todo[
  További lehetőségek
  + point splatting
  + marching squares/cubes
  + ...
]


= Eredmények

#todo[gyengén összenyomható folyadék]

#todo[különböző sűrűségű folyadékok (csak úgy működik, de akkor is menő)]

#todo[2d sok részecske (\~25000) egy laptop gpu-n kb. real-time]

= Összefoglalás

#todo[általános összefoglalás, jövőbeli tervek (önlab)]

#bibliography("references.yml")

