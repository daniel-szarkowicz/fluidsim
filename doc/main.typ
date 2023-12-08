#import "@preview/cetz:0.1.2"
#import "template.typ": *

#show: project.with(
  title: "Folyadék szimuláció",
  authors: (
    (name: "Nagy Áron Artúr", email: "nagyaronartur@edu.bme.hu"),
    (name: "Szarkowicz Dániel", email: "szarkowicz.daniel@edu.bme.hu"),
  ),
  date: "2023. December 8.",
)

#let todo(content) = text(red, content)
#let numbered(content) = math.equation(numbering: "(1)", content)

= Bevezetés
Ez a dokutmentum a folyadék szimulálásra tett kísérletünket fogja végigkísérni. A folyadék szimulálást SPH módszer alapján implementáltuk OpenGL-ben, valamint C++-ban.

= Folyadék szimulálás
A folyadék szimulálás egy összetett, komplex feladat, mely még napjainkban sem tökéletes. Több különböző algoritmus létezik a folyadékok és gázok szimulálására, hiszen mindkettőnek fontos szerepe van, mind labori tesztelésekben, mind pedig a számítógépes grafika területén, mint például filmekben, számítógépes animációkban.

A szimulálásra két főbb kategória alakult ki:
- Euler féle hálók
- Lagrange féle részecskék
Az Euleri megoldások hálókra bontják a tartományt és ezekben a cellákban számolják a folyadék folyamát(sebességét, irányát, adott pontban). Ezekkel a metódusokkal könnyen lehet egy folyadéknak az összenyomhatatlansági attribútumát biztosítani, azonban a folyadék tömegmegmaradása nem mindig biztosított. 

Ezzel szemben a Lagrange féle metódusok a folyadék tömegét bontják fel részecskékre. Mivel nem a tartományt osztottuk fel, hanem a folyadékot, és ennek a részeit követjük az idő függvényében, így a tömegmegmaradás biztosított, azonban az összenyomhatatlansági attribútum nem garantált.
Egy másik hátránya a részecske megoldásnak, hogy a vizualizációja a folyadék felszínének nem triviális, hiszen a részecskékkel aktívan elkerüljük a felszín szimulálását, így azt pontosan kiszámítani számításigényes feladat.

== SPH
A témalabor folyamán a lagrange részecskéken alapuló SPH metódust választottuk. Az SPH, Smoothed Particle Hydrodynamics, 1977-ben kifejlesztett algoritmus, melyet eleinte asztrofizikai problémákra hoztak létre. 

A metódus fő előnyei, hogy külön számításigény nélkül biztosított a tömegmegmaradás, valamint a nyomást egyes helyeken a szomszéd részecskékből számítja, nem pedig lineáris egyenletek megoldásával, így adódik az is, hogy disztinktív réteg fog kialakulni két elem között, ahol a részecskék reprezentálják a sűrűbb folyadékot(vizet), a hígabb folyadékot pedig az űr(ami a levegő).

=== Kernel függvény
Az SPH fő eleme a folyadék diszkretizálása úgynevezett kernel függvények segítségével. A valós életeben mintavételezni a dirac-$delta$ segítségével szoktak. A dirac-$delta$ a következő képpen lesz definiálva:

$
  delta(r) = cases(
    infinity quad &"ha" r = 0,
    0        quad &"különben"
  )
$
,valamint igaz az is, hogy 

$
  integral delta(r) italic(d)v = 1
$
Miután definiáltuk a dirac-$delta$-t már csak azokat a függvényeket kell megkonstruálni, amikkel ezt közelíteni fogjuk, hiszen a folytonos dirac-$delta$-t nem lehet diszkrét függvénnyé alakítani. Ezeket a függvényeket fogjuk Kernel függvényeknek hívni és W-vel fogjuk jelölni őket. 
Fontos attribútuma a Kernel függvényeknek, hogy 
$
  W(r, h) = 0 ", ha" ||r|| >= h ". "
$
Tehát, ha a két mintavételezett pont közötti távolság nagyobb, vagy egyenlő, mint a smoothing length, akkor mindig 0-át ad vissza a Kernel függvény.
Definiáljunk még egy A függvényt. Ekkor a dirac-$delta$ tulajdonságaiból következik, hogy ha egy függvényt konvolválunk a dirac-$delta$-val, akkor magát A-t kapjuk. Ezt közelítve a W-vel, a következő függvényt kapjuk:
#set math.equation(numbering: "(1)")
#math.equation[$
  A(x) approx (A convolve W)(x) = integral A(x')W(x-x', h)italic(d)v'
$] <a_appr_w>
, ahol $bold(h)$ a Kernel függvény simító hossza(smoothing length), ami azt adja meg, hogy az A értékét x helyen mennyire befolyásolják a közelben lévő értékek. Mivel azonban számítógépekkel nem tudunk folytonos időben dolgozni, ezért diszkretizálni kell az #emph[@a_appr_w]-et. Ezt úgy érhetjük el, hogy az integrált felcseréljük szummázásra. Az így kapott egyenlet pedig:
$
  A(x_i) = (A convolve W)(x_i)=sum_(j in F) (A_j m_j/rho_j W(x_i - x_j, h))
$
, ahol F olyan halmaz, amelyben az összes elem az ahhoz az indexhez tartozó A értéket tárolja, i.e.: $A_j = A(x_j)$.

==== Kernel függvény grádiense
Fontos még egy függvényt definiálni. Ez a pótfüggvény grádiense, amit $nabla A$-val jelölünk.
#math.equation($nabla A_i approx sum_j (A_j  m_j / rho_j  nabla W_("ij"))$) <a_der_appr_w>
, ahol $W_"ij" "analóg" W(x_i - x_j, h)$-val.  Az #emph[@a_der_appr_w]-ban található Kernel függvény grádiense pedig, az $x_i - x_j$ szerinti deriváltja, mert a smoothing length-et konstansnak tekintjük a szimuláció teljes lefutása során. 

==== Kernel választás
Kernel választásnál, amíg a szempontokat szem előtt tartjuk, addig nagyon sok féle lehetőségünk van. A papír@sph_tutorial szerint egy szokványos választás a köbös görbe.
Ezt a görbét a következő képpen fogjuk definiálni:#pagebreak()
#math.equation($W(bold(r), h) = sigma_d cases(6(q^3 - q^2)+1 quad &"ha " 0 <= q <= 1/2, 2(1-q)^3 &"ha " 1/2 < q <= 1, 0 &"különben")$)
, ahol $q = 1/h||bold(r)||$, valamint d= 1,2,3 esetén $sigma_1 = 3/(4h) "," sigma_2 = 40/(7pi h^2) " és " sigma_3 = 8/(pi h ^3)$. @sph_tutorial 

A csapatunk a
$
  W(r, h) = cases(
    (h-r)^2 / V quad & "ha" 0 <= r <= h,
    0 quad & "különben"
  )
$
függvényt választotta, ahol $2$ illetve $3$ dimenzió esetén
$V_2 = (pi dot h^4)/6$ és $V_3 = (2 pi dot h^5)/15$.\
Ez a függvény jobban kezeli az egymáshoz közeli részecskék nyomásának
számítását, mert a deriváltja $0$ közelében nem $0$-hoz tart@coding_adventures.

=== SPH algoritmus menete
Maga az algoritmus menete könnyen diszkretizálható disztinkt állomásokra. Először is végig kell mennünk minden részecskén minden iterációban, kiszámolni rá a kívánt attribútumokat a szomszédos részecskék segítségével. Ha ezzel végeztünk, akkor még egy ciklusban frissítjük a részecskék pozícióját, valamint a határokkal való esetleges ütközéseket lekezeljük.

Pszeudó kód:
```
for részecske_1 in részecskék:
  for részecske_2 in részecskék:
    számold ki a kívánt attribútumait részecske_1-nek részecske_2 függvényében

for részecske_1 in részecskék:
  számold ki a részecske_1-re ható erőket az attribútumok által

for részecske_1 in részecskék:
  számold ki részecske_1 új sebesség vektorát eltelt idő függvényében
  számold ki részecske_1 új pozícióját
  nézd meg, hogy nem megy-e ki a szimulációs térből a részecske
```
A kívánt attribútumok pedig a következők: 
- sűrűség
- közeli sűrűség
- viszkozitás
- nyomás
Ezeken kívűl más attribútumokat is ki lehet még számítani, hogy még pontosabb legyen a szimuláció, azonban mi csak ezeket számoltuk ki.

==== Attribútumok kiszámítása
===== Sűrűség kiszámítása
Az i. részecske sűrűség attribútumának kiszámítása az alábbi, egyszerű, módon elvégezhető:
#math.equation($rho_i = sum_j m_j W_(i j)$)
, ahol $rho_i$, az i. részecske sűrűsége, $m_j$ a j. részecske tömege, $W_(i j)$ pedig egy kernel függvény.

Ennek a megoldásnak egy hátránya, hogy a felületeknél alulbecsli egy részecske sűrűségét, abból az egyszerű okból kifolyólag, hogy kevesebb szomszédja van, amiatt, mert a levegőben, nincsenek részecskék.
#figure(
  image("imgs/neighbours.png", width: 80%),
  caption: [
    Látható, hogy pirosnak jóval kevesebb részecske fog hozzájárulni a sűrűségéhez, mint a zöldnek.
  ] 
)

===== Nyomás sűrűségből  
Miután meghatároztuk a részecskék sűrűségeit a következő kérdés az, hogy ezzel, hogyan tudjuk befolyásolni a mozgásukat. Erre ad egy megoldást az, ha kiszámoljuk minden részecskénél a nyomást. Ehhez két konstanst definiálnunk kell, hogy a konverzió működhessen:
+ nyugalmi sűrűség $rho_0$
+ rugalmassági konstans $k$.
A nyugalmi sűrűséggel azt határozzuk meg, hogy a folyadékban a részecskéknek, nyugalmi állapotban, milyen sűrűségekkel kellene rendelkezzenek. A másik konstanssal, rugalmassági konstanssal, azt határozzuk meg, hogy mekkora befolyása legyen a sűrűségnek a szimulációra. Minél nagyobb k, annál kevésbé összenyomható a folyadék, azonban finomabb időközönként kell szimulálni, tehát ez egy optimalizációs kérdés.

Ezekkel a konstansokkal már definiálhatjuk a konverziót a sűrűség és a nyomás között:
#math.equation($P_i = k(rho_i - rho_0)$)

Azonban ha csak ennyi attribútumot számolnánk ki, akkor sok felhalmozódás lenne, részecskék egymásra lapolódnának. Ennek elkerülésére számoljuk ki a közelségi sűrűséget. Ezt hasonló módon számoljuk, mint a sűrűséget, annyi különbséggel, hogy egy újabb konstanst és attribútumot bevezetünk:
+ közeli rugalmassági konstans $k^"near"$, valamint
+ közelségi sűrűség $rho^"near"$.
A közelségi sűrűgséget az alábbi módon:
#math.equation($rho_i^"near" = sum_(j in N(i))W_"near_ij"$)
, ahol N az i. részecske szomszédait tárolja, valamint $W_"near_ij"$ egy eltérő kernel függvény a sűrűségnél használt kernel függvénytől, olyan módon, hogy meredekebb a meredeksége, amint 0-át közelítjük. A közeli nyomást pedig:
#math.equation($P_i^"near" = k^"near" rho_i^"near"$) <near_density_eq>
, módon kapjuk meg. @near_density_eq -ből azért hiányzik a nyugalmi sűrűség, mivel ezt a nyomást, kizárólag taszító erőként szeretnénk számon venni.
===== Felületi feszültség
Fontos eleme a folyadékoknak a felületi feszültségük. Ez azért lényeges, mert enélkül a szimulált folyadék lényegesen darabosabb lesz. Ezt több attribútum együttesével lehetne elérni, azonban mi, csak a viszkozitással foglalkoztunk. Ezt az alábbi módon számoltuk:

Pszeudó kód:
```
for részecske_1 in részecskék:
  viszkozitás_impulzus := null_vec(d)
  for részecske_2 in szomszédok:
    távolság <= távolság részecske_1 és részecske_2 között
    irány_vek <= irányvektor részecske_2-ből részecske_1-be
    u <= részecske_2 sebessége részecske_1 irányába
    if ||u|| <= 0: continue
    viszkozitás_impulzus += viszkozitás_kernel(távolság) * (coeff * u) * irány_vek

  részecske_1.sebesség += viszkozitás_impulzus * eltelt_idő
```
, ahol `null_vec(d)` olyan sorvektor, ami d db nullát tartalmaz, valamint coeff a koefficiense az u-nak, azaz, azt adjuk meg, hogy mennyire akarjuk folyékonyra, vagy sűrűre a folyadékunk. Nagyobb coeff tartósabb, viszkózusabb folyadékot eredményez.

==== Erő meghatározása
Miután meghatároztuk a részecskék attribútumait, már csak egy lépés van hátra, hogy mozgassuk is őket. Ehhez először ki kell számolni a rájuk ható eredő erőt.
Az eredő erő kiszámítását a következő képpen tettük mi meg:

Pszeudó kód:
```
for részecske in részecskék:
  eredő_erő := null_vec(d)
  for szomszéd in szomszédok:
    táv <= szomszéd és részecske közötti távolság
    irány <= szomszédból részecskébe mutató egységvektor
    if táv = 0: véletlen irányba állítjuk a részecske irányvektorát
    nyomás <= (részecske.nyomás + szomszéd.nyomás) / 2
    eredő_erő -= irány * nyomás * derivált_kernel / szomszéd.sűrűség
    eredő_erő -= közelségi_sűrűségből származó erő

  részecske.sebesség += eredő_erő / részecske.sűrűség * eltelt_idő
```
Ennek a ciklusnak a végeztével minden elemünk megvan, hogy végre mozgassuk a részecskéket.
==== A részecskék pozícióinak módosítása
Ez a lépés igazán triviális. Annyi a lényege, hogy végig iterálunk a részecskéken és mindnek először a sebességét módosítjuk, majd a pozíciójukat az eltelt idő függvényében.

Pszeudó kód:
```
for részecske in részecskék:
  részecske.sebesség += gravitáció * eltelt_idő
  részecske.pozíció += részecske.sebesség * eltelt_idő
  if szimulációs téren kivül a részecske.pozíció: visszahelyezés a szimulációs térbe
```

Fontos megjegyezni, hogy stabilitás szempontjából, az attribútumok számításánál érdemes a jósolt pozíciókkal számolni. Ez azt jelenti, hogy, i.e. a szomszédok megkeresésénél, azt a pozíciót vesszük, amit úgy kapunk, hogy a részecske jelenlegi sebességét eltelt idővel szorozva hozzáadjuk a pozícióhoz. Ezt nevezzük jósolt pozíciónak. Ezzel az értékkel csak az attribútumok számolásánál dolgozunk, amikor az utolsó ciklusban a részecskéket mozgatjuk, akkor az eredeti pozícióhoz adjuk hozzá az elmozdulást, nem pedig a jósolthoz. Ez a metódus nagy különbséget jelent, mivel enélkül, nem csillapodnak le, vagy nagyon nehezen a részecskék.
=== Fejlődés  
Az SPH, amit implementáltunk, javítható több helyen is. Egyik nagy lehetőség az az, hogy nem csak viszkozitást, hanem úgymond rugókat, valamint nyúlást is szimulálunk, amik mind segítenék a felületi feszültség akkurátusabb szimulálását. @pvfs

Fontos még azt is megjegyezni, hogy egyszerű szimulációs teret készítettünk csak a programnak, azaz, szilárd testet nem lehetne még beleejteni. Erre egy jó megoldás az úgy nevezett határ generálás mintavételezéssel. Ez a módszer azt takarja, hogy a szilárd test határánál is részecskék vannak egységesen elosztva, több rétegben, amiknek pont akkora lenne a sűrűsége, hogy ne menjen bele a folyadék részecske, azonban, nem akkora, hogy ne tudjon lenyugodni a test. Ennek a módszernek az implementálása egy érdekes kitérés lehet, amellyel már a folyadék szilárd testekkel való interakcióját is tudnánk szimulálni.

Ezeken kívül használhatnánk még más metódust az időléptetésre. Erre léteznek kiforrott, biztonságos algoritmusok, amik nagyobb időintervallumokat is relatíve jól lekezelnek.

== Optimalizáció

A program célja valós idejű folyadékszimuláció volt, így
ahhoz, hogy minél részletesebb szimulálást hajthassunk végre, növelnünk kellett a szimulált részecskék számát. Ennek eléréséhez különböző optimalizálási
stratégiákat kellett implementálni.

_A mérések egy AMD RX Vega 6 mobil integrált videokártyán készültek.
Nagy valószínűséggel modern dedikált videokártyákon a szimulálható részecskék
száma ezeknek a többszöröse lenne._

=== Optimalizáció nélkül GPU-n

A program első verziója nem tartalmazott optimalizációt, ezért a GPU sok felesleges
számítást végzett. Ennek ellenére így is $~5000$ részecskét lehetett
szimulálni valós időben, ami jó kiindulási pont volt.

Pszeudo kód:
```
for részecske_1 in részecskék:
  for részecske_2 in részecskék:
    a részecske_2-höz tartozó paraméterváltozások kiszámítása
    részecske_1 paramétereinek frissítése
```

#figure(
  image("imgs/no_opti.png", width: 80%),
  caption: [5240 részecske szimulálása 58 fps-en.]
)

Ezen a kódon elég könnyű gyorsítani, ha kihasználjuk azt, hogy a smoothing
kernel a sugarán kívül mindig 0-val tér vissza, azaz a smoothing radius-nél (sugárnál) távolabb levő részecskék nem
befolyásolhatják egy adott részecskének a paramétereit. Ezzel az egyszerű
javítással már $~7500$ részecskét is tudtunk értelmes gyorsaság mellett szimulálni.

Pszeudo kód:
```
for részecske_1 in részecskék:
  for részecske_2 in részecskék:
    ha távolság(részecske_1, részecske_2) <= smoothing_radius:
      a részecske_2-höz tartozó paraméterváltozások kiszámítása
      részecske_1 paramétereinek frissítése
```

#figure(
  image("imgs/radius_check.png", width: 80%),
  caption: [7522 részecske 60 fps-en.]
)

=== Cellákra bontás, hash tábla

Az előző részben javított algoritmus még mindig minden részecskét megvizsgál,
csak a nagy részét rögtön eldobja. A további optimalizálások a megvizsgált
részecskék számának csökkentésére fognak fókuszálni.

Gyakran használt megoldás@sph_tutorial, hogy a szimulált teret cellákra bontjuk és minden részecske csak a szomszédos cellákban levő részecskéket vizsgálja, így csak 3x3(x3) cella tartalmát kell megvizsgálni.

Az első megoldás egy vödrös hash szerű adatstruktúrát használ.
Ezzel a megoldással az volt a kihívás, hogy egy cellában a részecskék száma 0-tól a részecskék számáig terjedhet és az OpenGL nem támogat dinamikus memória foglalást. A probléma áthidalására egy megoldást ad az, ha a részecskéket kulcsaik szerint rendezzük, és egy segéd tömbben eltároljuk, hogy az egy kulcshoz tartozó részecskék mettől meddig tartanak. A kulcsokat egy generikus hash függvény@hash segítségével számoljuk ki.

#figure(
  caption: [
    Az adatstruktúra. A felső sorban a kulcsokhoz tartozó indexek, az alsóban sorban a részecskék kulcs szerint rendezett tömbje látható. Az indexek tömbjében az egyszerűség kedvéért van egy extra elem, ami a részecskék tömbje után mutat. Ha egy kulcshoz nem tartozik részecske, akkor a kezdő indexe megegyezik a következő kulcs indexével.
  ],
)[
  #cetz.canvas({
    import cetz.draw: *
    let particle_count = 150
    let indicies = (0, 20, 40, 40, 50, 65, 86, 86, 100, 105, 120, particle_count)
    let element_width = 0.1
    let particles_width = element_width * particle_count
    let indicies_width = indicies.len() * 1
    let particles_offset = (indicies_width - particles_width)/2
    grid((0, 2), (indicies_width, 3))

    for (i, idx) in indicies.enumerate() {
      line(
        (0.5 + i, 2),
        (particles_offset + 0.05 + element_width*idx, 1),
        mark: (end: ">")
      )
    }
    grid(
      (particles_offset, 0),
      (particles_offset + particles_width, 1),
      step: (x: element_width, y: 1)
    )
  })
]

Maga az algoritmus a vödrös rendezésnek egy párhuzamos változata, 3 részből áll:
+ A kulcshoz tartozó részecskék megszámolása és a részecsék kulcson belüli indexének kiszámítása (párhuzamosan, atomi számlálókkal)
+ Kulcshoz tartozó indexek kiszámolása (párhuzamos prefix sum@par_algs)
+ Részecskék áthelyezése a megfelelő helyre

Ez az algoritmus $O(n+k)$ extra memóriát igényel, ahol $n$ a részecskék száma
és $k$ a kulcsok száma. Az extra memóriahasználat nem probléma, mert általában
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

Ezzel az algoritmussal már $~55000$ részecske is szimulálható volt érdemi gyorsasággal.

Sajnos mivel a hash függvény nem tökéletes, van amikor különböző celláknak azonos kulcsa van (akkor is ha a kulcsok száma sokkal több, mint a cellák száma), így nem csak a szomszédos cellák részecskéit vizsgáljuk meg, hanem néhány extrát is.

#figure(
  image("imgs/cell_collision.png", width: 80%),
  caption: [
    54957 részecske 42860 kulccsal 60 fps-en. A 2 egyedülálló piros cella
    kulcsa ütközik az egyik ellenőrzött cella kulcsával, nagy kulcsszám ellenére is.
  ]
)

=== Szimulációs határok kihasználása

Mivel a szimuláció tengelyhez igazított síkokkal van határolva, könnyű
kiszámítani a cellák számát és könnyű kitalálni egy képletet, ami minden
cellához egyedi kulcsot rendel. Tehát ha legalább annyi kulcs van, mint cella, akkor ütközésmentes lesz a szomszédos cellák vizsgálata.

Eddig határon levő cellák vizsgálatakor a határon kívüli szomszédokat is
megvizsgáltuk, sőt mivel a szimuláció 3 dimenzióra lett megírva, 2 dimenzióban
18 cellát mindig feleslegesen vizsgáltunk.

Ezzel a kettő változtatással már $~80000$ részecskét lehet valós időben
szimulálni.

#figure(
  image("imgs/cell_no_collision.png", width: 80%),
  caption: [
    79131 részecske 5757 kulccsal 56 fps-en. Látható, hogy az alacsony kulcsszám ellenére sincs ütközés.
  ]
)

=== További optimalizációs módszerek

A szimuláció még az előző optimalizálások ellenére is túl sok felesleges
részecskét vizsgál meg. Egyenletes eloszlás esetén 2 dimenzióban $~65%$-ban, 3
dimenzióban $~84%$-ban felesleges a vizsgálat. Ezeket az arányokat a cellák
méretének csökkentésével és a részecske sugarán kívül eső cellák ignorálásával
lehetne javítani.

Ha a cellák (kulcsok) száma több nagyságrenddel nagyobb lenne, mint a
részecskék száma, akkor a vödrös rendezés helyett a
Bitonic sort-ot#footnote[https://en.wikipedia.org/wiki/Bitonic_sorter]
érdemes kipróbálni, mert annak a futásideje a részecskék számától függ, nem a
cellákétól.

Ha a részecskék és a cellák száma tovább növekedne, akkor a cellákat érdemes
lehet egy jobb lokalitású sorrendben rendezni (pl. 
Hilbert curve#footnote[https://en.wikipedia.org/wiki/Hilbert_curve],
Z-order curve#footnote[https://en.wikipedia.org/wiki/Z-order_curve]),
mert így cache barátabb lenne az iterálás.

== Vizualizáció

A részecskék jelenleg egyszerű billboardokkal vannak megjelenítve. A GUI-ban
lehet állítani, hogy mi szerint legyenek kiszínezve a részecskék. A GUI-hoz a
Dear ImGui#footnote[https://github.com/ocornut/imgui/]-t használjuk.

A folyadék vizualizációjához többféle módszert lehet a jövőben kipróbálni. Egy
elterjedt megoldás a folyadékot
metaballs#footnote[https://en.wikipedia.org/wiki/Metaballs]-al implicit
felületté alakítani, majd ezt a felületet
marching cubes#footnote[https://en.wikipedia.org/wiki/Marching_cubes]-al vagy sugárkövetéssel
megjeleníteni.

Egy modernebb megoldás lehet Gaussian splatting
#footnote[https://en.wikipedia.org/wiki/Gaussian_splatting]-et használni
Neural radiance field
#footnote[https://en.wikipedia.org/wiki/Neural_radiance_field]-ekkel, de ez
nagy valószínűseggel túl mutat a BSc-n.


= Eredmények

#figure(
  caption: [
    Egy "Dam Break" szimuláció részecskéinek sűrűsége (bal oldal)
    és sebessége (jobb oldal)
  ]
)[
  #grid(
    columns: 2,
    gutter: 1%,
    image("imgs/simu_density.png", width: 100%),
    image("imgs/simu_speed.png", width: 100%),
  )
]

#figure(
  image("imgs/two_densities.png", width: 80%),
  caption: [Két különböző sűrűségű folyadék],
)

#figure(
  image("imgs/3d_random.png", width: 80%),
  caption: [
    3 dimenziós szimuláció pár pillanattal inicializáció után (50000 részecske 27 fps)
  ]
)

#figure(
  image("imgs/3d_dam_break.png", width: 80%),
  caption: [3 dimenziós "Dam Break" szimuláció (50000 részecske 27 fps)]
)

= Összefoglalás

#todo[általános összefoglalás, jövőbeli tervek (önlab)]

#bibliography("references.yml")

