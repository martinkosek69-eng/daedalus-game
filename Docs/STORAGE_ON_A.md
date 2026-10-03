# Umístění pracovních dat

Projekt na tomto PC používá pro pracovní kopie a velká vývojová data disk A:.
Migrace pracovní kopie, Unreal cache a Blender nastavení byla dokončena;
kontrolní součty, Git, čtení/zápis a aplikace byly po přesunu ověřeny.
Původní záloha hry zůstala zachovaná. Případné návratové kopie odstraňujte jen
po ověření a příslušném souhlasu; přesné místní záznamy zůstávají soukromé.

## Nastavení jiného počítače

Každý počítač volí vlastní existující disk a adresáře. Místní cesty k instalacím,
TEMP/TMP a cache patří do ignorované `.local/toolchain.json` nebo příslušných
proměnných prostředí. [Vzor](../Tools/toolchain.example.json) obsahuje zástupné
hodnoty, které musí být nahrazeny platnými absolutními cestami. Volitelný objekt
`environment` lze odstranit, pokud má klient správné prostředí již nastavené.
Místní konfigurace se neposílá do GitHubu.

Pro Unreal lze nastavit `UE-LocalDataCachePath` a `UE-ZenDataPath`. Blender má
předvolby pro temporary directory, render/texture cache a render output.
Pro nové procesy mohou být potřeba restart klienta nebo nové přihlášení.
Ukládání modelu/scény musí vždy výslovně určit správný projektový soubor.

Pokud byly staré cesty zachovány NTFS junction odkazy, program může stále
vypisovat starou logickou cestu. Před čištěním ověřte skutečný cíl odkazu;
rekurzivní čištění přes odkaz může odstranit data na cílovém disku.
Windows, SDK a malé soubory klientských aplikací mohou zůstat na systémovém disku.

## Dokumentace výrobců

- [Codex worktrees](https://learn.chatgpt.com/docs/environments/git-worktrees):
  kořen nových pracovních kopií lze zvolit v nastavení aplikace.
- [Unreal Zen storage](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-zen-storage-server-as-cooked-output-store-for-unreal-engine):
  místní cesty a proměnné pro umístění cache.
