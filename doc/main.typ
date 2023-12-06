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
Ez a dokutmentum a folyadék szimulálásra tett kísérletünket fogja végigkísérni. A folyadék szimulálást SPH módszer alapján implementáltuk OpenGL-ben, valamint C++-ban.

= Folyadék szimulálás
A folyadék szimulálás egy összetett, komplex feladat, mely még napjainkban sem tökéletes. Több különböző algoritmus létezik a folyadékok és gázok szimulálására, hiszen mindkettőnek fontos szerepe van, mind labori tesztelésekben, mind pedig a számítógépes grafika területén, mint például filmekben, számítógépes animációkban.

A szimulálásra két főbb kategória alakult ki:
- Euler féle hálók
- Lagrang féle részecskék
Az Euleri megoldások hálókra bontják a tartományt és ezekben a cellákban számolják a folyadék folyamát(sebességét, irányát, adott pontban). Ezekkel a metódusokkal könnyen lehet egy folyadéknak az összenyomhatatlansági attribútumát biztosítani, azonban a folyadék tömegmegmaradása nem mindig biztosított. 

Ezzel szemben a Lagrang féle metódusok a folyadék tömegét bontják fel részecskékre. Mivel nem a tartományt osztottuk fel, hanem a folyadékot, és ennek a részeit követjük az idő függvényében, így a tömegmegmaradás biztosított, azonban az összenyomhatatlansági attribútum nem garantált.
Egy másik hátránya a részecske megoldásnak, hogy a vizualizációja a folyadék felszínének nem triviális, hiszen a részecskékkel aktívan elkerüljük a felszín szimulálását, így azt pontosan kiszámítani számításigényes feladat.

== SPH
A témalabor folyamán a lagrange részecskéken alapuló SPH metódust választottuk. Az SPH, Smoothed Particle Hydrodynamics, 1977-ben kifejlesztett algoritmus, melyet eleinte asztrofizikai problémákra hoztak létre. 

A metódus fő előnyei, hogy külön számításigény nélkül biztosított a tömegmegmaradás, valamint a nyomást egyes helyeken a szomszéd részecskékből számítja, nem pedig lineáris egyenletek megoldásával, így adódik az is, hogy disztinktív réteg fog kialakulni két elem között, ahol a részecskék reprezentálják a sűrűbb folyadékot(vizet), a hígabb folyadékot pedig az űr(ami a levegő).

=== Kernel függvény
Az SPH fő eleme a folyadék diszkretizálása úgynevezett kernel függvények segítségével. A valós életeben mintavételezni a dirac-$delta$ függvény segítségével szoktak. A dirac-$delta$ függvényt a következő képpen fogjuk definiálni:

#math.equation($delta(r) = cases(infinity "ha" r = 0,
0 "különben")$)
#pagebreak()
,valamint igaz az is, hogy 

#math.equation($display(integral delta(r) italic(d)v = 1)$)
Miután definiáltuk a dirac-$delta$-t már csak azokat a függvényeket kell megkonstruálni, amikkel ezt közelíteni fogjuk, hiszen a folytonos dirac-$delta$-t nem lehet diszkrét függvénnyé alakítani. Ezeket a függvényeket fogjuk Kernel függvénynek hívni és W-vel fogjuk jelölni. 
Fontos attribútuma a Kernel függvényeknek, hogy 
#math.equation($display(W(r, h) = 0 ", ha" ||r|| >= h)" ."$)
Tehát, ha a két mintavételezett pont közötti távolság nagyobb, vagy egyenlő, mint a smoothing length, akkor mindig 0-át ad vissza a Kernel függvény.
Definiáljunk még egy A függvényt. Ekkor a dirac-$delta$ tulajdonságaiból következik, hogy ha egy függvényt konvolválunk a dirac-$delta$-val, akkor magát A-t kapjuk. Ezt közelítve a W-vel, a következő függvényt kapjuk:
#set math.equation(numbering: "(1)")
#math.equation($A(x) approx (A convolve W)(x) = integral A(x')W(x-x', h)italic(d)v'$,) <a_appr_w>
, ahol $bold(h)$ a Kernel függvény simító hossza(smoothing length), ami azt adja meg, hogy az A értékét x helyen mennyire befolyásolják a közelben lévő értékek. Mivel azonban számítógépekkel nem tudunk folytonos időben dolgozni, ezért diszkretizálni kell az #emph[@a_appr_w] -et. Ezt úgy érhetjük el, hogy az integrált felcseréljük szummázásra. Az így kapott egyenlet pedig:
#math.equation(numbering: none)[$A(x_i) = (A convolve W)(x_i)=sum_(j in F) (A_j m_j/rho_j W(x_i - x_j, h))$]
, ahol F olyan halmaz, amelyben az összes elem az ahhoz az indexhez tartozó A értéket tárolja, i.e.: $A_j = A(x_j)$.

==== Kernel függvény grádiense
Fontos még egy függvényt definiálni. Ez a kernel függvény grádiense, amit $nabla A$-val jelölünk.
#math.equation($nabla A_i approx sum_j (A_j  m_j / rho_j  nabla W_("ij"))$) <a_der_appr_w>
, ahol $W_"ij" "analóg" W(x_i - x_j, h)$-val.  Az #emph[@a_der_appr_w]-ben található Kernel függvény grádiense pedig, az $x_i - x_j$ szerinti deriváltja, mert a smoothing length-et konstansnak tekintjük a szimuláció teljes lefutása során.




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

#todo[kép, amin sok kulccsal is van ütközés]

=== Szimulációs határok kihasználása

Mivel a szimuláció tengelyhez igazított síkokkal van határolva, könnyű
kiszámítani a cellák számát és könnyű kitalálni egy képletet, ami minden
cellához egyedi kulcsot rendel. Tehát ha legalább annyi kulcs van, mint cella, akkor ütközésmentes lesz a szomszédos cellák vizsgálata.

Eddig határon levő cellák vizsgálatakor a határon kívüli szomszédokat is
megvizsgáltuk, sőt mivel a szimuláció 3 dimenzióra lett megírva, 2 dimenzióban
18 cellát mindig feleslegesen vizsgáltunk.

Ezzel a kettő változtatással már #todo[\~50000] részecskét lehet valós időben
szimulálni.

#todo[kép, amin kevés kulccsal sincs ütközés]

=== További optimalizációs módszerek

A szimuláció még az előző optimalizálások ellenére is túl sok felesleges
részecskét vizsgál meg. Tökéletes eloszlás esetén 2 dimenzióban $~65%$-ban, 3
dimenzióban $~84%$-ban felesleges a vizsgálat. Ezeket az arányokat a cellák
méretének csökkentésével és a részecske sugarán kívül eső cellák ignorálásával
lehetne javítani.

Ha a cellák (kulcsok) száma több nagyságrenddel nagyobb lenne, mint a részecskék száma, akkor a vödrös rendezés helyett a #todo[Bitonic sort]-ot érdemes kipróbálni, mert annak a futásideje a részecskék számától függ, nem a cellákétól.

Ha a részecskék és a cellák száma tovább növekedne, akkor a cellákat érdemes
lehet egy jobb lokalitású sorrendben rendezni (pl. #todo[Hilbert curve,
Z-curve]), mert így cache barátabb lenne az iterálás.

#todo[további ötletek...]

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

