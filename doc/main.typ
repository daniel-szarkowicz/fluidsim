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

#todo[első iteráció: gpu-n opti nélkül]

#todo[második iteráció: cellákra bontás, generic hash, bucket sort @par_algs]

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

