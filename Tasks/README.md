# Číslovaná zadání a předávání práce

Codex vede projekt a kontroluje, dokončuje a začleňuje výsledky. Claude nebo
jiný pomocník může převzít libovolný jednotlivý úkol. Role nejsou pevně rozdělené
podle herních oblastí. Uživatel rozhoduje o záměru a může zadání agentovi předat
krátkou zprávou v jeho chatu. Agenti si sami nesdílejí paměť konverzací.

V rámci uživatelem zadaného většího cíle Codex sám vybírá vhodnou dílčí práci,
rozsah, pořadí, závislosti a podmínky pro přijetí. Pomocník je příležitostný
subdodavatel; dostupnost jeho účtu není podmínkou dokončení projektu. Codex
zůstává odpovědný za celé původní zadání a při přerušení může práci převzít.
Delegování nemění uživatelův záměr ani neopravňuje překročit jeho původní rozsah.
Přímé automatické spouštění Claude Code zatím není propojené, proto uživatel
předává připravenou krátkou zprávu. Výsledné soubory se předávají přes GitHub.

## Kde najít úkol

Koordinátor přiděluje čísla v [INDEX.md](INDEX.md). Číslo má nejméně čtyři cifry:
zadání 26 je například `Tasks/0026/BRIEF.md`. Toto číslo je příklad, úkol 26
zatím není vypsaný. Šablony jsou v [Templates](Templates/BRIEF.md).

Každý skutečně vypsaný úkol má:

- `BRIEF.md`: cíl, vlastník, výchozí verze, povolené soubory, výstupy a kontroly.
- `PROGRESS.md`: poslední uložený stav, postup, problémy a konkrétní další krok.
- `HANDOFF.md`: výsledek, soubory, provedené kontroly a podklady pro převzetí.

Zadání a společný INDEX upravuje koordinátor. Pracovník ve vlastní větvi mění
PROGRESS, HANDOFF a výslovně přidělené výstupní cesty. Velká herní data patří
do sjednaných projektových složek, ne jako příloha textu zadání. Jednotlivé soubory
modelů a map mají vždy jednoho vlastníka.

## Spuštění a pracovní větev

Zpráva pro pomocníka musí obsahovat repozitář, číslo úkolu a větev nebo commit,
ve kterém zadání skutečně existuje. Dokud nové zadání není v main, nelze
předpokládat, že je pracovník najde po stažení main.

Pracovník nejprve přečte AGENTS.md, README.md, Docs/ENVIRONMENT.md, tento návod
a BRIEF svého úkolu. Ověří přístup k repozitáři a oprávnění nahrávat změny.
Vytvoří vlastní pracovní kopii na A: z předepsaného výchozího commitu a používá
větev uvedenou v zadání. Nepřepíná větev cizí pracovní kopie a nepřebírá
necommitované soubory z jiné otevřené relace.

Při nedostupném přístupu, chybějícím zadání, nepřidělené větvi či nejasném
rozměru potřebném pro práci zaznamená blokaci a vyžádá upřesnění. Pokud již
existuje vzdálená pracovní větev, nejprve zjistí aktuálního vlastníka; nové
založení stejné větve není povolení přepsat rozpracovanou práci.

## Průběžné ukládání

Ukládat lokálně znamená skutečně zapsat soubory, včetně .blend/.umap a potřebných
závislostí. Po uceleném kroku a před běžným ukončením relace pracovník:

1. Aktualizuje PROGRESS: co je hotové, co ne, co zkoušel a jak navázat.
2. Zkontroluje změněné soubory a vyloučí hesla, soukromé chaty a generovaná data.
3. Vytvoří commit s číslem úkolu a nahraje jej normálním push na pracovní větev.
4. Ověří, že vzdálená větev obsahuje daný commit. U LFS souborů navíc zkontroluje
   úspěšné odeslání potřebných objektů; samotný malý LFS ukazatel není asset.

V textu PROGRESS nemusí být SHA commitu, který PROGRESS právě vytváří: vznikl
by kruhový odkaz. Poslední commit se zjišťuje z historie pracovní větve.

GitHub je společné uložiště historie a předání. Nesynchronizuje každé stisknutí
klávesy ani neuloženou otevřenou scénu. Práci, kterou agent necommitoval a
neodeslal před náhlým vyčerpáním limitu, druhý agent nemusí mít k dispozici.
Proto ukládejte malé smysluplné kroky. Při chybě push výslovně napište, že práce
zůstala pouze lokální; pokud lze, uveďte bezpečné místo a poslední vzdálený stav.

Postup znamená stručný záznam provedených změn, použitých postupů, rozhodnutí
a ověřených výsledků. Nezveřejňujte interní myšlenkové stopy, kompletní osobní
chat ani surové výpisy s citlivými místními údaji. Tento repozitář je veřejný.

## Stav a předání

Pracovní stav v PROGRESS má hodnotu `NOT_STARTED`, `IN_PROGRESS`,
`BLOCKED`, `PAUSED` nebo `READY_FOR_REVIEW`. PAUSED zde označuje přerušený úkol,
nikoli ovládání automatických cílů nebo plánovače kteréhokoli AI klienta.
Pracovník může připravit výsledky k revizi, ale nemůže sám označit úkol za
ACCEPTED. Pouze koordinátor po provedených kontrolách aktualizuje INDEX na
ACCEPTED, případně NEEDS_CHANGES nebo CANCELLED.

Při dokončení nebo přerušení pracovník doplní HANDOFF a odešle poslední checkpoint.
Předá uživateli číslo úkolu, pracovní větev a poslední dostupný commit. Pokud
klient umí vytvořit GitHub návrh změn (pull request), použije pracovní větev jako
zdroj a cílovou větev ze zadání; jinak stačí odeslaná větev a její identifikace.
Přijetí a sloučení provádí koordinátor. Nikdo nevynucuje push ani neslučuje
pracovní výsledek přímo do main.

Codex při převzetí stáhne aktuální větev, zkontroluje obsah a původ assetů,
měřítko, názvy, potřebné závislosti, funkčnost a přiměřený výkon. Výsledky
ověří v vlastní určené kopii a v odpovídajícím editoru. Doplní nutné opravy a
teprve po kontrole začlení změny do hlavní hry. Hotový samostatný model ještě
není důkaz, že vše funguje ve hře.

Převzetí nedokončené práce je běžné. Koordinátor nejprve výslovně ukončí původní
přidělení a předá vlastnictví dalšímu agentovi. Nový pracovník naváže na poslední
odeslaný commit a HANDOFF. Dva agenti se nesmějí považovat za vlastníka stejného
úkolu zároveň. Změna společných měřítek či rozhraní musí být nejprve dohodnutá.

## Praktické omezení a ověření

Pravidla v dokumentu nejsou samostatný technický zámek. Oddělené pracovní
složky chrání běžné editace; jeden vlastník editoru a binárních souborů vyžaduje
výslovnou koordinaci. LFS zámky a přenos reálných binárních výsledků se musí
ověřit před jejich souběžnou výrobou. Na tomto PC používejte jeden Unreal editor
a jednu plnou kompilaci najednou, podle AGENTS.md.

Idle agent se o novém push sám nedozví. Uživatel může napsat „Převezmi úkol 26“;
Codex potom stáhne poslední stav. Automatické sledování větví není zatím zapnuté.
Agent na jiném počítači potřebuje vlastní programy a přihlášení. Agent bez
místního vykonávání nemá přes GitHub přístup k našemu Blenderu ani Unreal editoru.

Úkol [0001](0001/BRIEF.md) je první malá zkouška předání přes GitHub.
Nevytváří herní obsah a sám neověřuje všechny nástroje ani LFS přenos modelu.
