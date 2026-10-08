# 0022: Proč jsou planety rozmazané a co navrhuji (shrnutí)

## Co je problém

Na tvém snímku z oběžné dráhy je Země asi 5–6× „roztažená“. Jeden bod mapy
Země (asi 5 km) zabírá 5–6 pixelů obrazovky, a víc detailu v mapě prostě není.
Připojuje se k tomu několik dalších věcí:

- **Mraky jsou namalované přímo na zemi.** Nejsou jako samostatná vrstva, nemají
  stín ani hloubku. Zdrojová mapa mraků má nad Británií pruhovaný „kartáčovaný“
  vzor, a ten se po roztažení propíše i přes pevninu. Ověřil jsem to: když
  zdrojové mapy složím stejně jako hra, vyjde přesně stejný vzor jako na snímku.
- **Komprese textur.** Textury se ukládají v jednoduchém formátu po čtverečcích
  4×4 bodů. Při roztažení z toho vznikají kostičky na okrajích mraků.
- **Plochost.** Planeta nemá reliéf, odlesk moře ani stín mraků a barvy jsou
  přesycené. Přechody ostrosti k horizontu skáčou.

**Přegenerovat všechno do 4K nepomůže.** Země už má 8K, takže 4K by ji zhoršilo.
Zblízka nepomůže ani 16K, to by bylo pořád asi 3× roztažené. Vyšší rozlišení
pomůže jen tělesům, která mají dnes mapy velmi malé (některé měsíce 1440 bodů,
Uran a Neptun 2K).

## Co navrhuji (varianta B)

Chytřejší materiál planety ve vrstvách místo pouhého zvětšování obrázků:

1. **Mraky jako samostatná vrstva nad povrchem.** Mají vlastní stín a jejich
   okraje zostřuje jemný generovaný detail. Ostré jsou i zblízka, velké tvary
   zůstávají podle skutečných dat.
2. **Reliéf.** U Země, Měsíce a Marsu podle skutečných výškových map, u ostatních
   jemný generovaný detail. K tomu odlesk slunce na moři.
3. **Kvalitnější komprese a plynulé přechody ostrosti.** Bez kostiček a bez skoků
   k horizontu. Nastavení schválené lodi zůstane beze změny.
4. **Jemnější koule** bez hranatého okraje planety.

Funguje to pro skalnaté, ledové i plynné světy, měsíce i tvé vymyšlené soustavy.
Pokud by pak Země zblízka pořád nestačila, dá se pro ni přidat ostřejší mapa
16K (varianta A). Úplně nejostřejší skutečná geografie (varianta C) by pro Zemi
znamenala velká data a hodně práce.

## Jaký rozdíl uvidíš

- Mraky ostré a oddělené od země, se stínem, bez pruhů a kostiček.
- Viditelné hory a odlesk moře, přirozenější barvy.
- Pobřeží zůstane omezené mapou: s B o něco čistší, s 16K Zemí asi 2× ostřejší.

## Nevýhody

- Generovaný jemný detail je výtvarný doplněk, ne skutečná geografie.
- Paměť pro Zemi vzroste asi ze 64 na 130 MB, s 16K mapou asi na 300 MB.
- Práce je středně náročná: materiál, vrstva mraků a příprava výškových dat.

## Co od tebe potřebuji

1. Vybrat variantu: doporučuji **B**, případně B a k ní 16K Zemi.
2. Souhlas s prvním ověřením jen na Zemi, se 4 až 5 verzemi ze stejného záběru,
   jako je tvůj snímek. Teprve potom se změny rozšíří na ostatní tělesa.
3. Pokud ji znáš, informaci o grafické kartě (kvůli rozpočtu paměti).
