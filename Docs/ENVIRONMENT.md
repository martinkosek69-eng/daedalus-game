# Vývojové prostředí a spolupráce agentů

Technický základ pro Codex, Claude Code a další místní MCP klienty. Herní
architektura má základ popsaný v [ARCHITECTURE.md](ARCHITECTURE.md); kreativní design zůstává otevřený. Výsledky revize prostředí jsou v
ENVIRONMENT_AUDIT_2026-10-03.md.

## Společné nástroje

Unreal je engine a editor. Blender vytváří editovatelné modely přes `bpy`.
Visual Studio dodává C++ kompilátor, linker a MSBuild. GitHub uchovává zdroje
a historii; Git LFS spravuje binární projektové soubory. GitHub sám neovládá
běžící editor a nepřenáší paměť konverzací mezi agenty.

`Tools/apps-mcp.mjs` je verzovatelný místní stdio MCP server bez npm závislostí.
Každý klient může spustit vlastní instanci. Instalace čte z
`.local/toolchain.json` (vzor `Tools/toolchain.example.json`) nebo z proměnných
`DAEDALUS_BLENDER`, `DAEDALUS_STUDIO` a `DAEDALUS_ENGINE`.
Projekt vždy odvodí z umístění vlastního skriptu. Nesestavuje automaticky jinou
pracovní kopii na A:, pokud byl spuštěn ze zdejšího worktree.
Volitelná položka `environment` v místním JSON předává TEMP/TMP a cache nastavení
spouštěným programům, i když běžící AI klient má ještě staré systémové prostředí.

Oba klienti přímo spouštějí Node helper. Node musí být na jejich PATH; pro tento
PC musí být vhodný runtime dostupný přes uživatelský PATH. Spouštějte klienta
z kořene repozitáře, protože cesta ke skriptu v konfiguraci je relativní.
Při jiném pracovním adresáři nastavte v místní konfiguraci klienta absolutní
cestu k helperu a správný pracovní adresář. Nové PATH načtou nové procesy.

Codex má konfiguraci v `.codex/config.toml`; Claude Code v `.mcp.json`.
Codex používá `AGENTS.md`. `CLAUDE.md` importuje tento stejný soubor, aby
nevznikaly dvě rozdílné sady pravidel. Registrace konfigurace vyžaduje načtení
novou relací/restart MCP klienta. Oprava souboru již běžící Node proces nezmění.

Unreal MCP běží v otevřeném editoru na loopback adrese. Je experimentální;
seznam nástrojů není zárukou funkčnosti všech operací. V této revizi fungovaly
čtení a změny kamery, vytvoření/odstranění objektu, PIE a snímek viewportu.
Před použitím načtěte skutečné schéma. CaptureViewport v této instalaci potřebuje
explicitní captureTransform a annotations, přestože je schéma označuje jako
volitelné. Ověřený snímek používal aktuální transformaci, bShowUI=false a
annotations s nulovou mřížkou/popisnými štítky a classFilter=/Script/Engine.Actor.

Když nativní Unreal nástroje nejsou v aktuální relaci načtené, stejný lokální
server lze obsloužit `Tools/Unreal-Mcp.ps1`. Tento skript provede handshake,
předá požadavek a kontroluje chyby; nenahrazuje klientská oprávnění.

Blender spojení pracuje na uložených souborech v procesu na pozadí. Ovládání
neuložené GUI scény a ruční klikání v Blenderu není tímto řešením zajištěno.
Visual Studio spojení je ke kompilaci, nikoli k živému GUI/debuggeru. Pro
opakovatelný vývoj jsou tyto dvě schopnosti vhodným základem; další lze doplnit
a ověřit až pro konkrétní potřebu.

## Dva agenti

Codex vede projekt a přebírá, upravuje a integruje výsledky. Claude nebo jiný
pomocník dostává jednotlivé úkoly podle dostupnosti; nemá trvale přidělený svět
ani lodě. Číslovaná zadání, průběžné ukládání a převzetí popisují
[Tasks/README.md](../Tasks/README.md) a [Tasks/INDEX.md](../Tasks/INDEX.md).
Před zahájením pracovní kopie ověřte dostupnost společných pokynů v konkrétním
publikovaném commitu. Lokální necommitované soubory se novému agentovi nepředají.

Každý agent má vlastní větev a pracovní kopii, nejlépe na A:. Přidělujte konkrétní
soubory nebo úkoly. Větev sama nechrání společné soubory ani otevřený editor.

Pro výchozí místní spolupráci používejte jeden Unreal editor a jednu plnou kompilaci
současně. Druhý agent může pracovat na dokumentaci, datech či jiných souborech.
Ovládání editoru si předejte výslovně: vlastník, checkout, uproject, port, úkol.
Před převzetím ověřte cestu projektu z procesu/editorového logu, ne pouze to,
že port odpovídá. Na společný editor neposílejte překrývající se MCP požadavky.

Oddělené editory mohou později používat různé porty přes
`-ModelContextProtocolPort=N` a odpovídající klientské URL. To není v této revizi
testované ani doporučený výchozí režim pro tento projekt.

Binární .umap, .uasset a .blend soubory nemají běžné textové slučování.
Přidělte každému jednoho vlastníka. Git LFS již používáme; jeho serverové zámky
a skutečný přenos LFS objektu na GitHub zatím ověřené nejsou. Před souběžným
vývojem obsahu doplňte a otestujte zámky nebo důsledné přidělování souborů.

Výsledky předávejte přes změny ve větvi a kontrolu před sloučením do main.
Předání obsahuje: provedenou změnu, soubory, výsledky testů a otevřené problémy.
Přístup k veřejnému repozitáři neuděluje druhému účtu právo push. Na jiném účtu
ověřte přihlášení a oprávnění zvlášť. Na jiném PC nainstalujte nástroje a
vytvořte jeho vlastní místní konfiguraci. Cloudový agent sám nemá přístup k
127.0.0.1 na vašem PC; pro práci s editorem potřebuje místní vykonávání.

## Co ukládat do GitHubu

- C++, konfigurace, uproject, vlastní pluginy, skutečný Content.
- Zdrojové Blender soubory, potřebné exporty, textury a zvuky přes LFS.
- Opakovatelné skripty, testy, návody, verze nástrojů a technická rozhodnutí.
- Společné agentní pokyny, stručný stav projektu a původ/licence použitých assetů.

Neukládejte instalace Unreal/Blender/VS, Node binárku, cache, Intermediate,
Binaries, Saved, lokální nastavení, hesla a osobní exporty konverzací.
Do veřejného repozitáře patří relevantní veřejný kontext, nikoli automaticky
celá soukromá historie. Původní modely předlohy přidávejte až po výběru a
posouzení jejich podmínek; aktuální revize je nenahrála.

GitHub a místní worktrees nejsou nezávislá záloha všeho. Záloha původní hry
na A: je návratový bod. Pro nový projekt je vhodná další kopie na jiném médiu,
která zahrne i lokální podklady potřebné k obnově. Obnovu otestujte.

## Kontroly

`node Tools/Test-AppsMcp.mjs Tools/apps-mcp.mjs` z kořene, se zavřeným Unrealem,
ověří MCP, build, Blender save/reload, GLB export, render a předání chyb.
Výstupy jsou v ignorované `.local`. Pokud Node není na PATH, použijte jeho
explicitní cestu z místního nastavení. Skript vyžaduje nastavený toolchain.

`Tools/Test-BlenderImport.py` lze spustit přes UnrealEditor-Cmd s parametry
`-run=pythonscript -script=<absolutní cesta skriptu> -Unattended -NullRHI`.
Používá předchozí `.local/audit-probe.glb`, importuje vlastní dočasné assety
do `/Game/__EnvironmentAudit__`, zkontroluje StaticMesh a assety odstraní.
Pokud cílová složka již existuje, odmítne ji přepsat. Je to test základní geometrie,
nikoli potvrzení všech materiálů, animací a herních modelů.

Instalační soubory a pracovní data na tomto PC směrujte na A:.
Unreal DDC/Zen, pomocná data kompilace a Blender nastavení/cache již byla
přesměrována na A: a otestována. Uživatelské TEMP/TMP jsou nastavené na A:;
staré procesy je načtou až po novém přihlášení. Přesun Codex worktree se dokončil.
Po restartu bylo ověřeno čtení, zápis a zachování stavu Gitu. Přesné místní cesty
a návratové kopie jsou zdokumentované v soukromých záznamech mimo GitHub.
Podrobnosti a ověření jsou v [STORAGE_ON_A.md](STORAGE_ON_A.md).
Systémové komponenty a malé soubory klientských aplikací na C: přetrvají.

## Oficiální dokumentace

- [Unreal MCP](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-mcp-in-unreal-editor?application_version=5.8): kompatibilní klienti, serializace požadavků, porty a experimentální stav.
- [Codex MCP](https://learn.chatgpt.com/docs/extend/mcp?surface=cli): stdio/HTTP a projektová konfigurace.
- [Codex AGENTS.md](https://learn.chatgpt.com/docs/agent-configuration/agents-md): společné projektové pokyny.
- [Claude Code MCP](https://code.claude.com/docs/en/mcp): projektová .mcp.json a klientská oprávnění.
- [Claude Code memory](https://code.claude.com/docs/en/memory): import společných pokynů přes CLAUDE.md.

