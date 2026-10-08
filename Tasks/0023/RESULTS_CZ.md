# 0023 – Výsledek: kvalita planet (krátce)

## Země

- **Mapy:** ostrá skutečná 16K mapa povrchu (NASA, 500 m) místo dřívější 8K. Pobřeží je
  zhruba 2× ostřejší.
- **Mraky:** samostatná vrstva nad povrchem, se stínem na zemi a měkkými okraji bez
  „vystřižených fleků“.
- **Zdroj mraků:** pruhovaný zdroj jsem vyměnil za skutečný snímek počasí jednoho dne
  (satelity VIIRS, 16K). Švy mezi přelety, odlesky slunce i polární noc jsou opravené
  z dat druhého satelitu nebo druhého data.
- **Reliéf:** hory podle skutečných výškových dat (NOAA), viditelné ve světle, nejlépe při
  nízkém slunci.
- **Oceán:** odlesk slunce, odraz oblohy a modrý opar k okraji planety. Terminátor je měkký,
  v noci svítí jen města (Black Marble 2016) bez modrého podkladu.
- **Obrys:** jemnější koule, obrys planety je hladký i zblízka.

## Ostatní tělesa všech šesti soustav

- **Všechna těla:** všech 105 otexturovaných těles používá sdílené materiály podle typu:
  - pevná a ledová tělesa;
  - plynní obři bez kamenného reliéfu a s ztmavením okraje;
  - hvězdy jen září, mají ztmavení okraje a granulaci.
- **Nové skutečné mapy:** Io, Europa, Ganymedes a Kallisto v 8K, Enceladus ve 4K (USGS)
  místo map 1440 px. Měsíc a Mars mají reliéf podle skutečných výškových dat.
- **Fiktivní soustavy:** 9 světů s oblaky dostalo samostatnou vrstvu mraků. Barvy a ID
  zůstaly.
- **Jemný detail:** pod rozlišením mapy (kraje mraků, mikroreliéf, turbulence pásů,
  granulace) je výtvarný doplněk, ne skutečná geografie. Objeví se, jen když je mapa
  zvětšená.

## Co zůstalo beze změny

Schválený Daedalus, hvězdné body, černé pozadí, ovládání a rychlosti, R i Shift+R, kamera,
HUD a mapa. Bez rozmazání, časového AA i upscalingu, nativní 4K.

## Výkon (RTX 3070, 4K, celá soustava)

- **GPU:** 6–8 ms na snímek, prakticky stejně jako dřív.
- **Paměť textur planet:** 354 MiB → 1,12 GiB (balíček, Sluneční soustava). Hlavní podíl má 16K Země a lepší
  komprese všech map; pro 8 GB kartu je to v pořádku.

## Omezení

- Úplně blízko je pobřeží stále omezené 16K mapou. Dlaždicová varianta C nebyla dělána.
- Mraky jsou jeden skutečný den, ne průměr. Na ledu (Grónsko, Antarktida) mraky nejsou
  rozlišitelné, proto tam chybí.
- Malé měsíce Saturnu a Uranu, Triton, Phobos a Deimos mají dál původní menší mapy, jen
  s detailem v materiálu.

## Jak vyzkoušet

Balíček je v `A:/GPT-CODEX/Claude/worktrees/claude-0023/.local/solar/Build-PlanetQuality/Windows/Daedalus.exe`, nebo ho spusť přes `Tools/Invoke-SolarFlight.ps1 -Mode Play -BuildName Build-PlanetQuality` (vlastní data hráče, tvůj balíček ani savy se nemění).
