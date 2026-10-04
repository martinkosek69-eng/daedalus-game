# Vizuální standard: ostré 4K

Uživatelský požadavek z 4. října 2026. Platí pro Codex, Claude i další
pracovníky a pro všechen budoucí obsah a rozhraní hry.

- Výchozí cílový monitor je **32 palců, 4K (3840 × 2160)**. Celá hra musí
  podporovat ostré vykreslení minimálně v tomto nativním rozlišení, včetně
  scény, lodí, planet, hvězd, efektů, textu, HUD, nabídek a 3D mapy.
- Ve výchozím kvalitním režimu vykresluj na 100 % skutečného rozlišení okna/
  displeje. Automatické snížení rozlišení, dynamické rozlišení či zvětšení
  obrazu z nižšího rozlišení nesmí tiše nahrazovat nativní 4K. Nižší rozlišení
  okna zůstává podporované, ale není důkazem splnění 4K požadavku.
- Text, ikony a čáry rozhraní vytvářej v kvalitě odpovídající cílovým pixelům.
  Nezvětšuj hotový nízký obrázek celého HUD/mapy. Zohledni Windows DPI a
  změnu velikosti okna; samotné označení „4K“ nestačí.
- Na 32palcovém 4K monitoru používej kompaktní, dobře čitelné prvky. HUD nemá
  mít zbytečně obří čísla, tlačítka či panely. Využij prostor pro informace a
  přehlednou mapu, zachovej odstupy a umožni změnu velikosti rozhraní. Konkrétní
  velikost písma doladí uživatel; menší neznamená nečitelný.
- Detail scény musí obstát v běžně dostupném přiblížení i při oddálení.
  Volba zdrojů, geometrie, materiálů a filtrování musí odpovídat skutečnému
  záběru. Výchozí priorita je ostrost; rozmazání pohybem, hloubkou ostrosti
  nebo časovým filtrem ji nesmí znehodnotit. Přirozeně měkké jevy, například
  oblaka, nejsou důvodem pro rozpixelované nebo celkově rozmazané zobrazení.
- **4K výstup není příkaz udělat každou texturu 4096 × 4096.** Malá ikona
  může potřebovat méně, detail celé planety více nebo jiné řešení. Pouhé
  zvětšení souboru nevytváří nové detaily. Technologii volíme podle výsledku.
- Výkonové kompromisy zaváděj až podle změřeného problému; dokumentuj dopad
  na ostrost. Nenahrazuj schválený kvalitní výchozí režim tiše horší kvalitou.

Při relevantní vizuální změně zaznamenej skutečné rozlišení vykreslení, ne jen
rozměr následně zvětšeného snímku. Cíleně ověř text/HUD/mapu a měněný obsah
ve 3840 × 2160, při blízkém i vzdáleném pohledu podle změny. Kontrola ve 1440p
není ověření 4K; nedostupný skutečný 4K běh označ jako neověřený. Většinu
osobního vizuálního hodnocení provádí uživatel. Opakované dlouhé testovací sady
nejsou potřeba pro pouhý zápis pravidla.
