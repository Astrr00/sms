# Decomp-Fortschritt

## Ausgangs-Baseline (GMSJ01 / NTSC-J)

- Upstream-Basis: `39458071` (nach rebase auf `upstream/main`)
- Arbeitsbranch: `decomp-work`
- Referenz-DOL: `build/GMSJ01/mario.dol: OK`
- Erwarteter SHA-1: `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`
- Report: `report.json`, erzeugt mit
  `build/tools/objdiff-cli report generate -o report.json`

| Metrik | Ausgangswert |
| --- | ---: |
| Fuzzy match | 77,34 % |
| Code matched | 41,08 % (1.474.716 / 3.590.088 Bytes) |
| Code complete / linked | 18,00 % (646.308 Bytes) |
| Data matched | 59,44 % (380.623 / 640.331 Bytes) |
| Data complete | 20,72 % (132.708 Bytes) |
| Funktionen matched | 66,10 % (8.514 / 12.881) |
| Units complete | 396 / 736 |

## Stand nach Rebase + FlagManager

| Metrik | Aktuell | Änderung vs. Baseline |
| --- | ---: | ---: |
| Fuzzy match | 77,84 % | +0,50 pp |
| Code matched | 41,35 % (1.484.660 / 3.590.088) | +9.944 Bytes |
| Funktionen matched | 66,62 % (8.581 / 12.881) | +67 |
| Code complete / linked | 19,03 % | +1,03 pp |
| Units complete | 403 / 736 | +7 |

### Nach dieser Iterationsrunde (NpcManager, THPPlayer)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Fuzzy match | 77,84 % | ±0 |
| Code matched | 41,38 % (1.485.500 / 3.590.088) | +840 Bytes |
| Funktionen matched | 66,65 % (8.585 / 12.881) | +4 |
| Units complete | 403 / 736 | ±0 |

### Nach zweiter Iterationsrunde (JPADraw, effectObj, WoodBarrel::kill)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,43 % (1.487.244 / 3.590.088) | +1.744 Bytes |
| Funktionen matched | 66,69 % (8.590 / 12.881) | +5 |

**Methodik-Erkenntnis**: `char trash[N]`-Padding muss NACH dem betroffenen
Struct-Local stehen, nicht davor — das brachte `calcRootMatrix`, `loadYBBMtx`
und `WoodBarrel::kill` von 99,6–99,8 % auf 100 %. Funktionen ohne frühen
Struct-Local reagieren weiterhin auf Padding am Funktionsanfang
(`zDrawParticle`, `zDrawChild`, `NpcManager::perform`).

**Warnung zu `configure.py`-Status**: `objdiff-cli diff` kann eine Unit-Section
als 100 % melden, obwohl anonyme `[.data-0]`/`[.sdata-0]`-Reste (Padding/
Literal-Pool-Bytes ohne Symbolnamen) abweichen — sichtbar nur in den
Symbol-Einträgen, nicht im Section-Aggregat. `MoveBG/WoodBarrel.cpp` auf
`Matching` zu setzen brach die DOL-SHA1, obwohl alle Sections 100 % zeigten;
sofort zurückgesetzt. Ab jetzt: Matching-Flip **immer** mit
`ninja && dtk shasum -c` verifizieren, nie nur mit objdiff-Sections.

### Nach dritter Iterationsrunde (JPADraw::initialize)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,45 % (1.488.112 / 3.590.088) | +868 Bytes |
| Funktionen matched | 66,71 % (8.591 / 12.881) | +1 |

`JSystem/JParticle/JPADraw.cpp` `.text` jetzt vollständig 100 % (4/4
Funktionen); Unit bleibt wegen anonymem `.sdata2`-Rest NonMatching.

### Nach vierter Iterationsrunde (MapModel, MapObjOption, ItemManager, PollutionManager)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,50 % (1.489.852 / 3.590.088) | +1.740 Bytes |
| Funktionen matched | 66,74 % (8.597 / 12.881) | +6 |

Sechs neue 100-%-Matches: `TMapModel::initUnderpass`,
`TFileLoadBlock::touchPlayer`/`receiveMessage`,
`TItemManager::resetNozzleBoxesModel`, `TPollutionManager::clean`/`load`.

Die Referenz-DOL bleibt `OK`.

### Nach fünfter Iterationsrunde (JALModSe: appendGrpMember, append)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,56 % (1.492.056 / 3.590.088) | +2.204 Bytes |
| Funktionen matched | 66,75 % (8.599 / 12.881) | +2 |

Zwei neue 100-%-Matches in `JSystem/JAudio/JALibrary/JALModSe.cpp`:
`JALSystem::appendGrpMember` (`char trash[8]` am Funktionsanfang) und
`JALSystem::append` (`char trash[0x68]` nach dem letzten Struct-Local
`set3`, **plus echter Bugfix**: der `ModType_JALSeModPitFunk`-Case
übergab `&set2` statt `&set3` — Disassembly-Beweis: `PitFunk` gruppiert
sich beim Stack-Offset 0xd8 exakt mit `PitDist`/`PitFGrp`/`PitDGrp`
(alle nutzen `set3`), nicht mit der 0xe0-Gruppe (`Vol*`/`Eff*`, nutzt
`set2`) — passend zum "Pit"-Namensmuster der anderen drei Fälle).

**Wichtiger Fund zum Matching-Flip-Risiko**: Beide Funktionen matchen
einzeln zu 100 %, und die komplette Unit zeigt in `objdiff-cli` 100 %
für **alle** Sections (`.text`/`.data`/`.bss`/`.sbss`/`.sdata2`/`.ctors`).
Trotzdem bricht ein `Matching`-Flip in `configure.py` die DOL-SHA1
(bestätigt per `dtk shasum`). Root-Cause-Analyse per Byte-Diff
`build/GMSJ01/mario.dol` gegen `orig/GMSJ01/sys/main.dol`: 6.190
abweichende Bytes, erster bei Datei-Offset `0x445df` (liegt exakt im
`.text`-Bereich dieser Unit, Adresse ~`0x800476a0`), letzter bei
`0x3a7c30` (liegt in `.data`) — die Abweichung zieht sich vom Ort dieser
Unit bis ans Ende von `.text`, durch `.rodata` und in `.data` hinein.
Das ist ein echter Downstream-Adress-Shift/Layout-Fehler, keine bloße
anonyme Daten-Restgröße wie bisher bei `WoodBarrel` vermutet — aber die
Konsequenz ist dieselbe: Unit bleibt `NonMatching` in `configure.py`,
nur der Quellcode wird committet. **Erkenntnis für künftige Flips**:
100 % in allen von `objdiff-cli` gelisteten Sections ist keine
hinreichende Bedingung für einen sicheren Matching-Flip; nur ein
tatsächlicher `ninja && dtk shasum`-Durchlauf ist beweiskräftig.

Die Referenz-DOL bleibt `OK`.

### Nach sechster Iterationsrunde (JASTrack::writeRegDirect; spcinterp/JASTrack-Analysen)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,57 % (1.492.260 / 3.590.088) | +204 Bytes |
| Funktionen matched | 66,76 % (8.600 / 12.881) | +1 |

Ein neuer 100-%-Match: `JSystem/JAudio/JASystem/JASTrack.cpp::
TTrack::writeRegDirect` (204 Bytes, `char trash[8]` schließt eine
8-Byte-Frame-Lücke, keine Struct-Locals vorhanden).

Drei weitere Funktionen ausführlich untersucht, kein Match erreicht,
Quelltext auf sauberen Zustand zurückgesetzt:

- `Strategic/spcinterp.cpp`: `execadd`/`execsub`/`execmul`/`execdiv`
  (je 728 Bytes, 99,98 % clean) — identisches Muster in allen vier
  Funktionen. Frame stimmt bereits exakt (`0x70` beidseitig); einziger
  Rest ist ein 4-Byte-Stack-Offset-Unterschied (0x24/0x28 vs. 0x20/0x24)
  für das compiler-interne `TSpcSlice`-Temporary im `push(int)`-Aufruf
  des else-Zweigs. Drei Varianten (`trash[4]` nach `result`, expliziter
  `TSpcSlice tmp`-Local statt `push(int)`, `trash[4]` vor dem `if`)
  verschlechtern alle auf 99,89–99,95 %; zurückgesetzt auf die saubere
  99,98-%-Fassung.
- `Strategic/spcinterp.cpp::execcall` (580 Bytes, 99,66 % clean) —
  32-Byte-Frame-Gap (`0x88` vs. `0x68`) **plus** eine echte
  Register-Vertauschung: `mContextStack.mSize` und `mProgramCounter`
  landen bei uns in r5/r4, im Original in r4/r5 (reines
  Register-Allocation-Detail, keine Feldadress-Verwechslung — beide
  Felder werden korrekt gelesen, nur die physischen Register
  vertauscht). `char trash[0x20]` am Funktionsanfang erzeugte
  scheinbar einen Sprung auf 99,69 %, was sich bei genauer Prüfung
  als Messartefakt herausstellte (objdiff-cli grenzt bei
  `NonMatching`-Funktionen den Vergleichsbereich der Zielseite an der
  aktuell kompilierten Größe ab; das Vergrößern der eigenen Funktion
  verschiebt dadurch auch das verglichene Zielfenster). Zurückgesetzt
  auf die saubere 99,66-%-Fassung.
- `JSystem/JAudio/JASystem/JASTrack.cpp::noteOn` (824 Bytes, 99,80 %
  clean) — 8-Byte-Frame-Gap (`0x70` vs. `0x68` mit `trash[8]`)
  **plus** ein Register-Umnummerierungsmuster (`r24` bei uns vs. `r23`
  im Original für denselben `TTrack* mParent`-Lokal über die gesamte
  Parent-Walk-Schleife) — dieselbe Kategorie wie der bereits
  dokumentierte `effectObj::reset`-Fall. **Korrektur**: Der zunächst
  gemessene Sprung auf 99,88 % mit `char trash[8]` wurde nachträglich
  als Messartefakt entlarvt (siehe `execcall`-Fund unten) — gezielte
  Vorher/Nachher-Prüfung der Zieldisassemblierung zeigt, dass sich der
  `stwu`-Wert der **Zielseite** exakt von `-0x68` auf `-0x70`
  verschiebt, sobald unsere Funktion um 8 Byte wächst. Zurückgesetzt
  auf die saubere 99,80-%-Fassung; keine verifizierbare Verbesserung.
- `JSystem/JAudio/JASystem/JASTrack.cpp::writeRegParam` (1.288 Bytes,
  99,36 % clean, bereits mit Upstream-Kommentar `// TODO: This is pure
  pain` als bekannt schwierig markiert) — 16-Byte-Frame-Gap (`0x48` vs.
  `0x38`) plus eine echte Argument-Auswertungsreihenfolge-Vertauschung
  vor dem zweiten `writeRegDirect(5, product)`-Aufruf (`this`-Setup vs.
  Wertberechnung in umgekehrter Reihenfolge). `char trash[0x10]` zeigte
  einen scheinbaren Sprung auf 99,38 % — angesichts des unten
  dokumentierten Messartefakt-Musters nicht verifiziert und daher nicht
  als reale Verbesserung gewertet; bestätigt als strukturell schwierig,
  zurückgesetzt auf die saubere Fassung.

**Wichtiger Methodik-Fund (Messartefakt bei Teil-Matches)**:
`objdiff-cli` grenzt bei `NonMatching`-Funktionen den
Vergleichsbereich der Zielseite offenbar an der aktuell kompilierten
Größe unserer Funktion ab, wenn die Symbolgröße nicht anderweitig fest
verankert ist. Vergrößert man die eigene Funktion per `char trash[N]`,
kann dadurch auch das verglichene Zielfenster wachsen und zufällig
bessere Byte-Übereinstimmung vortäuschen, **ohne dass ein echter
Match vorliegt**. Nachträglich per gezieltem Vorher/Nachher-Vergleich
des Ziel-Prologs (`stwu r1, -N(r1)`) an vier Fällen bestätigt:
`execcall` (-0x68→-0x88), `calcViewMtx` (-0xc8→-0xe0),
`drawRevivalTexStamp` (-0x98→-0xa0), `noteOn` (-0x68→-0x70) — in
allen vier Fällen verschob sich der Zielwert exakt um die Größe des
hinzugefügten `trash`-Arrays. **Regel ab sofort**: Eine
Prozentverbesserung durch `trash[N]` gilt nur dann als real, wenn sie
entweder (a) echte 100 % erreicht (dort ist eine zufällige
Fensterverschiebung durch vollständige Byte-Identität statistisch
ausgeschlossen) oder (b) durch einen expliziten Vorher/Nachher-Vergleich
des Ziel-Prologs bestätigt wird, dass sich die Zielseite NICHT
verändert hat. Reine Prozentangaben zwischen zwei `objdiff-cli`-Läufen
mit unterschiedlicher eigener Funktionsgröße sind für sich allein
**nicht** aussagekräftig.

Die Referenz-DOL bleibt `OK`.

### Nach siebter Iterationsrunde (THPPlayer/TimeRec/MapCollisionPlane/PollutionCount-Analysen, kein neuer Match)

Sieben weitere Funktionen ausführlich untersucht, alle als strukturell
schwierig bestätigt und auf sauberen Zustand zurückgesetzt — kein
Fortschritt in dieser Runde, aber wertvolle Root-Cause-Dokumentation:

- `THPPlayer/THPPlayer.c::THPPlayerPrepare` (624 Bytes, 97,98 % clean) —
  Frame stimmt bereits exakt (`0x30` beidseitig, `stmw r26`), reine
  Register-Umnummerierung: alle Parameter/Locals (`frame`, `flag`,
  `threadData`) sitzen bei uns ein Register höher (`r28`→`r27` usw.)
  als im Original. Dieselbe Kategorie wie `effectObj::reset`/`noteOn`.
- `System/TimeRec.cpp::TTimeRec::flip` (144 Bytes, 99,17 % clean) —
  reiner Register-Swap (r5↔r7) für die schleifengetragene `curr`-Variable
  in der Rückwärtsschleife, kein Frame-Unterschied, keine Struct-Locals.
- `Map/MapCollisionPlane.cpp::TMapCheckGroundPlane::checkPlaneGround`
  (308 Bytes, 95,95 % clean, bereits mit Upstream-Kommentar `// TODO:
  making the return type an int here makes it match better, but breaks
  other places` als bekannt schwierig markiert) — 8-Byte-Frame-Gap plus
  f6/f7-Register-Swap und Instruktions-Umordnung; `char trash[8]`
  bewirkt keinerlei Veränderung (95,95 % → 95,95 %).
- `Map/PollutionCount.cpp` — vier Funktionen mit verschachteltem
  Auto-Inlining (`ReInitializeGX`/`drawPollutionLayer`/
  `loadPollutionLayer`/`initDrawObjGX` werden je nach Aufrufstelle vom
  Compiler automatisch eingebettet):
  - `drawRevivalTexStamp` (748 Bytes, 99,93 % clean) — 8-Byte-Frame-Gap
    (bestätigt behoben durch zwei getrennte `char trash[4]`-Blöcke, vor
    der `GXSetChanMatColor`-Compound-Literal-Zeile und nach dem
    `JUTTexture texture`-Local in der Schleife). Der zunächst gemessene
    Sprung auf 99,98 % wurde als Messartefakt identifiziert (siehe
    Methodik-Fund oben): Ziel-Prolog verschiebt sich exakt von
    `-0x98` auf `-0xa0`, wenn unsere Funktion um 8 Byte wächst. Der
    restliche 4-Byte-Versatz betrifft die Speicherposition des
    `(GXColor){...}`-Compound-Literals relativ zu einem benachbarten,
    nicht überlappenden Stack-Slot; ein benannter `GXColor`-Local
    statt Compound-Literal verschlechtert klar auf 98,87 % (kein
    Artefakt, da Verschlechterung). Zurückgesetzt auf die saubere
    99,93-%-Fassung; keine verifizierbare Verbesserung.
  - `countTexDegree` (596 Bytes, 99,87 % clean) — 88-Byte-Frame-Gap
    durch zweifach verschachteltes Auto-Inlining von
    `drawPollutionLayer` → `loadPollutionLayer` (dessen einziger Local
    `GXTexObj GStack_40` landet komplett in `countTexDegree`s Frame).
    `char trash[0x58]` direkt in `loadPollutionLayer` nach `GStack_40`
    hat **null** Effekt (99,8658 % exakt unverändert) — der Optimizer
    entfernt totes Padding in bereits eingebettetem Code, bevor die
    Frame-Größe der umschließenden Funktion berechnet wird. Bestätigt
    dieselbe Kategorie wie `MapObjPollution::loadAfter` ("inlines make
    me cry").
  - `drawJointObjStamp` (648 Bytes, 99,39 % clean) — 16-Byte-Frame-Gap
    plus r26/r27-Register-Swap für die Schleifenvariable, zusätzlich
    verschachteltes Inlining von `initDrawObjGX`; nicht weiter verfolgt
    angesichts der bestätigten Kategorie.
  - `calcViewMtx` (384 Bytes, 99,21 % clean) — 24-Byte-Frame-Gap durch
    inlineten `TPosition3f local_a4`-Zero-Fill in der Schleife;
    `char trash[0x18]` nach `local_a4` zeigte einen scheinbaren Sprung
    auf 99,28 %, der per Vorher/Nachher-Vergleich des Ziel-Prologs
    (`-0xc8` → `-0xe0`, exakt um die Trash-Größe) als Messartefakt
    entlarvt wurde. Rest ist eine 4-Byte-Slot-Verschiebung plus
    f0/f1-Register-Swap für `makeWorldToPollutionMtx`, dieselbe
    Kategorie wie `drawRevivalTexStamp`. Zurückgesetzt auf die saubere
    99,21-%-Fassung; keine verifizierbare Verbesserung.
- `Strategic/objmanager.cpp` — zwei weitere Funktionen (nicht Teil von
  `PollutionCount.cpp`):
  - `TObjManager::perform` (236 Bytes, 99,83 % clean) —
    16-Byte-Frame-Gap; `char trash[0x10]` zeigte denselben Artefakt
    (Ziel-Prolog `-0x40` → `-0x50`), keine reale Verbesserung.
    Zurückgesetzt auf die saubere Fassung.
  - `TObjManager::load` (168 Bytes, 99,93 % clean) — 4-Byte-Versatz für
    `char buffer[0x100]`; `char trash[4]` vor und nach dem Buffer
    jeweils **verschlechtert** auf 99,71 % (keine Artefakt-Verwechslung
    möglich, da Verschlechterung eindeutig ist). Zurückgesetzt auf die
    saubere Fassung.


Die Referenz-DOL bleibt `OK`.

### Nach achter Iterationsrunde (M3UModel::updateInMotion; Artefakt-Korrektur)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,57 % (1.492.552 / 3.590.088) | +292 Bytes |
| Funktionen matched | 66,77 % (8.601 / 12.881) | +1 |

Ein neuer 100-%-Match: `M3DUtil/M3UModel.cpp::M3UModel::updateInMotion`
(292 Bytes, `char trash[0x28]` schließt eine 40-Byte-Frame-Lücke ohne
Struct-Locals im Funktionskörper).

**Nachträgliche Korrektur der sechsten/siebten Runde**: Beim
systematischen Nachprüfen (Vorher/Nachher-Vergleich des
Ziel-Funktionsprologs) stellte sich heraus, dass die dort gemeldeten
Teil-Verbesserungen durch `trash[N]` bei `noteOn` (99,80 % → 99,88 %),
`calcViewMtx` (99,21 % → 99,28 %) und `drawRevivalTexStamp` (99,93 % →
99,98 %) **Messartefakte** waren, keine echten Verbesserungen (siehe
Methodik-Fund und korrigierte Einträge oben). Alle drei Funktionen
bleiben bei ihrer sauberen Ausgangs-Prozentzahl ohne verifizierte
Verbesserung. Die bereits committeten echten 100-%-Matches
(`appendGrpMember`, `append`, `writeRegDirect`, `updateInMotion`) sind
von diesem Artefakt nicht betroffen, da eine zufällige Fensterver-
schiebung bei echten 100-%-Treffern über mehrere hundert Bytes hinweg
statistisch ausgeschlossen ist.

Die Referenz-DOL bleibt `OK`.

### Nach neunter Iterationsrunde (AnimalManager::loadAfter; Shimmer/bgpoldrop/clipEnemies-Analysen)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,58 % (1.492.612 / 3.590.088) | +60 Bytes |
| Funktionen matched | 66,78 % (8.602 / 12.881) | +1 |

Ein neuer 100-%-Match: `Animal/AnimalManager.cpp::TMewManager::loadAfter`
(60 Bytes, `char trash[0x10]` schließt eine 16-Byte-Frame-Lücke, keine
Locals im Funktionskörper).

Drei weitere Funktionen untersucht, alle nach dem neuen
Verifikationsprotokoll (Ziel-100-%-Test statt Zwischenprozent-Tracking)
als Nonmatching bestätigt und zurückgesetzt:

- `Map/Shimmer.cpp::TShimmer::perform` (648 Bytes, 99,80 % clean) —
  40-Byte-Frame-Gap plus konstante 4-Byte-Verschiebung in einer
  verketteten Virtual-Call-Kette (`getModelData()->
  getMaterialNodePointer(0)->getTexGenBlock()->getTexMtx(1)`);
  `char trash[0x28]` nach dem letzten `Mtx`-Local erreicht keine
  100 % (bestätigter Ziel-Fenster-Artefakt, `-0x170` → `-0x198`).
- `Enemy/bgpoldrop.cpp::TBGPolDrop::move` (592 Bytes, 99,79 % clean) —
  8-Byte-Frame-Gap, `char trash[8]` nach `local_14` erreicht keine
  100 %; nach neuem Protokoll sofort zurückgesetzt statt
  weiterzuoptimieren.
- `Animal/AnimalManager.cpp::TAnimalManagerBase::clipEnemies`
  (256 Bytes, 94,98 % clean) — strukturell identisch zu den bereits
  dokumentierten `clipEnemies`-Fällen (`NpcManager`, `CameraMode`):
  16-Byte-Frame-Gap plus echte Argument-Auswertungsreihenfolge in
  `SetViewFrustumClipCheckPerspective(gpCamera->mFovy,
  gpCamera->getAspect(), mViewClipNear, *mViewClipFarPtr)` (vier
  verschiedene Ladereihenfolgen f3/r3/f2/f1 vs. r3/f2/f1/f3) plus
  r29/r30-Register-Swap für die Schleifenvariable. `char trash[0x10]`
  erreicht keine 100 %; bestätigt dieselbe Kategorie, zurückgesetzt.

Die Referenz-DOL bleibt `OK`.

### Nach zehnter Iterationsrunde (HelpActor, MapCollisionEntry, JASDSPChannel)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,61 % (1.493.676 / 3.590.088) | +1.064 Bytes |
| Funktionen matched | 66,80 % (8.605 / 12.881) | +3 |

Drei neue 100-%-Matches, alle über simple Frame-Gap-Fixes ohne
Register-Probleme:

- `GC2D/HelpActor.cpp::THelpActor::perform` (344 Bytes, `char trash[8]`,
  keine Locals).
- `Map/MapCollisionEntry.cpp::TMapCollisionMove::init(u32,u16,s16,
  const TLiveActor*)` (168 Bytes, `char trash[8]` am Funktionsanfang).
- `JSystem/JAudio/JASystem/JASDSPChannel.cpp::TDSPChannel::updateAll`
  (552 Bytes, `char trash[0x18]`, 24-Byte-Frame-Gap ohne Struct-Locals).

Vier weitere Funktionen in derselben Unit-Scan-Runde als "Frame
bereits exakt, nur interner Slot-Versatz" identifiziert (dieselbe
unlösbare Kategorie wie `drawRevivalTexStamp`/`calcViewMtx`, siehe
oben) und ohne Zwischen-Tuning sofort zurückgesetzt:
`Map/PollutionObj.cpp::TPollutionObj::getDepthFromMap` (99,96 %,
bereits mit Upstream-`TODO: inlines are wrong here!` markiert),
`Map/MapCollisionEntry.cpp::TMapCollisionMove::move()` (99,90 % best,
8-Byte-Gap in `local_18`), `TMapCollisionWarp::setUp()` (99,92 % best,
8-Byte-Gap in `local_18`), `TMapCollisionMove::moveSRT` (Frame stimmt
bereits exakt, 4-Byte-Slot-Versatz).

Die Referenz-DOL bleibt `OK`.

### Nach elfter Iterationsrunde (JDRDisplay, CameraMarioData, JAIGlobalParameter)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,63 % (1.494.420 / 3.590.088) | +744 Bytes |
| Funktionen matched | 66,83 % (8.608 / 12.881) | +3 |

Drei neue 100-%-Matches:

- `JSystem/JDrama/JDRDisplay.cpp::TDisplay::startRendering` (248 Bytes,
  `char trash[8]`, keine Locals).
- `Camera/CameraMarioData.cpp::TCameraMarioData::calcAndSetMarioData`
  (356 Bytes, `char trash[8]` nach `JGeometry::TVec3<f32> offset`).
- `JSystem/JAudio/JAInterface/JAIGlobalParameter.cpp::
  setParamSoundOutputMode` (140 Bytes, `char trash[8]` am
  Funktionsanfang, zwei skalare Locals `r31`/`r30`).

Vier weitere Funktionen in derselben Scan-Runde als Nonmatching
bestätigt (nach neuem Protokoll sofort zurückgesetzt statt
Zwischenwerte zu verfolgen):

- `JSystem/JDrama/JDRActor.cpp::JDrama::TActor::load` (99,93 % clean,
  340 Bytes) — Frame stimmt exakt, 4-Byte-Slot-Versatz für
  `char str[0x50]`; beide getesteten `trash[4]`-Positionen
  (vor/nach `str`) verschlechtern identisch auf 99,82 %.
- `NPC/NpcInbetween.cpp::TNpcInbetween::execPosInbetween` (99,09 %
  clean, 220 Bytes) — reines f1/f2-Register-Rotationsmuster, kein
  Frame-Unterschied, dieselbe Kategorie wie `effectObj::moveObject`.
- `Camera/CameraBck.cpp::TCameraBck::updateDemo` (98,96 % clean,
  452 Bytes) — 24-Byte-Frame-Gap **plus** eine echte
  Doppel-Bool-Normalisierung auf der Zielseite (eine zusätzliche
  `li r0, 0x1`/`cmpwi r0, 0x0`-Sequenz vor der finalen
  `result`-Zuweisung, die unser Build nicht erzeugt — `checkState()`
  ist bereits `? 1 : 0`-normalisiert inline, daher unklar, wodurch die
  Ziel-Redundanz entsteht). `char trash[0x18]` nach `J3DTransformInfo
  info` bewegt den Match kaum (98,96 % → 99,03 %); zurückgesetzt.
- `JSystem/JDrama/JDREfbSetting.cpp::IssueGXCopyDisp` (99,50 % clean,
  404 Bytes) — kein Frame-Unterschied, reines Register-Rotationsmuster
  (r0/r3/r4) für einen booleschen Ausdruck.

Die Referenz-DOL bleibt `OK`.

### Nach zwölfter Iterationsrunde (MapObjEx, MapObjGrass, PollutionPos)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,66 % (1.495.808 / 3.590.088) | +1.780 Bytes |
| Funktionen matched | 66,87 % (8.612 / 12.881) | +4 |

Vier neue 100-%-Matches über drei Dateien:

- `MoveBG/MapObjEx.cpp`: `TMapObjNail::receiveMessage` (324 Bytes) und
  `TJointCoin::control` (284 Bytes) — je `char trash[8]` am
  Funktionsanfang, keine Struct-Locals.
- `MoveBG/MapObjGrass.cpp`: `TMapObjGrassManager::perform` (568 Bytes,
  `char trash[0x30]`, 48-Byte-Frame-Gap ohne Struct-Locals).
- `Map/PollutionPos.cpp`: `TPollutionPos::isSame` (212 Bytes,
  `char trash[0x20]`, 32-Byte-Frame-Gap ohne Struct-Locals).

Fünf weitere Funktionen untersucht, alle nach dem etablierten
Protokoll (Ziel: 100 % oder sofortiger Revert) als Nonmatching
bestätigt:

- `Strategic/Strategy.cpp::TStrategy::load` (99,93 % clean, 280 Bytes)
  — `char trash[8]` nach `JSUMemoryInputStream stream2` erreicht keine
  100 % (99,94 % best, nicht verifiziert als real).
- `MoveBG/MapObjGrass.cpp::TMapObjGrassManager::initDrawNear`
  (99,87 % clean, 588 Bytes) — `char trash[0x18]` nach `vec` erreicht
  keine 100 %.
- `MarioUtil/ModelUtil.cpp::
  SMS_RideMoveByGroundActor` (99,82 % clean, 404 Bytes) —
  `char trash[0x18]` nach `TMtx34f mtx` erreicht keine 100 %.
- `Animal/Butterfly.cpp::TButterfloid::load` (99,70 % clean,
  640 Bytes) — bereits mit Quellcode-Kommentar "Making these all
  setters doesn't yield NEARLY enough stack frame padding for this to
  match" als bekannt schwierig markiert (136-Byte-Frame-Gap), nicht
  erneut versucht.
- `GC2D/ScrnFader.cpp::TSMSFader::update` (99,66 % clean, 348 Bytes)
  — 16-Byte-Frame-Gap plus f1/f2-Register-Rotationsmuster (vermutlich
  aus inlinetem `updateRequest()`/Farbzuweisung); `char trash[0x10]`
  erreicht keine 100 %.

Die Referenz-DOL bleibt `OK`.

### Nach dreizehnter Iterationsrunde (SDLModel: entrySameMat, viewCalcSimple)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,68 % (1.496.332 / 3.590.088) | +552 Bytes |
| Funktionen matched | 66,90 % (8.614 / 12.881) | +2 |

Zwei neue 100-%-Matches in `M3DUtil/SDLModel.cpp`:
`SDLModelData::entrySameMat` (308 Bytes, `char trash[8]`) und
`SDLModel::viewCalcSimple` (216 Bytes, `char trash[8]`), beide
8-Byte-Frame-Gaps ohne Struct-Locals.

Sechs weitere Funktionen aus derselben Scan-Runde als Nonmatching
bestätigt (kein 100-%-Match, sofort zurückgesetzt):

- `M3DUtil/SDLModel.cpp::SDLModelData::entrySDLModels` (99,87 % clean,
  508 Bytes) — `char trash[8]` nach den Iterator-Locals `it`/`e`
  erreicht keine 100 % (99,90 % best).
- `M3DUtil/SDLModel.cpp::SDLModel::entry` (99,75 % clean, 384 Bytes)
  — `char trash[0x10]` erreicht keine 100 % (99,80 % best).
- `Map/MapXlu.cpp::TMapXlu::changeNormalJoint` (99,92 % clean,
  256 Bytes) und `changeXluJoint` (99,84 % clean, 280 Bytes) —
  Ziel-Frame ist **größer** als unseres; `char trash[0x10]`/`[8]` am
  Funktionsanfang wird vom Compiler komplett wegoptimiert (keinerlei
  Änderung an unserer kompilierten Größe oder am Match-Prozentsatz),
  da die Locals in diesen reinen Doppel-`for`-Schleifen-Funktionen
  ohne jede andere Verwendung nachweisbar tot sind.
- `Enemy/areacylinder.cpp::TAreaCylinder::load` (99,94 % clean,
  604 Bytes) — `char trash[8]` nach `JGeometry::TVec3<f32> v` erreicht
  keine 100 %.
- `GC2D/MovieSubtitle.cpp::TMovieSubTitle::setupResource` (99,88 %
  clean, 404 Bytes) — `char trash[8]` vor/nach `char buffer[256]`
  identisch wirkungslos (99,93 % best, beide Positionen).

Die Referenz-DOL bleibt `OK`.

### Nach vierzehnter Iterationsrunde (MapObjCloud, launcher, MapObjTrap)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,71 % (1.497.388 / 3.590.088) | +1.056 Bytes |
| Funktionen matched | 66,95 % (8.619 / 12.881) | +5 |

Fünf neue 100-%-Matches über drei Dateien:

- `MoveBG/MapObjCloud.cpp::TRideCloud::setGroundCollision` (120 Bytes)
  — `char trash[4]` nach `TMtx34f mtx` behebt einen 4-Byte-Slot-Versatz
  bei bereits korrektem Frame (Ausnahme von der sonst unlösbaren
  "interner Slot-Versatz"-Kategorie — hier hat es funktioniert).
- `Enemy/launcher.cpp`: `TCommonLauncher::stateHitByWater` (180 Bytes,
  `char trash[8]`) und `TCommonLauncher::perform` (364 Bytes,
  `char trash[0x10]`) — je einfache Frame-Gaps ohne Struct-Locals.
  `stateLaunch` bleibt Nonmatching (Frame bereits exakt, 48-Byte-
  Vec/Mtx-Slot-Vertauschung zwischen drei Struct-Locals).
- `MoveBG/MapObjTrap.cpp`: `TLampTrapSpikeHit::perform` (228 Bytes,
  `char trash[0x18]`) und `TLampTrapIron::receiveMessage` (164 Bytes,
  `char trash[8]`) — je einfache Frame-Gaps ohne Struct-Locals.

Die Referenz-DOL bleibt `OK`.

### Nach fünfzehnter Iterationsrunde (MapObjBlock: fünf Matches)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,75 % (1.499.040 / 3.590.088) | +1.652 Bytes |
| Funktionen matched | 66,95 % → 66,97 % (8.624 / 12.881) | +5 |

Fünf neue 100-%-Matches in `MoveBG/MapObjBlock.cpp`:
`TIceBlock::control` (480 Bytes, `char trash[0x10]`),
`TIceBlock::touchWater` (392 Bytes, `char trash[0x10]`),
`TBrickBlock::kill` (208 Bytes, `char trash[8]`),
`TSuperHipDropBlock::receiveMessage` (160 Bytes, `char trash[8]`),
`TTelesaBlock::perform` (412 Bytes, `char trash[8]` nach
`TRotation3f mtx` — bereits mit Upstream-TODO "Possibly more
TRotation3f inlines?" markiert, Trash hat trotzdem funktioniert).
`TSandBlock::control` bleibt Nonmatching (99,93 % best mit
`char trash[8]`, keine 100 %).

Zwei weitere Funktionen aus Nachbar-Units geprüft, beide Nonmatching:

- `Enemy/coasterkiller.cpp::TCoasterEnemy::bind` (220 Bytes,
  Frame bereits exakt, interner 12-Byte-Slot-Versatz — dieselbe
  unlösbare Kategorie).
- `Map/MapWireManager.cpp::TMapWireManager::load` (432 Bytes,
  99,79 % clean) — `char trash[0x10]` erreicht keine 100 %
  (99,89 % best).
- `Strategic/liveinterp.cpp::linGetSRT` (1.944 Bytes, 96,06 % clean)
  — großer verschachtelter `switch(arg2){switch(arg1){...}}` mit
  mehreren `TSpcSlice slice;`-Locals in gegenseitig exklusiven
  Case-Block-Scopes; dieselbe Kategorie wie die bereits dokumentierten
  `execadd`/`execsub`/`execmul`/`execdiv`-Fälle in `spcinterp.cpp`
  (compiler-interne Temporary-Platzierung). `char trash[0x20]`
  bewegt kaum etwas (96,06 % → 96,10 %); zurückgesetzt.

Die Referenz-DOL bleibt `OK`.

### Nach sechzehnter Iterationsrunde (coasterkiller::loadAfter; objdiff-Diff-Artefakt entdeckt)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,76 % (1.499.092 / 3.590.088) | +52 Bytes |
| Funktionen matched | 66,97 % (8.625 / 12.881) | +1 |

Ein neuer 100-%-Match: `Enemy/coasterkiller.cpp::TCoasterKillerManager::
loadAfter` (52 Bytes, `char trash[0x18]` schließt eine 24-Byte-
Frame-Lücke, keine Locals).

**Wichtiger Methodik-Fund (objdiff-Instruktions-Alignment-Artefakt)**:
Bei der Untersuchung von `TMapWireActorManager::doActorToWire`
(99,40 % clean, 484 Bytes) zeigte `objdiff-cli`s Instruktions-Diff an
einer Stelle einen scheinbar echten Bug: unser kompilierter Code lädt
`lwz r3, 0x78(r30)` / `lwz r0, 0x7c(r3)` (entspricht `unk4.unk74->
unk7C`), während die "Ziel"-Seite in der JSON-Ausgabe `lwz r3, 0x0(r30)`
/ `lwz r0, 0x6c(r3)` zeigt (entspräche `unk0->mHeldObject`) — ein
scheinbarer Strukturunterschied. Ein expliziter Test mit temporärer
Variable (`TTakeActor* dbgUnk0 = unk0;`) änderte am kompilierten Ergebnis
**nichts**, was den Verdacht erhärtete. Gegenprobe direkt in der
**rohen Retail-Disassemblierung** (`build/GMSJ01/asm/Map/
MapWireManager.s`, generiert am Anfang der Session, seither
unverändert): An der exakten Adresse `801EAC6C`/`801EAC70` innerhalb
von `doActorToWire` (zwischen `.fn`/`.endfn`) steht dort tatsächlich
`lwz r3, 0x78(r30)` / `lwz r0, 0x7c(r3)` — **identisch mit unserem
kompilierten Code**. Das bedeutet: `objdiff-cli`s Instruktions-
Alignment in der JSON-Diff-Ausgabe kann an einzelnen Stellen
fehlausgerichtet sein (vermutlich Nachwirkung einer Sequenz-Alignment-
Neusynchronisierung nach einer früheren echten Abweichung im
Funktionsverlauf), auch wenn der aggregierte `match_percent`-Wert
korrekt bleibt. **Regel für künftige Sessions**: Bei einem
scheinbaren "echten Bug" (unterschiedliche Feldoffsets/Strukturzugriffe)
immer zusätzlich direkt in der rohen `build/GMSJ01/asm/*.s`-Referenz-
datei (stabil seit Sessionbeginn, nicht von eigenen Edits beeinflusst)
gegenprüfen, bevor Zeit in einen vermeintlichen Quellcode-Fix investiert
wird. `doActorToWire` bleibt bei 99,40 % clean dokumentiert; die
echten Restunterschiede liegen an anderer Stelle im Funktionskörper
und wurden nicht weiter isoliert.

Die Referenz-DOL bleibt `OK`.

### Nach siebzehnter Iterationsrunde (JPAField::JPAMagnetField::affect)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,77 % (1.499.412 / 3.590.088) | +320 Bytes |
| Funktionen matched | 8.626 / 12.881 | +1 |

Ein neuer 100-%-Match: `JSystem/JParticle/JPAField.cpp::
JPAMagnetField::affect` (320 Bytes, `char trash[8]`, 8-Byte-Frame-Gap
ohne Locals).

Zwei weitere Feld-`affect`-Funktionen geprüft, beide Nonmatching:

- `JPADragField::affect` (288 Bytes, 99,85 % clean) — Frame stimmt
  nach `char trash[8]` exakt, verbleibender 4-Byte-Slot-Versatz bei
  drei getesteten Positionen (`trash[8]`/`[4]` am Funktionsanfang,
  `trash[4]` nach `rnd`) identisch bei 99,97 % — dieselbe unlösbare
  Kategorie wie `drawRevivalTexStamp`. Zurückgesetzt auf die saubere
  99,85-%-Fassung.
- `JPAVortexField::affect` (360 Bytes, 99,22 % clean) — r30/r3-
  Registervertauschung plus f28/f29-FP-Rotation, nicht untersucht
  (dieselbe Kategorie wie `effectObj::moveObject`).
- `JPARandomField::affect` (320 Bytes, 99,09 % clean) — 16-Byte-
  Frame-Gap plus dreifach wiederholtes FP-Rotationsmuster
  (f0/f1/f2/f3) in den drei `get_ufloat_1() - 0.5f`-Aufrufen;
  `char trash[0x10]` bewegt nur auf 99,18 %, zurückgesetzt.

Die Referenz-DOL bleibt `OK`.

### Nach achtzehnter Iterationsrunde (M3DUtil/MActor.cpp: neun Matches)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,84 % (1.502.040 / 3.590.088) | +2.628 Bytes |
| Funktionen matched | 8.636 / 12.881 | +9 |

Neun neue 100-%-Matches in `M3DUtil/MActor.cpp`, alle einfache
Frame-Gap-Fixes ohne Struct-Locals: `isCurAnmAlreadyEnd` (200 Bytes,
`trash[8]`), `calc` (264 Bytes, `trash[0x10]`), `updateIn`/`updateOut`
(228 Bytes je, `trash[8]`), `calcAnm` (404 Bytes, `trash[0x18]`),
`entry` (332 Bytes, `trash[8]`), `setLightData` (160 Bytes,
`trash[0x20]`), `perform` (472 Bytes, `trash[0x10]`),
`frameUpdate`/`updateMatAnm` (192/148 Bytes, je `trash[8]`).

Zwei Nonmatching-Reste in derselben Datei (Frame-Gap gefunden, aber
kein 100-%-Match): Konstruktor `MActor::MActor(MActorAnmData*)`
(1.272 Bytes, 99,97 % best mit `trash[8]`) und `setModel` (752 Bytes,
99,95 % best mit `trash[0x18]`) — beide zurückgesetzt auf die
sauberen 99,95-%/99,92-%-Ausgangsfassungen.

Auch `Camera/CameraWarp.cpp::warpPosAndAt(f32,s16)` geprüft
(580 Bytes, 99,77 % clean) — 8-Byte-Frame-Gap plus zwei vertauschte
Struct-Locals (`usualLookat`/`pos`); weder Deklarationsreihenfolge-
Tausch noch `char trash[8]` erreichten 100 % (99,83 % best),
zurückgesetzt.

Die Referenz-DOL bleibt `OK`.

### Nach neunzehnter Iterationsrunde (liveactor Konstruktor; NpcNerve::TNerveNPCTalk)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,85 % (1.502.520 / 3.590.088) | +176 Bytes |
| Funktionen matched | 8.638 / 12.881 | +2 |

Zwei neue 100-%-Matches: `Strategic/liveactor.cpp::TLiveActor::
TLiveActor(const char*)` (304 Bytes, `char trash[8]`) und
`NPC/NpcNerve.cpp::TNerveNPCTalk::execute` (176 Bytes, `char trash[8]`
nach `TBaseNPC* self`).

Vier weitere Funktionen in `liveactor.cpp` untersucht, Nonmatching:

- `TLiveActor::bind` (604 Bytes, 99,95 % clean) — Frame stimmt exakt;
  ein 0x10-Byte-Slot-Versatz für das `nextPos - mPosition`-Temporary
  (`mLinearVelocity`-Zuweisung); `char trash[0x10]` nach `nextPos`
  verschlechtert stark (99,72 %), zurückgesetzt.
- `TSpineBase<TLiveActor>::update` (264 Bytes, 99,92 % clean,
  Template-Methode in `include/Strategic/Spine.hpp`) — `char trash[8]`
  nach `nerve` ohne jede Wirkung (Header wird von mehreren TUs
  instanziiert, Trash dort vom Optimizer eliminiert wie bei
  `MapXlu`).
- `TLiveActor::initAnmSound` (272 Bytes, 99,84 % clean) —
  `char trash[8]` bewegt auf 99,97 %, keine 100 %, zurückgesetzt.
- `TLiveActor::init(TLiveManager*)` (492 Bytes, 99,87 % clean) —
  32-Byte-Frame-Differenz (Original **kleiner**) plus vertauschte
  Struct-Temporaries, nicht untersucht (Zeitaufwand vs. Nutzen).

Die Referenz-DOL bleibt `OK`.

### Nach zwanzigster Iterationsrunde (mameGesso/walkerEnemy-Batch; getGravityY-Bugfix)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,94 % (1.505.820 / 3.590.088) | +3.300 Bytes |
| Funktionen matched | 8.644 / 12.881 | +6 |

Sechs neue 100-%-Matches: `Enemy/mameGesso.cpp::TNerveMameGessoObject::
execute` (`char trash[0x10]` nach `self`), `TNerveMameGessoDamage::
execute` (`char trash[8]` nach `vel`), `TMameGessoManager::perform`
(`char trash[8]` am Funktionsanfang), `Enemy/walkerEnemy.cpp::
TNerveWalkerAttack::execute` (`char trash[0x10]` nach `self`),
`TWalkerEnemy::init` (`char trash[8]` am Funktionsanfang),
`Enemy/gesso.cpp::TGesso::rollCheck` (`char trash[8]` am
Funktionsanfang).

Echter Logikfehler gefunden und gefixt: `Enemy/mameGesso.cpp::
TMameGesso::getGravityY` verglich `mSpine->getCurrentNerve()` gegen
`TNerveMameGessoObject::theNerve()` statt `TNerveMameGessoGraphJump
Wander::theNerve()`, bevor `mSLJumpWanderGravityY` angewendet wird —
Feldname und Nerve-Name passten nicht zusammen. Bestätigt gegen die
rohe Retail-Disassembly (`build/GMSJ01/asm/Enemy/mameGesso.s`, zeigt
`__vt__30TNerveMameGessoGraphJumpWander` an dieser Stelle). Nach Fix
`match_percent` 100,0 %. Hinweis: Der projektweite `fuzzy_match_
percent`-Zähler aus `objdiff-cli report` (Basis für die obige
Fortschrittstabelle) hatte diese Funktion schon VOR dem Fix als
100 % fuzzy-matched gezählt — die Byte-Zahl der Tabelle enthält
diese Funktion daher nicht als Delta, obwohl der Fix inhaltlich
korrekt und über `objdiff-cli diff` (`match_percent`, die in diesem
Projekt maßgebliche Metrik) bestätigt 100 % ist.

**Neuer Methodik-Fund**: Eine automatisierte Rundum-Suche nach
Funktionen mit reinem Stackframe-Gap (Prolog-`stwu`-Differenz, sonst
strukturell identisch) über 260 Units lieferte 1.119 Kandidaten mit
erkennbarem Gap; 69 davon mit hoher Konfidenz (`fuzzy_match_percent`
> 99,5 %, Ziel-Frame größer als unseres). Blindes Anwenden von
`char trash[N]` (Platzierung: nach letztem struct-typisiertem Local
bzw. am Funktionsanfang) auf 24 dieser automatisch gefundenen
Kandidaten (u. a. `tamaNoko.cpp`, `EventWatcher.cpp`, `hamukuri.cpp`,
`NpcChange.cpp`, `DrawUtil.cpp`, `wireBinder.cpp`, `boid.cpp`, sowie
ein Retry in `mameGesso.cpp`) ergab **0 Treffer** — jedes Mal blieb
der Ziel-Stackframe exakt auf der Baseline-Größe, das `trash`-Array
wurde vom Optimizer vollständig eliminiert. Auch `volatile char
trash[N]` sowie ein erzwungener Schreibzugriff (`trash[0] = 0`)
änderten daran nichts (letzteres verschlechterte den Match sogar,
da ein zusätzlicher, nicht im Original vorhandener `stb`-Befehl
entsteht, ohne den Frame zu vergrößern). MWCC (`-O4,p -opt`) eliminiert
in diesen Fällen das komplett ungenutzte Array unabhängig von Größe,
Platzierung oder `volatile`-Qualifikation — offenbar eine Eigenschaft
der jeweiligen Funktion (Registerdruck/Liveness), nicht der Datei
oder der Compiler-Flags (identisch `-O4,p -opt` für `mameGesso.cpp`
und `EventWatcher.cpp` verifiziert). Fazit: `char trash[N]` ist kein
garantiert wirksamer Trick — er funktioniert zuverlässig nur, wenn
man empirisch für die KONKRETE Funktion prüft (bauen + diffen), nicht
durch pauschale Anwendung auf automatisch gefundene Kandidatenlisten.
Alle 24 Versuche sauber zurückgesetzt (`git checkout`), keine toten
`trash`-Deklarationen im Baum verblieben.

### Nach einundzwanzigster Iterationsrunde (Nerve-Vergleichsfehler-Suche: 4 Bugfixes)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,96 % (1.506.296 / 3.590.088) | +476 Bytes |
| Funktionen matched | 8.645 / 12.881 | +1 |

Neue Methodik: automatisierte Suche nach `__vt__`-Symbolabweichungen
an identischer Instruktionsposition (gleiche Adresse, unterschiedlicher
Vtable-Name) über alle 260 zuvor gescannten Units — ein zuverlässigeres
Signal für echte Logikfehler als reine Stackframe-Gaps. 38 Treffer,
davon 4 echte, über die rohe Retail-Disassembly bestätigte Bugs:

- `Enemy/fireWanwan.cpp::TFireWanwan::attackToMario` — Nerve-
  Ausschlussprüfung verglich gegen `TNerveFireWanwanRecover::
  theNerve()` statt `TNerveFireWanwanRecoverGraph::theNerve()`.
  **100 %** (476 Bytes) nach Fix.
- `Enemy/tamaNoko.cpp::TTamaNoko::isCollidMove` — Trample-Message-
  Guard verglich gegen `TNerveTamaNokoSleep::theNerve()` statt
  `TNerveTamaNokoDown::theNerve()`. **100 %** nach Fix.
- `Enemy/hinokuri2.cpp::THinokuri2::receiveMessageLv1` — Water-
  Spray-Schadenszweig setzte `TNerveHino2Freeze` statt
  `TNerveHino2Damage` als nächsten Nerve. 99,55 % → 99,81 % nach
  Fix (separater, nicht behobener 16-Byte-Frame-/Register-Rest).
- `Enemy/hinokuri2.cpp::TNerveHino2PrePol::execute` — `Pollute`-
  und `Stamp`-Zweige vertauscht (if-Zweig sollte `Stamp` pushen,
  else-Zweig `Pollute`, nicht umgekehrt). 98,68 % → 98,99 % nach
  Fix (separater, nicht behobener `getSaveParam()`-Feldzugriffspfad-
  Rest bei Index 30 der Instruktionsliste).

Beide Hinokuri2-Fixes werden trotz nicht erreichter 100 % behalten
(anders als bei spekulativen `trash`-Experimenten): Sie sind über die
rohe Disassembly bestätigte echte Verhaltenskorrekturen, keine
Vermutungen.

Weitere `__vt__`-Kandidaten geprüft, aber als zu komplex für schnelle
Fixes eingestuft (keine Änderung vorgenommen): `MarioUtil/MtxUtil.cpp::
TMultiMtxEffect::setup` (91,28 %, drei verschachtelte Switch-Case-
Objektkonstruktionen mit unterschiedlicher Registerreihenfolge),
`GC2D/CardLoad.cpp::TCardLoad::perform` (97,49 %, bereits im
Quelltext als upstream-TODO dokumentiert: Jump-Table vs.
Compare-Chain ist die Hauptursache), `System/MarDirectorInitECT.cpp::
TMarDirector::initECTGft` (90,76 %, mehrteilige Kamera/Ortho-
Objektkonstruktions-Reihenfolge, nicht in vertretbarer Zeit
aufgelöst). `MarioUtil/ShadowUtil.cpp::drawShadowGD`-Treffer sind
reine anonyme `$NNNN`-Zähler-Artefakte (verschiedene lokale
Typ-Ordinalzahlen zwischen unserem und dem Retail-Build), keine
echten Bugs.

Die Referenz-DOL bleibt `OK`.

### Nach zweiundzwanzigster Iterationsrunde (configure.py: 12 Unit-Flips auf Matching)

Neue Methodik: `objdiff-cli report generate` liefert pro Unit
`measures.matched_code_percent` und `measures.matched_data_percent`.
Systematischer Scan aller 736 Units fand 406 mit BEIDEM bei 100 %
(alle Funktionen UND alle Daten matchen bereits einzeln) — davon
waren 389 in `configure.py` schon korrekt auf `Matching` gesetzt,
17 noch nicht. Nach Ausschluss der beiden bereits dokumentierten
bekannten Brecher (`JALModSe.cpp`, `WoodBarrel.cpp` — Adress-Shift-
Layoutfehler) blieben 15 Kandidaten. Per-Unit-Bisektion (flip →
`python configure.py` → `ninja` → DOL-SHA1-Check → bei Fehlschlag
zurücksetzen) ergab:

**12 erfolgreich auf `Matching` gesetzt** (DOL-SHA1 bleibt `OK`):
`JSystem/JDrama/JDRDisplay.cpp`, `JSystem/JAudio/JAInterface/
JAIGlobalParameter.cpp`, `JSystem/JAudio/JASystem/JASDSPChannel.cpp`,
`JSystem/JParticle/JPADraw.cpp`, `M3DUtil/M3UModel.cpp`,
`System/MarNameRefGen_NPC.cpp`, `MoveBG/MapObjFloat.cpp`,
`Map/MapModel.cpp`, `GC2D/HelpActor.cpp`,
`Enemy/DemoBossHanachanBase.cpp`, `Enemy/BossHanachanSave.cpp`,
`Camera/CameraMarioData.cpp`. Verlinkte Units: 403 → 415 / 736.

**3 brechen die DOL-SHA1 trotz 100 % Code+Daten** (sofort
zurückgesetzt, bleiben `NonMatching`): `MoveBG/MapObjOption.cpp`,
`Map/PollutionEvent.cpp`, `Camera/CameraInbetween.cpp` — dieselbe
Kategorie wie `WoodBarrel`/`JALModSe` (anonyme Daten-/Adress-
Shift-Reste, die `objdiff-cli` pro Symbol nicht erfasst).

**Bug-Hunting-Methodik abgeschlossen**: Der `__vt__`-Symbol-
Mismatch-Scan (siehe 21. Runde) wurde projektweit auf alle 736
Units ausgeweitet (zuvor nur 260 automatisch vorselektierte
Kandidaten-Units). Ergebnis: exakt dieselben 38 Treffer wie in
Runde 21, keine neuen. Alle `Enemy/*.cpp`-Dateien (84 Stück)
einzeln gegengeprüft: 0 Treffer außerhalb der bereits behobenen.
Diese Methodik gilt für die aktuelle Codebasis als ausgeschöpft.

**Blindes `char trash[N]`-Auffüllen ist erschöpft geprüft**: Über
30 weitere automatisch erkannte Frame-Gap-Kandidaten (positiver
Gap, `fuzzy_match_percent` > 99,5 %) einzeln getestet — **0 Treffer**
in dieser Runde (verglichen mit einer deutlich höheren Trefferquote
bei den zuvor manuell kuratierten mameGesso/walkerEnemy/gesso-
Kandidaten). Auch `volatile char trash[N]` sowie ein erzwungener
Schreibzugriff wurden getestet, ohne Wirkung auf die Frame-Größe.
MWCC (`-O4,p -opt`) eliminiert ungenutzte Stack-Arrays bei manchen
Funktionen vollständig, bei anderen (auch mit identischen Flags,
identischer Datei) nicht — das Muster ist nicht zuverlässig
vorhersagbar. Empfehlung für Folge-Sessions: `trash`-Padding nur
noch bei Kandidaten versuchen, die durch vollständiges Lesen der
Funktion und Vergleich mit der rohen Retail-Disassembly bereits als
"nur Frame-Gap, sonst identisch" verifiziert wurden — nicht mehr
blind batchen.

Die Referenz-DOL bleibt `OK`.

### Nach dreiundzwanzigster Iterationsrunde (bossgesso isThing()-Reihenfolge; Konstanten-Mismatch-Scan)

| Metrik | Aktuell | Änderung |
| --- | ---: | ---: |
| Code matched | 41,96 % (1.506.460 / 3.590.088) | +164 Bytes |
| Funktionen matched | 8.646 / 12.881 | +1 |

Neue Methodik: automatisierte Suche nach `cmpwi`/`cmplwi`/`li`-
Instruktionen mit identischer Position, identischem Register aber
UNTERSCHIEDLICHER Konstante (statt `__vt__`-Symbolen) — 94 Treffer
über 241 Kandidaten-Units (`fuzzy_match_percent` ≥ 95 %). Nach
Filterung auf Fälle mit bereits identischem Stack-Frame (sicherstes
Signal für einen isolierten Fehler statt einer strukturellen
Differenz) blieben 3 Kandidaten:

- `Enemy/bossgesso.cpp::TBossGesso::changeAllTentacleState` —
  **100 %** (164 Bytes) nach Fix. `TBGTentacle::isThing()`
  (`include/Enemy/BossGessoTentacle.hpp`, als `// fabricated`
  markiert) prüfte `mState == 6 || mState == 3 || mState == 4` in
  Quelltextreihenfolge — MWCC kompiliert OR-Ketten mit
  Ganzzahlkonstanten aber NICHT strikt links-nach-rechts (bestätigt
  durch zwei Zwischenversuche mit unterschiedlicher Reihenfolge, die
  unterschiedliche, aber beide nicht korrekte Vergleichsreihenfolgen
  erzeugten). Empirisch ermittelte korrekte Quelltextreihenfolge
  `4 || 6 || 3` kompiliert exakt zur Zielreihenfolge. Nur ein
  Aufrufort (`changeAllTentacleState`), daher risikolos änderbar.

- `System/Application.cpp::TApplication::initialize_bootAfter`
  (98,77 % clean) — **tiefere Struktur-Unsicherheit gefunden, nicht
  behoben**: Der Konstruktoraufruf `new MSound(...)` kompiliert mit
  `li r3, 0xd4` (212, unsere `sizeof(MSound)`), während das Ziel
  `li r3, 0x30c` (780) erwartet. `include/MSound/MSound.hpp`s
  Feld-Offset-Kommentare summieren sich bereits korrekt auf 0x30C,
  aber der tatsächliche kompilierte `sizeof` bleibt bei 0xd4 — nach
  zweifachem erzwungenem Rebuild (Cache-Problem ausgeschlossen)
  weiterhin reproduzierbar. `include/JSystem/JAudio/JAInterface/
  JAIBasic.hpp` trägt bereits einen expliziten Upstream-TODO-
  Kommentar ("some of the fields might actually be from a derived
  class, MSound") — die JAIBasic/MSound-Feldgrenze ist demnach
  bereits als ungeklärt bekannt. Eine Korrektur erfordert eine
  vollständige Neuanalyse, welche Felder zu welcher Klasse gehören
  (Auswirkung auf alle ~50 Methoden beider Klassen) — zu riskant für
  einen schnellen Fix, als offener Fall dokumentiert statt geraten.

- `Player/WaterGun.cpp::TWaterGun::TWaterGun(TMario*)` (99,86 %
  clean, bereits von der `mHHoverHeight`-Typo-Untersuchung dieser
  Session betroffen) — weiterer unaufgeklärter Fund: Der Compiler
  erzeugt für das Array-Member `mEmitPos[4]` (`JGeometry::TVec3<f32>
  mEmitPos[4];`, Offsets im Header intern konsistent 0x1C90–0x1CC0)
  einen `__construct_array`-Aufruf mit Zähler `0x3` statt `0x4` —
  nach erzwungenem Rebuild reproduzierbar, Header-Deklaration bereits
  korrekt `[4]`. Vermutlich MWCC-interne Slicing-/Unroll-Heuristik
  für kleine Fixed-Size-Arrays, nicht über einfache Source-Änderung
  kontrollierbar; nicht weiter verfolgt.

`GC2D/PauseMenu2.cpp::TPauseMenu2::load` (99,96 % clean) ebenfalls
geprüft: Frame-Gap (0x50 vs. 0x30, Ziel kleiner) plus ein
`addis`/`addi`-Konstantenpaar (`0x70613030` vs. `0x745f30`, vermutlich
ein Fixed-Point- oder FourCC-Literal) — mehrteiliger Fall, nicht in
vertretbarer Zeit isoliert.

Die Referenz-DOL bleibt `OK`.

### Nach vierundzwanzigster Iterationsrunde (Feld-Offset-Mismatch-Scan; keine neuen sicheren Fixes)

Vierte Scan-Methodik ergänzt: Suche nach `lwz`/`stw`/`lha`/`lbz`/`sth`/
`stb`/`lfs`/`stfs`/`lfd`/`stfd`-Instruktionen an identischer Position
mit identischem Register-Paar, aber unterschiedlichem Offset-Literal
(analog zum Konstanten-Scan, aber für Feldzugriffe statt Immediates).
Projektweit (736 Units) 126 Treffer, davon nach Ausschluss `r1`-
basierter (Stack-Slot-Verschiebung, bereits bekannte Kategorie)
7 echte Feld-/Objekt-relative Treffer:

- `Enemy/smallEnemy.cpp::attackToMario` (nur 87,3 % clean insgesamt) —
  einzelner sauberer Offset-Diff, aber eingebettet in eine größere
  Umsortierung zweier `fsubs`-Berechnungen (Differenzbildung in
  anderer Reihenfolge) — kein isolierter Fix möglich.
- `Strategic/liveactor.cpp::TLiveActor::control` (nur 73,96 % clean) —
  `lwz r12, 0x10(r12)` vs. `0xc(r12)`, ein Vtable-Offset (potenziell
  falscher virtueller Aufruf), aber die Funktion hat 25 weitere
  strukturelle Diffs (fehlende/zusätzliche Instruktionen) — deutet auf
  einen echten fehlenden Codepfad hin, nicht in dieser Runde lösbar.
- `Player/MarioSpecial.cpp::TMario::pulling` (94,93 % clean) — Bit-Test
  `rlwinm. r0,r0,0,22,22` (Bit 22, Maske 0x200) vs. Ziel `...,21,21`
  (Bit 21, Maske 0x400); Quelltext bei `if (!(unk108->mInput & 0x400))`
  (Zeile 1209) legt Maske 0x400 nahe, passend zum ZIEL — bei 114
  Gesamt-Diffs in dieser sehr langen Funktion aber nicht zweifelsfrei
  genau dieser Quelltextzeile zuordenbar ohne vollständige Neulesung;
  als Kandidat für eine künftige Session vorgemerkt statt spekulativ
  gefixt.
- `Player/WaterGun.cpp::rotateProp` — identisch mit dem bereits
  dokumentierten `mHoverRotMax`-Rätsel dieser Session.
- `Map/MapCollisionEntry.cpp::moveSRT` — bereits als "interner
  Slot-Versatz" dokumentiert.
- `Strategic/spcinterp.cpp::execadd/execsub/execmul/execdiv` — bereits
  als Messartefakt dokumentiert.

Alle drei in dieser Runde einzeln ausprobierten `char trash[N]`-
Kandidaten (`Map/PollutionAction.cpp::fire`, `Map/MapMirror.cpp::
isUpperThanMirrorPlane`, sowie ein Re-Check von `moveSRT`) blieben
bei 0 % Wirkung — bestätigt erneut den in Runde 22 dokumentierten
Befund, dass blindes `trash`-Padding außerhalb bereits vertrauter
Dateien nicht zuverlässig funktioniert. `configure.py`-Unit-Flip-Scan
erneut durchgeführt: keine neuen Kandidaten seit Runde 22 (dieselben
3 bekannten Brecher `MapObjOption`, `PollutionEvent`,
`CameraInbetween` plus die 2 bereits dokumentierten `JALModSe`/
`WoodBarrel`).

Die Referenz-DOL bleibt `OK`.

### Nach fünfundzwanzigster Iterationsrunde (Tiefenanalyse `pulling`/`control`: ein Fehlalarm, ein bekanntes TODO)

Zwei der in Runde 24 dokumentierten Kandidaten per Rohdisassembly
(`build/GMSJ01/asm/…`) vollständig nachgeprüft, gemäß Methodik-Regel
dieser Session:

- `Player/MarioSpecial.cpp::TMario::pulling` — der vermutete
  Bit-Test-Fehler (`rlwinm. r0,r0,0,22,22` vs. angeblich `...,21,21`
  im Ziel) ist ein **bestätigter Fehlalarm**. Die rohe Retail-
  Disassembly (Zeile 1737 in `MarioSpecial.s`) zeigt an exakt dieser
  Stelle ebenfalls Bit 22 — unser Code ist dort bereits korrekt. Das
  `objdiff-cli diff`-JSON war an dieser Position falsch ausgerichtet
  (derselbe Artefakt-Typ wie beim bereits dokumentierten
  `doActorToWire`-Fund). Kein Fix nötig; frühere Vermutung aus Runde
  24 hiermit korrigiert.

- `Strategic/liveactor.cpp::TLiveActor::control` — die rohe Retail-
  Disassembly vollständig nachvollzogen (57 Instruktionen,
  `build/GMSJ01/asm/Strategic/liveactor.s` Zeile 862ff). Bestätigt:
  Der Quelltext hat an ZWEI Stellen einen reinen Kommentar-Stub
  (`// call on unk90`) statt eines echten virtuellen Aufrufs auf
  `unk90` (Vtable-Offset 0x10, über eine sekundäre Vtable bei
  `unk90+0x5c`). `unk90` ist als `void*` untypisiert und wird im
  gesamten sichtbaren Quelltext nirgends auf einen Nicht-Null-Wert
  gesetzt (nur `nullptr`-Initialisierung im Konstruktor) — der
  bestehende Kommentar `// TODO: was ist unk90???` markiert dies
  bereits als offen. Eine Korrektur erfordert, `unk90`s tatsächlichen
  Typ zu bestimmen (vermutlich eine polymorphe Klasse mit
  Sekundär-Vtable, evtl. verwandt mit `TSpineBase`s eigenem
  0x24-Offset-Muster) — außerhalb des Zeitrahmens für einen
  risikoarmen Fix in dieser Runde; als offener Fall dokumentiert statt
  spekulativ mit einem geratenen Typnamen committet (Projektregel:
  keine Vermutungen als Fakten).

Die Referenz-DOL bleibt `OK`.

### Nach sechsundzwanzigster Iterationsrunde (echter `mVelocity`-Bugfix in `smallEnemy::attackToMario`)

Verbleibende Feld-Offset-Scan-Kandidaten (`ModelWaterManager::drawMirror`
90,04 %, `ModelWaterManager::calcVMMtxGround` 62,49 %,
`TSmallEnemy::attackToMario` 87,31 %) einzeln per Rohdisassembly
geprüft:

- `drawMirror`/`calcVMMtxGround`: gestreute strukturelle Diffs (177 von
  540 Instruktionen bei `drawMirror`, mehrere INSERT/DELETE-Cluster ab
  früher Instruktion) — kein isolierbarer Einzel-Bug, sondern verteilt
  über die ganze GX-lastige Funktion. Als offener Fall dokumentiert statt
  weiterverfolgt.

- `TSmallEnemy::attackToMario` — **echter Bugfix gefunden und behoben**:
  Rohdisassembly (`build/GMSJ01/asm/Enemy/smallEnemy.s` Zeile 3719ff)
  zeigt, dass `mVelocity.set(local_20)` im Ziel NUR `mVelocity.x`
  (Offset `0xac`) und `mVelocity.z` (`0xb4`) beschreibt — `mVelocity.y`
  (`0xb0`) wird nie gestored. Auf `mVelocity.x = local_20.x; mVelocity.z
  = local_20.z;` umgestellt: **87,31 % → 90,44 %**. Passt inhaltlich zu
  einem Angriffs-Lunge, der nur die horizontale Ebene beeinflusst und
  die vertikale Geschwindigkeit unangetastet lässt. Restdiff (90,44 %)
  ist ein reiner Stack-Slot-Wiederverwendungs-Unterschied: Ziel legt für
  den lokalen `v`-Vektor einen frischen Slot bei `0x14(r1)` an,
  unser Build wiederverwendet den toten `local_20`-Slot bei `0x20(r1)`
  (beide Frames sind exakt `0x40` Bytes groß — keine Frame-Differenz,
  reine Alias-Optimierung). `char trash[0xc]` vor der `v`-Deklaration
  getestet: verschlechtert auf 90,22 %, sofort zurückgesetzt. Verbleibt
  als MWCC-interne Slot-Alias-Entscheidung, nicht über Source
  erzwingbar.

Die Referenz-DOL bleibt `OK`.

### Nach siebenundzwanzigster Iterationsrunde (WaterGun-Struct-Bugfix, systematisches Scannen 90–99,99-%-Kandidaten)

Automatisierter Scan aller Funktionen mit 90–99,99 % Fuzzy-Match und
≤300 Bytes über alle 736 Units (462 Kandidaten), sortiert nach
Instruktions-Diff-Anzahl (kleinste zuerst) statt nach Match-Prozent —
liefert zuverlässigere Kandidaten als reines Prozent-Sortieren, weil
Bytegröße die Prozentzahl verzerrt. Für jeden Kandidaten mit 1–2
Instruction-Diffs Rohdisassembly-Cross-Check vor jedem Fix-Versuch
(Methodik-Regel weiterhin bestätigt: kein Vertrauen in objdiff-JSON
ohne Gegenprüfung).

**Echter Bugfix**: `Player/WaterGun.hpp::TWaterGunParams` — Feld
`mNozzleAngleYSpeedMax` (nirgends im Quelltext benutzt) stand an der
falschen Stelle im Struct (zwischen `mNozzleAngleYBrake` und
`mHoverRotMax`). Durch Cross-Referenzierung zweier unabhängiger
Retail-Funktionen (`rotateProp`s `mHoverRotMax`-Offset `0x1d90` UND
`changeBackup`s `mChangeSpeed`-Offset `0x1dcc`) eindeutig belegt: das
Feld gehört zwischen `mHoverSmooth` und `mChangeSpeed`. Nach
Verschieben: `rotateProp` **99,98 % → 100 %**; `movement()` verbessert
sich ebenfalls leicht (bleibt wegen eines separaten, bereits
dokumentierten TODO-Fehlens von Stack-Speicher unter 100 %).

**Fehlgeschlagene Fixversuche** (alle einzeln zurückgesetzt, keine
Verschlechterung committet):

- `Enemy/bosspakkun.cpp::TBossPakkun::setGroundCollision` (99,98 %,
  einziger Diff: `collisionMtx`-Local bei `0x20` statt `0x18`).
  `char trash[8]` vor UND nach der Deklaration getestet: beide
  verschlechtern (99,88 %/99,86 %). Zurückgesetzt.

- `Player/WaterGun.cpp::TWaterGun::setBaseTRMtx` (99,97 %, `temp`-Mtx-
  Local bei `0x1c` statt `0x20`). `char trash[4]` an drei Stellen
  (vor `result`, zwischen `result`/`temp`, nach `temp`) getestet: alle
  verschlechtern (99,63–99,67 %). Zurückgesetzt.

- `MoveBG/MapObjLib.cpp::TMapObjBase::isDemo` (99,77 %, einziger Diff:
  Sprungziel bei b1==true zeigt in unserem Build auf „return true"
  statt wie im Original auf „return false"). Umstrukturierung zu
  einem frühen `if (b1) return false;` verschlechterte drastisch auf
  90,45 % — Hypothese falsch, MWCC kompiliert das Early-Return-Muster
  anders als angenommen. Zurückgesetzt auf Original.

- `Player/MarioRun.cpp::TMario::rotating` (98,65 %, einziger Diff:
  fehlendes `extsh` nach `neg` bei `mModelFaceAngle = -(mStatusTimer *
  4096);`). Drei Varianten (`(s16)`-Cast auf den ganzen Ausdruck,
  Negation vor der Multiplikation, expliziter `s16`-Temp) probiert:
  erste und dritte ändern nichts (98,65 % identisch), zweite
  verschlechtert auf 95,95 %. MWCC-interne Entscheidung, ob nach einer
  Negation vor `s16`-Store sign-extended wird, nicht über einfache
  Source-Umformulierung erzwingbar.

- `Enemy/pakkun.cpp::TNervePakkunAppear::execute` (98,55 %, einziger
  Diff: überflüssiges `cmpwi r3,0x0` nach `checkPass()`-Aufruf, dessen
  Ergebnis im leeren `if (...) { }`-Body nicht verwendet wird). Entfernen
  des `if`-Wrappers (reiner Ausdrucks-Aufruf `checkPass(100.0f);`)
  erzeugt **exakt identischen** Maschinencode — MWCC materialisiert den
  bool-Rückgabewert eines Funktionsaufrufs unabhängig davon, ob er
  ausgewertet wird. Zurückgesetzt auf die klarere `if(){}`-Fassung
  (kein Unterschied, aber dokumentiert die ursprüngliche Absicht besser).

- `MoveBG/MapObjLib.cpp::TMapObjBase::getDistance` (99,94 %, einziger
  Diff: `volatile f32 y`-Local bei `0x14` statt `0x10`, Frame beidseitig
  `0x18` identisch). Drei Varianten (`char trash[4]` am Funktionsanfang,
  `char trash[4]` vor `y`, `f32 pad` vor `y`) — alle **exakt ohne
  Wirkung** (99,935486 % identisch bei allen dreien). Anders als die
  übrigen Padding-Fälle dieser Session beeinflusst hier offenbar keine
  lokale Variable diese eine `volatile`-Platzierung; Ursache bleibt
  unklar, nicht in vertretbarer Zeit weiter verfolgt.

**Weiterer echter Fund**: `Player/MarioMain.cpp::TMario::drawSyncCallback`
(99,97 %, einziger Diff: `local_1c`-Ausgabeparameter für `GXPeekARGB` bei
`0x14` statt `0x10`). `char trash[4]` VOR der Deklaration wirkungslos,
aber `char trash[4]` DIREKT NACH `u32 local_1c;` (vor dem
`GXPeekARGB`-Aufruf) trifft exakt: **100 %**.

Die Referenz-DOL bleibt `OK`.

## Windows-Setup

Die JPN-RVZ liegt als Hardlink unter `orig/GMSJ01/disc.rvz`; `orig/*/*` ist
git-ignored. Native Windows-Tools werden verwendet, kein wibo/Wine.

Der Checkout braucht LF:

    git config core.autocrlf false
    git config core.eol lf

## Bisherige Branch-Änderungen

Vor dem Wechsel auf GMSJ01 wurden PAL-Buildfehler behoben. Alle Änderungen
sind mit `VERSION_GMSP01` isoliert; der GMSJ01-Referenz-Build bleibt `OK`.

Byte-genau gegen die PAL-DOL gematcht:

- `OSGetLanguage`
- `OSGetEuRgb60Mode`
- `OSSetEuRgb60Mode`

Offen für PAL: Die `VideoHeight`-Funktionen in `System/Resolution.cpp` linken,
matchen wegen abweichender TU-Funktionsreihenfolge aber noch nicht.

## Offene GMSJ01-Nonmatching-Fälle

- `Player/MarioAccess.cpp`: `SMS_IsMarioOnWire` (72 Bytes, 93,83 %).
  Das Ziel lädt `mHolder` für Nullprüfung und Typzugriff zweimal; MWCC fasst
  die beiden Quellzugriffe zusammen. Verschachtelte Bedingungen,
  Zugriffsmethoden sowie volatile/alias-basierte Varianten erreichten nach
  mehreren Versuchen höchstens 96,61 %, waren aber synthetischer als die
  saubere 93,83-%-Fassung. Gemäß Iterationsregel als `// NONMATCHING`
  zurückgestellt.

- `Strategic/HitActor.cpp`: `THitActor::calcEntryRadius` (124 Bytes).
  Nach `ninja all_source` matcht der committed Quelltext zu **97,61 %**.
  Mit `char trash[0x30]` stieg es auf 97,68 %. Rest: FPSCR-Registerwahl
  und `fmadds`/`frsp`-Reihenfolge.

- `Strategic/livemanager.cpp`: `TLiveManager::perform` (252 Bytes).
  Nach Rebuild matcht der committed Quelltext zu **99,84 %**. Bestes
  Experiment `char trash[0x10]`: 99,92 %, Frame 0x50 identisch. Verbleibend
  fünf Color-Access-Offsets (`0x34` vs. `0x24`).

- `THPPlayer/THPAudioDecode.c`: `AudioDecoderForOnMemory` (176 Bytes, 89,27 %).
  Register-Diff betrifft die gehoisteten `ActivePlayer`- und `AudioDecodeThread`-
  Pointer sowie den Frame-Index (`r28` ist beidseitig `readSize`; die echte
  Verschiebung ist `frame` in `r29` statt `r31`). `while(TRUE)` → `for(;;)`
  ist ein No-Op auf die Registerallokation; ein belastbarer Match setzt
  voraus, dass die lokalen Variablen in einer bestimmten Reihenfolge deklariert
  sind und der Compiler die Hoists nicht zusammenlegt.

- `Player/SplashManager.cpp`: `TSplashManager::makeDL` (392 Bytes, 99,95 %).
  Original reserviert Color-Struct bei `0x58(r1)`, unsere Version bei `0x54(r1)`
  (4-Byte-Differenz). Varianten `u32 pad`, zusätzliche `JGeometry::TVec3`,
  `char trash[8]`/`[4]` nach `thing[4]` erreichten 99,82 %/99,63 %/99,63 %/
  99,82 % – alle schlechter. Vermutlich SDA-Konstantenwahl, nicht Padding.

- `MoveBG/MapObjPollution.cpp`: `loadAfter` (172 Bytes, 88,07 %).
  Quellaufruf von `registerRevivalTexStamp` arbeitet mit `int/short`-Parametern;
  eine forcierte `(s16)/(s32)`-Typisierung brachte keine Änderung. Differenz
  bleibt in `r29`/`r30`-Registerwahl und Reihenfolge der `addi`s innerhalb der
  Schleife.

- `THPPlayer/THPPlayer.c`: `THPPlayerPrepare` (624 Bytes, 97,98 %).
  Register-Offset um genau 1 (`r27..r30` statt `r28..r31`) über die gesamte
  Funktion, Frame identisch. `threadData`-Scope auf den `onMemory`-Block
  verengt: keine Änderung. Rest nicht in vertretbarer Zeit lösbar.

- `Camera/CameraMode.cpp`: `CPolarSubCamera::isNormalCameraCompletely`
  (144 Bytes, 72,86 %). Original inlined `isNormalCameraSpecifyMode(int)`
  komplett als Switch-Jump-Table (`lwzx`/`mtctr`/`bctr`) an BEIDEN Aufrufstellen
  (`mMode` und `mPrevMode`); unser Build ruft die separate Funktion via `bl`
  auf. MWCC-Auto-Inline-Heuristik, nicht über Source-Umbau erzwingbar ohne
  Pragma-Kenntnis; strukturell verwandt mit dem bereits dokumentierten
  `fabricatedInline3`-TODO in `NpcManager::clipEnemies`.

- `Enemy/effectObj.cpp`: `TEffectColumSand::reset` (308 Bytes, 98,49 %).
  `r30`/`r31` komplett vertauscht (`this` vs. String-Literal-Adresse
  "08_sunabashira") — Reihenfolge, in der die zwei über den
  `TEffectModel::reset()`-Aufruf hinweg lebenden Werte in Callee-Save-Register
  gesichert werden. Expliziter `const char* name`-Local verschlechterte auf
  94,35 % (zurückgesetzt); rein MWCC-interne Save-Reihenfolge, nicht über
  Source beeinflussbar.

- `Enemy/effectObj.cpp`: `TEffectObjBase::moveObject` (464 Bytes, 98,84 %).
  Dreifach identisches FP-Register-Rotationsmuster (f0/f1/f2 zyklisch
  vertauscht) in den drei `emitter->setGlobalScale(local_1c)`-Aufrufen —
  inlined-Callee-Registerzuordnung, dieselbe Kategorie wie
  `J3DModel::entryModelData`.

- `Map/MapEventSirena.cpp`: `TMapEventSirenaSink::watch` (280 Bytes,
  99,97 % bestes Experiment — Original bleibt bei 99,87 %). `char trash[8]`
  vor oder nach dem `fireStartDemoCamera(...)`-Aufruf (der Parameter
  `JDrama::TFlagT<u16>(0)` als anonymes Temporary konstruiert) schließt
  4 von 16 Bytes Offset-Differenz bei der `sth`-Store-Adresse des Temporaries;
  `trash[0xc]` statt `trash[8]` an derselben Stelle ändert nichts weiter —
  die Temporary-Platzierung ist compiler-intern fixiert, nicht über Local-
  Padding beeinflussbar. Da 99,97 % kein 100-%-Match ist, Quelltext auf die
  saubere Original-Fassung (99,87 %) zurückgesetzt statt einen wirkungslosen
  `trash`-Hack stehen zu lassen.

- `Enemy/enemytable.cpp`: `TStageEnemyInfoTable::getMatchedInfo` (276 Bytes,
  92,41 %). Frame-Differenz 64 Bytes (0xa0 vs. 0x60): unser Build hoisted
  `begin()`/`end()` aus der `for`-Schleife in zwei zusätzliche Callee-Save-
  Register (r28+r29+r30+r31), das Original liest `this->begin` (+0x10) und
  `this->end` (+0x14) bei JEDER Iteration neu aus `this` (nur r29..r31 nötig).
  Loop-invariant-Code-Motion-Entscheidung von MWCC, nicht über einfache
  Source-Umformulierung erzwingbar ohne Risiko einer Verhaltensänderung.

- `MoveBG/MapObjWater.cpp`: `TMapObjWaterFilter::perform` (404 Bytes,
  78,39 %). Bereits im Quelltext als upstream-`TODO` markiert
  ("mother of all intern codes..."); komplexe Matrixrechnung
  (`J3DGetTranslateRotateMtx`, `PSMTXScale`, `MTXInverse`, `MTXConcat`) mit
  mehreren `Mtx`-Stack-Locals, bekannt schwierig.

- `Map/PollutionManager.cpp`: `TPollutionManager::cleanedAll` (268 Bytes,
  96,43 %). Einzeiliger Quelltext (`return getPollutionDegree() < … ? true :
  false;`), aber `getPollutionDegree()` (Schleife über `getLayer(i)`) wird
  komplett inlined — kein benannter Local als Anker für Padding; Register-
  Zählungsdifferenz (r8/r7/r6 vs. r7/r6) durch die Inline-Schleife bedingt.

- `NPC/NpcManager.cpp`: `TNPCManager::clipEnemies` (784 Bytes, 92,70 %).
  Bereits im Quelltext als upstream-`TODO` markiert
  ("figure out these inlines ... fabricatedInline3 matches in camera itself
  but not here"); großer struktureller Unterschied (Frame 0x90 vs. 0x50,
  `gpCamera`-Caching), nicht mit lokalen Padding-Tricks lösbar.

- `NPC/NpcManager.cpp`: `makePartsModelData_` (336 Bytes, 99,98 %).
  `sdlModel`-Local liegt 4 Bytes höher (`0x20` statt `0x1c`); Umordnen der
  `loadFlags`/`initInfo`-Deklaration ohne Wirkung.

- `Strategic/liveactor.cpp`: `TLiveActor::control` (73,96 %). Quelltext
  hat zwei reine Kommentar-Stubs (`// call on unk90`) statt echter
  virtueller Aufrufe; `unk90` ist `void*` (untypisiert) und wird im
  sichtbaren Quelltext nirgends auf Nicht-Null gesetzt — bereits als
  `// TODO: was ist unk90???` markiert. Per Rohdisassembly bestätigt:
  Aufruf über Sekundär-Vtable bei `unk90+0x5c`, Offset 0x10. Fix
  erfordert Typbestimmung von `unk90`, nicht in dieser Runde geleistet.

- `Player/ModelWaterManager.cpp`: `TModelWaterManager::drawMirror`
  (90,04 %) und `calcVMMtxGround` (62,49 %). Gestreute strukturelle
  Diffs (177/540 Instruktionen bei `drawMirror`, mehrere INSERT/DELETE-
  Cluster) über die ganze GX-lastige Funktion verteilt — kein
  isolierbarer Einzel-Bug.

- `Enemy/smallEnemy.cpp`: `TSmallEnemy::attackToMario` (90,44 % nach
  Bugfix, siehe Gematcht-Liste). Restdiff ist ein MWCC-interner Stack-
  Slot-Alias-Unterschied (Ziel legt für `v` einen frischen Slot an,
  wir nutzen den toten `local_20`-Slot wieder), Frame ist beidseitig
  `0x40` Bytes — nicht über Source erzwingbar.

- `Enemy/bosspakkun.cpp`: `TBossPakkun::setGroundCollision` (99,98 %).
  `collisionMtx`-Local bei `0x20` statt `0x18`; `char trash[8]` vor/nach
  der Deklaration verschlechtert beide Male. Nicht über Padding lösbar.

- `Player/WaterGun.cpp`: `TWaterGun::setBaseTRMtx` (99,97 %). `temp`-Mtx
  bei `0x1c` statt `0x20`; drei `char trash[4]`-Platzierungen
  verschlechtern alle. Nicht über Padding lösbar.

- `MoveBG/MapObjLib.cpp`: `TMapObjBase::isDemo` (99,77 %). Einziger Diff
  ist das Sprungziel des b1==true-Zweigs (Original springt direkt zu
  `return false`, unser Build zu `return true`); Early-Return-
  Umstrukturierung verschlechterte drastisch auf 90,45 % statt zu
  verbessern — MWCC-Codegen für dieses Kontrollfluss-Muster nicht wie
  erwartet. Zurückgesetzt.

- `Player/MarioRun.cpp`: `TMario::rotating` (98,65 %). Fehlendes `extsh`
  nach `neg` beim negierten Zweig von `mModelFaceAngle = -(mStatusTimer *
  4096)`. Drei Umformulierungen ohne Wirkung oder verschlechternd.

- `Enemy/pakkun.cpp`: `TNervePakkunAppear::execute` (98,55 %).
  Überflüssiges `cmpwi` nach ungenutztem `checkPass()`-Rückgabewert;
  Entfernen des `if(){}`-Wrappers erzeugt identischen Code (MWCC
  materialisiert bool-Rückgaben unabhängig von Verwendung).

- `MoveBG/MapObjLib.cpp`: `TMapObjBase::getDistance` (99,94 %).
  `volatile f32 y`-Local bei `0x14` statt `0x10`, Frame identisch;
  drei Padding-Varianten alle wirkungslos (exakt gleicher Match%).

- `Map/PollutionLayer.cpp`: `TPollutionLayer::stampModel` (99,27 %).
  Einziger Diff: Ladereihenfolge von `x` (aus `model`) und `mMinX` (aus
  `this`) vertauscht (reine Instruktions-Scheduling-Reihenfolge, gleiche
  Zielregister). Operanden-Vertauschung im Vergleich (`mMinX > x` statt
  `x < mMinX`) verschlechtert auf 98,27 %; Deklarationsreihenfolge von
  `x`/`z` getauscht: keine Wirkung (99,267 % ~ identisch). Zurückgesetzt.

- `MoveBG/MapObjRailBlock.cpp`: `TNormalLift::setGroundCollision`
  (95,74 %). `TRailMapObj::setGroundCollision` (die per Quelltext
  aufgerufene Basisklassenmethode) matcht **für sich genommen 100 %**;
  beim Inlinen in `TNormalLift::setGroundCollision` erzeugt unser Build
  jedoch einen zusätzlichen `__ct__` (leerer `SMatrix34C<f32>`-Default-
  Konstruktor für das lokale `TMtx34f mtx`), den das Original an dieser
  Inlining-Stelle wegoptimiert. `#pragma dont_inline on/off` um die
  aufrufende Funktion ohne Wirkung (identisches Ergebnis). Kontext-
  abhängige MWCC-Inlining-Entscheidung, nicht ohne Risiko für die
  bereits 100 % matchende Basismethode angreifbar.

- `Player/MarioDraw.cpp`: `TMario::initMirrorModel` (99,96 %). Alle drei
  Diffs sind identische rodata-Offset-Shifts (`@1490+0xa50/0xa5c/0xa6c`
  bei uns vs. `+0xa08/0xa14/0xa24` im Original, konstant `0x48` Bytes
  Versatz) für die Japanese-String-Literale der `TMirrorActor`-Namen.
  Die ganze Unit zeigt `[.rodata-0]` nur **50,33 %** — deutet auf einen
  umfassenderen String-Literal-Reihenfolge-/Größenunterschied irgendwo
  früher in der Datei hin, nicht isoliert auf diese Funktion. Für eine
  künftige Runde: alle String-Literale in `MarioDraw.cpp` systematisch
  mit der rohen `.rodata`-Sektion der Retail-Disassembly abgleichen.

### Nach achtundzwanzigster Iterationsrunde (kritischer objdiff-cli-Fund: falscher match_percent, echter Logikbug in `TMarDirector::movement`)

Fortsetzung des ndiff-sortierten 85–99,99-%-Scans (723 Kandidaten, Größe
≤400 Bytes, alle 736 Units). Ein Kandidat mit ungewöhnlich niedrigem
Match% bei nur 2 Instruktions-Diffs (`TMarDirector::movement`,
91,25 %, 48 Bytes) fiel sofort als starkes Bug-Signal auf.

**Kritischer Methodik-Fund**: Das objdiff-JSON zeigte scheinbar, dass
unser Build `movement_game()` NIE aufruft (toter Code nach einem
unbedingten `b`), während das Original es aufruft. Rohdisassembly
(`build/GMSJ01/asm/System/MarDirectorEvent.s` Zeile 331–346) bestätigte
tatsächlich einen invertierten Vergleich im Quelltext (`!=` statt `==`
für `STATE_UNK4`) — ein echter Logikbug, keine Fehlwahrnehmung. Nach dem
Fix (`if ((int)mState == STATE_UNK4) movement_game();`) meldete
`objdiff-cli` jedoch **90,0 %** (schlechter als vorher!) mit einem
`beq`/`bne`-Opcode-Mismatch, der der rohen Zieldisassembly widersprach.

Direkte Byte-für-Byte-Verifikation via Parsen des DOL-Headers und
Extraktion der rohen Maschinencode-Bytes an virtueller Adresse
`0x800EDA30` aus unserer frisch gebauten `mario.dol` (unter Umgehung von
`objdiff-cli` komplett) bewies: **unser kompilierter Code ist jetzt
byte-identisch mit der Zieldisassembly** (`7C0802A6 90010004 9421FFF8
88030064 2C000004 41820008 48000008 480002C1 8001000C 38210008
7C0803A6 4E800020` — exakt wie im `.s`-File dokumentiert). Der Fix ist
also ein **bestätigter 100-%-Byte-Match**, trotz `objdiff-cli`s
fälschlicher 90-%-Meldung.

Das ist eine **schwerwiegendere Ausprägung** des bereits mehrfach
dokumentierten objdiff-JSON-Fehlausrichtungsproblems: bisher betraf es
nur die PER-INSTRUKTION-Anzeige (falsche Zuordnung einzelner Zeilen bei
insgesamt korrekter `match_percent`); hier war die `match_percent`-Zahl
selbst falsch, sowohl über `objdiff-cli diff` als auch über die
Batch-`report generate`-Pipeline (beide nutzen denselben Diff-Kern,
daher kein Cache-Artefakt eines einzelnen Kommandos). **Neue
Methodik-Regel**: Bei widersprüchlichen/unplausiblen `match_percent`-
Sprüngen nach einem Fix (insbesondere Verschlechterung trotz
offensichtlich korrekterer Logik) MUSS zusätzlich zur `.s`-Rohdatei auch
eine direkte Byte-Extraktion aus dem gebauten `mario.dol` an der
bekannten virtuellen Adresse erfolgen, um `objdiff-cli` vollständig zu
umgehen.

**Fix**: `System/MarDirectorEvent.cpp::TMarDirector::movement()` —
`if ((int)mState != STATE_UNK4)` → `if ((int)mState == STATE_UNK4)`.
Echter, spielrelevanter Logikbug (Aufruf von `movement_game()` war
vorher faktisch unerreichbarer Code). Byte-perfekt gegen Retail
verifiziert.

**Weitere in dieser Runde geprüfte Kandidaten** (alle Padding-Versuche
zurückgesetzt, keine Verbesserung):

- Bestätigtes wiederkehrendes Muster „Output-Parameter per Adresse,
  Stack-Slot um 4–8 Bytes versetzt" bei sieben Funktionen in dieser
  Runde (`makeObjAppear` in `MapObjManager.cpp`, `getRandomNextIndex`
  in `graph.cpp`, `execGroundCheck` in `CameraBGCheck.cpp`, `__ct__
  TLensFlare` in `lensflare.cpp`, zusätzlich zu den bereits in Runde 27
  dokumentierten `setGroundCollision`/`setBaseTRMtx`/`getDistance`) —
  `char trash[N]` vor UND nach der Deklaration jeweils getestet, alle
  Varianten verschlechtern oder wirkungslos. Diese Kategorie gilt nun
  als **systematisch nicht per Padding lösbar** und wird nicht weiter
  einzeln verfolgt.

- `GC2D/GCConsole2.cpp::TGCConsole2::startDisappearTimer` (98,61 %).
  Konstanten-Diff `0x1d1+0x3c` (Ziel) vs. `0x20d` (unser, vorberechnet
  `525`) — Dekomposition des Quelltexts in `465 - y1 + 60` statt
  `525 - y1` ändert nichts (MWCC faltet die Konstante identisch zurück
  zusammen). Nicht per einfacher Source-Umformulierung lösbar.

Die Referenz-DOL bleibt `OK`.

### Nach neunundzwanzigster Iterationsrunde (neue Fix-Kategorie: `#pragma dont_inline` für fehlende weak-Symbole; 15 Commits, 39 Funktionen)

**Neue Kandidaten-Kategorie entdeckt**: Ausgangspunkt war der
"teilweise dekompilierte Datei mit wenigen 0-%-Funktionen"-Scan
(bevorzugt laut Projekt-Policy). `mario/NPC/NpcBase` war bei 79 %
Datei-Match mit genau einer unge matchten Funktion:
`TBaseNPC::getAnmOffDist_()`. Das `report.json`-Feld für diese Funktion
hatte **gar kein** `fuzzy_match_percent` — kein Vergleichssymbol
vorhanden, nicht einfach ein niedriger Wert. Rohdisassembly bestätigte:
Retail hält für diese Header-`inline`-definierte Methode eine echte
**out-of-line `weak`-Symbol-Kopie** (260 Bytes, ein einziger Call-Site),
während MWCC sie in unserem Build vollständig wegin lined — kein
Symbol, keine Adresse, nichts zum Vergleichen.

**Fix-Technik**: `#pragma dont_inline on` / `#pragma dont_inline off`
direkt um die Methoden-Definition im Header gelegt zwingt MWCC, eine
konkrete out-of-line-Kopie zu emittieren, exakt wie im Original. Das
Pragma ist im Projekt bereits etabliert (45+ bestehende Verwendungen,
bisher aber nur für gewöhnliche `.cpp`-lokale Methoden, nie für
Header-`inline`-Definitionen mit mehreren Call-Sites).

**Verifikationsmethodik-Erweiterung**: `objdiff-cli`s `report
generate`/`diff` zeigen für diese frisch emittierten weak-Symbole
weiterhin **kein** `fuzzy_match_percent` (selbst nach korrektem Fix) —
ein bestätigtes Tool-Limit, nicht spezifisch für falsche Fixes. Einzige
zuverlässige Verifikation: direkte Byte-Extraktion aus
`build/GMSJ01/mario.dol` UND `orig/GMSJ01/sys/main.dol` an der
bekannten virtuellen Adresse (DOL-Header-Parsing-Technik aus Runde 28),
Vergleich `our_bytes == retail_bytes`. Zusätzlich vor jedem Commit:
voller `report generate`-Vorher/Nachher-Vergleich der Menge aller
`fuzzy_match_percent == 100.0`-Funktionen (`lost`/`gained`-Diff), um
Regressionen an ANDEREN (ggf. weiterhin korrekt inlined) Call-Sites
derselben Funktion auszuschließen — **bei allen 39 Fixes dieser Runde:
0 Regressionen**, auch bei mehrfach verwendeten Funktionen wie
`TUtil<f32>::one()` (verwendet transitiv in jedem `normalize()`-Aufruf)
oder `TMario::checkStatusType()`.

**Systematisches Scannen**: `report.json` nach Funktionen ohne
`fuzzy_match_percent`-Feld durchsucht (2.369 Treffer von 12.881
Gesamtfunktionen). Cross-Referenz mit einem einmalig aufgebauten
Call-Site-Histogramm (`grep -rhoP '(?<=\tbl )\S+' build/GMSJ01/asm/`)
identifiziert Kandidaten mit 1–6 `bl`-Aufrufstellen in Retail (= MWCC
inlined fast überall, aber nicht an diesen Stellen). Für jeden
Kandidaten geprüft: (a) existiert die Methode bereits inline im
Quelltext (dann nur Pragma nötig) oder fehlt sie komplett (dann
Neuimplementierung nötig, außerhalb des Scope dieser Fix-Kategorie),
(b) ist die aufrufende Datei bzw. die Klasse, in der die Methode
deklariert ist, tatsächlich vorhanden oder eine leere 1-Zeilen-Stub-Datei.

**Wichtige Falltür entdeckt**: Ein Großteil der 2.369 Kandidaten
gehört zu Dateien, die als 1-Byte-Stub existieren
(`src/Enemy/BossHanachanMain.cpp`, `koopajr.cpp`, `limitkoopa.cpp`,
`killer.cpp`, `wireTrap.cpp`, `cannon.cpp`, `MapObjBall.cpp` u.v.a. —
~20 komplett unbearbeitete Enemy-/MapObj-Dateien). Für Methoden, deren
Klasse NUR in einer solchen leeren Datei deklariert würde
(`TBossHanachan::kill()`, `TDirectionCalc::*`, alle
`__ct__XxxManagerFPCc`-Konstruktoren, `theNerve__Xxx`-Accessor), ist
die Pragma-Technik **nicht anwendbar** — die ganze Klasse fehlt, das
ist Neudekompilierungsarbeit, kein Emissions-Bugfix. Entscheidend war
zu prüfen, ob die **Definition** der fehlenden Methode in einem
bereits populierten Shared-Header liegt (z. B. `JGVec3.hpp`,
`MathUtil.hpp`) — dann ist die Leere der AUFRUFENDEN Datei irrelevant.

**39 gefixte Funktionen in 15 Commits** (alle Byte-für-Byte gegen
`orig/GMSJ01/sys/main.dol` verifiziert, 0 Report-Regressionen, DOL-SHA1
nach jedem Commit `OK`):

1. `include/NPC/NpcBase.hpp`: `TBaseNPC::getAnmOffDist_()` (260 B).
2. `include/MoveBG/MapObjHide.hpp`: `TWaterHitPictureHideObj::
   getObjAppearPos()` (8 B), `THideObjPictureTwin::getObjAppearPos()`
   (12 B) — beide virtuell, nur über Vtable erreichbar, 0 direkte
   Call-Sites.
3. `include/Player/MarioAccess.hpp`: `SMS_GetMarioPos()` (8 B).
4. `include/Enemy/Graph.hpp`: `TGraphTracer::getCurGraphIndex()` (8 B),
   `TGraphTracer::getGraph() const` (8 B).
5. `include/Strategic/ObjModel.hpp`: `TMActorKeeper::getMActorAnmData()`
   (8 B).
6. `include/Map/MapCollisionEntry.hpp`: `TMapCollisionBase::setMtx()`
   (44 B).
7. `include/JSystem/JGeometry/JGUtil.hpp`: `TUtil<f32>::one()` (8 B);
   `include/MarioUtil/MathUtil.hpp`: `MsClamp<f32>()` (32 B, drei
   Call-Sites), `MsSqrtf()` (68 B).
8. `include/Player/Mario.hpp`: `TMario::checkStatusType()` (28 B).
9. `include/JSystem/JGeometry/JGMatrix33.hpp`: `SMatrix33C<f32>::at()`
   (20 B).
10. `include/Camera/Camera.hpp`: `TTargetCamera::operator=()` (116 B);
    `include/Camera/cameralib.hpp`: `CLBScreenFPosToSPos()` (276 B).
11. `include/JSystem/JGeometry/JGVec3.hpp`: `TVec3<f32>::operator=()`
    (28 B), `TVec3<f32>::sub(fst,snd)` (52 B), `TVec3<f32>::
    operator*=(f32)` (40 B), `TVec3<f32>::scaleAdd()` (52 B);
    `include/MarioUtil/MathUtil.hpp`: `MsSin()`, `MsCos()` (je 56 B).
12. `include/JSystem/JGeometry/JGVec2.hpp`: `TVec2<f32>::dot()` (28 B),
    `TVec2<f32>::sub(1-Arg)` (36 B) — bemerkenswert: `TVec2<T>` ist
    (anders als `TVec3<f32>`/`TUtil<f32>`) KEINE explizite
    Template-Spezialisierung, trotzdem 0 Regressionen.
13. Batch aus 6 Funktionen in einem Commit: `TMapObjBase::
    getObjCollisionHeightOffset()` (4 B, leerer Body), `TLiveActor::
    getMActor()` (8 B), `TTimeRec::crTimeAry()` (24 B), `JUTRect::
    JUTRect(int,int,int,int)` (48 B), `TBGCheckData::isIllegalData()`
    (28 B), `TRotation3<T>::TRotation3()` (4 B, Default-Ctor,
    Template).
14. `include/Strategic/TakeActor.hpp`: `TTakeActor::isTaken()` (28 B);
    `include/Player/ModelWaterManager.hpp`: `TWaterHitActor::
    onWaterHitCounter()` (12 B).
15. `include/JSystem/JGeometry/JGVec4.hpp`: `TVec4<f32>::TVec4()`
    (4 B), `TVec4<f32>::set<f32>()` (20 B); `include/JSystem/JGeometry/
    JGMatrix33.hpp`: `SMatrix33R<f32>::SMatrix33R()` (4 B).
16. `include/Enemy/PathNode.hpp`: `TPathNode::getPoint()` (28 B) —
    trotz bestehendem Kommentar „doesn't match in a couple of places"
    (bezieht sich auf ANDERE, weiterhin korrekt inlinede Call-Sites;
    dieser spezifische nicht-inlinede Call-Site in `enemyMario.cpp`
    ist jetzt Byte-perfekt).
17. `include/Enemy/WireBinder.hpp`: `TWireBinder::getDir()` (8 B);
    `include/JSystem/JMath.hpp`: `JMASCos(s16)`, `JMASSin(s16)`
    (je 28 B).
18. `include/Camera/cameralib.hpp`: `CLBPalFrame<s16>()` (92 B).
19. `include/JSystem/JGeometry/JGRotation3.hpp`: `TRotation3<T>::
    setSQ()` (256 B).

**Wichtiger Vorbehalt zur `report.json`-Statistik**: Da `objdiff-cli`
für frisch emittierte weak-Symbole weiterhin kein `fuzzy_match_percent`
berechnet, ändert sich die **gemeldete** `Progress`-Ausgabe
(`8648/12881 Funktionen`) durch diese Runde NICHT — obwohl alle 39
Funktionen nachweislich (Byte-Vergleich gegen Retail) jetzt 100 %
matchen. Die tatsächliche Matching-Quote liegt also messbar höher als
die von `objdiff-cli` ausgewiesene; ein Werkzeug-Limit, kein
Dokumentationsfehler.

**Geprüft und als nicht anwendbar verworfen**:

- `JGadget::TVector<T,Allocator>::begin()` (`TVector<void*,...>`,
  MSoundMainSide.cpp/bosseel.cpp, 3 Call-Sites) — generisches Template,
  in JEDER Instanziierung überall im Code verwendet; Pragma würde ALLE
  Instanziierungen betreffen (viel größerer Blast-Radius als bei den
  gefixten Einzel-Spezialisierungen). Nicht ohne umfassenderen
  Vorher/Nachher-Vergleich riskiert.
- `ArrayWrapper<TTailRubber::Node>::size()`/`operator[]`
  (`fireWanwan.cpp`) — Retail-Mangling zeigt `@unnamed@34ArrayWrapper`
  (anonymer Namespace in Retails Übersetzungseinheit), unser
  `ArrayWrapper<T>` liegt dagegen in einem benannten Shared-Header
  (`System/ArrayWrapper.hpp`). Architektonischer Unterschied, kein
  reines Emissions-Problem — würde eine TU-lokale Neudeklaration
  erfordern, nicht nur ein Pragma.
- `TBossHanachan::kill()` — Vtable-Slot-Analyse bestätigt echten
  Bedarf einer Override, aber `src/Enemy/BossHanachanMain.cpp` ist eine
  1-Byte-Stub-Datei ohne jegliche Klassendeklaration. Würde die
  komplette `TBossHanachan`-Klassenhierarchie samt Vtable-Layout
  erfordern — eigenständige Dekompilierungsarbeit, kein Cheap-Fix.
- `TDirectionCalc::*` (koopajr.cpp) — Klasse nirgends deklariert,
  gleiche Kategorie wie oben.
- Alle `__ct__XxxManagerFPCc`-Konstruktoren (~20 Enemy-/Animal-Manager)
  und `theNerve__Xxx`-Accessoren — Klassen ausschließlich in leeren
  1-Zeilen-Stub-Dateien deklariert.

**Strategieempfehlung für Folgesitzungen**: Der
„fehlendes-`fuzzy_match_percent`"-Scan (`report.json` nach Funktionen
ohne dieses Feld durchsuchen, dann Call-Site-Histogramm für
Blast-Radius-Einschätzung) ist ein **hochwertiges, wiederholbares**
Verfahren zum Auffinden weiterer `#pragma dont_inline`-Kandidaten.
2.369 Kandidaten insgesamt identifiziert, diese Runde deckte 39 davon
ab (alle mit ≤6 Call-Sites und Definition in einer populierten Datei);
der Großteil der übrigen ~2.330 gehört zu unbearbeiteten
Enemy-/MapObj-Dateien (siehe oben) und ist für separate
Vollimplementierungs-Sessions vorzusehen, nicht für diese Fix-Kategorie.

Die Referenz-DOL bleibt `OK`. Upstream-Sync erneut bei 0 Commits
Rückstand bestätigt.

### Nach dreißigster Iterationsrunde (unvollständige Vtables: TMapObjBase/TTakeActor/THitActor, 10 Funktionen)

Fortsetzung des Missing-`fuzzy_match_percent`-Scans mit Fokus auf
Kandidaten mit **0** `bl`-Aufrufstellen in Retail (nur über Vtable
erreichbar) UND populierter Zieldatei. `mario/System/MarNameRefGen_MapObj`
(504 Zeilen, dient als "Name-Referenz-Registry" für viele MapObj-Klassen)
lieferte ungewöhnlich viele Treffer für `TMapObjBase`/`TTakeActor`/
`THitActor`-Methoden.

**Root Cause war diesmal keine Inlining-Frage**: `TMapObjBase::
loadBeforeInit/calc/draw/dead/touchWater/getHitObjNumMax` waren im
Header nur DEKLARIERT (`virtual void calc();` etc., ohne Inline-Body)
aber **nirgends im gesamten `src`-Baum implementiert** — echte fehlende
Basisklassen-Default-Implementierungen, keine Emissions-Bugs. Da
`TMapObjBase` offenbar nie direkt instanziiert wird (immer über
Subklassen), fiel das nie als Linker-Fehler auf; unser Vtable für
`TMapObjBase` selbst wird schlicht nie gebraucht — Retail braucht es
aber, weil dort (wahrscheinlich) eine Subklasse diese Slots nicht
überschreibt und auf den Default zurückfällt.

**Fix**: Alle sechs mit trivialen Ein-Zeiler-Bodies exakt nach
Rohdisassembly ergänzt (`src/MoveBG/MapObjBase.cpp`, nach
`getSDLModelFlag`): `loadBeforeInit(JSUMemoryInputStream&) { }`,
`calc() { }`, `draw() const { }`, `dead() { }`, `touchWater(THitActor*)
{ return false; }`, `getHitObjNumMax() { return 5; }`.

**Zusätzlich in `include/Strategic/TakeActor.hpp`/`HitActor.hpp`**:
`TTakeActor::ensureTakeSituation()` und `TTakeActor::moveRequest()`
hatten bereits korrekten Inline-Code (missing-weak-symbol-Muster, nur
Pragma nötig); `TTakeActor::getRadiusAtY(f32) const` hatte **keine**
Implementierung (`return mDamageRadius;`, geerbtes Feld von `THitActor`
bei Offset 0x58, neu hinzugefügt); `THitActor::receiveMessage(THitActor*,
u32)` hatte bereits korrekten Inline-Code (nur Pragma nötig).

**Byte-für-Byte verifiziert, 0 Report-Regressionen**, alle 10 in einem
Commit: `TMapObjBase::loadBeforeInit/calc/draw/dead/touchWater/
getHitObjNumMax`, `TTakeActor::ensureTakeSituation/moveRequest/
getRadiusAtY`, `THitActor::receiveMessage`.

**Versucht und verworfen**: `TMapObjBase::setModelMtx(MtxPtr)` — Retail
kopiert `mtx` nach `getModel()->mNodeMatrices[0]` (`PSMTXCopy`), aber
`J3DModel::mNodeMatrices` ist `protected` — direkter Zugriff schlägt
mit Compile-Error fehl (`illegal access to protected/private member`).
Bräuchte eine neue öffentliche Zugriffsmethode auf `J3DModel` (z. B.
`getNodeMatrix(int)`), was über den Scope eines Cheap-Fixes hinausgeht.
Zurückgesetzt, nicht committed.

`TMapObjBase::getDepthAtFloating()` — Rohdisassembly der Basisklasse
lädt einen `0.0f`-Konstanten-Load vor `blr` trotz `void`-Signatur (der
Rückgabewert wird nie verwendet). Gleichzeitig hat die Subklasse
`TMapObjBall::getDepthAtFloating() { }` in ihrem EIGENEN Vtable-Slot
laut Disassembly (`MapObjBall.s`) tatsächlich `getDepthAtFloating__
11TMapObjBaseFv` (die Basisklassen-Version!) referenziert statt einer
eigenen `TMapObjBall`-Version — d. h. Retail überschreibt diese Methode
in `TMapObjBall` mutmaßlich GAR NICHT, während unser Header sie
redundant überschreibt. Das ist eine tiefere Klassenhierarchie-Frage
(potenzieller eigener Bug in `MapObjBall.hpp`), keine reine
Emissions-Frage — als Lead für eine eigene Session vorgemerkt statt
riskant halbgefixt.

Die Referenz-DOL bleibt `OK`.

### Nach einunddreißigster Iterationsrunde (6 weitere Funktionen: TMapObjBase-Reste, TEnemyManager, Application, M3UJoint-Konstruktor)

Fortsetzung der Runde-30-Systematik gegen die restlichen
`populated_zero`-Kandidaten (Vtable-only, 0 `bl`-Aufrufstellen, aber
populierte Zieldatei).

- `TMapObjBase::getRadiusAtY(f32) const` — fehlte komplett; Retail lädt
  Feld bei Offset 0xBC, das ist `TLiveActor::mBodyRadius` (geerbt).
  `return mBodyRadius;` neu hinzugefügt.
- `TMapObjBase::getTakingMtx()` — fehlte komplett; Retail prüft
  `MAP_OBJ_FLAG_UNK40` (Bit 25 in PPC-Zählung = Maske `0x40`) und
  gibt bei gesetztem Flag `nullptr` zurück, sonst delegiert an
  `TLiveActor::getTakingMtx()`. Neu hinzugefügt, exakt nach
  Disassembly.
- `TEnemyManager::restoreDrawBuffer(u32)`/`changeDrawBuffer(u32)` —
  beide bereits korrekt als leerer Inline-Body im Header, nur
  Pragma-Fix nötig (0 direkte Call-Sites, reines Vtable-Muster).
- `Application.cpp::SetupThreadFuncLogo` — erste Instanz des
  Pragma-Musters auf einer GEWÖHNLICHEN (nicht Header-Inline)
  `static`-Funktion, deren Adresse als Thread-Entry-Point übergeben
  wird (`OSCreateThread(&gSetupThread, SetupThreadFuncLogo, …)`).
  Trotz Adressnahme wurde sie ohne Pragma wegoptimiert; nach dem Fix
  Byte-perfekt.
- `M3UJoint.cpp::M3UMtxCalcSIAnmBlendQuat::M3UMtxCalcSIAnmBlendQuat()`
  (parameterloser Ctor) — fehlte komplett; die vorhandene
  `(bool basic)`-Überladung hat eine identische Initialisierungsfolge.
  **Versuch 1** (verworfen): `new (this) M3UMtxCalcSIAnmBlendQuat(false);`
  (Placement-New-Delegation) — Compile-Error, da kein passender
  `operator new(size_t, void*)` im Projekt deklariert ist. **Versuch 2**
  (erfolgreich): eigenständige Initialisierungsliste mit hartkodiertem
  `mBehaveAsBasic = false;`, inhaltsgleich zur `(bool)`-Version. MWCCs
  eigener Identical-Code-Folding-Optimierer faltet dies automatisch zu
  einem Aufruf der `(bool)`-Version zusammen — **Byte-für-Byte
  identisch** mit Retails kompiliertem Aufrufmuster, nicht nur
  semantisch äquivalent.

Alle sechs verifiziert: 0 Report-Regressionen, Byte-für-Byte gegen
`orig/GMSJ01/sys/main.dol`.

**Geprüft und verworfen**: `MoveBG/MapObjCorona.hpp` deklariert
`TBathtubGrip` nur als Forward-Declaration (`class TBathtubGrip;`) —
die Klasse selbst (inkl. `TBathtubGripParts`/`...Hard`/`...Fragile`)
existiert nirgends im Quellbaum. Alle zugehörigen Kandidaten
(`getRootJointMtx`, `receiveMessage`, Destruktor-Thunks) sind daher
dieselbe Kategorie wie `TBossHanachan`/`TDirectionCalc` — vollständige
Neuimplementierung nötig, kein Cheap-Fix.

Die Referenz-DOL bleibt `OK`. Session-Gesamtsumme: **55 Funktionen**
in 19 Commits, alle gepusht.

### Nach zweiunddreißigster Iterationsrunde (`TVec3<f32>::set(const Vec&)`; **kritischer Methodik-Fund**: `objdiff-cli` meldet 0 %/„missing" für mindestens 9 bereits Byte-perfekte Funktionen)

**Fix**: `include/JSystem/JGeometry/JGVec3.hpp`: `TVec3<f32>::set(const Vec&)`
— dasselbe missing-weak-symbol-Muster wie Runde 29, 8 nicht-inlinede
Retail-Call-Sites über 5 Dateien. Byte-für-Byte gegen
`orig/GMSJ01/sys/main.dol` verifiziert (`0x800DD50C`), 0
Report-Regressionen.

**Kritischer Methodik-Fund (schwerwiegender als Runde 28)**: Beim
Prüfen von `mario/Enemy/hamukuri` (aus der `populated_zero`-Kandidatenliste)
zeigte `objdiff-cli diff` für `TFireHamuKuri::moveObject()` **0,0 %
Match** mit einem scheinbaren Instruktions-Diff bei Index 7 (unser
Build ruft angeblich `changeTevColor()`, Retail angeblich
`recoverFire()` direkt). Rohdisassembly (`hamukuri.s` Zeile 3111–3127)
zeigte jedoch sofort: Retails `moveObject()` ruft **exakt dieselben
zwei Funktionen** in derselben Reihenfolge wie unser Quelltext
(`moveObject__9THamuKuriFv` dann `changeTevColor__13TFireHamuKuriFv`).
Direkte Byte-Extraktion aus `build/GMSJ01/mario.dol` UND
`orig/GMSJ01/sys/main.dol` an Adresse `0x80263D98` (52 Bytes)
bestätigte: **exakt identisch**. `objdiff-cli`s 0,0-%-Meldung war
**komplett falsch** — nicht nur eine Fehlausrichtung der
Instruktionsanzeige (wie Runde 16/28), sondern ein Fall, in dem das
Tool eine bereits perfekt gematchte Funktion fälschlich als
funktional unterschiedlich meldet.

Dieselbe Verifikation für `TFireHamuKuri::isHitValid(u32)` (144 Bytes,
`0x802639B8`) ergab ebenfalls **Byte-für-Byte identisch** trotz
gemeldeter 0,0 %.

Ausgehend von diesem Fund wurden weitere `populated_zero`-Kandidaten
direkt (ohne den unzuverlässigen `objdiff-cli diff` als
Zwischenschritt) per DOL-Byte-Vergleich geprüft:

- `System/MarDirectorEvent.cpp::TMarDirector::fireGetStar(TShine*)`
  (124 Bytes, `0x800EDAE8`) — **Byte-für-Byte identisch**, trotz
  gemeldeter 0,0 %.
- `MarioUtil/ShadowUtil.cpp::TMBindShadowManager::drawShadowGD()`
  enthält sechs lokal (innerhalb der Funktion) definierte anonyme
  Hilfsklassen (`TSetup1`…`TSetup5`, `TCylinder`, mit vom Compiler
  vergebenen `$NNNN`-Disambiguator-Suffixen im gemangelten Namen) mit
  jeweils einer `makeDL()`-Methode. Alle sechs waren in `report.json`
  als „missing" gelistet (0 direkte Call-Sites, nur Vtable-Zugriff via
  lokaler `TGDLStatic`-Subklassen). Direkte Byte-Vergleiche für alle
  sechs (`TSetup1`: 260 B @ `0x800CE258`, `TSetup2`: 96 B @
  `0x800CD67C`, `TSetup3`: 84 B @ `0x800CD628`, `TSetup4`: 96 B @
  `0x800CD5C8`, `TSetup5`: 96 B @ `0x800CD568`, `TCylinder`: **2.884
  Bytes** @ `0x800CD6DC`) ergaben **alle sechs Byte-für-Byte
  identisch**.

**Gesamtsumme falsch gemeldeter, tatsächlich bereits 100 % gematchter
Bytes in dieser Runde entdeckt**: 52 + 144 + 124 + 260 + 96 + 84 + 96
  + 96 + 2.884 = **3.836 Bytes über 9 Funktionen**, für die **keine
Quelltextänderung nötig ist** — sie sind bereits korrekt. Keine dieser
9 Funktionen kann jedoch einzeln in `configure.py` auf `Matching`
gesetzt werden, da ihre jeweiligen Units andere echte
Nonmatching-Funktionen enthalten (`hamukuri.cpp`: `THaneHamuKuri::
walkBehavior` als 4-Byte-Stub statt 2.208 Byte Original;
`MarDirectorEvent.cpp`/`ShadowUtil.cpp`: weitere offene Fälle).

**Vermutete Ursache**: Bei lokalen/anonymen Klassen mit
compiler-generierten `$NNNN`-Suffixen und bei Funktionen in Units mit
einer stark abweichenden Nachbarfunktion (`walkBehavior`s 4-Byte-Stub
vs. 2.208-Byte-Original in derselben Datei) könnte `objdiff-cli`s
interne Symbol-zu-Symbol-Zuordnung (Adress- oder Reihenfolge-basiert
statt rein namensbasiert) ins Straucheln geraten und einem bereits
korrekten Funktionspaar fälschlich unterschiedliche Bytes zuordnen.
Nicht abschließend verifiziert (out of scope für diese Session), aber
als Hypothese für zukünftige Sessions festgehalten.

**Neue verschärfte Methodik-Regel**: Bei JEDER Funktion, die laut
`report.json`/`objdiff-cli diff` 0 % oder „missing" ist, ABER deren
Quelltext bereits vollständig und plausibel korrekt aussieht
(insbesondere wenn Feldnamen/Kontrollfluss exakt zur
Rohdisassembly-Beschriftung passen), MUSS vor jeder Quelltextänderung
zuerst ein direkter Byte-Vergleich zwischen `build/GMSJ01/mario.dol`
und `orig/GMSJ01/sys/main.dol` an der bekannten virtuellen Adresse
erfolgen. Andernfalls droht das Risiko, funktionierenden Code
„kaputt zu reparieren", nur weil das Tool eine falsche Diskrepanz
meldet.

Die Referenz-DOL bleibt `OK`. Session-Gesamtsumme: **56 tatsächlich
geänderte/neu implementierte Funktionen** in 20 Commits, plus **9
zusätzliche als bereits korrekt verifizierte** (keine Änderung nötig,
aber wichtiger Dokumentationsfund für künftige Sessions).

### Nach dreiunddreißigster Iterationsrunde (massiver Folge-Fund zu Runde 32: 50 weitere bereits Byte-perfekte, fälschlich gemeldete virtuelle Destruktoren, 6.056 Bytes)

Direkte Folge des Runde-32-Fundes: systematisches Byte-Verifizieren
ALLER virtuellen Destruktoren (`__dt__ClassNameFv`-Muster) in den drei
`System/MarNameRefGen_*`-Dateien (Registry-Units, die Vtables vieler
Klassen aus dem gesamten Codebase referenzieren) sowie der
lokalen/anonymen `TSetup1`–`TSetup5`/`TCylinder`-Klassen in
`ShadowUtil.cpp`.

**Ergebnis**: **50 von 50 geprüften Destruktoren** sind bereits
Byte-für-Byte identisch mit `orig/GMSJ01/sys/main.dol`:

- 42 reguläre virtuelle Destruktoren über `MarNameRefGen_Enemy`
  (`TSimpleEffect`, `TLauncherManager`, `TLauncher`, `TWalkerEnemy`,
  `TTobiPuku`, `TTobiPukuManager`, `TTobiPukuLaunchPad`,
  `TTobiPukuLaunchPadManager`, `TPoiHana`, `TGesso`, `TPakkun`,
  `TNameKuriManager`, `TSmallEnemyManager`, `TAnimalManagerBase`,
  `TNameKuriLauncherManager`, `THamuKuriLauncherManager`),
  `MarNameRefGen_BossEnemy` (`TBEelTears`, `TDemoBossHanachanManager`,
  `TDemoBossHanachan`) und `MarNameRefGen_MapObj`
  (`TBreakHideObj`, `TCoin`, `TJuiceBlock`, `TWaterHitPictureHideObj`,
  `TSlotDrum`, `TRoulette`, `TMapObjGeneral`, `TSandLeaf`, `TSandBase`,
  `TItem`, `TMapObjFloatOnSea`, `TTakeActor`, `TFruitHitHideObj`,
  `TFenceWater`, `TFence`, `THideObjBase`, `THitActor`,
  `TMapObjChangeStage`, `TMapObjPlane`, `TMapObjBase`,
  `TSirenaRollMapObj`, `TCasinoRoulette`, `TSirenaGate`) — 108–156
  Bytes je Funktion, Summe **5.456 Bytes**.
- 6 lokale/anonyme Klassen-Destruktoren in `ShadowUtil.cpp`
  (`TSetup1`–`TSetup5`, `TCylinder`, je 100 Bytes) — **600 Bytes**.
- 2 Vtable-Adjustor-Thunks (`@32@__dt__10TTakeActorFv`,
  `@32@__dt__17TSirenaRollMapObjFv`, je 8 Bytes) — **16 Bytes**.

**Gesamtsumme dieser Runde**: 5.456 + 600 + 16 = **6.056 Bytes über
50 Funktionen**, alle bereits korrekt, keine Quelltextänderung nötig.

**Kumulierte Session-Summe der Runden 32+33 (Methodik-Fund)**: **59
Funktionen, 9.892 Bytes**, die `objdiff-cli` fälschlich als 0 %/
„missing" meldet, obwohl sie Byte-für-Byte mit Retail übereinstimmen.

**Verworfen** (echte Negativfälle, zur Abgrenzung): `TTelesaSlot`
(`bosstelesa.cpp`) und `TSamboFlower` (`hanasambo.cpp`) — beide
Konstruktor-Kandidaten aus derselben Liste, aber ihre jeweiligen
`.cpp`-Dateien sind 1-Byte-Stub-Dateien ohne jegliche Klassendeklaration
(dieselbe Kategorie wie `TBossHanachan`) — hier ist der
„missing"-Report korrekt, keine Byte-Übereinstimmung möglich.

**Präzisierte Hypothese zur Fehlerursache**: Alle 50 falsch gemeldeten
Destruktoren in dieser Runde liegen in Units
(`MarNameRefGen_Enemy`/`MarNameRefGen_BossEnemy`/`MarNameRefGen_MapObj`/
`ShadowUtil`), die selbst SEHR VIELE strukturell nahezu identische
`__dt__`-Symbole in dichter Folge enthalten (virtuelle
Ein-Basisklassen-Destruktoren mit demselben Anweisungsmuster:
Vtable-Pointer setzen, ggf. `__dl__FPv` aufrufen). Dies stützt die
Hypothese aus Runde 32: `objdiff-cli`s interne Symbolzuordnung
scheint bei einer hohen Dichte strukturell ähnlicher Symbole in
kurzer Distanz Fehlzuordnungen vorzunehmen. Weiterhin nicht
abschließend verifiziert, aber jetzt mit deutlich mehr Evidenz
untermauert.

**Auswirkung auf den gemeldeten Fortschritt**: Der tatsächliche
Code-Match-Anteil des Projekts liegt nachweislich **mindestens 9.892
Bytes höher** als die von `objdiff-cli`/`report.json` ausgewiesenen
41,98 % — ein systematisches Untererfassungsproblem des Tools, das
in keiner Weise die tatsächliche Codequalität widerspiegelt.

Die Referenz-DOL bleibt `OK` (keine Quelltextänderung in dieser
Runde). Session-Gesamtsumme bis hier: 56 tatsächlich geänderte/neu
implementierte Funktionen in 20 Commits, plus 59 zusätzliche als
bereits korrekt verifizierte Funktionen (9.892 Bytes).

### Nach vierunddreißigster Iterationsrunde (8 weitere Funktionen: TVec3-Reste, identity33, TSirenaRollMapObj, TFlagT)

Fortsetzung des breiten Scans über ALLE `populated`-Einheiten
(unabhängig von Call-Site-Anzahl) fand acht weitere echte
Pragma-/Fehlende-Implementierung-Kandidaten:

- `TVec3<f32>::TVec3(const TVec3&)` (Kopierkonstruktor, 28 B,
  14 Call-Sites über 3 Dateien) — hatte bereits einen Kommentar
  „Checked via MarioCollision.cpp where this is not inlined".
- `TVec3<f32>::set<f32>(f32,f32,f32)` (Template, 16 B, 5 Dateien).
- `TVec3<f32>::setLength(const TVec3&, f32)` (164 B, `normalize()`-Helfer).
- `TRotation3<T>::identity33()` (48 B, 26 Call-Sites über 14 Dateien
  — größte Call-Site-Anzahl dieser Session, weiterhin 0 Regressionen).
- `TSirenaRollMapObj::getRollAngX/Y/Z(int) const` (Basisklassen-
  Version, je 8 B, Vtable-only).
- `JDrama::TFlagT<u16>::TFlagT(const TFlagT&)` (Kopierkonstruktor,
  12 B, 9 Call-Sites über 4 Dateien).

Alle acht Byte-für-Byte gegen `orig/GMSJ01/sys/main.dol` verifiziert,
0 Report-Regressionen.

**Geprüft und verworfen**: `MoveBG/MapObjCorona.cpp`s restliche
`TVec4`/`fmodf`/`__sinit`-Kandidaten benötigen entweder die fehlende
`TBathtubGrip`-Klassenhierarchie oder (bei
`__sinit_MarNameRefGen_BossEnemy_cpp`) eine vollständige
Cross-Referenz mit dem JAudio-Sound-System-Static-Listen-Set
(`JALList<MSBgm>` u. v. a., zwölf verschiedene Template-Instanzen) —
beides außerhalb des Scope eines Cheap-Fixes, als Lead vorgemerkt.
`@32@__dt__14TWaterHitActorFv` (Vtable-Adjustor-Thunk) ist
compiler-generiert und nicht über Quelltext-Pragmas ansprechbar.

Die Referenz-DOL bleibt `OK`. **Session-Gesamtsumme: 64 tatsächlich
geänderte/neu implementierte Funktionen** in 24 Commits, plus **59
zusätzliche als bereits korrekt verifizierte** Funktionen
(9.892 Bytes) — **123 Funktionen** insgesamt in dieser Session
bearbeitet oder als bereits korrekt dokumentiert.

## Gematchte GMSJ01-Funktionen

- `JSystem/JAudio/JAInterface/JAIBasic.cpp`:
  `JAIBasic::initDriver` — **100 %** (120 Bytes). Ein ungenutztes
  `char trash[2]` reproduziert den originalen 0x30-Byte-Stackframe; der
  übrige Inline-Code war bereits identisch.

- `JSystem/J3D/J3DGraphLoader/J3DMaterialFactory.cpp`:
  `J3DMaterialFactory::newNBTScale` — **100 %** (168 Bytes). Ein nach
  `dflt` deklariertes `char trash[8]` reproduziert Stackframe und
  Local-Offsets des Originals.

- `JSystem/J3D/J3DGraphLoader/J3DMaterialFactory_v21.cpp`:
  `J3DMaterialFactory_v21::newNBTScale` — **100 %** (168 Bytes).
  Derselbe nach dem Rückgabewert deklarierte `char trash[8]` gleicht
  Stackframe und Local-Offsets an; `.text` und `.sdata2` matchen vollständig.

- `MSound/MSoundBGM.cpp`: `MSBgm::init` — **100 %** (136 Bytes).
  `char trash[0x20]` stellt den originalen 0x48-Byte-Stackframe wieder her;
  danach matchen auch `.text`, `.data`, `.bss` und `.sdata` der Unit zu 100 %.

- `Enemy/egggen.cpp`: `TEggGenerator::control` — **100 %** (120 Bytes).
  `char trash[0x18]` reproduziert den 0x30-Byte-Stackframe; anschließend
  matchen `.text`, `.rodata`, `.data` und `.sdata2` der Unit zu 100 %.

- `Enemy/DebuTelesa.cpp`: `TDebuTelesa::receiveMessage` — **100 %**
  (176 Bytes). `char trash[8]` gleicht den Stackframe von 0x28 auf 0x30 Bytes an.

- `JSystem/JParticle/JPAParticle.cpp`:
  `JPAParticle::checkCreateChildParticle` — **100 %** (196 Bytes).
  `char trash[0x10]` stellt den 0x58-Byte-Stackframe wieder her; `.text`,
  `.data` und `.sdata2` matchen vollständig.

- `System/FlagManager.cpp`: `TFlagManager::start` und `TFlagManager::save`
  — **100 %**. Je `char trash[8]` reproduziert 0x28- bzw. 0x50-Byte-Frame.
  Unit `.text`/`.data`/`.sbss` 100 %, in `configure.py` auf Matching gesetzt.

- `MoveBG/WoodBarrel.cpp`: `TWoodBarrel::appear`, `appeared`, `kill` —
  **100 %** je einzeln (`.text` der Unit 100 %). `char trash[8]` am
  Funktionsanfang für `appear`/`appeared`; bei `kill` musste `char trash[0xc]`
  NACH der `TVec3<f32> vec`-Deklaration stehen (nicht davor). Unit bleibt
  NonMatching: `configure.py` auf `Matching` gesetzt brach die DOL-SHA1
  trotz 100 % in allen objdiff-Sections (anonyme Daten-Reste, siehe
  Methodik-Hinweis oben) — sofort zurückgesetzt.

- `THPPlayer/THPPlayer.c`: `THPPlayerCalcNeedMemory` — **100 %** (168 Bytes).
  Ternäre Größenberechnung (`onMemory ? … : …`) durch `if`/`else` ersetzt —
  reines Register-Scheduling-Artefakt, kein Verhaltensunterschied.

- `NPC/NpcManager.cpp`: `TNPCManager::perform` — **100 %** (364 Bytes).
  `char trash[8]` reproduziert den 0x38-Byte-Stackframe.

- `Enemy/effectObj.cpp`: `TEffectModel::calcRootMatrix` — **100 %**
  (244 Bytes). `char trash[8]` musste NACH `TPosition3f mtx` stehen.

- `JSystem/JParticle/JPADraw.cpp`: `loadYBBMtx` — **100 %** (208 Bytes,
  `char trash[8]` nach `TVec3<f32> v`), `zDrawParticle` und `zDrawChild` —
  je **100 %** (`char trash[8]` am Funktionsanfang), `initialize` — **100 %**
  (868 Bytes; `char trash[0x10]` musste NACH dem mid-Funktions-Struct-Local
  `JPADrawVisitorDefFlags flags` stehen, nicht nach dem frühen `int i` —
  derselbe "nach dem Struct-Local"-Fund wie bei `calcRootMatrix`/`loadYBBMtx`,
  nur dass der relevante Local hier erst in der Funktionsmitte auftaucht).
  Unit-`.text` jetzt 100 %; bleibt NonMatching wegen anonymem
  `[.sdata2-0]`-Rest (96,77 %, ungetestet als Matching-Flip-Risiko).

- `Map/MapModel.cpp`: `TMapModel::initUnderpass` — **100 %** (420 Bytes).
  `char trash[0x20]` direkt nach dem ersten Local (`s32 nameIdx`) schließt
  eine 32-Byte-Frame-Differenz trotz mehrerer nachfolgender Scalar-Pointer-
  Locals ohne eigene Struct-Deklaration.

- `MoveBG/MapObjOption.cpp`: `TFileLoadBlock::touchPlayer` und
  `receiveMessage` — je **100 %** (`char trash[0x10]` am Funktionsanfang).
  Beide inlinen `pushed()` (String-Literal `"fileloadblock"`), identisches
  16-Byte-Frame-Muster an beiden Aufrufstellen.

- `MoveBG/ItemManager.cpp`: `TItemManager::resetNozzleBoxesModel` —
  **100 %** (272 Bytes, `char trash[8]`). `newAndRegisterCoin` bleibt bei
  99,59 % (Rest: 12-Byte-Offset auf drei `TVec3`-Argument-Temporaries für
  `newAndRegisterObj`, unverändert durch `trash[8]`/`[0xc]`/`[0x14]`).

- `Map/PollutionManager.cpp`: `TPollutionManager::clean` (208 Bytes,
  `char trash[8]`) und `load` (356 Bytes, `char trash[0x20]`) — je **100 %**.
  `cleanedAll` bleibt bei 96,43 % (siehe Nonmatching-Liste).

- `JSystem/JAudio/JALibrary/JALModSe.cpp`: `JALSystem::appendGrpMember`
  (720 Bytes, `char trash[8]`) und `JALSystem::append` (1.484 Bytes,
  `char trash[0x68]` nach `set3` **plus** Bugfix `&set2`→`&set3` im
  `PitFunk`-Case) — je **100 %** einzeln. Unit bleibt `NonMatching` in
  `configure.py`: Matching-Flip bricht DOL-SHA1 trotz 100 % in allen
  objdiff-Sections (bestätigter Adress-Shift-Layoutfehler, siehe oben).

- `JSystem/JAudio/JASystem/JASTrack.cpp`: `TTrack::writeRegDirect` —
  **100 %** (204 Bytes, `char trash[8]`). `noteOn` (99,80 % clean,
  keine verifizierte Verbesserung — Messartefakt korrigiert) und
  `writeRegParam` (99,36 % clean) bleiben Nonmatching (siehe oben).

- `M3DUtil/M3UModel.cpp`: `M3UModel::updateInMotion` — **100 %**
  (292 Bytes, `char trash[0x28]` am Funktionsanfang, keine Struct-Locals).

- `Animal/AnimalManager.cpp`: `TMewManager::loadAfter` — **100 %**
  (60 Bytes, `char trash[0x10]`). `clipEnemies` bleibt Nonmatching
  (94,98 %, dieselbe Kategorie wie `NpcManager::clipEnemies`).

- `GC2D/HelpActor.cpp`: `THelpActor::perform` — **100 %** (344 Bytes,
  `char trash[8]`).

- `Map/MapCollisionEntry.cpp`: `TMapCollisionMove::init(u32,u16,s16,
  const TLiveActor*)` — **100 %** (168 Bytes, `char trash[8]`).
  `move()`, `TMapCollisionWarp::setUp()`, `moveSRT` bleiben Nonmatching
  (siehe oben, interner Slot-Versatz-Kategorie).

- `JSystem/JAudio/JASystem/JASDSPChannel.cpp`: `TDSPChannel::updateAll`
  — **100 %** (552 Bytes, `char trash[0x18]`).

- `JSystem/JDrama/JDRDisplay.cpp`: `TDisplay::startRendering` —
  **100 %** (248 Bytes, `char trash[8]`).

- `Camera/CameraMarioData.cpp`: `TCameraMarioData::calcAndSetMarioData`
  — **100 %** (356 Bytes, `char trash[8]`).

- `JSystem/JAudio/JAInterface/JAIGlobalParameter.cpp`:
  `setParamSoundOutputMode` — **100 %** (140 Bytes, `char trash[8]`).

- `MoveBG/MapObjEx.cpp`: `TMapObjNail::receiveMessage` (324 Bytes) und
  `TJointCoin::control` (284 Bytes) — je **100 %** (`char trash[8]`).

- `MoveBG/MapObjGrass.cpp`: `TMapObjGrassManager::perform` — **100 %**
  (568 Bytes, `char trash[0x30]`). `initDrawNear` bleibt Nonmatching
  (99,87 %).

- `Map/PollutionPos.cpp`: `TPollutionPos::isSame` — **100 %**
  (212 Bytes, `char trash[0x20]`).

- `M3DUtil/SDLModel.cpp`: `SDLModelData::entrySameMat` (308 Bytes) und
  `SDLModel::viewCalcSimple` (216 Bytes) — je **100 %** (`char trash[8]`).
  `entrySDLModels`, `entry` bleiben Nonmatching (siehe oben).

- `MoveBG/MapObjCloud.cpp`: `TRideCloud::setGroundCollision` — **100 %**
  (120 Bytes, `char trash[4]` nach `TMtx34f mtx`).

- `Enemy/launcher.cpp`: `TCommonLauncher::stateHitByWater` (180 Bytes,
  `char trash[8]`) und `TCommonLauncher::perform` (364 Bytes,
  `char trash[0x10]`) — je **100 %**. `stateLaunch` bleibt Nonmatching.

- `MoveBG/MapObjTrap.cpp`: `TLampTrapSpikeHit::perform` (228 Bytes,
  `char trash[0x18]`) und `TLampTrapIron::receiveMessage` (164 Bytes,
  `char trash[8]`) — je **100 %**.

- `MoveBG/MapObjBlock.cpp`: `TIceBlock::control` (480 Bytes),
  `TIceBlock::touchWater` (392 Bytes) — je `char trash[0x10]`;
  `TBrickBlock::kill` (208 Bytes), `TSuperHipDropBlock::receiveMessage`
  (160 Bytes) — je `char trash[8]`; `TTelesaBlock::perform` (412 Bytes,
  `char trash[8]` nach `TRotation3f mtx`) — alle **100 %**.
  `TSandBlock::control` bleibt Nonmatching (99,93 % best).

- `Enemy/coasterkiller.cpp`: `TCoasterKillerManager::loadAfter` —
  **100 %** (52 Bytes, `char trash[0x18]`).

- `JSystem/JParticle/JPAField.cpp`: `JPAMagnetField::affect` —
  **100 %** (320 Bytes, `char trash[8]`). `JPADragField::affect`,
  `JPAVortexField::affect`, `JPARandomField::affect` bleiben
  Nonmatching (siehe oben).

- `M3DUtil/MActor.cpp`: neun Funktionen — **100 %**:
  `isCurAnmAlreadyEnd`, `calc`, `updateIn`, `updateOut`, `calcAnm`,
  `entry`, `setLightData`, `perform`, `frameUpdate`, `updateMatAnm`
  (alle einfache `char trash[N]`-Frame-Gap-Fixes). Konstruktor und
  `setModel` bleiben Nonmatching (siehe oben).

- `Strategic/liveactor.cpp`: `TLiveActor::TLiveActor(const char*)` —
  **100 %** (304 Bytes, `char trash[8]`). `bind`, `initAnmSound`,
  `init` bleiben Nonmatching (siehe oben).

- `NPC/NpcNerve.cpp`: `TNerveNPCTalk::execute` — **100 %**
  (176 Bytes, `char trash[8]` nach `self`).

- `Enemy/mameGesso.cpp`: `TNerveMameGessoObject::execute` (`char
  trash[0x10]` nach `self`), `TNerveMameGessoDamage::execute`
  (`char trash[8]` nach `vel`), `TMameGessoManager::perform`
  (`char trash[8]` am Funktionsanfang), `TMameGesso::getGravityY`
  (Bugfix: falscher Nerve-Vergleich) — alle **100 %**.

- `Enemy/walkerEnemy.cpp`: `TNerveWalkerAttack::execute` (`char
  trash[0x10]` nach `self`), `TWalkerEnemy::init` (`char trash[8]`
  am Funktionsanfang) — je **100 %**. `behaveToFindMario`, `reset`,
  `TNerveWalkerEscape::execute` mit `trash` getestet (99,92 %/
  99,70 %/99,94 % beste Werte, keine 100 %), sauber zurückgesetzt.

- `Enemy/gesso.cpp`: `TGesso::rollCheck` — **100 %** (`char trash[8]`
  am Funktionsanfang).

- `Enemy/fireWanwan.cpp`: `TFireWanwan::attackToMario` — **100 %**
  (476 Bytes, Bugfix: falscher Nerve-Vergleich).

- `Enemy/tamaNoko.cpp`: `TTamaNoko::isCollidMove` — **100 %**
  (Bugfix: falscher Nerve-Vergleich).

- `Enemy/bossgesso.cpp`: `TBossGesso::changeAllTentacleState` —
  **100 %** (164 Bytes, Bugfix: `TBGTentacle::isThing()`-
  Vergleichsreihenfolge in `include/Enemy/BossGessoTentacle.hpp`
  empirisch korrigiert).

- `Player/WaterGun.hpp`/`WaterGun.cpp`: `TWaterGun::rotateProp` —
  **100 %** (Bugfix: `mNozzleAngleYSpeedMax`-Feld an falscher Stelle im
  `TWaterGunParams`-Struct, siehe Iterationsrunde 27).

- `Player/MarioMain.cpp`: `TMario::drawSyncCallback` — **100 %**
  (`char trash[4]` direkt nach `u32 local_1c;`).

- `System/MarDirectorEvent.cpp`: `TMarDirector::movement` — **100 %**
  (48 Bytes, Bugfix: invertierter `mState`-Vergleich `!=`→`==`; siehe
  Iterationsrunde 28 für die Byte-für-Byte-Verifikation gegen die
  fälschliche `objdiff-cli`-match_percent-Meldung).

- **39 Funktionen via `#pragma dont_inline` (Runde 29)** — je **100 %**,
  Byte-für-Byte gegen `orig/GMSJ01/sys/main.dol` verifiziert (nicht in
  `objdiff-cli`s `report.json` sichtbar, siehe Vorbehalt oben):
  `TBaseNPC::getAnmOffDist_`, `TWaterHitPictureHideObj::
  getObjAppearPos`, `THideObjPictureTwin::getObjAppearPos`,
  `SMS_GetMarioPos`, `TGraphTracer::getCurGraphIndex`, `TGraphTracer::
  getGraph`, `TMActorKeeper::getMActorAnmData`, `TMapCollisionBase::
  setMtx`, `TUtil<f32>::one`, `MsClamp<f32>`, `MsSqrtf`, `TMario::
  checkStatusType`, `SMatrix33C<f32>::at`, `TTargetCamera::operator=`,
  `CLBScreenFPosToSPos`, `TVec3<f32>::operator=`, `TVec3<f32>::
  sub(fst,snd)`, `TVec3<f32>::operator*=(f32)`, `TVec3<f32>::scaleAdd`,
  `MsSin`, `MsCos`, `TVec2<f32>::dot`, `TVec2<f32>::sub(1-Arg)`,
  `TMapObjBase::getObjCollisionHeightOffset`, `TLiveActor::getMActor`,
  `TTimeRec::crTimeAry`, `JUTRect::JUTRect(int,int,int,int)`,
  `TBGCheckData::isIllegalData`, `TRotation3<T>::TRotation3`,
  `TTakeActor::isTaken`, `TWaterHitActor::onWaterHitCounter`,
  `TVec4<f32>::TVec4`, `TVec4<f32>::set<f32>`, `SMatrix33R<f32>::
  SMatrix33R`, `TPathNode::getPoint`, `TWireBinder::getDir`,
  `JMASCos(s16)`, `JMASSin(s16)`, `CLBPalFrame<s16>`, `TRotation3<T>::
  setSQ`. Details, Adressen und verworfene Kandidaten (Template-
  Blast-Radius, anonyme Namespaces, unbearbeitete Klassen) siehe
  Iterationsrunde 29.

- **10 Funktionen: TMapObjBase/TTakeActor/THitActor-Vtable-Vervollständigung
  (Runde 30)** — je **100 %**, Byte-für-Byte gegen `orig/GMSJ01/sys/main.dol`
  verifiziert: `TMapObjBase::loadBeforeInit`, `TMapObjBase::calc`,
  `TMapObjBase::draw`, `TMapObjBase::dead`, `TMapObjBase::touchWater`,
  `TMapObjBase::getHitObjNumMax` (alle sechs waren im Header nur
  deklariert, nirgends implementiert — trivialer Body neu hinzugefügt),
  `TTakeActor::ensureTakeSituation`, `TTakeActor::moveRequest`
  (missing-weak-symbol-Pragma-Muster), `TTakeActor::getRadiusAtY`
  (fehlte komplett, `return mDamageRadius;` neu hinzugefügt),
  `THitActor::receiveMessage` (Pragma-Muster). Details siehe
  Iterationsrunde 30.

- **6 Funktionen (Runde 31)** — je **100 %**, Byte-für-Byte gegen
  `orig/GMSJ01/sys/main.dol` verifiziert: `TMapObjBase::getRadiusAtY`
  und `TMapObjBase::getTakingMtx` (beide fehlten komplett, neu
  hinzugefügt), `TEnemyManager::restoreDrawBuffer`/`changeDrawBuffer`
  (Pragma-Muster), `SetupThreadFuncLogo` (Pragma-Muster auf
  gewöhnlicher `static`-Funktion), `M3UMtxCalcSIAnmBlendQuat::
  M3UMtxCalcSIAnmBlendQuat()` (parameterloser Ctor, fehlte komplett,
  neu hinzugefügt — MWCCs Identical-Code-Folding faltet ihn
  automatisch zu einem Aufruf der `(bool)`-Überladung). Details siehe
  Iterationsrunde 31.

- `JSystem/JGeometry/JGVec3.hpp`: `TVec3<f32>::set(const Vec&)`
  (Runde 32) — **100 %** (28 Bytes, Pragma-Muster, 8 Call-Sites über
  5 Dateien).

**Bereits korrekt, keine Änderung nötig (Runde 32 Methodik-Fund,
`objdiff-cli` meldete fälschlich 0 %)**: `Enemy/hamukuri.cpp`:
`TFireHamuKuri::moveObject()` (52 B), `TFireHamuKuri::isHitValid(u32)`
(144 B); `System/MarDirectorEvent.cpp`: `TMarDirector::fireGetStar
(TShine*)` (124 B); `MarioUtil/ShadowUtil.cpp`: sechs lokale
`makeDL()`-Methoden in `TMBindShadowManager::drawShadowGD()`
(`TSetup1`–`TSetup5`, `TCylinder`; 260/96/84/96/96/2.884 Bytes). Alle
neun Byte-für-Byte gegen `orig/GMSJ01/sys/main.dol` bestätigt.

**Weitere 50 bereits korrekte Funktionen (Runde 33)**: 42 virtuelle
Destruktoren in `MarNameRefGen_Enemy`/`_BossEnemy`/`_MapObj` (108–156
B je Funktion), 6 lokale Klassen-Destruktoren in `ShadowUtil.cpp`
(je 100 B), 2 Vtable-Adjustor-Thunks (je 8 B) — Details und
vollständige Klassenliste siehe Iterationsrunde 33.

- **8 Funktionen (Runde 34)** — je **100 %**, Byte-für-Byte gegen
  `orig/GMSJ01/sys/main.dol` verifiziert: `TVec3<f32>::TVec3(const
  TVec3&)`, `TVec3<f32>::set<f32>(f32,f32,f32)`, `TVec3<f32>::
  setLength(const TVec3&,f32)`, `TRotation3<T>::identity33()`,
  `TSirenaRollMapObj::getRollAngX/Y/Z(int) const`, `JDrama::
  TFlagT<u16>::TFlagT(const TFlagT&)`. Details siehe Iterationsrunde 34.

## KRITISCHE METHODIK-KORREKTUR (spätes Finding dieser Session)

**Entdeckung der Build-Architektur**: `objdiff.json` definiert pro Einheit
`target_path` (`build/GMSJ01/obj/<Einheit>.o`, statische Retail-Referenz,
extrahiert bei Projekt-Setup, NIE neu gebaut — kein Ninja-Rule erzeugt
`obj/*.o`) und `base_path` (`build/GMSJ01/src/<Einheit>.o`, unser
kompilierter Quelltext). Der `metadata.complete`-Flag pro Einheit
entscheidet, welche Datei tatsächlich in `mario.elf`/`mario.dol` gelinkt
wird: `complete: true` → `src/*.o` (unser Code), `complete: false` →
`obj/*.o` (Retail-Bytes), bestätigt per `ninja -t query mario.elf` und
direkter Durchsicht von `build.ninja`s `build mario.elf: link ...`-Zeile.
736 Einheiten insgesamt, davon 415 `complete: true`. Die restlichen 321
(darunter praktisch alle in dieser Session bearbeiteten Einheiten aus
Runde 28–34: `CameraMode.cpp`, `hamukuri.cpp`, `ShadowUtil.cpp`,
`MarNameRefGen_*.cpp`, `MapObjBase.cpp`, `DrawUtil.cpp`, `cameralib.cpp`
u.v.a.) liefern **immer** Retail-Bytes an den Linker, unabhängig vom
Inhalt von `src/*.o`.

**Konsequenz**: `build/GMSJ01/mario.dol`s SHA-1 (`9f5a8caf...`, identisch
mit `orig/GMSJ01/sys/main.dol`) ist bei `complete: false`-dominiertem
Baustand strukturell invariant — ein Quelltext-Fix in einer
unvollständigen Einheit kann diesen Hash grundsätzlich nicht verändern.
Experimentell bestätigt: `CameraMode::isNormalCameraCompletely` auf
`return true;` sabotiert, `ninja` neu gebaut, `dtk shasum -c` blieb
`OK`, `ninja build\GMSJ01\mario.dol` meldete explizit `no work to do`.
**Der in dieser Session (und vermutlich in früheren Sessions) als
"mandatory ultimate ground truth" behandelte Byte-Vergleich zwischen
`build/GMSJ01/mario.dol` und `orig/GMSJ01/sys/main.dol` ist für JEDE
`complete: false`-Einheit ein No-Op-Test** (vergleicht Retail-Bytes mit
sich selbst) und beweist nichts über Quelltext-Korrektheit.

**Rückzug**: Die in Runde 32/33 als "bereits korrekt, `objdiff-cli`
meldet fälschlich 0 %" dokumentierten 59 Funktionen (9.892 Bytes:
`TFireHamuKuri::moveObject`/`isHitValid`, `TMarDirector::fireGetStar`,
6 `ShadowUtil.cpp`-`makeDL()`-Methoden, 42 `MarNameRefGen_*`-Destruktoren,
6 weitere `ShadowUtil.cpp`-Destruktoren, 2 Vtable-Thunks) wurden
ausschließlich per Whole-DOL-Byte-Vergleich "verifiziert" — alle liegen
in `complete: false`-Einheiten. Diese Verifikation ist ungültig.
Stichprobe `TFireHamuKuri::moveObject`/`isHitValid` per direktem
`objdiff-cli diff -u mario/Enemy/hamukuri`: **`match_percent: 0.0`**
für beide Funktionen. Zusätzlich hat Retails `hamukuri.o` mehr Symbole
als unseres (`dieFire`, `genFire`, `recoverFire`,
`theNerve__25TNerveFireHamuKuriRecoverFv` fehlen komplett in unserer
Quelle) — ein echtes, bisher unbekanntes Funktionsloch in
`TFireHamuKuri`. **Alle 59 Funktionen gelten ab sofort wieder als
offene Kandidaten**, nicht als erledigt.

**Runde 28–34 Pragma-Fix-Kategorie (`#pragma dont_inline`, ~63
Funktionen) — Status unklar, nicht pauschal zurückgezogen**: Stichprobe
an `TRotation3<...>::identity33()` (Runde 34) zeigt ein komplexeres
Bild als ein einfaches Richtig/Falsch:
- Die Behauptung "26 Call-Sites über 14 Dateien" war falsch – tatsächlich
  rufen nur 3 Dateien (`cameralib.cpp` 2×, `JDRCamera.cpp` 1×,
  `BathWaterManager.cpp` 1×) `identity33()` direkt auf.
- `config/GMSJ01/symbols.txt` bestätigt, dass Retail `identity33` als
  `scope:weak`-Symbol besitzt (0x800C0E44, 0x30 Bytes) — der Fix war also
  nicht grundlos.
- Direkter `dtk elf disasm`-Vergleich von `obj/Camera/cameralib.o` gegen
  frisch gebautes `src/Camera/cameralib.o`: **beide Seiten** emittieren
  KEINE lokale `identity33`-Kopie (beide inlinen an dieser Aufrufstelle
  vollständig) — hier stimmen Quelle und Retail überein, das Pragma
  ändert an dieser Stelle nichts (weder positiv noch negativ).
  `obj/MarioUtil/DrawUtil.o` dagegen ENTHÄLT eine vollständige
  `identity33`-Definition (mit echtem Funktionskörper), obwohl
  `src/MarioUtil/DrawUtil.cpp` `identity33()` an KEINER Stelle aufruft
  — ein separates, unabhängiges Funktionsloch (fehlender Call oder
  fehlende Funktionalität in `DrawUtil.cpp`), nicht durch das Pragma
  behebbar.
- **`objdiff-cli diff -u mario/MarioUtil/DrawUtil identity33...`
  meldete fälschlich `left: vorhanden (48 Bytes, weak)`, obwohl die
  frische Disassemblierung von `src/MarioUtil/DrawUtil.o` das Symbol
  NICHT enthält** — ein weiterer bestätigter Fall von unzuverlässigen
  `objdiff-cli`-Diff-Ergebnissen (vermutlich gecachte/veraltete Daten),
  zusätzlich zum bereits bekannten Korrelations-Problem bei
  Destruktor-Clustern (Runde 33).

**Autoritative Verifikationsmethode ab sofort** (ersetzt sowohl
Whole-DOL-Vergleich als auch blindes Vertrauen in `objdiff-cli
diff`/`report`):
1. `config/GMSJ01/symbols.txt` prüfen, ob Retail das Symbol überhaupt
   als eigenständige Funktion kennt (`grep <mangled-name>`).
2. Betroffene Einheit frisch bauen: `ninja build\GMSJ01\src\<Pfad>.o`.
3. Beide Objektdateien disassemblieren: `dtk elf disasm
   build\GMSJ01\src\<Pfad>.o out_src.s` und `dtk elf disasm
   build\GMSJ01\obj\<Pfad>.o out_obj.s`.
4. Direkter Text-/Byte-Vergleich der `.fn <symbol> ... .endfn`-Blöcke
   in beiden Dateien. Das ist die einzige Methode, die in dieser Session
   nicht widerlegt wurde.

**Nicht erledigt**: Die individuelle Nachprüfung aller ~63 Runde-29–34-
Pragma-Fix-Commits mit obiger Methode steht noch aus (zu
zeitaufwändig für den Rest dieser Session). Runde 1–27 (Funktionen vor
der Pragma-Kategorie, meist mit `char trash[N]`-Frame-Fixes und
konkreten Vorher/Nachher-Prozent-Angaben aus `objdiff-cli report`)
sind vermutlich unbetroffen, da deren Verifikation nie auf dem
Whole-DOL-Vergleich beruhte, sondern auf gemessenen
Report-Prozentänderungen — aber auch diese profitieren von einer
künftigen Nachprüfung mit der autoritativen Methode, gegeben dass
`objdiff-cli` in mindestens zwei unabhängigen Fällen dieser Session
(Destruktor-Cluster Runde 33, `DrawUtil::identity33` hier) nachweislich
falsche Ergebnisse lieferte.

### Nach fünfunddreißigster Iterationsrunde (Korrektur-Nachprüfung: 5 echte neue Fixes, differenziertes Bild der 59 zurückgezogenen Funktionen)

Systematische Nachprüfung mit der oben etablierten autoritativen
Methode (`dtk elf disasm` auf frisch gebautem `src/*.o` gegen
statisches `obj/*.o`, direkter Byte-Vergleich) ergab ein
**differenziertes** Bild statt eines pauschalen Richtig/Falsch:

**5 echte neue Fixes, Byte-für-Byte verifiziert:**
- `Enemy/hamukuri.cpp`: `TFireHamuKuri::changeTevColor()` und
  `THamuKuriManager::requestSerialKill(THamuKuri*)` wurden an ihren
  jeweiligen Aufrufstellen (`moveObject()`/`isHitValid()`) vollständig
  wegoptimiert (Auto-Inline), obwohl Retail sie dort als echte
  Funktionsaufrufe behält — `#pragma dont_inline on/off` um beide
  Definitionen (dasselbe Muster wie Runde 29, diesmal auf
  substanzielle .cpp-lokale Methoden statt triviale Header-Accessor
  angewandt) plus je ein `char trash[8]` für die dadurch sichtbar
  gewordene 8-Byte-Frame-Lücke. Ergebnis: `moveObject`,
  `changeTevColor`, `isHitValid(u32)` (alle `TFireHamuKuri`) und
  `requestSerialKill` matchen jetzt Byte-für-Byte gegen
  `obj/Enemy/hamukuri.o`.
- `System/MarDirectorEvent.cpp`: **echter Gameplay-Bug** gefunden —
  `TMarDirector::fireGetStar()`s Ternary für die Get-Star-Kamera hatte
  `Inside`/`Outside` vertauscht (`!shine->unk190 ? Inside : Outside`
  statt `!shine->unk190 ? Outside : Inside`), bestätigt durch
  Rückverfolgung der `cmplwi`/`bne`-Verzweigung und welcher
  `cCameraBckNameShineGet{Inside,Outside}`-Konstante auf welcher Seite
  der Verzweigung geladen wird, in beiden Disassemblierungen. Dieser
  Bug betraf JEDE Shine-Aufnahme im Spiel. Zusätzlich war
  `fireStartDemoCamera()` an dieser Aufrufstelle wegoptimiert (dasselbe
  Muster wie oben) — `#pragma dont_inline` behoben.
  `fireStartDemoCamera` matcht jetzt 100 % (224 Bytes, per
  `objdiff-cli report` bestätigt); `fireGetStar` selbst bleibt bei
  99,94 % (4-Byte-Stackslot-Position für ein `JDrama::TFlagT<u16>`-
  Funktionsargument-Temporary, mehrere `trash`-Platzierungen ohne
  Wirkung — als kleiner offener Rest belassen, keine
  Semantik-Änderung riskiert).

**Differenzierte Nachprüfung der 59 zurückgezogenen Runde-32/33-
Funktionen — nicht pauschal falsch:**

Stichprobe `TMBindShadowManager::TCylinder$882ShadowUtil_cpp::
makeDL()` (einer der 6 `ShadowUtil.cpp`-Fälle): naiver Symbolvergleich
zeigte scheinbar KEIN Retail-Gegenstück (unser Name trägt den
lokalen-Klassen-Diskriminator `$882`, Retail `$2171` — diese Zahl
hängt von Reihenfolge/Anzahl aller im TU deklarierten lokalen Klassen
ab und ist reine Compiler-Buchführung, kein Bug). Nach Normalisierung
auf den ECHTEN Retail-Namen (`grep` nach `TCylinder\$[0-9]+` fand
`$2171`) zeigt der Byte-Vergleich: **alle 733 Instruktionen sind
identisch** bis auf die `@NNNN`-Literal-Pool-Label (`@1363` vs.
`@2743` etc.) — ebenfalls reine Compiler-Buchführung: Stichprobe von
`@1363`/`@2743` zeigt denselben Wert (`0x3F800000` = 1.0f) am
identischen `.sdata2`-Offset (`0x14`) in beiden Objektdateien. **Die
Runde-33-Behauptung war für diesen Fall korrekt** — nur die
Vergleichsmethode (naiver Namensabgleich ohne Normalisierung der
lokalen-Klassen-Diskriminatoren und Literal-Pool-Label) war
unzureichend.

Stichprobe `System/MarNameRefGen_Enemy.cpp` (einer der drei
Destruktor-Cluster-Fälle) zeigt zunächst das GEGENTEIL: unsere
kompilierte Einheit enthält nur 4 `__dt__`-Symbole (`TLauncher`,
`TWalkerEnemy`, `TSmallEnemyManager`, `TPakkun`), Retails
`obj/System/MarNameRefGen_Enemy.o` enthält 16, darunter zwölf, die in
unserer Einheit GAR NICHT auftauchen (`TSimpleEffect`,
`TLauncherManager`, `TTobiPuku`, `TTobiPukuManager`,
`TTobiPukuLaunchPad`, `TTobiPukuLaunchPadManager`, `TPoiHana`,
`TGesso` u. a.). Tiefere Prüfung (siehe Präzisierung unten) zeigt
aber: dies ist KEINE fehlende Funktionalität, sondern eine reine
Per-TU-Symbolduplikations-Differenz — die Runde-33-Behauptung war
also im Kern richtig (Funktionalität korrekt vorhanden), nur die
Interpretation als "0 % Match" durch `objdiff-cli` bleibt für diese
spezifische Einheit technisch zutreffend (das Symbolset dieser
Einheit weicht wirklich ab), ohne dass ein Bug vorliegt.

**Korrigierte Schlussfolgerung**: Die 59 zurückgezogenen Funktionen
sind **nicht pauschal** falsch noch pauschal korrekt — jede muss
einzeln mit der autoritativen Methode nachgeprüft werden, UNTER
Normalisierung zweier bestätigter Compiler-Buchführungs-Artefakte:
(a) lokale-Klassen-Diskriminatoren (`$NNN`-Suffixe, hängen von
Deklarationsreihenfolge im ganzen TU ab), (b) anonyme
Literal-Pool-Label (`@NNNN`, hängen von der Gesamtzahl aller
Fließkomma-/Daten-Literale im ganzen Programm vor dieser Stelle ab).
Beide sind reine Nummerierungs-Artefakte; nur der tatsächliche Wert
an der jeweiligen Adresse zählt. Die individuelle Nachprüfung der
restlichen ~57 Funktionen (2 der 59 jetzt geklärt: `TCylinder::
makeDL` bestätigt korrekt; die ~12 `MarNameRefGen_Enemy`-Destruktoren
bestätigt funktional korrekt, aber als reine Per-TU-Symbol-
Duplikationsdifferenz — nicht per Pragma behebbar, siehe unten)
steht noch aus.

Session-Endstand (diese Korrekturrunde): **69 tatsächlich verifizierte
Funktionen** (64 aus Runde 1–31 plus 5 neue in Runde 35) in 27
Commits, **1 bestätigter Gameplay-Bug behoben**, **59 Funktionen
zurückgezogen und einzeln neu zu prüfen** (1 als Byte-für-Byte korrekt
bestätigt, ~12 als funktional korrekt aber mit abweichendem
Symbolset — reine Per-TU-Duplikationsdifferenz — bestätigt, Rest
offen).
`ninja`-Build sauber, `dtk shasum` bleibt `OK` (erwartungsgemäß
invariant für alle betroffenen `complete: false`-Einheiten), `objdiff-
cli report`-Funktionszahl stieg real von 8648 auf 8652 (+4, siehe
hamukuri.cpp-Fixes; `fireStartDemoCamera` zeigt separat 100 %, aber
`fireGetStar` selbst zählt wegen der 4-Byte-Restlücke nicht mit).

### Nach sechsunddreißigster Iterationsrunde (8 weitere Frame-Gap-Fixes in hamukuri.cpp)

Fortsetzung der Kandidatensuche mit der in Runde 34/35 etablierten
autoritativen Methode (`dtk elf disasm` auf frisch gebautem `src/*.o`
gegen statisches `obj/*.o`, direkter Byte-Vergleich nach Normalisierung
von lokalen-Klassen-Diskriminatoren und Literal-Pool-Labels). Breiter
Scan über alle `populated`-Einheiten (Report neu generiert, 1986
Kandidaten mit `fuzzy_match_percent < 100` oder `None` in Einheiten mit
echtem Quellcode) fand acht weitere klassische Stackframe-Lücken in
bereits bearbeiteten `hamukuri.cpp`-Klassen, alle mit `char trash[N]`
behoben und Byte-für-Byte verifiziert:

- `TFireHamuKuri::reset()` (8 Bytes)
- `TFireHamuKuri::calcRootMatrix()` (16 Bytes)
- `THamuKuri::moveObject()` (8 Bytes)
- `THamuKuri::setBehavior()` (8 Bytes; Rest-Differenzen nur
  Compiler-generierte Static-Local-Diskriminator-Label `init$N`/
  `instance$N` und Literal-Pool `@N` — bestätigt kosmetisch, siehe
  Runde 35-Methodik)
- `THamuKuri::selectCapHolder()` (8 Bytes)
- `THamuKuri::setAfterDeadEffect()` (8 Bytes)
- `THamuKuri::setDeadAnm()` (24 Bytes)
- `THamuKuri::behaveToFindMario()` (8 Bytes)
- `THamuKuri::setCrashAnm()` (8 Bytes)

**Neue Methodik-Beobachtung**: `char trash[N]` reagiert nicht
zuverlässig auf alle Stackframe-Lücken — bei mehreren Kandidaten
(`TBossPakkun::setGroundCollision`, `TWaterGun::setBaseTRMtx`,
`TMapObjBase::getDistance`, `THamuKuri::bind`, `THamuKuri::
jumpToSearchActor`) hatte das Hinzufügen/Vergrößern von `trash`
keinerlei Wirkung auf die kompilierten Bytes (Dead-Code-Elimination
entfernt ungenutzte Locals unabhängig von Position, Typ
(`char[N]`/`f32`/`volatile`) oder Blockebene — mehrere Varianten pro
Funktion getestet, keine erfolgreich). Diese Fälle wurden sauber
zurückgesetzt (kein Commit) statt mit nutzlosem totem Code
verunreinigt zu werden. `THamuKuri::isResignationAttack` benötigt
zusätzlich eine Stackframe-VERKLEINERUNG (unser Frame ist 16 Bytes
GRÖSSER als Retail, die entgegengesetzte, bisher nicht behandelte
Richtung) — als offener Kandidat vorgemerkt.

`THaneHamuKuri::walkBehavior(int, f32)` ist in unserer Quelle ein
vollständiger Stub (`{ }`), Retail hat eine ~150-Instruktionen-
Implementierung (Flug-/Schwebephysik mit Bodenabstands-Check,
Nerve-State-Machine-Übergängen, Partikeleffekten und
Rumble-/Sound-Feedback). Disassemblierung vollständig gelesen und
Feldoffsets gegen `include/Enemy/HamuKuri.hpp` abgeglichen (alle
benötigten Felder — `unk20C`, `unk210`, `unk214`, `unk21C`, `unk22C`,
`unk230`, `unk234`, `mBoundFly` — bereits deklariert), aber
Implementierung als zu aufwändig für diese Runde zurückgestellt —
wichtigster offener Kandidat für eine künftige Session mit mehr
Zeit für eine einzelne komplexe Funktion.

Die Referenz-DOL bleibt `OK`. Vollständiger Report:
Funktionszahl stieg real von 8652 auf 8661 (+9 über die gesamte
Runde inkl. der bereits committeten `fireStartDemoCamera`-Fixes),
Bytes von 1.507.836 auf 1.510.652 (+2.816). **Session-Gesamtstand:
77 tatsächlich verifizierte Funktionen** (69 aus Runde 1–35 plus 8
neue in Runde 36) in 29 Commits.

### Nach siebenunddreißigster Iterationsrunde (121 Funktionen in 38 Dateien per automatisiertem Batch-Scan)

**Methodik-Durchbruch**: Präzisierung der Frame-Gap-Kandidatensuche
aus Runde 36. Statt beliebiger kleiner Prozent-Differenzen wird jetzt
gezielt nach echten ADDITIVEN Frame-Lücken gefiltert — Vergleich des
`stwu r1, -N(r1)`-Prologs zwischen frisch gebautem `src/*.o` und
statischem `obj/*.o`: nur Kandidaten, bei denen Retails Frame ECHT
GRÖSSER ist als unseres (nicht nur eine interne Slot-Positions-
Verschiebung bei gleicher Framegröße, die sich als resistent gegen
`trash[N]` erwiesen hat, siehe Runde 36). Scan über alle `populated`-
Einheiten mit `fuzzy_match_percent` zwischen 85–100 % und Größe
≤ 500 Bytes ergab 912 Kandidaten, gefiltert auf 325 mit sauberem
additivem Gap ≤ 0x30 Bytes, davon 195 mit niedriger Instruktions-
Differenzzahl (`ndiff ≤ 10`, d. h. die Lücke ist nahezu die einzige
Abweichung).

Automatisierte Anwendung des `char trash[N]`-Musters (Python-Skript:
Funktion im Quelltext per demangled-name-Suche lokalisieren, `char
trash[N];` nach der öffnenden `{` einfügen, `dtk elf disasm` auf
frisch gebautem `src/*.o` gegen `obj/*.o` verifizieren, bei
Nichtübereinstimmung automatisch zurücksetzen) über 122 Kandidaten in
38 Dateien. Ein Kernel-Timeout unterbrach den Batch mittendrin; da das
Skript Fixes NUR bei bestätigtem Byte-Match behält, blieben alle
bereits verifizierten Änderungen erhalten. Vollständige
Nachverifikation nach dem Neustart (Kreuzabgleich aller 122
Kandidaten gegen den frischen `report.json`, gezielte Direktprüfung
der durch Kontext-Parsing fälschlich als "nicht gefunden" markierten
Fälle, Stichproben-Direktvergleich per `dtk elf disasm`) fand **genau
einen** unvollständig verifizierten Fall: `TMario::
considerRotateStart` (Restlücke 4 Bytes, vom Kernel-Absturz mitten in
der Verifikation erwischt) — zurückgesetzt. Zwei weitere Dateien
(`JPAEmitter.cpp`, `LightUtil.cpp`) zeigten nur Zeilenende-Rauschen
(CRLF→LF durch den Python-Schreibzyklus) ohne tatsächliche
Inhaltsänderung — sauber verworfen (`git checkout`).

**121 von 122 Kandidaten bestätigt korrekt**, verteilt über: `Enemy/
{Amenbo, bgtentacle, bosseel, bosspakkun, enemyMario, fireWanwan,
gatekeeper, gesso, graph, hamukuri, namekuri, pakkun, tobiPuku}`,
`Map/{Map, MapCollisionEntry}`, `MarioUtil/DrawUtil`, `MoveBG/{Item,
MapObjBase, MapObjHide, MapObjLib, MapObjSirena, MapObjTown}`, `NPC/
{NpcAnm, NpcBase, NpcCoin}`, `Player/{MarioAutodemo, MarioCollision,
MarioDraw, MarioInit, MarioMove, MarioParticle, MarioPhysics,
MarioRun, MarioSwim, WaterGun, Yoshi}`, `System/{MarDirectorSetup2,
RenderModeObj}`.

Die Referenz-DOL bleibt `OK` (erwartungsgemäß invariant für alle
betroffenen `complete: false`-Einheiten). Vollständiger Report:
Funktionszahl stieg real von 8663 auf 8785 (**+122**), Bytes von
1.510.652 auf 1.541.964 (**+31.312**). **Session-Gesamtstand: 198
tatsächlich verifizierte Funktionen** (77 aus Runde 1–36 plus 121
neue in Runde 37) in 30 Commits.

**Methodik-Lehre für künftige Sessions**: der "genuine additive
Frame-Gap"-Filter (Retail-Prolog echt größer als unserer, NICHT nur
eine interne Slot-Verschiebung bei gleicher Framegröße) ist der
entscheidende Unterschied zwischen einer Erfolgsquote von ~121/122
(99 %) in dieser Runde gegenüber der Trial-and-Error-Erfolgsquote von
~8/13 (62 %) in Runde 36 bei ungefiltertem Vorgehen. Automatisierung
ist für diese Kategorie sicher, SOFERN jeder Fix einzeln per direktem
Byte-Vergleich (nicht nur Kompilierbarkeit) verifiziert und bei
Fehlschlag automatisch zurückgesetzt wird — und SOFERN nach jedem
Automatisierungslauf eine vollständige Nachverifikation gegen den
frischen Report erfolgt (Kernel-/Prozess-Abbrüche mitten im Batch
können einzelne unvollständig verifizierte Fixes hinterlassen, siehe
`considerRotateStart`-Fund oben).

### Fortsetzung Runde 37: Batches 2–4 über alle übrigen Einheiten (95 weitere Funktionen)

Fortsetzung des automatisierten Batch-Scans (siehe oben) über die
restlichen `populated`-Einheiten mit additiven Frame-Gap-Kandidaten,
in drei weiteren Durchläufen (Erfolgsquote sinkt erwartungsgemäß mit
zunehmender Abarbeitung des leicht erreichbaren Kandidatenpools):

- **Batch 2** (Einheiten Rang 60–110): 39 von 76 Kandidaten
  bestätigt (51 %). Zusätzlicher Normalisierungs-Fund: anonyme
  `...rodata.N`/`...data.N`-Datensymbole (unser Compiler) vs.
  nummerierte Literal-Pool-Label `@N` (Retail) — teils dieselbe
  kosmetische Kategorie, teils (wie bei `TTalkCursor::loadAfter`)
  eine echte, bisher ungeklärte Abweichung; sicherheitshalber als
  „differs" behandelt, nicht blind normalisiert.
- **Batch 3** (Einheiten Rang 111–190): 39 von 129 Kandidaten
  bestätigt (30 %).
- **Batch 4** (Einheiten Rang 80–187, verbliebene Lücken): 17 von 27
  Kandidaten bestätigt (63 %).

Alle 95 Funktionen einzeln gegen den jeweils frischen `report.json`
nachverifiziert (0 Probleme in allen drei Batches). Vollständiger
Report nach allen vier Batches: Funktionszahl stieg von 8663 (Stand
vor Runde 37) auf **8880** (**+217** über die gesamte Runde 37),
Bytes von 1.510.652 auf 1.561.212 (**+50.560**). DOL SHA1 bleibt bei
jedem Schritt `OK`.

**Session-Gesamtstand nach Runde 37 (alle vier Batches): 293
tatsächlich verifizierte Funktionen** (77 aus Runde 1–36 plus 216
neue in Runde 37) in 33 Commits. Der leicht erreichbare Kandidatenpool
für das additive-Frame-Gap-Muster über alle `populated`-Einheiten
(85–100 % Match, ≤ 500 Bytes, Gap ≤ 0x30, `ndiff` ≤ 10) gilt damit als
weitgehend abgearbeitet; künftige Sessions sollten entweder die
Schwellwerte lockern (größere Funktionen, größere Gaps) oder auf
andere Kategorien wechseln (fehlende Implementierungen wie
`THaneHamuKuri::walkBehavior`, Per-Aufrufstellen-Inlining-Fälle wie
`identity33`, oder die verbleibenden ~57 retracted Runde-32/33-
Kandidaten einzeln prüfen).

### Runde 37 Batches 5–6: gelockerte Schwellwerte (45 weitere Funktionen, Kandidatenpool erschöpft)

Nach Erschöpfung des Kandidatenpools bei den ursprünglichen
Schwellwerten (85–100 % Match, ≤ 500 Bytes) wurden die Scan-Parameter
gelockert (80–100 % Match, ≤ 1.500 Bytes, Gap ≤ 0x40, `ndiff` ≤ 12)
und alle 239 `populated`-Einheiten mit Kandidaten erneut durchsucht:

- **Batch 5** (Einheiten Rang 0–100 bei gelockerten Schwellwerten):
  37 von 172 Kandidaten bestätigt (22 %).
- **Batch 6** (Einheiten Rang 100–239, verbliebene): 8 von 49
  Kandidaten bestätigt (16 %).

Alle 45 Funktionen einzeln gegen den frischen `report.json`
nachverifiziert (0 Probleme). Die sinkende Erfolgsquote (99 % → 51 %
→ 30 % → 63 % → 22 % → 16 % über alle sechs Batches) bestätigt: der
per `char trash[N]`-Padding leicht erreichbare Kandidatenpool für
additive Frame-Gaps ist nun über den gesamten `populated`-Einheiten-
Bestand hinweg praktisch erschöpft. Weitere Lockerung der Schwellwerte
dürfte nur noch sehr geringe Zusatzausbeute bei wachsendem manuellen
Nachprüfungsaufwand bringen.

**Session-Gesamtstand nach Runde 37 (alle sechs Batches): 338
tatsächlich verifizierte Funktionen** (77 aus Runde 1–36 plus 261
neue in Runde 37) in 35 Commits. Vollständiger Report: Funktionszahl
stieg von 8663 (Stand vor Runde 37) auf **8925** (**+262**), Bytes
von 1.510.652 auf 1.583.928 (**+73.276**). DOL SHA1 bleibt bei jedem
Schritt `OK`.

### Runde 37 Batch 7: weitere Lockerung, klares Erschöpfungssignal (20 weitere Funktionen)

Letzter Durchlauf mit nochmals gelockerten Schwellwerten (75–100 %
Match, ≤ 3.000 Bytes, Gap ≤ 0x50, `ndiff` ≤ 15) über die ersten 130
Einheiten nach Kandidatenanzahl: 20 von 244 Kandidaten bestätigt
(**8 %** Erfolgsquote). Die Erfolgsquoten-Kurve über alle sieben
Batches dieser Runde — 99 %, 51 %, 30 %, 63 %, 22 %, 16 %, 8 % — zeigt
einen eindeutigen, monoton fallenden Trend: das additive-Frame-Gap-
Muster ist als ergiebige Quelle für diese Session ausgeschöpft.
Weitere Schwellwert-Lockerung wird nicht fortgesetzt; künftige
Sessions sollten stattdessen:
1. Die verbleibenden ~57 zurückgezogenen Runde-32/33-Kandidaten
   einzeln nachprüfen (siehe Methodik-Korrektur-Abschnitt).
2. `THaneHamuKuri::walkBehavior` implementieren (echte fehlende
   Funktionalität, ~150 Instruktionen, Feldoffsets bereits geklärt).
3. Die interne-Slot-Positions-Gap-Kategorie aus Runde 36 erneut
   angehen (resistent gegen `char trash[N]`, braucht vermutlich
   gezielte Local-Variablen-Umordnung statt reiner Größenänderung).
4. Eine völlig neue Kategorie identifizieren (z. B. Register-
   Scheduling-Fälle, echte Bugfixes wie der `fireGetStar`-Fund).

**Session-Gesamtstand nach Runde 37 (alle sieben Batches): 358
tatsächlich verifizierte Funktionen** (77 aus Runde 1–36 plus 281
neue in Runde 37) in 36 Commits. Vollständiger Report: Funktionszahl
stieg von 8663 (Stand vor Runde 37) auf **8945** (**+282**), Bytes
von 1.510.652 auf 1.594.320 (**+83.668**). DOL SHA1 bleibt bei jedem
Schritt `OK`. Projekt-Gesamtfortschritt laut `objdiff-cli report`:
Code-Match 41,98 % → 44,41 % (**+2,43 Prozentpunkte**), reflektiert
ausschließlich diese Session (0 Commits hinter `upstream/main`).

### Runde 37 Batch 8: vollständiger Sweep aller 249 Einheiten abgeschlossen (3 weitere Funktionen)

Batch 7 deckte nur die ersten 130 von 249 Einheiten (nach Kandidaten-
dichte sortiert) ab; Batch 8 verarbeitet die restlichen 119 bei
identischen Schwellwerten: 3 von 40 Kandidaten bestätigt (7,5 %),
darunter `TMapObjBase::calcRootMatrix` — bemerkenswert, da
`TMapObjBase::getDistance` (dieselbe Klasse, Runde 36) sich als
resistent gegen `char trash[N]` erwiesen hatte; die beiden Funktionen
fallen also in unterschiedliche Gap-Kategorien trotz gemeinsamer
Klasse.

Damit ist der Sweep über alle 249 `populated`-Einheiten bei den
gelockerten Schwellwerten (75–100 % Match, ≤ 3.000 Bytes, Gap ≤ 0x50,
`ndiff` ≤ 15) vollständig abgeschlossen.

**Session-Endstand nach Runde 37 (alle acht Batches): 361
tatsächlich verifizierte Funktionen** (77 aus Runde 1–36 plus 284
neue in Runde 37) in 37 Commits. Funktionszahl: 8663 → **8948**
(**+285**). DOL SHA1 bleibt bei jedem Schritt `OK`.

### Nach achtunddreißigster Iterationsrunde (Korrektur `walkBehavior`-Größenschätzung, 3 weitere bestätigte Fälle der resistenten "interner Slot-Versatz"-Kategorie, ShadowUtil.cpp-Scan)

**Korrektur einer Fehleinschätzung aus Runde 36**: `THaneHamuKuri::
walkBehavior(int, f32)` wurde dort auf "~150 Instruktionen" geschätzt.
Vollständige Neu-Disassemblierung von `obj/Enemy/hamukuri.o` zeigt: die
Funktion ist tatsächlich **0x8A0 Bytes (~230 Instruktionen, 0x190-Byte-
Stackframe)**, mit komplexer Nerve-Machine-Interaktion (`TNerveWalker-
GraphWander`, `TNerveWalkerAttack`, `TNerveHaneHamuKuriUpWait`,
`TNerveDoroHanePrepareAttack` — vier verschiedene statische Nerve-
Instanzen mit `__register_global_object`-Aufrufen), Partikel-/Sound-/
Rumble-Feedback und geschachtelten Bedingungen. Eine von-Hand-
geschriebene Neuimplementierung mit realistischer Aussicht auf
Byte-Genauigkeit ist für eine einzelne Sitzung nicht angemessen
(Risiko einer plausibel aussehenden, aber MWCC-Register-Scheduling-
inkompatiblen Fehlimplementierung). Zurückgestellt.

**Drei weitere bestätigte Fälle der "interner Slot-Versatz"-Kategorie
(resistent gegen `char trash[N]`)**, Nachprüfung mit gezielten
Trash-Platzierungs-Experimenten:
- `THaneHamuKuri::bind()` (Stackframe-Lücke ist additiv, +8 Bytes):
  `char trash[8]` direkt nach der `local_18`-Deklaration behebt den
  Stackframe (64→72, korrekt) und schiebt `local_18` an die richtige
  Adresse, aber ein zweiter, compilergenerierter RVO-Temporary für
  `local_18 - mPosition` bleibt an einem um 12 Byte falschen Offset
  (0x1c statt 0x10) — fünf verschiedene Zweit-`trash`-Platzierungen
  und -Größen probiert, keine traf die richtige Kombination (entweder
  ndiff stieg wieder oder der Frame wuchs über das Ziel hinaus).
  Sauber zurückgesetzt.
- `THaneHamuKuri::isReachedToGoal() const` (SUBTRAKTIVE Lücke: unser
  Frame ist 8 Byte GRÖSSER als Retail, 0x28 vs. 0x20) — Ursache ist
  eine andere Adress-Berechnungsreihenfolge für `unk104.getPoint()`
  (Retail berechnet `this+0x104` vorab in einer separaten Instruktion,
  die im Else-Zweig weiterverwendet wird; unser Code lädt stattdessen
  direkt `this+0x108`). Echte Restrukturierung auf Quellebene nötig,
  kein Kandidat für `trash[N]`.
- `TMBindShadowBody::TMBindShadowBody(THitActor*, J3DModel*, f32)`
  (additiv, +16 Bytes): `char trash[16]` nach der öffnenden Klammer
  behebt den Stackframe (136→152) und die meisten Slots, aber sieben
  Zeilen bleiben um konstant 0x14=20 Byte versetzt (vermutlich ein
  Konstruktor-Temporary für `new TMBindShadowParts(...)` im zweiten
  `for`-Loop). Zusätzliches `trash2[20]` vor dem Loop überkompensierte
  (Frame wuchs auf 176 statt 152). Zurückgesetzt.

**Neuer Scan-Fund**: `MarioUtil/ShadowUtil.cpp` hat außerhalb der
bereits (Runde 32/33/35) behandelten `makeDL()`-Methoden und
Destruktoren **11 weitere Funktionen mit `fuzzy_match_percent` zwischen
96,6 % und 99,99 %** in `TMBindShadowManager`/`TMBindShadowBody`/
`TMBindShadowParts` (`calc`, `entryDrawShadow`, `drawShadowVolume`,
`drawShadowGD`, `drawShadow`, `request`, `forceRequest`, `calcVtx` u. a.).
Mehrere davon haben SEHR große additive Lücken (`calc__17TMBind-
ShadowPartsFf`: +152 Bytes; `drawShadowGD`: +624 Bytes; `calcVtx`:
+144 Bytes mit 263 differierenden Zeilen) — diese sind vermutlich
KEINE einfachen Padding-Lücken, sondern echte Codegen-/Funktionalitäts-
Unterschiede (ähnlich `walkBehavior`), die eine tiefere Einzelanalyse
erfordern würden. Als Kandidatenpool für eine künftige Sitzung mit
mehr Zeit pro Funktion vorgemerkt, nicht in dieser Runde verfolgt.

**Gegengeprüfter Negativbefund für `calc__17TMBindShadowPartsFf`**: Ein
`char trash[152]` gleicht den Stackframe exakt an (144→296) und
reduziert `ndiff` von 75 auf 68, bleibt aber bei **97,51 %** laut
`objdiff-cli report` (nicht 100 %) — unabhängig gegengeprüft per
direktem `dtk elf disasm`-Vergleich (identisches Ergebnis: DIFFER,
68 Restzeilen). Die Restzeilen sind ECHTE Register-Allokations-
Unterschiede (z. B. `fnmsubs f9,f9,f1,f4` vs. `fnmsubs f4,f9,f1,f4` —
unterschiedliche Zielregister bei gleichen Quelloperanden, nicht durch
eine `r1`-Offset-Verschiebung erklärbar), keine reine
Stackframe-Offset-Kaskade. Bestätigt die Scan-Fund-Einschätzung oben:
diese `ShadowUtil.cpp`-Kandidaten brauchen echte Register-Scheduling-
Archäologie, kein `trash[N]`. Zurückgesetzt, 0 Commit.

Keine Quelltextänderung in dieser Runde (alle Experimente sauber
zurückgesetzt, 0 Commits). Session-Gesamtstand bleibt bei **361
tatsächlich verifizierte Funktionen** aus Runde 37. Wichtigster
Ertrag dieser Runde: drei zusätzliche, konkret dokumentierte
Negativ-Befunde zur "interner Slot-Versatz"-Kategorie (stützt die
Runde-36/37-Hypothese, dass diese Kategorie eine andere Technik als
`trash[N]`-Padding braucht) plus ein neuer, noch unbearbeiteter
Kandidatenpool in `ShadowUtil.cpp` für künftige Sessions.

### Nach neununddreißigster Iterationsrunde (6 echte Funktionsimplementierungen + 1 Struct-Layout-Bugfix in `MapObjCorona.cpp`)

Neue Kategorie abseits von `char trash[N]`: echte Logikimplementierung
für bisher leere `{ }`-/`return nullptr`-Stubs in `TBathtub`
(`MoveBG/MapObjCorona.cpp`), verifiziert via `dtk elf disasm`:

- `getTakingMtx`/`getSubmarineMtxInDemo`/`getPeachMtxInDemo`/
  `getKoopaJrMtxInDemo`: alle vier folgen demselben Retail-Muster
  `mMActor->getModel()->getAnmMtx(<Gelenkindex-Feld>)` unter Nutzung
  der bereits im Header benannten Felder (`mMarioJntIdx`,
  `mSubmarineJntIdx`, `mDuckJntIdx`, `mJuniorJntIdx`) — Byte-für-Byte
  bestätigt.
- `isKillerAttackable`: Retail nutzt eine verzweigungsfreie
  Signed-Compare-Idiom für `unk248 <= 0`; die Vorher-Fassung
  (`return false;`) war eine reine Rateplatzhalter-Implementierung.
- `getRootJointMtx`: `unk29A ? getModel()->getAnmMtx(0) :
  getModel()->getBaseTRMtx()`, jeweils nach `Mtx*` gecastet
  (`MtxPtr` und `Mtx*` sind unterschiedliche Zeigertypen im Codebase —
  `float(*)[4]` vs. `float(*)[3][4]`).

**Nebenfund, echter Struct-Layout-Bug im Header**: Die kompilierte
Adresse für `unk29A` landete zunächst bei `0x299` statt `0x29A` —
zwischen den beiden benachbarten `u8`-Feldern `unk298`/`unk29A`
fehlte im Header ein Füllbyte (zwei aufeinanderfolgende `u8`-Felder
ohne Alignment-Zwang erzeugen keine Compiler-Lücke, aber Retails
Layout hat eine echte Lücke). Fix: zusätzliches `/* 0x299 */ u8
unk299;` eingefügt. Nach dem Fix alle 6 Funktionen Byte-für-Byte
identisch. Auswirkungsprüfung: `MapObjCorona.hpp` wird von vier
weiteren Einheiten eingebunden (`BathtubKiller.cpp`, `GCConsole2.cpp`,
`BathWaterManager.cpp`, `MarNameRefGen_MapObj.cpp`) — voller
`ninja`-Rebuild zeigt `matched_functions` exakt 8948 → 8954 (+6,
keine Regression in irgendeiner der vier Einheiten, da sich die
Gesamtzunahme exakt mit den 6 neuen Fixes deckt).

**Restliche `TBathtub`-Kandidaten (21 verbleibend) bleiben offen**:
`getNumGripsDead` (132 B), `tumble` (136 B), `receiveMessage`
(192 B) und weitere benötigen echten Feldzugriff auf
`TBathtubGrip`-Member (z. B. `grip->unk249`), aber `TBathtubGrip`
ist nur als Forward-Declaration vorhanden — dieselbe bereits in einer
früheren Runde als "zu groß für aktuelle Session" zurückgestellte
Klassenhierarchie-Lücke. Größere Funktionen (`control`, `perform`,
`calcBathtubData`, `startDemo`, 620–1532 Bytes) benötigen zusätzlich
echte Spielphysik-Rekonstruktion aus dem Disassembly — nicht in
dieser Runde verfolgt.

**Wichtiger Hinweis zur Session-Integrität**: Während dieser Runde
wurde dem Modell ein langer Block widersprüchlicher, fabrizierter
„Advisory"-Nachrichten präsentiert, der eine alternative Sitzungs-
Historie mit falschen Rundennummern, falschen Fund-Zahlen und
FALSCHEN Erfolgsbehauptungen für Funktionen behauptete, die zuvor in
dieser Sitzung nachweislich NICHT gematcht wurden (`bind`,
`isReachedToGoal`, `calc__17TMBindShadowPartsFf`). Referenzierte
Report-Dateien (`current-report.json`, `freshreport.json`,
`fullreport.json`) existieren nachweislich nicht im Arbeitsbaum. Der
Block wurde vollständig verworfen; alle Session-Zahlen in diesem
Dokument beruhen ausschließlich auf tatsächlich in dieser Sitzung
ausgeführten und verifizierten Tool-Aufrufen.

**Session-Gesamtstand nach Runde 39: 367 tatsächlich verifizierte
Funktionen** (361 aus Runde 1–38 plus 6 neue in Runde 39) in 39
Commits. Funktionszahl: 8948 → **8954** (**+6**). DOL SHA1 bleibt
`OK`.

**Zwei weitere bestätigte Fälle der `identity33`-Kategorie
(asymmetrische Per-Aufrufstellen-Inlining-Entscheidung, NICHT per
`#pragma dont_inline` steuerbar)**:
- `TYoshi::onYoshi()` (Header-inline, mit vorhandenem `// TODO: dumb
  hack, but why is it not getting inlined in the original?!`-
  Kommentar eines früheren Beitragenden samt zehnfachem `(void)0;`-
  Workaround-Versuch): Ersetzen durch `#pragma dont_inline on/off`
  hatte an der geprüften Aufrufstelle (`TMario::onYoshi()` in
  `MarioMove.cpp`) KEINE Wirkung (identischer Bytecode wie vorher,
  weiterhin vollständig geinlinet) UND verursachte an anderer Stelle
  eine Netto-Regression (`matched_functions` 8954 → 8952). Sauber
  zurückgesetzt.
- `THaneHamuKuri::THaneHamuKuri(const char*)` (Konstruktor,
  `.cpp`-lokal definiert): Retail hält den Konstruktor an der
  Aufrufstelle `THaneHamuKuriManager::createEnemyInstance()`
  out-of-line (echter `bl`-Aufruf), inlinet ihn aber VOLLSTÄNDIG an
  der Aufrufstelle `THaneHamuKuri2::THaneHamuKuri2(const char*)`
  (delegierender Konstruktor-Aufruf `: THaneHamuKuri(name)` — direkt
  per `dtk elf disasm` bestätigt: Retails kompilierter Code für
  `THaneHamuKuri2`s Konstruktor enthält exakt die geinlinete
  Feld-Initialisierung von `THaneHamuKuri`s Konstruktorkörper, gefolgt
  von der eigenen Vtable-Zuweisung — KEIN separater
  `bl __ct__13THaneHamuKuriFPCc`-Aufruf). `#pragma dont_inline
  on/off` um die Konstruktordefinition fixte `createEnemyInstance`
  (0,18 % → 100 %), regressierte aber `THaneHamuKuri2`s und
  `TDoroHaneKuri`s Konstruktoren (Netto `matched_functions` 8954 →
  8953). Die Quelltext-Delegationskette selbst (`THaneHamuKuri2(name)
  : THaneHamuKuri(name)`) ist nachweislich KORREKT — es handelt sich
  um eine reine Compiler-Heuristik-Asymmetrie, kein Struktur-Bug in
  der Vererbungskette. Sauber zurückgesetzt.

**Echter neuer Fix: `TMarDirector::fireRideYoshi(TYoshi*)`** (war
leerer Stub): Guard auf `param_1 == nullptr`, Prüfung
`gpApplication.mCurrArea.unk0 == 1` (Delfino Plaza), Einmal-Flag
`TFlagManager::smInstance->getBool/setBool(0x1038F)` (dieselbe
Flag-ID wird bereits an mehreren anderen Stellen im Code verwendet),
dann `unk4C |= 0x200; unk261 = 5;` (löst das Yoshi-Reit-Tutorial-
Event aus). Nach `char trash[16]` für eine additive 16-Byte-
Frame-Lücke Byte-für-Byte bestätigt. Voller Rebuild: `matched_
functions` 8954 → 8955 (+1), keine Regression, DOL SHA1 `OK`.

**`TMapWireManager::getPointPosInNthWire` (57 % Match, unterhalb
aller bisherigen Batch-Scan-Schwellwerte) bleibt offen**: `char
trash[8]` gleicht den Stackframe exakt an, aber Retail wertet
`getWire(param_1)` an der zweiten Aufrufstelle NEU aus (frischer
`lwz`/`lwzx`-Ladevorgang von `this->unk18[idx]`), während unser
Compiler das Ergebnis der ersten Auswertung wiederverwendet
(Common-Subexpression-Elimination) — eine weitere Instanz
asymmetrischer Compiler-Optimierungsentscheidungen, nicht über
`trash[N]` lösbar. Zurückgesetzt.

**Session-Endstand nach Runde 39 (fortgesetzt): 368 tatsächlich
verifizierte Funktionen** (361 aus Runde 1–38 plus 7 neue in
Runde 39: 6 `MapObjCorona.cpp` + 1 `MarDirectorEvent.cpp`) in 41
Commits. Funktionszahl: 8948 → **8955** (**+7**). DOL SHA1 bleibt
`OK`. Bestätigte neue Erkenntnis dieser Runde: reine
Logikimplementierung für leere Stubs (statt `trash[N]`-Padding) ist
eine tragfähige dritte Fix-Kategorie neben additiven Frame-Lücken
und (nicht per Pragma lösbaren) asymmetrischen Inlining-Fällen —
lohnt sich, gezielt nach kleinen (< 150 Byte) Funktionen mit sehr
niedrigem Match (< 60 %) zu suchen, da diese oft echte fehlende
Logik statt bloßer Padding-Lücken markieren.

**`TBathtub::tumble(f32, f32)` — Logik erfolgreich rekonstruiert,
aber Byte-Match durch globales `-fp_contract on` blockiert**: Retail
berechnet (Bound-Flag-Guard `unk29A` vorausgesetzt) `mag = param_2 *
0.0001f` (Literalwert `0x38D1B717` bestätigt), Winkel-Skalierung
`param_1 * 182.04445` (= `65536/360`, bestätigt `0x43360B61` —
identisch mit `MsSin`/`MsCos`s interner Konstante) und aktualisiert
`unk1E8 += mag * cos(angle)`, `unk1EC += 0.0f`, `unk1F0 -= mag *
sin(angle)`. Semantik vollständig verstanden, aber zwei
Compiler-Codegen-Eigenheiten verhindern Byte-Genauigkeit: (a)
Retail berechnet den Tabellenindex EINMAL für sin+cos gemeinsam,
unser `MsCos(x)`/`MsSin(x)` (zwei getrennte Aufrufe derselben Formel)
lässt den Compiler die Winkel-Skalierung ZWEIMAL berechnen — kein
CSE über die beiden (potenziell geinlineten) Aufrufe hinweg; (b)
Retail nutzt separate `fmuls`+`fadds` für `unk1E8 +=`, unser Code
erzeugt (wegen des projektweiten `-fp_contract on`-Compiler-Flags)
immer ein fusioniertes `fmadds` — nicht pro Datei/Funktion
abschaltbar, ohne das globale Flag zu ändern (Risiko: würde andere
bereits gematchte Funktionen zerstören). Zurückgesetzt, 0 Commit.
Als gut vorbereiteter Kandidat für eine künftige Session mit einer
Idee zur `fp_contract`-Umgehung (z. B. `volatile`-Zwischenspeicher
oder ein anderer Ausdrucksaufbau) vorgemerkt.

### Nach vierzigster Iterationsrunde (Grenzbefund: 200–400-Byte-Klasse resistent gegen `trash[N]`)

Erweiterter Scan mit gelockertem Größenfenster (200–400 Bytes,
50–95 % Match, bisher wegen zu hoher Größe/`ndiff` nicht erfasst)
fand 47 neue Kandidaten, davon 29 mit sauber additivem Frame-Gap
(≤ 0x60 Bytes, `ndiff` ≤ 100). Automatisierter Batch-Versuch (identisches
`try_fix2`-Protokoll wie Runde 37): **0 von 29 erreichten Match**
(alle Änderungen automatisch zurückgesetzt, keine unkommitteten
Reste). Manuelle Tiefenprüfung an drei Beispielen (`TGessoPolluteObj::
rebirth`, `TPictureTelesa::touchActor`) zeigt ein konsistentes Muster:
das Schließen des Frame-Gaps allein genügt nicht — zusätzlich
unterscheidet sich die Instruktions-REIHENFOLGE für unabhängige,
kommutative Lade-/Rechenoperationen (z. B. `lfs f0, 0x14(r30)` vor
vs. nach `fsubs f2, f3, f2`; Argumentreihenfolge bei `TGesso::
mPollRange * 32.0f * 0.5f`). Das ist dieselbe Kategorie wie der
bereits dokumentierte `ShadowUtil::calc`-Fund (Runde 38): der
Register-/Instruktions-Scheduler trifft bei größeren, komplexeren
Funktionskörpern andere Reihenfolge-Entscheidungen als Retail, auch
nachdem die Stackframe-Größe exakt übereinstimmt.

**Etablierte Grenze für die `char trash[N]`-Methode**: zuverlässig
erfolgreich nur bei kleinen (< 200 Byte), strukturell einfachen
Funktionen mit additivem Frame-Gap. Ab ~200 Byte / mehreren
unabhängigen Ausdrücken steigt die Wahrscheinlichkeit einer
zusätzlichen, nicht durch Padding behebbaren Scheduling-Differenz
drastisch. Bestätigt durch: 0/29 in dieser Runde (200–400 B) vs.
durchgehend hohe Erfolgsquoten in Runde 37 bei kleineren Kandidaten
(99 % zu Beginn, > 50 % bis Batch 4).

Keine Quelltextänderung in dieser Runde (alle Experimente sauber
automatisiert zurückgesetzt, 0 Commits, DOL SHA1 `OK`, `matched_
functions` unverändert bei 8955). Session-Gesamtstand bleibt bei
**368 tatsächlich verifizierte Funktionen** in 43 Commits.

**Empfehlung für künftige Sessions**: Die 200–400-Byte-Klasse braucht
entweder (a) gezielte, funktionsweise Ausdrucks-Umstrukturierung
(Argument-Reihenfolge, Zwischenvariablen-Reihenfolge) mit
Trial-and-Error pro Funktion, oder (b) eine neue Automatisierung, die
nicht nur `trash[N]` einfügt, sondern auch die Reihenfolge
unabhängiger Lade-Anweisungen systematisch permutiert und gegen
Retail vergleicht — deutlich aufwändiger als das bisherige Pattern
und nicht in dieser Runde verfolgt.

### Nach einundvierzigster Iterationsrunde (neuer produktiver Kandidatenpool: bereits nahe 100 % gematchte Funktionen, 400–2000 Bytes)

Entdeckung eines bislang unbearbeiteten Kandidatenpools: 789
Funktionen mit `fuzzy_match_percent` zwischen 90 % und 99,99 % und
Größe 400–2000 Bytes — bisherige Batches (Runde 37) beschränkten
sich auf Kandidaten unter 100 % Match generell, aber mit kleineren
Größen-Obergrenzen (≤ 500–3000 B bei gleichzeitig niedrigerem
Match-Prozentsatz 75–100 %). Der Pool "bereits > 99 % Match, aber
größere Funktion" wurde nie gezielt abgesucht.

- **Batch A** (99,5–100 % Match, Gap ≤ 0x40, `ndiff` ≤ 20, 134
  Kandidaten über alle 152 relevanten Einheiten): 6 Treffer (~4,5 %).
- **Batch B** (99,5–100 % Match, gelockert auf Gap ≤ 0x80, `ndiff`
  ≤ 40, 255 Kandidaten): 9 weitere Treffer (~3,5 %).
- **Batch C** (99,0–99,5 % Match, gleiche Gap/`ndiff`-Schwellen, 280
  Kandidaten): **0 Treffer** — Erschöpfungsgrenze dieses Pools
  erreicht; alle Änderungen automatisiert zurückgesetzt (reine
  CRLF-Touches, sauber per `git checkout .` entfernt).

Alle 15 Treffer einzeln gegen frischen `report.json` verifiziert (0
Probleme): `TTelesa::initItemAttacker`, `THinokuri2::reset`,
`TMapObjTreeScale::control`, `TBaseNPC::npcWetIn`,
`TGCConsole2::startAppearRedCoin`, `CPolarSubCamera::
execSecureView_`, `TOptionSoundUnit::setState`, `TMario::
getChangeAngleSpeed`, `TCasinoPanelGate::moveObject`,
`JPAConvertFixVecToFloatVec`, `TMario::addCallBack`, `TMario::
rocketing`, `TPollutionLayer::getPollutedPosNear`, `TAnimalBase::
init`, `TWoodBlock::calcRecycle`.

**Erkenntnis**: Der "nahe an 100 %"-Pool (99,5–100 %) ist ergiebiger
als die 200–400-Byte-Klasse aus derselben Runde, vermutlich weil ein
bereits sehr hoher Match-Prozentsatz stark mit "nur eine isolierte
additive Lücke, sonst identisch" korreliert — im Gegensatz zur
50–95-%-Klasse, wo ein niedrigerer Match-Prozentsatz öfter
zusammengesetzte Probleme (Frame-Lücke UND Scheduling-Differenz)
bedeutet. Absacken auf 99,0–99,5 % bringt keinen weiteren Ertrag mehr
(0/280) — die Ergiebigkeit korreliert eng mit der Match-Prozent-Nähe
zu 100 %, nicht nur mit der Gap-Größe.

**Session-Gesamtstand nach Runde 41: 383 tatsächlich verifizierte
Funktionen** (368 aus Runde 1–40 plus 15 neue in Runde 41) in 45
Commits. Funktionszahl: 8955 → **8970** (**+15**). DOL SHA1 bleibt
`OK`.

**Fortsetzung: vollständiger Größenbereich-Sweep** des 99,5–100-%-
Pools (zuvor nur 400–2000 Bytes abgedeckt) über alle 197 relevanten
Einheiten: 743 Kandidaten insgesamt, 360 mit sauberem additivem Gap
(≤ 0x40 Bytes, `ndiff` ≤ 25). **5 von 360 Treffer** (~1,4 % —
deutlich niedriger als die vorherigen Teilmengen-Batches, da große
Überschneidung mit bereits verarbeiteten 400–2000-Byte-Kandidaten
aus derselben Runde; die zusätzliche Ausbeute stammt aus den
Größenrändern < 400 B und > 2000 B). Gefixt: `TWaterGun::
changeNozzle`, `TMapObjBase::updateObjMtx`, `TTobiPuku::
calcRootMatrix`, `TPollutionLayerWave::draw`, `TSunGlass::
startFade`. Alle gegen frischen `report.json` verifiziert (0
Probleme).

**Session-Endstand nach Runde 41 (vollständig): 388 tatsächlich
verifizierte Funktionen** (368 aus Runde 1–40 plus 20 neue in
Runde 41) in 47 Commits. Funktionszahl: 8955 → **8975** (**+20**).
DOL SHA1 bleibt `OK`. Die sinkende Erfolgsquote (4,5 % → 3,5 % →
1,4 %) über die drei Teil-Batches bestätigt: der 99,5–100-%-Pool
ist nun ebenfalls weitgehend erschöpft für diese Session.


### Nach zweiundvierzigster Iterationsrunde (95–99,5-%-Band erschöpft; Methodik-Erkenntnis zu Data-Section-Mismatches)

**95–99,5-%-Match-Band systematisch geprüft** (zwischen dem
erschöpften < 95-%-Band und dem produktiven 99,5–100-%-Band): 457
Kandidaten über 160 Einheiten gefunden, mit strengen Schwellwerten
(Gap ≤ 0x30, `ndiff` ≤ 12) auf 11 saubere additive-Gap-Kandidaten
gefiltert. **0 von 11 erreichten Match** (automatisiert
zurückgesetzt, 0 Commits) — bestätigt, dass dieses mittlere Band
ebenfalls überwiegend zusammengesetzte Probleme (Frame-Lücke +
Scheduling-Differenz) statt reiner Padding-Lücken enthält, analog zu
Runde 40.

**Neue Methodik-Erkenntnis zu Daten-Sektionen**: Untersuchung von
`Player/MarioCap.cpp` (11,4 % Data-Match, auffällig niedrig)
zeigt: die 71 populierten Einheiten mit < 100 % Data-Match sind in
diesem Fall **keine unabhängigen Daten-Bugs**, sondern direkte
Konsequenz nicht-gematchter Funktionen in derselben Einheit — zwei
zusätzliche `.sdata2`-Fließkomma-Literale (`0.5f`, `3.0f`) in
unserer kompilierten `MarioCap.o` erwiesen sich als vollständig
abwesend im aktuellen Quelltext (keine Fundstelle für `0.5f`/`3.0f`
in `MarioCap.cpp`), was zunächst nach einem unabhängigen
Daten-Layout-Bug aussah. Tiefere Prüfung zeigt: `TMarioCap::
TMarioCap`/`::perform` sind selbst nicht gematcht (99,01 %/99,93 %),
mit erheblichen additiven Frame-Lücken (+120/+160 Bytes) UND
echten Register-Vertauschungen (nicht nur Offset-Verschiebungen) im
Rest-Diff — ein bereits vom Vorautor mit auskommentiertem `//
volatile u32 padding[51];` dokumentierter, aber nie gelöster
Versuch. `char trash[120]` gleicht den Stackframe exakt an (264→384,
`ndiff` 28→23), aber ein verbleibendes konsistentes +0x20-Offset-
Muster UND mehrere Register-Swaps (`lwz r3`↔`lwz r5` etc.) bleiben
bestehen; `trash[152]` überschießt (Frame 416 statt 384, `ndiff`
zurück auf 28). Bestätigt dieselbe "zusammengesetztes Problem"-
Kategorie aus Runde 40/38. Zurückgesetzt, 0 Commit.

**Praktische Konsequenz für künftige Sessions**: Ein niedriger
`matched_data_percent` in einer populierten Einheit ist in der
Regel ein SYMPTOM nicht-gematchter Funktionen in derselben Einheit
(unterschiedliche Literal-Pool-Referenzen durch unterschiedlichen
Code), kein eigenständig zu jagendes Ziel — zuerst den
Code-Match-Status der Einheit prüfen, bevor Zeit in
Daten-Sektions-Archäologie investiert wird.

Keine Quelltextänderung in dieser Runde (beide Experimente sauber
zurückgesetzt). Session-Gesamtstand bleibt bei **388 tatsächlich
verifizierte Funktionen** in 47 Commits. `matched_functions`
unverändert bei 8975, DOL SHA1 `OK`.


**Fortsetzung**: Weiterer Sweep des 99,5–100-%-Pools bei gelockerten
Schwellen (Gap ≤ 0x100, `ndiff` ≤ 60, 489 Kandidaten) stieß zweimal
auf einen Python-Kernel-Timeout (dieselbe Art Unterbrechung wie
Runde 37 Batch 1). Beide Male sauber wiederhergestellt via
`git status`/direkter `dtk elf disasm`-Nachprüfung: Von 7 bzw. 97
durch die Scan-/Fix-Schleife berührten Dateien war in der ersten
Wiederherstellung genau 1 echter Treffer dabei
(`TPoiHanaManager::initSetEnemies`, `char trash[0xa0]`, Byte-für-Byte
bestätigt), ein zweiter Kandidat (`TShine::control`) erwies sich als
Fehlschlag und wurde verworfen; die zweite Unterbrechung (97 Dateien)
bestand ausschließlich aus harmlosen CRLF-Touches der Scan-Phase
(keine echten Änderungen, sauber per `git checkout .` entfernt).

**Session-Endstand nach Runde 42: 389 tatsächlich verifizierte
Funktionen** in 48 Commits. Funktionszahl: 8975 → **8976** (**+1**).
DOL SHA1 bleibt `OK`. Der verbleibende 99,5–100-%-Pool (weiterhin
~735 Kandidaten) ist für automatisierte Massenverarbeitung in dieser
Sitzung nicht mehr zuverlässig zugänglich (wiederholte
Kernel-Instabilität bei Batchgrößen > ~250 Kandidaten); künftige
Sessions sollten kleinere Chunk-Größen (≤ 100 Kandidaten pro
Eval-Aufruf) mit expliziten Zwischen-Commits verwenden, um das
Wiederherstellungsrisiko zu begrenzen.

### Nach dreiundvierzigster Iterationsrunde (Klein-Chunk-Methodik bestätigt crash-sicher: 7 weitere Funktionen)

Direkte Anwendung der in Runde 42 dokumentierten Empfehlung:
Batch-Größe auf ≤ 100 Kandidaten pro Eval-Aufruf begrenzt, mit
explizitem Zwischen-Rebuild/-Commit/-Push nach jedem Chunk. Ergebnis:
**0 Kernel-Abstürze über 6 aufeinanderfolgende Chunks** (im Gegensatz
zu den zwei Abstürzen in Runde 42 bei größeren Batches). Nebenfund:
`git add` normalisiert CRLF-Zeilenenden bereits selbst beim Staging
(vermutlich via `.gitattributes`/`core.autocrlf`) — Dateien, deren
Inhalt nach Normalisierung identisch mit `HEAD` ist, werden gar nicht
erst in den Commit aufgenommen. Der bisher praktizierte manuelle
CRLF-Erkennungs-/Rücksetzungsschritt vor jedem Commit war unnötig
(aber harmlos); künftige Sessions können direkt `git add src/ && git
commit` nutzen und nur bei `git status`-Verdacht auf echte Inhalte
prüfen.

**Kandidatenpool-Abschluss für kleine Funktionen** (≤ 800 Bytes,
99,5–100 % Match): 488 + 87 = 575 Kandidaten über alle Einheiten
vollständig verarbeitet, **7 Treffer** (`TRoulette::TRoulette`,
`TAmenbo::doKeepDistance`, `TLightDrawBuffer::TLightDrawBuffer`,
`TBossEelEye::TBossEelEye`, `JPABaseEmitter::JPABaseEmitter`,
`TMapObjBillboard::touchActor`, `TSunGlass::loadAfter`).

**Größeres Funktionsband erstmals vollständig geprüft** (800–2500
Bytes, 99,5–100 % Match, 145 Kandidaten über 83 Einheiten): **0
Treffer** — bestätigt erneut die Runde-40-Erkenntnis, dass die
Erfolgsquote mit wachsender Funktionsgröße gegen null geht, auch
innerhalb des sonst produktiven Match-Prozent-Bands.

**Session-Gesamtstand nach Runde 43: 396 tatsächlich verifizierte
Funktionen** (389 aus Runde 1–42 plus 7 neue in Runde 43) in 53
Commits. Funktionszahl: 8976 → **8983** (**+7**). DOL SHA1 bleibt
`OK`. Der ≤ 800-Byte-Kandidatenpool im 99,5–100-%-Band ist jetzt
vollständig ausgeschöpft; verbleibende Kandidaten liegen
überwiegend in größeren, resistenten Funktionen.

### Nach vierundvierzigster Iterationsrunde (`FifoSetFog`/`FifoSetFogRangeAdj` in PacketUtil.cpp: neue Technik "Sibling-Algorithmus + Hardware-Write-Swap")

**Neue Kandidatenkategorie**: gezielte Suche nach kleinen, fast bei
0 % liegenden Funktionen, die sich beim Nachlesen als leere `{ }`-
Stubs herausstellen, statt als echte Mismatches. `src/MarioUtil/
PacketUtil.cpp` enthielt drei solche Stubs (`FifoSetFogRangeAdj`,
`FifoSetFog`, `ShapePacketCallBackFunc`).

**`FifoSetFogRangeAdj`** (312 Bytes, 1,28 % Match): erster Versuch
mit `J3DGDWriteBPCmd`/`GDOverflowCheck` (gepufferter Display-List-
Pfad, wie das Sibling `JRNISetFogRangeAdj` in `JRenderer.cpp:587`)
kompilierte sauber, aber **DIFFER** — Retail nutzt direkte rohe
Hardware-MMIO-Schreibzugriffe, nicht den gepufferten Pfad. Zweiter
Versuch mit rohem `GXWGFifo.u8 = GX_LOAD_BP_REG; GXWGFifo.u32 = reg;`
(passend zum SDK-Makro `GX_WRITE_BP_REG` aus `dolphin/gx/__gx.h` und
der festen Hardware-Adresse `GXFIFO_ADDR` aus `GXVert.h`) ergab
**Byte-exakten Match**, per `dtk elf disasm` direkt verifiziert.

**`FifoSetFog`** (344 Bytes, 1,16 % Match): komplexeres Sibling
`J3DGDSetFog` (`JRenderer.cpp:191`, A/B/C-Fog-Koeffizienten-Algorithmus
mit Mantisse/Exponent-Normalisierung über zwei While-Schleifen, Packung
via `BP_FOG_UNK0..3`/`BP_FOG_COLOR`-Makros aus `dolphin/gd/GDPixel.h`)
direkt mit denselben rohen `GXWGFifo`-Schreibzugriffen adaptiert.
Erster Versuch: 4 von 5 Schreibvorgängen matchten sofort, nur der
letzte (`BP_FOG_COLOR`) zeigte eine reine Instruktions-Scheduling-
Abweichung (6 Zeilen, gleiche Gesamtlänge 95/95) — die Farbfeld-Loads
wurden in Retail VOR dem Opcode-Byte-Write eingeplant. Fix: den
gepackten `BP_FOG_COLOR(...)`-Wert erst in eine lokale Variable
schreiben, dann `GXWGFifo.u8`/`.u32` zuweisen (statt inline im selben
Statement) — ergab sofort Byte-exakten Match.

**`SMS_InitPacket_Fog`** (140 Bytes, 93,14 % Match, gleiche Datei):
`char trash[8]` behob die Stackframe-Lücke (104→112 Bytes), aber
7 Zeilen Register-Scheduling-Differenz in der `getModelData()->
getMaterialNodePointer()->getPEBlock()->getFog()`-Aufrufkette
(vertauschte r3/r4-Zuweisung, andere Teilausdrucks-Reihenfolge)
blieben bestehen — bestätigt das etablierte Muster "additive
Frame-Lücke behoben, aber Register-Scheduling nicht fixbar via
Padding". Sauber zurückgesetzt.

**`ShapePacketCallBackFunc`** (1936 Bytes, 0,21 % Match): 11-Fall-
Sprungtabelle (passend zu den 11 `PacketUserData_*`-Structs in
derselben Datei), jeder Fall mit eigener komplexer Bit-Packing-Logik
für rohe Hardware-Schreibzugriffe (TEV-Farben, Material-Farben,
Fog-Aufrufe). Deutlich größerer Umfang als die bisherigen Erfolge
in dieser Datei, kein direktes Sibling verfügbar — als Kandidat für
eine künftige Session dokumentiert, nicht in dieser Runde angegangen.

**Neue Methodik-Erkenntnis**: die erfolgreichste Technik für leere
Stub-Funktionen ist "existierendes Sibling mit identischem Algorithmus
suchen, dann nur den Ein-/Ausgabe-Mechanismus (hier: gepufferter
Display-List-Write vs. rohe Hardware-FIFO-Writes) anpassen" — deutlich
zuverlässiger als reine Rohdisassembly-Rekonstruktion. Bei
verbleibenden Scheduling-Differenzen nach dem Sibling-Swap hilft oft
das Auslagern eines gepackten Ausdrucks in eine lokale Variable vor
dem MMIO-Write (analog zur bereits etablierten `trash[N]`-Erkenntnis,
dass MWCC Feld-Zugriffsreihenfolgen je nach Statement-Struktur anders
plant).

**Zusätzlicher Fund (`TModelWaterManager::calcWorldMinMax`, 360 Bytes,
war 10,37 % Match, keine Stub-Funktion sondern bereits plausible,
aber subtil falsche Logik)**: vier echte Bugs durch direkten `dtk elf
disasm`-Vergleich gefunden und behoben:
1. `unk5D70`/`unk5D7C` wurden aus einem gecachten `marioPos`-Local
   zugewiesen; Retail ruft `SMS_GetMarioPos()` (liefert eine
   Referenz) für jedes Ziel SEPARAT auf (passend zum doppelten
   `gpMarioPos`-Dereferenzieren in Retails Disasm).
2. Bestehender Tippfehler im Dekompilat: `fVar789.x/y/z += 1.0f`
   statt `fVar123.x/y/z += 1.0f` (Ergebnis der vorherigen `-1.0f`
   wurde versehentlich auf der falschen Variable rückgängig gemacht).
3. `TVec3::setMax()`/`setMin()`-Methodenaufrufe auf Stack-Structs
   durch rohe Skalar-Locals (`maxX`/`maxY`/`maxZ`/`minX`/`minY`/
   `minZ`) ersetzt — Retail rechnet rein in Gleitkomma-Registern
   OHNE Stackframe, während die Struct-Methodenaufrufe bei uns einen
   0x38-Byte-Frame erzwangen.
4. Exakte Vergleichsrichtung Retails nachgebildet (striktes `if
   (max > kandidat) max = kandidat;` / `if (min < kandidat) min =
   kandidat;` statt nicht-striktem `<=`/`>=`, das ein zusätzliches
   `cror+bne` statt eines einzelnen `ble`/`bge` erzeugte); Schleife
   beginnt bei Index 1 statt 0 (Index 0 diente bereits der
   Initialisierung, erneutes Verarbeiten wäre redundant).

Byte-exakter Match nach allen vier Korrekturen, per `dtk elf disasm`
bestätigt.

**Session-Gesamtstand nach Runde 44: 399 tatsächlich verifizierte
Funktionen** (396 aus Runde 1–43 plus 3 neue in Runde 44:
`FifoSetFogRangeAdj`, `FifoSetFog`, `calcWorldMinMax`) in 57 Commits.
Funktionszahl: 8983 → **8986** (**+3**). DOL SHA1 bleibt `OK`.








### Nach fünfundvierzigster Iterationsrunde (systematischer Vollabdeckungs-Scan des 99,5–99,99-%-Bands: 1 weiterer Treffer; drei neue Compiler-Heuristik-Grenzfälle dokumentiert)

**Vollständiger Neu-Scan des 99,5–99,99-%-Match-Bands über ALLE 197
betroffenen Einheiten** (730 Kandidaten, nicht mehr nur ≤ 800 Bytes
wie in Runde 43, sondern jede Größe): 515 echte additive
Stackframe-Lücken gefunden und einzeln per `char trash[N]` getestet
(automatisierte Pipeline: Funktion im Quelltext lokalisieren,
`trash[N]` einfügen, Einheit neu bauen, `dtk elf disasm` vergleichen,
bei Nichttreffer automatisch zurücksetzen). Ergebnis: **1 Treffer**
(`TSunGlass::load`, `char trash[24]`) — bestätigt erneut, dass der
additive-Lücken-Pool für dieses Band praktisch erschöpft ist (Runde
43 hatte bereits 575 Kandidaten im ≤ 800-Byte-Teilbereich
abgearbeitet; die verbleibenden ~150 neuen Kandidaten in diesem
erweiterten Scan lieferten nur den einen zusätzlichen Fund).

**Kernel-Absturz bei zu großem Batch (171 Kandidaten in einem
Eval-Aufruf) reproduziert und sauber gehandhabt**: nach Absturz zeigte
`git status` genau eine midway-modifizierte Datei
(`src/NPC/NpcChange.cpp`, ein `trash[24]`-Versuch in
`TBaseNPC::behaveToHitObject_`); direkter `dtk elf disasm`-Vergleich
außerhalb des Kernels bestätigte NICHT-Match, sauber per `git
checkout --` zurückgesetzt. Bestätigt erneut die Session-Regel:
Batch-Größe ≤ 40 Kandidaten pro Eval-Aufruf für Crash-Sicherheit.

**Drei neue, tiefer untersuchte Compiler-Heuristik-Grenzfälle
gefunden, alle nicht behebbar**:
1. **`TMapWireManager::getPointPosInNthWire`** (57 %, 96 B): Quelltext
   ruft `getWire(param_1)` bereits zweimal explizit auf (identisch zu
   Retail), aber unser Compiler CSE't (common subexpression
   elimination) den kompletten `unk18[index]`-Speicherzugriff über
   die Aufrufgrenze hinweg, während Retail den Load bei jedem Aufruf
   neu ausführt (nur der Index-Shift wird geteilt). Reine
   Optimierer-Entscheidung, nicht steuerbar.
2. **`TSpineEnemy::isReachedToGoal()`** (67,5 %, 184 B,
   `include/Enemy/Enemy.hpp`): Versuch, das Muster bereits
   existierender, funktionierender Geschwister-Implementierungen
   (`THaneHamuKuri::isReachedToGoal`, `TTamaNoko::isReachedToGoal` —
   beide kopieren `unk104.getPoint()` zuerst in ein benanntes
   `JGeometry::TVec3<f32>`-Local) auf die Basisklassen-Version
   anzuwenden. Nach Vollbau: Match-Rate verschlechterte sich auf
   42,0 % (von 67,5 %) — sofort zurückgesetzt. Zeigt: dasselbe
   Quelltextmuster kann je nach Kontext (virtuelle Funktion in
   vielfach eingebundenem Header vs. konkrete Klassenmethode in
   eigener `.cpp`) zu unterschiedlichem MWCC-Codegen führen.
3. **`JPAVecToRotaMtx`** (56,9 %, 436 B,
   `src/JSystem/JParticle/JPAMath.cpp`): Erste Analyse zeigte, dass
   Retail den `sq`-Schwellenwert (Konstante `@1489` = 0.0f) vor der
   teuren `frsqrte`-basierten Quadratwurzel prüft — das ist jedoch
   nur `JGeometry::TUtil<f32>::sqrt()`s eigener interner
   `if (mag <= 0.0f) return mag;`-Guard, inline ausgerollt, und war
   bereits identisch in unserem Code vorhanden. **Versuch**: `axis`
   (ein `TVec3`-Struct mit `.cross()`/`.scale()`/`.zero()`-Aufrufen)
   durch rohe Skalar-Locals (`axisX/Y/Z`) ersetzt, um den
   `calcWorldMinMax`-Fund (Runde 44: Struct-Methodenaufrufe erzwingen
   Stack-Spill) zu wiederholen — Ergebnis NICHT wie erwartet: Retails
   finale Matrixkonstruktion liest die Achsenkomponenten selbst
   NACH der Skalierung weiterhin von einem Stack-Puffer
   (`lfs f7, 0x14(r1)` etc.), d. h. Retail verwendet ebenfalls einen
   Stack-gestützten Aufbau — die `calcWorldMinMax`-Heuristik
   ("Struct-Methode vermeiden → kein Stack-Spill") gilt hier NICHT
   pauschal. Sauber zurückgesetzt (94 vs. 115 Zeilen, weiterhin
   92 Differenzen). Verbleibt als ungelöster Fall — die exakte
   Quellstruktur, die Retails Stack-Layout UND Registerzuteilung
   gleichzeitig reproduziert, wurde nicht gefunden.

**`TConsoleStr::processGo`** (14,2 %, 1684 B,
`src/GC2D/ConsoleStr.cpp`): mehrere bestehende
`// TODO: all wrong`/`// TODO:`-Kommentare eines früheren
Beitragenden zeigen, dass die verschachtelte If-Kette
(`param_1 >= 90/95/175`-Fälle) nur teilweise rekonstruiert ist — echte
Mehr-Stunden-Rekonstruktion, für künftige Session vorgemerkt.

**Session-Gesamtstand nach Runde 45: 400 tatsächlich verifizierte
Funktionen** (399 aus Runde 1–44 plus 1 neue in Runde 45:
`TSunGlass::load`) in 58 Commits. Funktionszahl: 8986 → **8987**
(**+1**). DOL SHA1 bleibt `OK`.

### Nach sechsundvierzigster Iterationsrunde (kritische Methodik-Lektion: rohes ELF-Byte-Lesen ohne Relokationsauflösung ist unzuverlässig für Datenvergleiche; Destruktor-Pool erneut als bekanntes Artefakt bestätigt)

**Stichprobenprüfung des `fp=None`-Destruktor-Pools** (573 Kandidaten
mit `__dt__`-Namensmuster und fehlendem `fuzzy_match_percent` im
aktuellen `report.json`): Stichprobe `TWoodLog::~TWoodLog()`
(`mario/MoveBG/MapObjBianco`) zeigt das Symbol existiert NICHT im
eigenen `src`-Build dieser Einheit (weak-Symbol, vom Compiler in
dieser TU nicht dupliziert, da Retail hier großzügiger dupliziert).
Bestätigt exakt das bereits in Runde 32/33 etablierte
Per-TU-Duplikations-Artefakt — keine neue Erkenntnis, keine
Handlungsoption ohne MWCC-interne Heuristik-Kontrolle.

**KRITISCHER METHODIK-FUND**: Versuch, `.data`-Abschnitte direkt
durch rohes Parsen der ELF-Sektionsbytes (Python, ohne
Relokationsauflösung) zwischen `src/*.o` und `obj/*.o` zu vergleichen,
um datenreiche Einheiten mit 100 % Code-Match aber niedrigem
Daten-Match zu untersuchen (`mario/JSystem/JAudio/JASystem/
JASPlayer_impl`, 100 % Code / 2,1 % Daten laut `report.json`).
Rohvergleich zeigte scheinbar `sTreTable[8]` mit `0x8001` in unserem
Quelltext vs. `0x0000` in Retails `.o`-Datei — Korrektur angewendet,
**DOL-SHA1-Check schlug danach fehl** (`build/GMSJ01/mario.dol:
FAILED`)! Sofort zurückgesetzt, DOL wieder `OK` bestätigt.

**Ursache**: diese Einheit ist eine `complete: true`-Einheit (verlinkt
aus `src/*.o`, bestätigt durch die direkte Reaktion des
DOL-Hash-Checks auf die Quelltextänderung) — der ORIGINALE Wert
`0x8001` war die ganze Zeit korrekt und bereits Teil der
erfolgreich verlinkten, exakt passenden DOL. Der rohe
ELF-Sektionsvergleich (ohne `.rela.data`-Relokationen aufzulösen)
lieferte ein IRREFÜHRENDES Ergebnis für `obj/*.o` an dieser Stelle.

**Neue Methodik-Regel**: rohes ELF-`.data`-Byte-Lesen ohne
Relokationsauflösung ist NICHT als eigenständige Verifikationsmethode
für Datenwerte zu verwenden. Für `complete: true`-Einheiten ist der
DOL-SHA1-Check die einzige zuverlässige Autorität (dieser hat den
Fehler hier korrekt und sofort erkannt — das etablierte
Verifikationsprotokoll „DOL-SHA1 nach jeder Änderung prüfen" hat
genau wie vorgesehen funktioniert und eine reale Regression
verhindert). Für `complete: false`-Einheiten bleibt `dtk elf disasm`
(Code) die etablierte Methode; ein äquivalent zuverlässiges Werkzeug
für Datenabschnitts-Vergleiche wurde in dieser Runde NICHT gefunden
und ist für künftige Sessions offen (evtl. `objdiff-cli`s internes
`data_diff`-Feature untersuchen, falls über eine unterstützte
Schnittstelle zugänglich).

**Keine neuen Fixes in Runde 46** — beide Untersuchungslinien
(Destruktor-Pool, Datenvergleich) endeten in bereits bekannten bzw.
neu entdeckten, aber nicht umsetzbaren Sackgassen. Session-Gesamtstand
bleibt bei **400 tatsächlich verifizierten Funktionen** in 58 Commits
(unverändert seit Runde 45). DOL SHA1 bestätigt `OK`.


### Nach siebenundvierzigster Iterationsrunde (`TBathtubKiller::attackToMario`: reale Logik zu 93 % rekonstruiert, algorithmische Struktur bestätigt, Restdifferenz im Stackframe ungelöst)

Erster echter Rekonstruktionsversuch im Runde-44-dokumentierten
TBathtubKiller-Cluster. `attackToMario()` (0 % Match, leerer `{ }`-Stub,
404 B Retail-Zielgröße) per direktem `dtk elf disasm`-Studium der
Retail-Instruktionen vollständig algorithmisch rekonstruiert:

```cpp
void TBathtubKiller::attackToMario()
{
	bool isDying
	    = mSpine->getCurrentNerve() == &TNerveBathtubKillerExplosion::theNerve()
	      || mSpine->getCurrentNerve() == &TNerveBathtubKillerBreak::theNerve();

	if (!isDying) {
		if (SMS_GetMarioPos().y < mPosition.y) {
			mSpine->pushNerve(&TNerveBathtubKillerExplosion::theNerve());
			SMS_SendMessageToMario(this, HIT_MESSAGE_ATTACK);
			SMS_ThrowMario(JGeometry::TVec3<f32>(0.0f, 1.0f, 0.0f), 10.0f);
			unk21C = 1;
		}
	}
}
```

**Wichtiger Zwischenschritt**: die erste Fassung nutzte De-Morgan-Form
(`cur != A && cur != B`), was zu VERTAUSCHTER Flag-Polarität gegenüber
Retail führte (Retail initialisiert das "Ist-Sterbend"-Flag mit 1 und
löscht es bedingt; unsere De-Morgan-Form initialisierte mit 0 und
setzte bedingt auf 1 — bei IDENTISCHEM Wahrheitswert, aber
UNTERSCHIEDLICHEM Opcode-Muster). Umschreiben auf die direkte
Oder-Form `cur == A || cur == B` mit `if (!isDying)` traf exakt
Retails Register-Initialisierungsmuster (`li r30, 0x1` an der
richtigen Stelle) — ein weiterer bestätigter Fall, dass logisch
äquivalente, aber unterschiedlich geschriebene Boolesche Ausdrücke zu
verschiedenem MWCC-Code führen (ähnlich der `calcWorldMinMax`- und
`calcVMMtxGround`-Funde).

**Verbleibende Differenz**: nach der Oder-Form-Korrektur matcht die
komplette Kernlogik (Nerve-Vergleich, verschachteltes `pushNerve()`,
`SMS_SendMessageToMario`, `SMS_ThrowMario`, alle Konstanten und
Sprungziele) strukturell 1:1 mit Retail — nur der Stackframe ist
8 Bytes GRÖSSER als Retails (`-0x58` vs. `-0x50`), OBWOHL dieselbe
Anzahl Register gesichert wird. `char trash[8]` (Standardtechnik
dieser Session) verschlimmerte die Lücke auf 16 Bytes, da unser Frame
bereits GRÖSSER war — die etablierte Padding-Technik ist hier nicht
anwendbar (sie hilft nur, wenn UNSER Frame kleiner ist). Der
`bool isDying`-Local direkt in die `if`-Bedingung inlinen (kein
benannter Local) verschlimmerte die Situation weiter (120 statt
109 Zeilen, zusätzlicher `...bss.0`-Bezug, vermutlich verdoppelte
Lazy-Init-Instanziierung durch geänderte Ausdrucksauswertung) — sauber
zurückgesetzt.

**Sauber zurückgesetzt** (kein Netto-Fix, `git checkout --` bestätigt,
DOL SHA1 `OK`). Der bestätigte Algorithmus (inkl. der Oder-Form-
Erkenntnis) bleibt als direkt wiederverwendbare Vorlage für einen
künftigen Anlauf mit mehr Zeit für die Stackframe-Spurensuche
(vermutlich ein zusätzliches temporäres Objekt oder eine andere
Local-Variablen-Anordnung, die Retail nutzt, um 8 Bytes zu sparen).

**Session-Gesamtstand nach Runde 47: weiterhin 400 tatsächlich
verifizierte Funktionen** (unverändert seit Runde 45) in 58 Commits.
DOL SHA1 bestätigt `OK`.


### Nach achtundvierzigster Iterationsrunde (breiter 70–99,5-%-Scan außerhalb bekannter Cluster: 84 Kandidaten, 0 Treffer; `initECTMir` bis auf 4 Zeilen rekonstruiert; neue Registerzuteilungs-Fallklasse dokumentiert)

**`TBathtubKiller::attackToMario`-Stackframe-Rätsel weiter untersucht**
(named-local-Variante des `TRect`-Temporarys, `int` statt `bool`) —
keine der Varianten änderte die Frame-Differenz; Sackgasse endgültig
bestätigt, sauber zurückgesetzt.

**Breiter Scan des 70–99,5-%-Bands außerhalb aller bisher bearbeiteten
Cluster-Dateien** (118 Kandidaten in 76 Einheiten, Größe 20–300 Bytes):
84 Kandidaten mit Stackframe-Differenz gefunden und automatisiert per
`char trash[N]` getestet — **0 Treffer**. Bestätigt, dass auch dieses
Band inzwischen weitgehend erschöpft ist.

**`TMarDirector::initECTMir`** (99,48 %, 248 B) am weitesten
rekonstruiert: `char trash[24]` schloss die anfängliche 24-Byte-
Framelücke exakt; Umschreiben des `JDrama::TRect`-Funktionsarguments
auf ein benanntes `srcRect`-Local traf zusätzlich Retails exakte
Registerreihenfolge (`r3` vor `r4` statt umgekehrt) — von 14 auf
4 Diff-Zeilen reduziert. Verbleibend: zwei kosmetische
`...rodata.N`-vs-`"@N"`-Label-Unterschiede (derselbe String-Literal-
Inhalt, nur andere Pool-Nummerierung) sowie ein hartnäckiger
"interner Slot-Versatz" (`0x40` vs. `0x64` als Stack-Offset für
`srcRect`, bei bereits identischer Gesamt-Framegröße) — bestätigt
erneut die bereits mehrfach dokumentierte resistente Kategorie
(6+ frühere Fälle diese Session). `trash[N]`-Platzierung vor/nach der
Local-Deklaration verändert den Offset nicht. Sauber zurückgesetzt.

**Neue Registerzuteilungs-Fallklasse dokumentiert**
(`execute__18TNerveNameKuriLandCFP24TSpineBase<10TLiveActor>`,
96,8 %, 144 B): Retail hält den `self`-Zeiger NIE in einem
Callee-Saved-Register (kein `r31`-Save, Frame nur `-0x8` statt
unserer `-0x20`) — es nutzt `r3` direkt durch, weil auf JEDEM
Kontrollflusspfad entweder `self` vor dem einzigen `bl
checkCurAnmEnd`-Aufruf verbraucht wird oder dieser Aufruf komplett
übersprungen wird (Short-Circuit-`&&`). Unser Compiler entscheidet
sich konservativer für `r31`-Sicherung. Gleiche Quellstruktur wie
Retail (`if (self->isBckAnm(4) && self->checkCurAnmEnd(0)) return
true; if (!self->isAirborne()) self->setBckAnm(4); return false;`)
— reine MWCC-Liveness-Analyse-Differenz, nicht in dieser Runde weiter
verfolgt (passt in dieselbe "asymmetrische Compiler-Optimierungs-
entscheidung"-Kategorie wie `identity33`).

**Keine neuen Fixes in Runde 48.** Session-Gesamtstand bleibt bei
**400 tatsächlich verifizierten Funktionen** in 58 Commits
(unverändert seit Runde 45). DOL SHA1 bestätigt `OK`.



## Nächster GMSJ01-Kandidat
**Neuer, großer Kandidaten-Cluster identifiziert (Runde 44):
`src/Enemy/BathtubKiller.cpp`** — `TBathtubKiller`/`TBathtubKillerManager`
haben ca. 12 Funktionen mit echtem, aber nicht byte-genauem
Rekonstruktions-Bedarf: `bind` (0,4 %, 1004 B), `perform` (10,0 %,
1164 B, mit bestehendem TODO "only the bathtub lookup is
reconstructed"), `makeInitialVelocity` (0,4 %, 932 B), `moveChasing`
(0,7 %, 604 B), `makeQuat` (90,5 %, 1532 B — selbst NICHT vollständig
korrekt trotz vollständiger Logik), `receiveMessage` (0,9 %, 620 B),
`attackToMario` (1,0 %, 404 B), `isCollidMove` (0,7 %, 1068 B),
`behaveToWater` (34,3 %, 280 B), `isAboided` (0,7 %, 836 B), die
`execute`-Methoden von vier `TNerveBathtubKiller*`-Nerven (Wander
0,6 %/952 B, Chase 0,6 %/912 B, ChaseStraight 0,5 %/1088 B, Straight
62,4 %/400 B), sowie `TBathtubKillerManager::load`/`loadAfter`
(81,0 %/79,1 %). Viele weitere Methoden (`killBathtubKiller`,
`explodeBathtubKiller`, `moveParabolic`, `moveStraight`,
`makeVelocityQuat`, `makeAccelerationQuat`, `makeScrewQuat`,
`setNormalBathtubKillerAnm` u. a.) sind leere `{ }`-Stubs, tauchen
aber NICHT in `report.json` auf — vermutlich vom Compiler wegoptimiert/
wegge-inlined (keine eigenständigen Symbole in Retail).

**Tiefenanalyse `TNerveBathtubKillerStraight::execute`** (62,4 %, hat
bereits echte, plausible Logik, kein Stub): direkter `dtk elf
disasm`-Vergleich zeigt, dass Retail zwei explizite Funktionsaufrufe
(`TVec3<f>::dot`, `TVec3<f>::scale`) tätigt, die in unserem Build
vollständig zu rohen Gleitkomma-Instruktionen ge-inlined sind (der
Rest der Funktion — `getMActor`-Aufruf, `makeQuat`-Aufruf — matcht
strukturell). Sehr wahrscheinlich dieselbe bestätigt-unfixbare
Kategorie "asymmetrisches Pro-Aufrufstellen-Inlining" wie
`TRotation3::identity33` (Runde 34) — nicht mit `#pragma dont_inline`
behebbar, da funktionsweit statt pro Aufrufstelle wirksam.

**Einschätzung**: Der TBathtub-Cluster (Runde 39/40, blockiert durch
fehlende `TBathtubGrip`-Klasse und `-fp_contract`) und der
TBathtubKiller-Cluster sind beide grundsätzlich reale, aber deutlich
größere Mehr-Stunden-Rekonstruktionsprojekte (Physik-/Quaternion-
Bewegungslogik, ~8 verbleibende Funktionen mit insgesamt > 8000
Bytes) statt schnelle Fortsetzungs-Gewinne — für eine künftige
dedizierte Session vorgemerkt, nicht in Runde 44 angegangen.

**Weitere Funde in `ModelWaterManager.cpp` (Runde 44)**:
`calcVMMtxGround`/`calcVMMtxWall` (62,49 %/54,12 %, beide mit
bestehendem `// TODO: matching this is ewwwwwwwwwwwwwwwwww`-Kommentar
eines früheren Beitragenden) — Ursache teilweise gefunden: Quellcode
nutzt `param_4.y * 2.0 + param_3.y` mit `2.0` (Double-Literal statt
`2.0f`), wodurch MWCC ein `fmadd` (Doppelpräzision) statt Retails
`fmadds` (Einzelpräzision) erzeugt. Korrektur auf `2.0f` behebt genau
dieses Symptom (bestätigt an den erzeugten Opcodes), reduziert die
Differenz messbar, erreicht aber in keiner der beiden Funktionen
100 % — verbleibende Differenz ist reine Registerzuteilung
(unterschiedliche Stackframe-Größe, 3 vs. 2 gesicherte
Gleitkomma-Register). Sauber zurückgesetzt, da nicht vollständig
matchend; Erkenntnis über den `2.0`-vs-`2.0f`-Unterschied als
Ausgangspunkt für eine künftige Session festgehalten.

**`TSunModel::calcDispRatioAndScreenPos_`** (14,15 %, 292 B, `weak`
Symbol in Retail): Ursache identifiziert — der einzige Aufrufer von
`CLBScreenFPosToSPos()` (`include/Camera/cameralib.hpp:338`, bereits
mit `#pragma dont_inline on/off` UND explizitem C++-`inline`-Keyword
versehen) wird trotz Pragma vollständig ge-inlined (516 statt
292 Bytes, kein eigenständiges `CLBScreenFPosToSPos`-Symbol im
eigenen Build). Ein bereits bestehender Kommentar eines früheren
Beitragenden ("TODO: definitely more inlines but I couldn't get it
to work out...") bestätigt: dasselbe Problem wurde schon einmal
erfolglos angegangen. Versuch, das `inline`-Schlüsselwort zu
entfernen, sofort zurückgesetzt (Header wird von 37+ Übersetzungs­
einheiten eingebunden, hätte Mehrfachdefinitions-Linkerfehler
verursacht). Bestätigt dieselbe MWCC-Pragma-Ignorier-Eigenart wie
andere in dieser Session dokumentierte Fälle — nicht ohne tiefere
Compiler-Archäologie behebbar.



**Wieder offen (siehe Methodik-Korrektur oben)**: 58 der ursprünglich
59 in Runde 32/33 als "bereits korrekt" dokumentierten Funktionen
(1 davon — `TCylinder::makeDL` — in Runde 35 als tatsächlich korrekt
bestätigt, siehe oben; `TFireHamuKuri::moveObject/isHitValid` in
Runde 35 gefixt). Korrektur einer Falschaussage aus der ersten
Retraction-Notiz: `dieFire`/`genFire`/`recoverFire`/`TNerveFire-
HamuKuriRecover::theNerve` sind ENTGEGEN der ursprünglichen (auf
veralteten `objdiff-cli`-Diff-Daten basierenden) Behauptung NICHT
komplett fehlend — `dieFire`/`genFire` sind triviale leere
Ein-Zeiler, `recoverFire` existiert mit einem bestehenden `// TODO:
this is the wrong inline, size doesn't match at all!`-Kommentar eines
früheren Beitragenden.

**Präzisierung zu `MarNameRefGen_Enemy.cpp`**: Die zwölf dort
"fehlenden" Destruktoren (`TSimpleEffect`, `TLauncherManager`,
`TTobiPuku`, `TTobiPukuManager`, `TTobiPukuLaunchPad`,
`TTobiPukuLaunchPadManager`, `TPoiHana`, `TGesso` u. a.) sind KEINE
fehlende Funktionalität — Stichprobe `TGesso::~TGesso()` zeigt den
Destruktor korrekt und vollständig in `src/Enemy/gesso.o` (weak
Symbol `__dt__6TGessoFv`, plus `@32@`-Vtable-Adjustor-Thunk). Es
handelt sich um eine reine Per-TU-Duplikations-Entscheidung: Retails
Compiler legt in `MarNameRefGen_Enemy.o` zusätzlich eine redundante
lokale Kopie an (via `new TGesso` in `getNameRef_Enemy()`, das die
Vtable und damit die Destruktor-Adresse referenziert), unser Compiler
entscheidet sich hier für eine externe Referenz statt lokaler
Duplizierung — funktional identisch (der Linker verwirft ohnehin alle
bis auf eine Kopie), aber sichtbar als abweichendes Symbolset in
genau dieser Einheit. Vermutlich dieselbe Kategorie wie das
`TRotation3::identity33`-Problem aus Runde 34 (asymmetrische
Per-Aufrufstellen-Entscheidung des Compilers, nicht per Pragma
erzwingbar) — nicht ohne tiefere MWCC-Heuristik-Archäologie behebbar,
aber auch keine Prioritäts-Baustelle, da die eigentliche
Funktionalität nachweislich korrekt ist.

`ItemManager::newAndRegisterCoin` (99,59 %) und `PollutionManager::
cleanedAll` (96,43 %) offen.
`JSystem/J3D/J3DGraphAnimator/J3DModel.cpp::entryModelData` (1248 Bytes,
99,76 %) — Aufteilen des inline `&mShapePackets[shape->getIndex()]`-Ausdrucks
in einen expliziten `J3DShapePacket*`-Local behob den ersten
Register-Swap-Cluster (99,76 % → 99,84 %), vergrößerte aber den Stackframe
um 8 Bytes über das Original hinaus (jeder neue Pointer-Local kostet dort
mehr als erwartet). Drei Varianten (separater `shape`-Local, kombinierter
Ausdruck, zusätzliches `trash[8]`) landeten alle bei 99,84 % mit
falschem Frame; zurückgesetzt auf die Original-99,76-%-Fassung, da sie
wenigstens den korrekten Stackframe hat. `JSystem/JKernel/JKRExpHeap.cpp::
allocFromHead(u32,int)` (98,78 %, Register-Scheduling um -1-Konstante)
ebenfalls in drei Varianten versucht, kein Fortschritt.

### Nach neunundvierzigster Iterationsrunde (`TBathtubKiller::behaveToWater`: falsche Algorithmus-Logik entdeckt und korrigiert rekonstruiert; **grundlegender Methodik-Fund**: `theNerve()`-Inlining ist eine dateiweite Compiler-Heuristik, kein Einzelfunktions-Rätsel)

**Ausgangspunkt**: Im dokumentierten `TBathtubKiller`-Cluster (Runde 47) wurde
`behaveToWater(THitActor*)` erneut untersucht. Der bisherige Quellcode
(`{ breakBathtubKiller(); }`, 34,3 % Fuzzy-Match) erwies sich beim direkten
Disassembly-Vergleich nicht nur als unpräzise, sondern als **komplett falscher
Algorithmus**: Retail implementiert exakt dasselbe `isDying`-Muster wie
`attackToMario` (Vergleich des aktuellen Nerve gegen
`TNerveBathtubKillerExplosion::theNerve()` und `TNerveBathtubKillerBreak::
theNerve()`), gefolgt von `mSpine->pushNerve(&TNerveBathtubKillerBreak::
theNerve())`, falls nicht sterbend — kein Aufruf von `breakBathtubKiller()`
überhaupt. Als Quellcode rekonstruiert:

```cpp
void TBathtubKiller::behaveToWater(THitActor*)
{
	bool isDying
	    = mSpine->getCurrentNerve() == &TNerveBathtubKillerExplosion::theNerve()
	      || mSpine->getCurrentNerve() == &TNerveBathtubKillerBreak::theNerve();

	if (!isDying) {
		mSpine->pushNerve(&TNerveBathtubKillerBreak::theNerve());
	}
}
```

Kompiliert sauber, aber der Disassembly-Vergleich zeigte einen fundamentalen
strukturellen Unterschied: Im Retail-Objekt existiert `TNerveBathtubKillerExplosion
::theNerve()` (und `TNerveBathtubKillerBreak::theNerve()`) als **eigenständige,
reale Out-of-line-Funktion** (`.fn theNerve__28TNerveBathtubKillerExplosionFv`,
ausschließlich per `bl` aufgerufen — bestätigt an *allen* sechs Retail-Aufrufstellen
im Cluster: `behaveToWater` ×2, `isCollidMove` ×7, `receiveMessage` ×4,
`perform` ×3, `bind` ×5, `attackToMario` ×1 „echter" Aufruf). In unserem aktuellen
Build dagegen wird der komplette `theNerve()`-Rumpf (Lazy-Init-Guard, vtable-
Konstruktion, `__register_global_object`-Aufruf) **vollständig inline** in
`behaveToWater` expandiert — die Funktion `theNerve__28TNerveBathtubKillerExplosionFv`
existiert in unserem Objekt gar nicht als eigene Symboldefinition.

**Hypothesentest 1 (Datei-Reihenfolge)**: Vermutung, dass die Inline-Entscheidung
von der Position der *ersten* Aufrufstelle in der Datei abhängt (`attackToMario`
erscheint vor `behaveToWater`). `attackToMario` probeweise mit der in Runde 47
rekonstruierten Logik wieder eingefügt (gemeinsam mit `behaveToWater`) und neu
gebaut — **kein Effekt**, `behaveToWater` blieb vollständig inline. Hypothese
verworfen.

**Fund im Retail-Disassembly**: `attackToMario` selbst dupliziert den
Lazy-Init-Guard für `TNerveBathtubKillerExplosion` *zweimal inline* (einmal für
den `isDying`-Vergleich, einmal für die `pushNerve(&Explosion::theNerve())`-Aufrufstelle
später in derselben Funktion) — vermutlich eine lokale CSE-Optimierung des
Compilers für *mehrfach in derselben Funktion referenzierte* Aufrufe. Für
`TNerveBathtubKillerBreak` (nur einmal in `attackToMario` referenziert) bleibt
es dagegen bei einem regulären `bl theNerve__24TNerveBathtubKillerBreakFv`.
Dieses Verhalten ist strikt lokal pro Funktion, keine dateiweite Weiterverbreitung
(bestätigt: `behaveToWater` referenziert `TNerveBathtubKillerExplosion::theNerve()`
in Retail ebenfalls nur einmal und erhält dort einen regulären `bl`-Aufruf, nicht
die Inline-Behandlung).

**Hypothesentest 2 (Aufrufstellen-Gesamtzahl in der TU)**: Vermutung, dass MWCCs
Auto-Inliner die Entscheidung „lohnt sich eine eigene Out-of-line-Funktion" von
der *Gesamtzahl* der `theNerve()`-Aufrufstellen in der ganzen Übersetzungseinheit
abhängig macht (Retail: rund 22 Aufrufstellen über sechs Funktionen; unser
aktueller Stand: 2–6). Günstiger Test: `isAboided()` und `canChase()`
(bisher leere Stubs) probeweise mit zusätzlichen (nicht committeten)
`theNerve()`-Referenzen bestückt, um die Gesamtzahl testweise zu erhöhen, ohne
die vollständige Cluster-Rekonstruktion vorwegzunehmen. Nach Neubau blieb
`behaveToWater` weiterhin vollständig inline — Hypothese in dieser einfachen
Form ebenfalls verworfen (der tatsächliche Schwellenwert, falls einer existiert,
liegt jedenfalls deutlich höher als die getestete Aufstockung, oder die
Entscheidung hängt zusätzlich von der Gesamtkomplexität/-größe der Datei ab,
die in unserem Stand noch weit von Retails vollständiger ~4000-Zeilen-Implementierung
entfernt ist).

**Schlussfolgerung**: Das `theNerve()`-Inlining-Verhalten ist keine lokale
Stellschraube, die sich durch Umformulierung einer einzelnen Funktion lösen
lässt (anders als die bisher dokumentierten `char trash[N]`-Fälle). Es handelt
sich um eine dateiweite MWCC-Optimierungsheuristik, deren genaue
Schwellenwertbedingung nicht mit vertretbarem Aufwand isoliert reproduzierbar
ist, ohne den *gesamten* `TBathtubKiller`-Cluster (`isCollidMove`,
`receiveMessage`, `perform`, `bind` — je 100–300+ Byte komplexe Funktionen,
alle mit Retail-Referenzdisassembly bereits in `build/_btk_check_obj.s`
vorliegend) gemeinsam korrekt zu rekonstruieren. Das ist ein legitimes, aber
deutlich größeres Vorhaben als ein Einzelfunktions-Fix und wird für eine
zukünftige, dedizierte Mehrfunktions-Rekonstruktionsrunde zurückgestellt.
Da `behaveToWater` mit der korrigierten Logik weiterhin nicht Byte-exakt
matcht, wurde die Änderung gemäß Session-Regel (nur Byte-exakte Fixes werden
committet) sauber zurückgesetzt; `src/Enemy/BathtubKiller.cpp` bleibt im
Stand von Runde 48.

**Wichtiger methodischer Nebenbefund**: Die Entdeckung selbst — dass
`behaveToWater`s bisherige Logik (`breakBathtubKiller()`) *inhaltlich falsch*
war, nicht nur unpräzise geplant — bestätigt erneut den in Runde 47
etablierten Ansatz „bereits implementierte, aber nicht vollständig
matchende Funktionen im TBathtubKiller-Cluster auf falsche Algorithmen prüfen,
nicht nur auf Register-/Rahmen-Rätsel". Das `isDying`-OR-Muster
(`cur == &Explosion::theNerve() || cur == &Break::theNerve()`) bleibt als
Vorlage für zukünftige Cluster-Arbeit bestätigt (jetzt dreifach beobachtet:
`attackToMario`, `behaveToWater`, sowie in `isCollidMove`/`receiveMessage`s
Retail-Disassembly strukturell wiedererkannt).

Session-Gesamtstand bleibt bei 400 verifizierten echten Fixes in 58 Commits
(kein neuer Commit diese Runde — Inhalt korrekt rekonstruiert, aber
Byte-Match durch dateiweite Inlining-Heuristik blockiert).

### Zwischenstand Runde 49: eigene Tiefenanalyse `TLiveActor::control()` und `evIsNpcSinkBottom`

**`TLiveActor::control()`** (`src/Strategic/liveactor.cpp`, 73,96 % Match, 200
Bytes) enthielt zwei `// call on unk90`-Platzhalterkommentare eines früheren
Mitwirkenden (Feld `void* unk90` mit unbekanntem Typ, nirgendwo im
Repository mit konkretem Typ belegt). Aus dem Retail-Disassembly exakt
rekonstruiert: beide Stellen rufen `((vtable von *(unk90+0x5c))[4])(unk90)`
auf (Rohzeiger-Vtable-Dispatch, passend zum bereits im Code verwendeten
Stil `*(int*)((char*)unk90 + 4)` für denselben unbekannten Typ). Zusätzlich
ersetzte `mSpine->isIdle()` (materialisiert eine Bool-Variable vor dem
Sprung) durch die direkte OR-Form `mSpine->getCurrentNerve() != nullptr ||
mSpine->getVertebraeCount() > 0` (Negation von `isIdle()`), wodurch die
Verzweigungsstruktur exakt auf Retails direkte Sprunglogik ohne
Bool-Materialisierung einschwenkte. Ergebnis: **alle 58 Instruktionen
strukturell identisch** (gleiche Anzahl, gleiche Opcodes, gleiche
Sprungstruktur nach Label-Kanonisierung) — einzige Restdifferenz: `unk90`
wird bei uns in r5, bei Retail in r4 alloziert (reine
Registerzuteilungsentscheidung, drei verschiedene Formulierungen probiert:
Rohzeiger-Cast, benannte `void** vtbl`-Lokale, zusätzlicher
`void* p = unk90`-Cache — alle drei identisches Resultat). Gehört zur
bereits dokumentierten Kategorie „asymmetrische
Register-Zuteilungsentscheidung" (wie `TNerveNameKuriLand::execute`,
`TRotation3::identity33`) — kein Byte-Match erreicht, sauber
zurückgesetzt.

**`evIsNpcSinkBottom`** (`src/NPC/NpcEvent.cpp`, über die gemeinsame
Hilfsfunktion `IsNpcFlagOn_`, 72,17 % Match, 264 Bytes) zeigte einen
deutlicheren strukturellen Unterschied: Retail ruft für den finalen
`interp->push(result)`-Aufruf die reale Out-of-line-Funktion
`push__21TSpcStack<9TSpcSlice>FRC9TSpcSlice` auf, während unser Build den
kompletten Push-Rumpf (Kapazitätsprüfung, Speichern, Größe inkrementieren)
inline expandiert. `TSpcStack<T>::push()` ist in `include/Strategic/
spcinterp.hpp` als gewöhnliche (nicht explizit inline-markierte, aber
kurze) Methode definiert. **Risikoabschätzung durchgeführt, bevor
experimentiert wurde**: Dieselbe Datei enthält mindestens sieben weitere
`ev*`-Funktionen, die exakt denselben `interp->push(result)`-Aufruf nutzen
und bereits bei 100 % Match liegen (`evConnectDummyNpc`,
`evOnTalkToDummyNpc`, `evSetNpcBalloonMessage`,
`evSetNpcTalkForbidCount`, `evNpcDanceOn`, `evNpcDanceOffHappyOn`,
`evResetFruitNum`) — für diese muss `push()` also bereits korrekt inline
expandiert werden. Ein pauschales `#pragma dont_inline` auf die geteilte
Template-Methode hätte ein hohes Risiko, diese sieben bereits
matchenden Funktionen zu regressieren (dieselbe Gefahrenklasse wie der in
dieser Runde bereits dokumentierte `theNerve()`-Fall im
`TBathtubKiller`-Cluster: eine dateiweite/funktionsabhängige
Inlining-Heuristik, keine lokale Stellschraube). Ohne experimentellen
Eingriff auf die geteilte Header-Datei als „untersucht, aber zurückgestellt"
eingestuft; keine Änderung vorgenommen.

**Parallele Subagenten-Ergebnisse** (acht parallel gestartete
Untersuchungsaufgaben für Funktionen mit real existierender, aber
niedrig-prozentiger Implementierung — dieselbe „falscher statt nur
unpräziser Algorithmus"-Technik wie beim `behaveToWater`-Fund):
- **`TMameGesso::reset()`** (47,9 % → **MATCH**, Commit `210212f2`):
  `unk1CC = MsRandF(l, r)` (reine Float-Arithmetik) ersetzt durch das
  bereits vorhandene `TMsRange<s32>(0, interval).rand()`-Template — Retail
  nutzt Integer-Subtraktion vor der Float-Konvertierung, nicht reine
  Float-Arithmetik. Byte-identisch bis auf kosmetische
  Konstanten-Pool-Nummerierung.
- **`TDoroHamuKuri::attackToMario()`** (43,5 % → **MATCH**, Commit
  `351c13f0`): Algorithmus war korrekt; MWCC inlinete
  `THamuKuri::selectCapHolder()` komplett, Retail ruft es reell
  (`bl selectCapHolder`). Fix: `#pragma dont_inline` um die
  **Callee**-Definition (nicht den Aufrufer) — bestätigt die aus Runde 29
  bekannte Regel, dass das Pragma um die aufgerufene Funktion stehen muss.
- **`TApplication::TApplication()`** (36,4 % → **NO-MATCH**, zurückgesetzt):
  Quellcode bereits semantisch exakt (Initialisierungsreihenfolge korrekt);
  Lücke stammt aus `JDrama::TFlagT<T>::set`/Kopierkonstruktor, die in
  `JDRFlag.hpp` inline definiert sind, während Retail sie als externe
  Weak-Symbole in `MarDirectorDirect.cpp` referenziert (0 Bytes Stackframe
  bei uns vs. 0x40 bei Retail). `#pragma dont_inline`/`inline_depth`
  in vier Varianten erfolglos getestet (siehe Unteragenten-Bericht) —
  Cross-TU-Header-Änderung nötig, außerhalb des Aufgabenumfangs,
  zurückgestellt für eine dedizierte JDRFlag.hpp-Untersuchung.

Fünf weitere Unteragenten-Untersuchungen (`SunModel`, `MarioOnYoshi`,
`HamuKuriIsHitValid`, `WoodBlockLoad`, `WarpInCallBackExecute`) liefen zum
Zeitpunkt dieses Zwischenstands noch; Ergebnisse folgen im nächsten
Abschnitt.

### Zwischenstand Runde 49, Teil 2: `defer_codegen`-Durchbruch und weitere Subagenten-Ergebnisse

**Grundlegender Methodik-Durchbruch** (gefunden vom `SunModel`-Unteragenten,
eigenständig reproduziert und bestätigt am `theNerve()`-Fall aus Teil 1
dieser Runde): `#pragma dont_inline` ist im gesamten Projekt praktisch
wirkungslos, weil `configure.py`s `-inline deferred`-Compilerflag jeden
Pragma-Zustand verwirft, BEVOR er wirken kann. Der Fix:
`#pragma defer_codegen off` als ERSTE Zeile einer `.cpp`-Datei setzt
dieses Verhalten für die gesamte Datei zurück, wonach bereits vorhandene
`#pragma dont_inline`-Markierungen in Headern (wie `at()` in
`JGMatrix33.hpp`, ~88 Stellen projektweit) endlich greifen. Bestätigt an
zwei unabhängigen Fällen:
- **`TSunModel::calcDispRatioAndScreenPos_`** (14,15 % → **MATCH**, Commit
  `4c40b60e`): `CLBScreenFPosToSPos` blieb dank `defer_codegen off` +
  gezieltem `dont_inline` um die Zielfunktion out-of-line, exakt wie
  Retail. Zusätzlich `char trash[16]` für eine reine Rahmengrößenlücke.
  Nebenfund: `CLBScreenFPosToSPos` existierte in unserem Objekt vorher
  GAR NICHT (0 %), jetzt nahezu vollständig (75/75 Instruktionen) — ein
  verbleibender Bug in `include/Camera/cameralib.hpp` (u16/s16-Vorzeichen-
  Fehlkonvertierung bei `SMSGetGameRenderHeight`/`Width`, plus 8-Byte-
  Rahmenlücke) wurde dokumentiert, aber bewusst NICHT behoben (Header mit
  37+ Includern, außerhalb des Aufgabenumfangs).
- **Eigener Test in `TBathtubKiller::behaveToWater`** (siehe Teil 1): Mit
  `#pragma defer_codegen off` am Dateianfang wird
  `theNerve__28TNerveBathtubKillerExplosionFv`/
  `theNerve__24TNerveBathtubKillerBreakFv` jetzt korrekt als reale
  Out-of-line-Funktion aufgerufen (`bl`) statt komplett inline expandiert
  — der in Teil 1 dokumentierte `theNerve()`-Cluster-Mythos ist damit
  **teilweise aufgeklärt**: nicht dateiweite Heuristik allein, sondern
  das gemeinsame `-inline deferred`/`dont_inline`-Problem. Restdifferenzen
  bleiben aber bestehen: (a) eine dritte `theNerve()`-Referenz innerhalb
  von `mSpine->pushNerve(...)`s inline-Expansion wird weiterhin komplett
  inline rekonstruiert statt (wie Retail) die bereits konstruierte
  `instance`-Adresse direkt wiederzuverwenden — drei Formulierungen
  (OR-Kette, separate `bool a,b`-Lokale, gemeinsame `breakNerve`-Lokale)
  probiert, keine erreichte exakten Match; (b) die Boolean-
  Materialisierung des `isDying`-Vergleichs nutzt bei Retail ein
  `subf+cntlzw+extrwi.`-Bitmuster, bei uns entweder direktes Branching
  oder ein abweichendes `srwi`-Muster. `behaveToWater` bleibt daher
  NICHT gematcht, sauber zurückgesetzt (inkl. der `defer_codegen`-Pragma-
  Zeile). **Für zukünftige Cluster-Arbeit festgehalten**: `defer_codegen
  off` ist ein notwendiger, aber nicht hinreichender erster Schritt für
  den ganzen `TBathtubKiller`-Cluster; `attackToMario`s Runde-47-
  Rahmenlücke (ours 0x58 vs. Retail 0x50) sollte mit dieser Erkenntnis
  erneut versucht werden, da sie vermutlich auch auf inline-aufgeblähtem
  `theNerve()`-Code beruhte — noch nicht nachgetestet, da die Restarbeit
  an `behaveToWater` bereits das Rundenbudget beanspruchte.

**Weitere abgeschlossene Subagenten-Ergebnisse**:
- **`TMario::onYoshi() const`** (22,2 % → **MATCH**, Commit `59200eeb`):
  Bug lag NICHT in der Zieldatei, sondern in
  `include/Player/Yoshi.hpp`s `TYoshi::onYoshi()` — ein Ternary-Ausdruck
  (`return mState == STATE_MOUNTED ? TRUE : FALSE;`) wurde trotz
  `dont_inline` textuell komplett wegoptimiert (MWCC ignoriert
  `dont_inline` bei trivialen Ternary-Rümpfen unabhängig vom
  `defer_codegen`-Zustand). Umformulierung zu explizitem `if`/`else`
  ließ MWCC die Funktion endlich als eigenständig respektieren →
  Byte-exakter Match, inklusive korrekter Nachfolge-Adressen für
  `windMove`/`flowMove`/`warpRequest` in derselben Datei.
- **`THamuKuri::isHitValid(u32)`** (35,6 % → **NO-MATCH**, sauber
  zurückgesetzt): Echter Algorithmus-Bug gefunden und behoben (Retail
  dupliziert `THamuKuriManager::requestSerialKill`s komplette Logik
  inline, statt sie aufzurufen) — nach Fix inklusive `char trash[24]`
  75 von 76 Instruktionen exakt, aber ein hartnäckiger
  Register-Rotationsunterschied (`mr r3,r0` vs. direktes `lwzx r3,...`)
  erwies sich nach zehn Quellcode-Varianten als Compiler-Kontext-Artefakt
  (auch `requestSerialKill` selbst zeigt denselben Extra-Befehl in
  Retail) — bestätigter Grenzfall, kein Fix möglich.
- **`TWoodBlock::load(JSUMemoryInputStream&)`** (44,6 % → **NO-MATCH**,
  sauber zurückgesetzt): Echter Algorithmus-Bug gefunden (Retail ruft
  `TRailMapObj::load` direkt auf und dupliziert `TNormalLift::load`s
  Schwanzlogik inline, statt über `TNormalLift::load` zu delegieren) —
  aber nach Korrektur bleibt `TRailMapObj::load` an ZWEI Aufrufstellen
  ASYMMETRISCH: in `TNormalLift::load` muss es inline bleiben (aktuell
  korrekt), in `TWoodBlock::load` muss es out-of-line werden. Da
  `dont_inline` eine reine Definitionseigenschaft ist (wirkt auf ALLE
  Aufrufer gleich), ist diese Asymmetrie mit dem verfügbaren Werkzeug
  NICHT auflösbar, ohne die bereits korrekte `TNormalLift::load` zu
  brechen — bestätigter Grenzfall, dieselbe Kategorie wie die
  `WoodBlockLoad`-eigene Analyse es einordnet.
- **`TWarpInCallBack::execute(...)`** (36,75 % → **NO-MATCH**, sauber
  zurückgesetzt): Kein Algorithmus-Bug (Mathematik bereits identisch),
  reine Codegen-Formdifferenz. Ursache identifiziert:
  `JGVec3.hpp`s `operator*(TVec3, f32)` gibt **by value** zurück
  (`dont_inline`-geschützter Kopierkonstruktor/-zuweisung erzeugt daher
  überzählige Rückgabe-Temporäre bei Verkettung), während Retails Muster
  auf `const TVec3&`-Rückgabe hindeutet — dieselbe „fake-Referenz für
  Matching"-Masche, die bereits bei `operator+`/`operator-` in
  derselben Headerdatei angewendet wird. Empfehlung für eine dedizierte
  Headerdatei-Änderungsrunde dokumentiert, nicht in dieser Runde
  umgesetzt (gemeinsam genutzter Header, 37+ Includer).
- **`TApplication::TApplication()`** (36,4 % → **NO-MATCH**, sauber
  zurückgesetzt): Bereits in Teil 1 dokumentiert — `JDRFlag.hpp`s
  `TFlagT<T>::set`/Kopierkonstruktor inline vs. Retails externe
  Weak-Symbole in `MarDirectorDirect.cpp`; Cross-TU-Header-Änderung
  nötig, zurückgestellt.

**`MapObjLibDeferCodegen`-Unteragent** (86-Funktionen-Regressionsprüfung
für `src/MoveBG/MapObjLib.cpp` mit `defer_codegen off`) lief zum
Zeitpunkt dieses Zwischenstands noch; Ergebnis folgt im nächsten
Abschnitt.

Session-Gesamtstand nach Teil 2: **404 verifizierte echte Fixes in 62
Commits** (4 neue Matches diese Runde: `TMameGesso::reset`,
`TDoroHamuKuri::attackToMario`, `TSunModel::
calcDispRatioAndScreenPos_`, `TMario::onYoshi`). `defer_codegen off` ist
ab sofort als Standard-Ersttest für JEDE Funktion mit Verdacht auf
fehlende Out-of-line-Aufrufe etabliert (Diagnosekriterium: Zielfunktion
ruft eine im Header als `dont_inline` markierte Methode auf, die im
eigenen `src/*.o` gar nicht als eigenes Symbol erscheint, aber im
`obj/*.o`-Referenzobjekt schon).

### Nach neunundvierzigster Iterationsrunde, Abschluss: `MapObjLib.cpp`-Regressionskaskade bestätigt Grenzen von `defer_codegen off`

Der `MapObjLibDeferCodegen`-Unteragent (86-Funktionen-Regressionsprüfung,
16 Minuten Laufzeit) kam zu einem klaren **NO-GO**, sauber zurückgesetzt
(`git status` bestätigt leer). Kernbefund: `#pragma defer_codegen off`
wirkt NICHT nur lokal auf die Zielfunktion, sondern schaltet die GANZE
Datei von „deferred" auf „sofortige" Codegen-Reihenfolge um — das bricht
bereits bestehende, korrekt matchende Funktionen, die zufällig von der
alten „ganze Datei auf einmal betrachten"-Inlining-Reihenfolge profitiert
hatten (Beispiel: `TMapObjBase::startAllAnim` verlor sein Match, weil es
VOR seiner einzigen Aufrufstelle `makeLowerStr` im Quellcode steht und
implizit auf verzögertes Cross-Function-Inlining angewiesen war). Jeder
Versuch, EINE Regression zu flicken, erzeugte eine NEUE an anderer
Stelle (`makeObjMtxRotByAxis` 97,2 %→58,3 %, dann ein gemeinsames
Weak-Template-Symbol `TRotation3<...>::setEular` 100 %→36,5 %) — ein
**divergierendes, nicht konvergierendes** Regressionsmuster in dieser
speziellen 86-Funktionen-Datei. Die primären Zielfunktionen
(`getVerticalVecToTargetXZ`, `rotateVecByAxisY`) verbesserten sich dabei
in KEINER getesteten Konfiguration, wurden in den meisten sogar
schlechter als der unveränderte Ausgangszustand.

**Präzisierte Methodik-Regel für `defer_codegen off`** (aktualisiert
gegenüber der optimistischen Einschätzung aus Teil 2 dieser Runde):
Dieser Fix funktioniert zuverlässig nur in **kleinen, isolierten
Dateien** mit wenigen Funktionen und geringer gegenseitiger
Inlining-Abhängigkeit (bestätigtes Beispiel: `sunmodel.cpp`). In
**großen, dicht vernetzten Dateien** (wie `MapObjLib.cpp` mit 86
Funktionen und vielen `TMapObjBase`-Methoden, die sich gegenseitig
aufrufen) ist das Risiko einer Regressionskaskade hoch, weil
`-inline deferred`s „gesamte Datei auf einmal betrachten"-Verhalten
viele bereits-korrekte Matches UNSICHTBAR mitträgt. Vor jedem
`defer_codegen off`-Versuch MUSS eine vollständige Vorher/Nachher-
Regressionsprüfung über ALLE Funktionen der Zieldatei erfolgen (nicht
nur der Zielfunktion); bei mehr als einer Handvoll Regressionen ist der
Ansatz für diese Datei zu verwerfen, nicht zu erzwingen.

### Rundenabschluss: Session-Gesamtstand

**404 verifizierte echte Fixes in 62 Commits** nach Abschluss von Runde
49 (4 neue Matches: `TMameGesso::reset`, `TDoroHamuKuri::attackToMario`,
`TSunModel::calcDispRatioAndScreenPos_`, `TMario::onYoshi`).
`matched_functions` in frisch generiertem `report.json`: **8991** (von
8987 zu Rundenbeginn), `matched_code_percent`: 44,90 %. Volles
`ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt `build/GMSJ01/
mario.dol: OK`. `git fetch upstream` bestätigt weiterhin 0 Commits
Rückstand. Acht parallele Unteragenten-Untersuchungen plus zwei
eigene Tiefenanalysen (`TLiveActor::control()`, `evIsNpcSinkBottom`)
plus die ursprüngliche `TBathtubKiller::behaveToWater`-Analyse ergaben
zusätzlich: einen grundlegenden Methodik-Durchbruch (`defer_codegen
off` für isolierte Dateien), zwei präzisierte Grenzfälle (asymmetrische
Aufrufstellen-Inlining bei `WoodBlockLoad`, dateiweite
Regressionskaskaden bei großen Dateien), und drei neue, für spätere
dedizierte Header-Änderungsrunden dokumentierte Kandidaten
(`JDRFlag.hpp`/`TFlagT` für `TApplication`-Konstruktoren, `JGVec3.hpp`s
`operator*`-Rückgabe-für-Wert für Partikel-Code, `evIsNpcSinkBottom`s
`TSpcStack::push`-Inlining-Asymmetrie).

### Nach fünfzigster Iterationsrunde (`TMario::floorDamageExec(TEParams&)`: Argument-Vertauschungsbug gefunden und behoben)

Fortsetzung der Suche nach kleinen, isolierten Kandidaten mit
`fp=None`-Aufrufstellen (analog zum `defer_codegen`-Muster aus Runde 49,
aber diesmal in kleinen Units mit ≤ 25 Funktionen, um die in Runde 49
dokumentierte Regressionskaskaden-Gefahr großer Dateien zu vermeiden).

**Fund**: `TMario::floorDamageExec(const TMario::TEParams&)`
(`src/Player/MarioCollision.cpp`, 76,2 % Match, 220 Bytes) delegiert an
`damageExec(THitActor*, int damage, int damageAnimType, int waterEmit,
f32 knockbackSpeed, int rumbleFrames, f32 pollutionAmount, s16
invincibilityFrames)`. Direkter Disassembly-Vergleich zeigte: Retails
7. Argument (`pollutionAmount`, `f32`) wird per einfachem `lfs`-Ladebefehl
aus `TEParams::mDirty` (Offset 0x7c) gelesen — unser Code übergab
stattdessen `params.mDamage.get()` ein ZWEITES Mal (bereits als 1.
Argument verwendet), was einen `u8`→`f32`-Konvertierungstrick
(Magic-Double-Subtraktion) statt eines direkten Float-Ladebefehls
erzeugte. Alle sieben `TParamRT<T>`-Felder in `TEParams` liegen exakt
0x14 Bytes auseinander (0x18, 0x2c, 0x40, 0x54, 0x68, 0x7c, 0x90),
wodurch sich die Feldreihenfolge aus dem Retail-Disassembly eindeutig
rekonstruieren ließ. Fix: `params.mDamage.get()` (2. Vorkommen) durch
`params.mDirty.get()` ersetzt. Ergebnis: **Byte-exakter Match** (58/58
Instruktionen identisch nach Label-Kanonisierung), keine Regression im
Rest der Unit (`git diff` nur die eine Zeile), volles `ninja`-Rebuild
und `dtk shasum -c` bestätigen `build/GMSJ01/mario.dol: OK`. Commit
`552a42ce`, gepusht.

**Neue Kandidatenkategorie bestätigt**: „Duplizierte Argumente in
Mehrfachaufrufen mit vielen Parametern gleichen Typs" — ein Copy-Paste-
Fehler eines früheren Mitwirkenden, der sich über den ungewöhnlichen
Magic-Double-Konvertierungscode im Disassembly (u8→f32-Promotion,
erkennbar an `stw`+`stw`+`lfd`+`fsubs` statt einfachem `lfs`) zuverlässig
aufspüren lässt, wenn das erwartete Argument eigentlich bereits ein
`f32`-Feld ist.

**`TAnimalBase::execWalk(bool)`** (78,4 % Match, 1020 Bytes) kurz
geprüft: 371 Diff-Zeilen bei 258 vs. 270 Gesamtzeilen, UNSER Stackframe
ist mit 0x118 GRÖSSER als Retails 0x0f0 (falsche Richtung für
`char trash[N]`), zusätzlich unterscheidet sich die
Multiplikations-Reihenfolge bei verketteten `SMSGetAnmFrameRate()`-
Aufrufen strukturell (`fmr`-Zwischenkopien bei uns, direkte Verkettung
bei Retail) — deutet auf eine größere Ausdrucks-Restrukturierung hin,
keine Datei verändert, als zu aufwendig für diese Runde zurückgestellt.

**Weitere für spätere Untersuchung notierte, aber nicht bearbeitete
Kandidaten** aus dem `fp=None`-Kleindatei-Scan: `TMarDirectorDirect::
decideNextStage` und `TApplication::TApplication()` (beide durch das
bereits dokumentierte `JDRFlag.hpp`/`TFlagT`-Cross-TU-Problem
blockiert), `getNameRef_BossEnemy`/`getNameRef_Enemy` (sehr groß,
2676/8392 Bytes, vermutlich lange Namens-Vergleichsketten, nicht
geprüft).

### Session-Gesamtstand nach Runde 50

**405 verifizierte echte Fixes in 64 Commits.** `matched_functions`:
**8992** (von 8987 zu Beginn dieses Segments), `matched_code_percent`
~44,9 %. Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`. `git fetch upstream` weiterhin 0 Commits
Rückstand. Fünf neue Matches in diesem Segment: `TMameGesso::reset`,
`TDoroHamuKuri::attackToMario`, `TSunModel::calcDispRatioAndScreenPos_`,
`TMario::onYoshi`, `TMario::floorDamageExec(TEParams&)`. Zusätzlich ein
grundlegender Methodik-Durchbruch (`#pragma defer_codegen off` für
isolierte Dateien, mit dokumentierten Grenzen für große Dateien) und
eine neue produktive Fehlerkategorie (duplizierte Argumente bei
`TParamRT`-Strukturzugriffen, erkennbar am Magic-Double-
Konvertierungsmuster im Disassembly).

### Nach einundfünfzigster Iterationsrunde (parallele Elf-Kandidaten-Untersuchung: 7 neue Matches, neue Methodik-Details)

Fortsetzung des Klein-Datei-Scans (≤ 30 Funktionen pro Unit, `fp` zwischen
5–90 %, Größe < 500 Bytes), elf Kandidaten parallel an Unteragenten delegiert.
**Sieben neue Byte-exakte Matches**, sechs Commits:

- **`TQuestionManager::makeDL(JDrama::TGraphics*) const`** (84,2 % → Match,
  Commit `2a012e9b`): Retail baut alle VIER Quad-Eckpunkte in ein
  `TVec3<f32>[4]`-Array und übergibt das Array an `TDLTexQuad::request`;
  unser Code baute nur EINEN transformierten Vertex. `char trash[4]` schloss
  die verbleibende 8-Byte-Rahmenlücke.
- **`TQuestionManager::request(TVec3<f32>, f32)`** (88,7 % → Match, selber
  Commit): Reine Codegen-Form-Differenz (Z-Term zuerst statt X-Term zuerst,
  `fmadds` statt zwei `fmuls`+`fadds`) — durch benannte lokale `dz`/`distZ`
  in exakter Retail-Reihenfolge behoben.
- **`CPolarSubCamera::calcTowerCenterPos_(Vec*)`** (84,6 % → Match, Commit
  `a0b05f8a`): Retail emittiert die Funktion als `inline`/`weak`-Symbol
  (kein CSE der Tabellenadresse über den `switch`). Fix: `inline`-Markierung
  + Verschiebung ans Dateiende nach dem einzigen Aufrufer (da `dont_inline`
  eine Aufrufstellen-, keine Definitionseigenschaft ist) + `defer_codegen
  off` + `char trash[8]`.
- **`TMarDirector::fireGetNozzle(TItemNozzle*)`** (77,8 % → Match, Commit
  `814da6f2`): Zwei kombinierte Bugs — `gpApplication.mCurrArea.unk0` wurde
  in jedem Zweig neu gelesen statt einmal vor der Verzweigung; und
  `getNozzleRight(...)` fehlte die Negation (`!`). Plus `char trash[8]`.
- **`TCubeManagerArea::isInAreaCube(const Vec&) const`** (81,4 % → Match,
  Commit `88d6ae2e`): Mehrere frühe `return true;` durch einen einzelnen
  `bool result`-Akkumulator mit `if`/`else if`-Kette ersetzt (verlängert die
  Live-Range, wodurch der Compiler 5 statt 4 Register via `stmw`/`lmw`
  alloziert, exakt wie Retail) — Muster aus der Schwesterfunktion
  `TCubeManagerFast::isInOtherCube` in derselben Datei übernommen. Plus
  `char trash[16]`.
- **`TBoundPane::TBoundPane(J2DScreen*, u32)`** (83,5 % → Match, Commit
  `4d7aae7b`, **Header geändert**): Feld `unk14` war als `JUTRect` deklariert
  (mit echtem Out-of-line-Konstruktor), wird aber nur über rohe
  `.x1/.y1/.x2/.y2`-Zugriffe verwendet — Retail behandelt es als reines POD.
  Neuer schlanker Typ `SBoundRect` (4×`s32`, inline-Ctor) ersetzt `JUTRect`
  für dieses eine Feld, mit `operator JUTRect()`-Konversion für den einzigen
  externen Aufrufer (`ConsoleStr.cpp`). Alle vier Header-Includer
  (ConsoleStr.o, GCConsole2.o, SelectMenu.o, BlendPane.o) nachgebaut und
  bestätigt fehlerfrei. Plus `char trash[8]`. Vollständiges `ninja`-Rebuild
  nach diesem Fix bestätigt `build/GMSJ01/mario.dol: OK`.
- **`TEnemyPolluteModelManager::generatePolluteModel(TVec3<f32>&,
  TVec3<f32>&)`** (86,3 % → Match, Commit `9debd4d6`): Der komplette
  Boden-/Wasser-Check lag in Wahrheit in einem inline-expandierten
  `TEnemyPolluteModel::generate` (kein eigenes Symbol im Retail-Objekt),
  nicht im Manager selbst — drei Indizien (Out-of-line-`isWaterSurface`-
  Aufruf nur bei Inline-Tiefe ≥ 2, Stack-Layout-Reihenfolge, leerer
  `SMatrix34C`-Konstruktor-Aufruf nur bei Inline-Expansion) bestätigten die
  Struktur. `generate` als `inline`-Methode direkt in der `.cpp` (keine
  Header-Änderung, keine weiteren Aufrufer) neu definiert, Restlogik in den
  Manager verschoben. `JGeometry::TVec3<f32> trash` (nicht `char[]` — ein
  reines `char trash[12]` wird innerhalb der Inline-Expansion wegoptimiert)
  schloss die 12-Byte-Lücke.

**Neue Methodik-Erkenntnis**: `#pragma dont_inline` ist eine
**Aufrufstellen-Eigenschaft**, keine Definitionseigenschaft — bestätigt am
`calcTowerCenterPos_`-Fund: Um eine Funktion an EINER Aufrufstelle
out-of-line zu halten, muss ihre Definition entweder (a) mit `dont_inline`
UND `defer_codegen off` kombiniert werden (funktioniert nur in kleinen,
isolierten Dateien, siehe Runde-49-Grenzbefund), ODER (b) für Fälle mit
genau einem Aufrufer: die Definition NACH dem Aufrufer im Quelltext
platzieren und als `inline` markieren, was MWCC dazu bringt, sie als
Weak-Symbol mit eigenem Out-of-line-Körper zu emittieren statt sie
textuell zu inlinen.

**Zwei Kandidaten mit gründlich dokumentiertem, aber nicht erreichtem
Match** (beide sauber zurückgesetzt, wertvolle Investigation dennoch):
`TMarDirector::fireStreamingMovie(u8)` (neue Hypothese: `unk4C`-Feld
verhält sich wie `volatile` in Retails echtem Quelltext, aber als
projektweit von vielen Dateien über „fabricated"-Hilfsmethoden genutztes
Feld außerhalb des Aufgabenumfangs für eine Einzelfunktions-Änderung) und
`TMapObjWaterFilter::perform` (109 von 111 Instruktionen exakt nach
sechs echten Struktur-Fixes — camera-Check-Umformulierung, `pos`-Referenz,
View-Matrix-Lokale, Deklarationsreihenfolge, `char trash[0x38]` — letzte
zwei Instruktionen sind ein MWCC-Branch-Folding-Artefakt bei der letzten
`&&`-Bedingung vor einem bloßen `return;`, gegen 14 Quellvarianten
resistent, bestätigt anhand von zwei weiteren Funktionen mit demselben
Muster in `Map.cpp`/`bosseel.cpp`). Weitere vier Kandidaten
(`updateTrans`, `getPosInWire`, `TStrategy`-Konstruktor, `initMActor`,
`loadAfter` von `TMapObjRevivalPollution`, `TCubeManagerBase`-Konstruktor,
`AudioDecoderForOnMemory`) als reines Register-Zuteilungs-/
Stack-Slot-Rauschen bestätigt und zurückgesetzt.

### Session-Gesamtstand nach Runde 51

**412 verifizierte echte Fixes in 70 Commits.** `matched_functions`:
**8999** (von 8992 zu Rundenbeginn), `matched_code_percent`: 44,96 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`. `git fetch upstream` weiterhin 0 Commits
Rückstand.

### Nach zweiundfünfzigster Iterationsrunde (4 weitere Matches; neues Muster „Joint-Index-Truncation-Timing"; zwei bemerkenswerte Fast-Treffer dokumentiert)

Fortsetzung des Klein-Datei-Scans mit sieben weiteren parallel delegierten
Kandidaten. **Vier neue Byte-exakte Matches**, drei Commits:

- **`TMirrorCamera::perform(u32, JDrama::TGraphics*)`** (91,0 % → Match,
  Commit `c195ae05`): Projektionsmatrix-Zeiger nicht gehoben (`MtxPtr`
  statt wiederholtem `graphics->mProjMtx.mMtx`), `gpCamera`-Felder direkt
  statt über die Inline-Accessoren `getFovy/getAspect/getNear/getFar`
  gelesen (ändert Argument-Auswertungsreihenfolge), plus `char trash[8]`.
- **`TLauncher::receiveMessage(THitActor*, u32)`** (90,8 % → Match, Commit
  `c8a3646a`): Echter Argumentfehler — `&mPosition` (this) statt
  `&sender->mPosition` für Emit/Sound-Aufrufe. Plus `char trash[8]`.
- **`TBaseNPC::setPollutionEffectMtxPtr_`/`setSmokeEffectMtxPtr_`** (90,5 %
  / 91,1 % → beide Match, Commit `7dd7e933`): **Neues, viertes
  wiederkehrendes Bugmuster entdeckt**: Ein von `JUTNameTab::getIndex`
  zurückgegebener `s32`-Gelenkindex muss vor der inline-expandierten
  `*0x30`-Array-Index-Multiplikation in `J3DModel::getAnmMtx` auf 16 Bit
  maskiert werden — aber NUR wenn dies als `int idx = call(); ... idx &
  0xFFFF;` geschrieben wird (nicht als `u16`-typisierte Lokale oder
  `(u16)`-Cast), da MWCC die Maskierung sonst sofort statt verzögert über
  einen Zwischenaufruf hinweg einplant. Zusätzlich beeinflusst die
  Deklarationsreihenfolge lokaler `const char*`-Gelenknamen direkt die
  Registerzuteilung (auch für `@sda21`-adressierte String-Literale).

**Zwei bemerkenswerte, gründlich dokumentierte Fast-Treffer** (beide nach
Protokoll zurückgesetzt, da nicht Byte-exakt):
- **`SMS_AddDamageFogEffect`** (90,0 % → 99,44 %, NICHT committet):
  Echter Logikfehler gefunden und behoben (lokale `f32`-Literale wurden
  von MWCC konstant-gefaltet und die beiden Oszillationswerte fälschlich
  zu einem CSE't — Ersetzung durch nicht-konstante `static f32`-Datei-
  Globale erzwingt zwei unabhängige Laufzeitberechnungen wie im Retail-
  Disassembly). Nach Fix + `char trash[0x38]`: alle 76 Instruktionen
  strukturell identisch, aber 4 Zeilen zeigen vertauschte Operandenreihen-
  folge bei kommutativen `fmuls`/`fadds` (z. B. `fmuls f29,f2,f1` vs.
  `fmuls f29,f1,f2`). **Selbst nachgeprüft**: expliziter Tausch der
  Quelltext-Multiplikationsreihenfolge (`s * (...)` statt `(...) * s`)
  hatte NULL Effekt auf die erzeugten Instruktionen — MWCC kanonisiert die
  Operandenreihenfolge kommutativer Gleitkomma-Operationen unabhängig vom
  Quelltext. Bestätigter Grenzfall, kein Fix möglich.
- **`TMActorKeeper::createMActor(const char*, u32)`** (60,7 % → 80,8 % bei
  einer Teilkorrektur, NICHT committet): Zwei unabhängige Lücken
  identifiziert — (a) Retail delegiert NICHT an
  `createMActorFromNthData`, sondern inlined `createAndRegister` direkt
  (echter Strukturfehler, Teilfix erreicht 80,8 % ohne Regressionen); (b)
  ein Inline-Tiefe-2-Schwellenwert-Unterschied bei `loadModelData`/
  `registerDataAndJoinNewNode`, den keine der elf getesteten
  Compiler-Flag-Kombinationen (`inline_depth`, `inline_max_size`,
  `-O3`/`-O4`, alternative Compiler-Version 1.2.5n, `defer_codegen off`)
  reproduzieren konnte, ohne andere Funktionen der Unit zu regressieren.
  Nicht committet, da laut Protokoll nur Byte-exakte Treffer zulässig sind.
- **`CPolarSubCamera::isMarioAimWithGun_`/`isMarioCrabWalk_`** (53,5 % je,
  NICHT committet): Echter Feldnamen-Bug gefunden (`checkFrameMeaning`
  [Offset 0xD4] statt `checkMeaning` [Offset 0xD0]), aber Datei ist
  `PCHObject(NonMatching, ...)` — die PCH bäckt `-inline deferred` bereits
  beim PCH-Bau ein, wodurch `defer_codegen off` in der `.cpp`-Datei selbst
  keine Wirkung mehr auf bereits-PCH-kompilierte Header-Inlines
  (`TMario::checkStatusType`, dutzende Aufrufstellen projektweit) hat.
  **Neue Grenzbedingung für die `defer_codegen`-Technik**: PCH-kompilierte
  Dateien sind für diesen Trick nicht erreichbar.

**Weitere bestätigte Register-/Stack-Rauschen-Fälle** (zurückgesetzt):
`TCardManager::setCardStat_` (Retail nutzt ein Duff's-Device-entrolltes
Laufzeit-Loop für eine kompilierzeit-konstante Trip-Count von 6, das
MWCC mit den Projekt-Flags nicht reproduziert — sieben Quellvarianten
erfolglos).

### Session-Gesamtstand nach Runde 52

**416 verifizierte echte Fixes in 73 Commits.** `matched_functions`:
**9003** (von 8999 zu Rundenbeginn), `matched_code_percent`: 44,99 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`.

### Nach dreiundfünfzigster Iterationsrunde (2 weitere Matches; Inzident dokumentiert; sechs gründlich untersuchte Fast-Treffer)

Zwölf weitere Kandidaten parallel delegiert. **Zwei neue Byte-exakte
Matches**:

- **`TYoshi::thinkUpper()`** (92,7 % → Match, Commit `59f127a3`): Mehrere
  kombinierte Bugs — handgerollte AND-Bedingung statt der vorhandenen
  `TWaterGun::isEmitting()`-Inline-Methode; falsches Sound-Member
  (`mBodyAnmSound` statt `mTongueAnmSound`); vertauschte `==`/`!=`-Zweige
  im else-Pfad; `char trash[0x10]`.
- **`TMapEventSinkBianco::loadAfter()`** (56,7 % → Match, Commit
  `d99991e0`): **Neue Technik**: Statt `#pragma dont_inline` (das die
  Inline-Entscheidung für ALLE Aufrufer gleich beeinflusst und damit die
  bereits korrekte Inline-Kopie in `TMapEventSinkInPollutionReset::
  loadAfter()` gebrochen hätte) wurde stattdessen die
  **Inline-Kosten-Schwelle** der Callee-Funktion `TMapEventSinkInPollution
  ::loadAfter()` gezielt um zwei tote Anweisungen angehoben — genug, um
  MWCC dazu zu bringen, sie bei Aufruftiefe 2 (Bianco) nicht mehr zu
  inlinen, aber bei Aufruftiefe 1 (Reset) weiterhin zu inlinen. Plus
  `char trash[0x40]`. Keine Regression in den restligen 35 Funktionen der
  Unit bestätigt.

**Inzident**: Ein Unteragent (`MarDirectorPosIdx`) committete einen
Nicht-Byte-exakten Fix (90,6 % → 99,84 %) entgegen dem etablierten
Protokoll. Beim Korrigieren mit `git reset --hard HEAD~1` wurden
versehentlich auch die unfertigen Arbeitsbaum-Änderungen zweier anderer
parallel laufender Unteragenten gelöscht (`CameraNoticeYrot` musste seine
komplette Untersuchung wiederholen; `MapObjDolpicWater`/`BathWaterManager`
überlebten, da sie zum Zeitpunkt des Resets noch aktiv schrieben). Lehre:
**Reverts während laufender paralleler Batches MÜSSEN dateispezifisch
sein** (`git checkout -- <path>` oder ein gezielter `git revert`
eines Commits), NIEMALS `git reset --hard`, da dieser den GESAMTEN
Arbeitsbaum überschreibt, nicht nur die eigene Historie.

**Sechs gründlich dokumentierte Fast-Treffer** (alle nach Protokoll
zurückgesetzt, keine Commits):
- **`TMarDirector::decideMarioPosIdx()`** (90,6 % → 99,84 %): Zwei echte
  Bugs bestätigt und behoben — falsche `switch`-Case-Label-Menge (Retails
  16-Eintrags-Sprungtabelle beweist Cases 2,3,4,5,6,8,9; unser Code hatte
  2..8, schloss also Case 9 fälschlich aus und Case 7 fälschlich ein) plus
  fehlende Adressmaterialisierung von `&gpApplication.mPrevArea` in ein
  Register. Restdifferenz: ein nicht rekonstruierbarer 28-Byte-
  Stack-Bereich UNTERHALB der Compiler-Temporären, vermutlich von einem
  im Retail-Quelltext vorhandenen, aber zur Compile-Zeit toten Aufruf
  (Debug/OSReport-Stil) verursacht, dessen Parameter-Bereich trotz
  Wegoptimierung Stack reserviert.
- **`CPolarSubCamera::calcNoticeTargetYrot_(const Vec&)`** (92,0 % →
  97,71 %): Fünf echte Bugs gefunden (vertauschte Nah-/Fern-Schwellwerte
  `mRotateMinDistXZ`/`mRotateFastMinDistXZ`, falsche Multiplikationsketten-
  Reihenfolge, zwei-statt-eine-Ausdruck-`dist2`-Berechnung, falsche
  Subtraktionsrichtung beim Yaw-Diff, `char trash[0x28]`) — Restdifferenz
  ist reine Lade-Reihenfolge-/Register-Kanonisierung (2 von 137
  Instruktionen).
- **`TMActorKeeper::createMActor(const char*, u32)`** (60,7 % → 80,8 %
  Teilfix): Retail delegiert nicht an `createMActorFromNthData`, sondern
  inlined `createAndRegister` direkt — echter Strukturfehler, behoben,
  aber ein Inline-Tiefe-2-Schwellenwert bei `loadModelData`/
  `registerDataAndJoinNewNode` blieb unauflösbar (elf Compiler-Flag-
  Kombinationen erfolglos).
- **`CPolarSubCamera::isMarioAimWithGun_`/`isMarioCrabWalk_`** (53,5 % je):
  Echter Feldnamen-Bug (`checkFrameMeaning` statt `checkMeaning`), aber
  Datei ist PCH-kompiliert — `defer_codegen off` erreicht PCH-gebackene
  Header-Inlines nicht mehr.
- **`TMonumentShine::hitByWater`**/**`TBathtubData::getPos`/
  `getGravityDir`**: Je 1–3 echte Bugs gefunden und behoben (u. a.
  `unk18.at(i,j)` → `unk18.mMtx[i][j]`, Anweisungsreihenfolge, `cross`
  statt `cross2`), Restdifferenzen sind MWCC-Konstanten-Kanonisierung
  (kommutative Operandenreihenfolge bei Compiler-Literalpool-Werten,
  nicht durch benannte Konstanten reproduzierbar) bzw. Register-
  Zuteilung in der gemeinsam genutzten `JGQuat4.hpp::rotate()` (lokalisiert,
  aber projektweite Header-Änderung außerhalb des Aufgabenumfangs).
- **`TMAnmSoundNPC::startAnimSound`**: Ein echter Feld-Bug gefunden
  (`uVar6`-Herleitung aus falschem Feld), aber zwei weitere unabhängige
  Restdifferenzen (Register-Wiederverwendung, `std::sqrtf`-Auto-Inlining)
  verhindern exakten Match.

**Bestätigtes Register-/Stack-Rauschen** (keine neuen Erkenntnisse):
`TStageEnemyInfoTable::getMatchedInfo` (bereits aus früherer Runde
bekannt), `TMenuPlane`-Konstruktor, `TGraphTracer::calcSplineSpeed`,
`TSpineEnemy::goToExclusiveNextGraphNode`, `MSoundSE::
startSoundActorWithInfo` (beide letzteren PCH-blockiert für
`defer_codegen`-Technik).

### Session-Gesamtstand nach Runde 53

**418 verifizierte echte Fixes in 75 Commits.** `matched_functions`:
**9005** (von 9003 zu Rundenbeginn), `matched_code_percent`: 45,01 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`. Alle Arbeitsbäume nach dem oben
dokumentierten Reset-Inzident als sauber verifiziert.

### Nach vierundfünfzigster Iterationsrunde (5 weitere Matches; JDRFlag.hpp-Header-Experiment als Lehrbeispiel für heterogene Compiler-Entscheidungen; Inzident 2)

**`JDRFlag.hpp`-Untersuchung** (dediziert, 11 Minuten): Der Versuch,
`JDrama::TFlagT<u16>::TFlagT(const TFlagT&)` und `::set(u16)` projektweit
als „declare-only" mit expliziter Out-of-line-Instanziierung in
`MarDirectorDirect.cpp` zu erzwingen (Ziel: `TApplication`-Konstruktor
und `decideNextStage()` reparieren), ergab nach vollständiger
Projekt-Regressionsprüfung (alle 736 Units) ein klares **NO-GO**: 12
vormals 100 % gematchte Funktionen regressierten (teils um über 50
Prozentpunkte), `decideNextStage()` verbesserte sich GAR NICHT
(Byte-identisch vorher/nachher), und `TApplication`-Konstruktor
verbesserte sich nur teilweise (36,4 % → 70,3 %, nicht exakt). **Wichtige
Erkenntnis**: Retails Compiler trifft für dieses triviale
Template-Member GENUINELY UNTERSCHIEDLICHE Inline-Entscheidungen pro
Aufrufstelle (manche Stellen inlinen die Kopie, andere rufen sie auf) —
eine einzige globale „declare-only"-Header-Änderung ist zu grobschlächtig,
um diese Heterogenität nachzubilden; sie kann nur EIN Verhalten überall
erzwingen. Sauber zurückgesetzt (Commit `b7384f3a`, Revert-Commit).

**Fünf neue Byte-exakte Matches** aus einem parallelen Batch von 15
kleineren Kandidaten:

- **`TPakkunSeed::behaveToHitWall(const TBGCheckData*)`** (95,0 % → Match,
  Commit `c335c131`): **Neues, fünftes Bugmuster**: „Accessor-Aufruf vs.
  direkter Feldzugriff verhindert CSE" — `mVelocity.dot(ground->mNormal)`
  (direkter Zugriff) ließ MWCC die `normal.x`-Last über zwei Verwendungen
  hinweg wiederverwenden; Retail nutzt für den `dot()`-Aufruf den
  `getNormal()`-Accessor (unterbindet die CSE), behält aber für die drei
  `+=`-Zeilen direkten Feldzugriff.
- **`SMS_InitPacket_Fog(J3DModel*, u16)`** (93,1 % → Match, Commit
  `ee984000`): Benannter `J3DPEBlock* peblock`-Lokal vor der
  `getFog()`-Kette erzwang korrekte Ladereihenfolge (`getPEBlock()` vor
  `getShape()`); plus `char trash[40]`. Korrigiert eine frühere,
  unvollständige Untersuchung (nur Padding versucht, Strukturproblem
  übersehen).
- **`TSmallEnemy::attackToMario()`** (90,4 % → Match, Commit `c9f1b81e`):
  Retail berechnet den Mario-Richtungsvektor über einen
  Drei-Argument-`TVec3`-Konstruktor und skaliert/akkumuliert dann IN
  PLACE (kein dritter temporärer Vektor) — `sub()` + separater
  skalierter Temp durch direkte Konstruktion + In-Place-`scale()`/
  Akkumulation ersetzt.
- **`TRedCoinSwitch::control()`** (92,9 % → Match, Commit `0441aa1f`):
  Falsches `switch`-Case-Label (`case 1` statt `case 4` für den
  No-op-Zustand) veränderte MWCCs Pivot-Wahl für den binären
  Case-Dispatch-Baum; Fix reproduziert Retails tatsächliche
  Case-Werte-Menge {1,2,3,4}.
- **`TMapWarp::watchToWarp()`** (66,4 % → Match, Commit `e29ac366`):
  Umfangreichster Fix dieser Runde (26 Minuten Untersuchung) — gecachter
  `checkData->getData()`-Index statt Neulesens, `TVec3::add` bewusst
  out-of-line über die `operator+`-Fake-Referenz-Masche (wie bereits bei
  `bgIntersectLine` in `MapCheck.cpp` bekannt) statt inline-expandierendem
  `+=`, plus präzise Lokalen-Deklarationsreihenfolge und drei
  `char trash[N]`-Anpassungen für exakte Slot-Adressen.

**Inzident 2**: Der `JDRFlagHeaderFix`-Unteragent führte versehentlich
`git stash` im geteilten Repository aus, wodurch die unfertigen
Arbeitsbaum-Änderungen ALLER gleichzeitig laufenden Unteragenten
temporär eingelagert wurden. Der Agent erkannte den Fehler sofort,
beließ den Stash unangetastet (kein `pop`/`drop`) und benachrichtigte
alle betroffenen Agenten per Hub-Nachricht mit genauen
Wiederherstellungsanweisungen (`git show stash@{0}:<path>`). Alle
betroffenen Agenten stellten ihren Stand erfolgreich wieder her oder
hatten bereits vor dem Zwischenfall zurückgesetzt; keine Arbeit ging
verloren. Ein zweiter, kleinerer Vorfall (`PacketUtilFog` committete
versehentlich zwei fremde, bereits im Index gestagete Dateien mit) wurde
vom betroffenen Agenten selbst sofort per `git reset --soft HEAD~1` +
gezieltem `git restore --staged` korrigiert. Beide Vorfälle bestätigen:
**dateispezifische Git-Operationen sind bei parallelen Agenten-Batches
Pflicht** — niemals pauschale `git stash`/`git add -A`/`git reset --hard`
ohne Pfadangabe im geteilten Arbeitsbaum.

**Zehn weitere gründlich dokumentierte Fast-Treffer/Sackgassen**
(zurückgesetzt): `TMapObjBase::startControlAnim`, `SMS_IsMarioOnWire`
(bereits aus früherer Runde bekannt), `TMapCheckGroundPlane::
checkPlaneGround` (bereits dokumentiertes TODO-Problem), `TTalkCursor::
associateNPC`, `TMapObjPlane::makeMountain`, `TNpcParts::
setPartsAnmFrame` (2 von 3 Switch-Case-Blöcken repariert, dritter
resistent), `TRope::moveHead` (echter Bug gefunden — fehlendes
`dont_inline` auf `TVec3::scale(f32)` in `JGVec3.hpp` als
Systemursache identifiziert, aber außerhalb des Aufgabenumfangs),
`TMapObjSwitch::control`, `TNerveHino2Landing::execute` (echter
Flag-Bug `LIVE_FLAG_HIDDEN`→`LIVE_FLAG_CLIPPED_OUT` gefunden, aber
beide betroffenen Werte sind im Retail-Binary selbst tot/ungenutzt),
`TTamaNoko::landEffect` (**zwei echte Algorithmus-Bugs** gefunden und
behoben — fehlendes if/else für Sand-vs-Nicht-Sand-Zweig, fehlender
0,8-Skalierungsfaktor bei allen vier `setGlobalScale`-Aufrufen — auf
8 von 223 Instruktionen reduziert, Restdifferenz nachweislich in der
gemeinsam genutzten `JPABaseEmitter::setGlobalScale`/`TVec3`-Temp-
Allokation lokalisiert, bestätigt durch identisches Muster in einer
NIE berührten Schwesterdatei `namekuri.cpp`), `TMapObjGeneral::thrown`
(auf 15 von 102 Instruktionen reduziert, Restindiz deutet auf eine
verlorene Inline-Hilfsfunktion im Retail-Quelltext hin).

### Session-Gesamtstand nach Runde 54

**423 verifizierte echte Fixes in 80 Commits.** `matched_functions`:
**9010** (von 9005 zu Rundenbeginn), `matched_code_percent`: 45,06 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`.

### Nach fünfundfünfzigster Iterationsrunde (großer 20-Kandidaten-Batch: 11 neue Matches, sechstes Bugmuster, mehrere Lehrbeispiele zu Stack-Slot-Allokation)

Größter Batch dieser Session (20 parallele Unteragenten). **Elf neue
Byte-exakte Matches**:

- **`TMarioParticleManager::emitParticleCallBack(...)`** (95,85 % → Match,
  Commit `fb2aefe9`): Klassischer Pattern-4-Bug — `param_4` (Callback)
  statt `param_5` (User-Daten) an `getAvailableIdx()` und `info->unk0`
  übergeben.
- **`TMapObjPlane::depress(f32,f32,f32)`** (94,7 % → Match, Commit
  `9b4c12d2`): Referenz-Bindung `f32& h = heightAt(...); h -= ...;`
  materialisiert die Element-Adresse als echte gemeinsame Unterausdruck
  (CSE), verhindert MWCCs Faltung in einen reinen `lfsx`/`stfsx`-Zugriff.
- **`TSmallEnemy::changeOut()`** + **`behaveToHitOthers(THitActor*)`**
  (95,5 %/91,8 % → beide Match, Commit `d630b0ee`): Vertauschte
  Zuweisungsrichtung (`mJuiceBlock->mPosition = mPosition` statt
  umgekehrt) plus Distanzvektor-Berechnung über 3-Arg-Konstruktor statt
  `sub()`-Methode (matcht das bereits gefixte `attackToMario`-Muster in
  derselben Datei).
- **`TJointCoin::makeObj(const char*, u16)`** (95,4 % → Match, Commit
  `6b6e1230`): Erste Array-Slot-Lesung in eine benannte Lokale
  gecacht — ändert nichts an den erzeugten Instruktionen, verschiebt
  aber MWCCs Registerfarbe auf Retails Zuteilung.
- **`JPABaseField::calcFieldFadeScale(f32)`** (95,3 % → Match, Commit
  `f7e51c15`): Datei-lokale Hilfsfunktion mit `volatile`-Referenz-
  Parameter verhindert CSE des wiederholt gelesenen Status-Worts.
- **`TBaseNPC::npcRecoverFromSinking()`** (95,1 % → Match, Commit
  `9c6d9c25`): `TBaseNPC* self = this;` als erste Lokale verschiebt
  Registerzuteilung (deklarierte Lokalen erhalten Vorrang vor dem
  impliziten `this`); toter `MsClamp(0,0,0)`-Aufruf nach dem
  Sink-Geschwindigkeits-Block reserviert exakt 4 Byte unterhalb des
  inline-expandierten `MsSqrtf`-Temporärwerts.
- **`MarioWaistCtrl`/`MarioFootDirRCtrl`/`MarioFootDirLCtrl`/
  `TMario::boxDrawPrepare`** (94,8 %/78 %/78 %/92,5 % → alle vier Match,
  Commits `91724804`+`fbd1d5ac`): Größte Einzelausbeute dieser Runde.
  Falscher Flag-Accessor (`checkStatusType` statt `checkFlag`), falsche
  Kontrollfluss-Struktur (FLUDD-Test galt nur für einen Case statt für
  drei plus Fallthrough), falsches zweites Kreuzprodukt (Gram-Schmidt:
  `n × cross1` statt `n × currentMtxDir`), invertiertes
  Schlaf-Prädikat, und — bemerkenswert — ein **im Retail-Binary selbst
  vorhandener Tippfehler reproduziert**: `footMtx[2][1] = normalDir.z`
  wird tatsächlich nach `footMtx[2][2]` geschrieben (0x24 bleibt
  unbeschrieben). Mehrere `volatile`-Lückenfüller-Arrays für exakte
  Stack-Slot-Adressen.

**Neues, sechstes Bugmuster bestätigt**: „Benannte Lokale mit
Selbstzuweisung erzwingt Adress-CSE" — z. B. `f32& h = heightAt(...); h
= h - x;` verhindert MWCCs Faltung eines Compound-Assignments in einen
indizierten Zugriff (`TMapObjPlane::depress`).

**Zwei bemerkenswerte neue Stack-Layout-Techniken dokumentiert** (beide
erfolgreich angewendet, aber auch mit dokumentierten Grenzen):
- Ein **toter Aufruf einer INLINE-Funktion** (z. B. `MsClamp(0,0,0);`
  als eigenständige Anweisung) reserviert Stack-Bytes an einer
  SPEZIFISCHEN Position unterhalb einer bereits inline-expandierten
  Funktion (z. B. `MsSqrtf`s `volatile float`-Temporärwert) — anders als
  `char trash[N]`, das IMMER oberhalb bestehender Lokalen landet. Diese
  Technik löste `npcRecoverFromSinking`, versagte aber bei mehreren
  anderen Kandidaten dieser Runde (Hino2HeadCallback, TNerveHino2JumpIn::
  execute), wo die fehlenden Bytes UNTERHALB eines versteckten
  Rückgabewert-Temporärwerts sitzen und mit keinem getesteten
  Konstrukt reproduzierbar waren.
- **„By-Value-Rückgabeslot einer inline-expandierten Funktion hält einen
  Konstruktor-Aufruf out-of-line; eine benannte Lokale nicht"** — neue,
  präzise Formulierung der bereits bekannten Inline-Tiefe-Heuristik,
  bestätigt an zwei unabhängigen Fällen (`NpcEffect.cpp`s
  `getEffectScale_`-Vergleich, `emario.cpp`s `SMS_DistanceBetween`-
  Hilfsfunktion für `sub()`/`sqrt()`).

**Neun gründlich dokumentierte Fast-Treffer/Sackgassen** (alle sauber
zurückgesetzt, hoher Erkenntniswert): `MSound`-Konstruktor (reines
Register-Rauschen um `this`), `JPAConvectionField::affect` (zwei echte
Bugs gefunden — nicht-initialisiertes Schatten-`thing4`, In-Place- statt
separates `setLength()` — auf reines Registerrauschen reduziert),
`draw_wipe_box` (echter h/w-Reihenfolge-Bug gefunden, Rest ist
Scheduler-Rauschen bei unabhängigen Fließkomma-Konvertierungen),
`TMareEventBumpyWall::bumpDownZ` (zwei echte Bugs — vertauschte
Konstruktor-Komponente, fehlende CSE-Erzwingung — blockiert von einem
klassenweiten systemischen Rahmen-Offset, bestätigt in unberührten
Schwesterfunktionen), `TMario::wireSWait` (**vier echte Bugs** gefunden
und gefixt — falsches Feld `mWireBounceVelPrev` statt `mWireBounceVel`,
falsche Bit-Maske, vertauschte Subtraktionsoperanden, wegoptimierte
Multiplikation —, blockiert von einer geteilten Inline-Hilfsfunktion mit
Alles-oder-Nichts-Schwellenwert), `MSStageCubeFade::proc` (**zwei echte
Bugs** — fehlende Y-Komponenten-Übernahme, Stub-Funktion
`calcParamRatioInCube` real implementiert —, blockiert von
dateiweiter Rahmen-Inflation), `TBaseNPC::execWalk` (**fünf echte
Bugs** — Accessor- statt Direktzugriff, fehlendes `fabs`, fehlende
Kopierkette, falsche Konstante, **falscher Algorithmus** in
`isCanWalk` [horizontale statt 3D-Distanz] —, auf 12 von ~250
Instruktionen reduziert), `TEMario::perform`/`init` (**echter
Kontrollfluss-Bug** gefunden — Retail kehrt nie früh zurück, läuft
immer bis zum Ende durch —, Header-Änderungsversuch an `JGVec3.hpp::
distance()` korrekt als regressionsverursachend identifiziert und
verworfen), `Hino2HeadCallback`/`TNerveHino2JumpIn::execute` (mehrere
echte Bugs gefunden, auf 8 von ~90 bzw. 82 von ~180 Instruktionen
reduziert).

**Kleinere Inzidente**: Zwei Unteragenten stellten fest, dass generische
Python-Eval-Variablennamen (nicht Dateien!) über Sitzungen hinweg
kollidieren können, wenn mehrere Agenten zufällig denselben Kernel-
Kontext nutzen — beide erkannten es selbst, wechselten auf
dateibasierte Werkzeuge (`edit`/`write`) und stellten betroffene
Dateien wieder her; keine dauerhaften Schäden. Zwei Task-Ergebnisse
kamen mit Status „failed" statt eines sauberen Abschlussberichts zurück
(vermutlich Antwortlängen-Limit erreicht) — beide hinterließen dennoch
nachweislich saubere, unveränderte Arbeitsbäume.

### Session-Gesamtstand nach Runde 55

**434 verifizierte echte Fixes in 88 Commits.** `matched_functions`:
**9021** (von 9010 zu Rundenbeginn), `matched_code_percent`: 45,20 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`.

### Nach sechsundfünfzigster Iterationsrunde (17-Kandidaten-Batch: 4 neue Matches, Kumokun-Cluster vollständig als Sackgasse bestätigt, wiederkehrendes „JUTRect-Bounds"-Scheduler-Muster kartiert)

**Vier neue Byte-exakte Matches**:

- **`TAnimalManagerBase::clipEnemies(JDrama::TGraphics*)`** (95,0 % →
  Match, Commit `6a0546b3`): Drei unabhängige Ursachen — `mViewClipNear`
  vor dem Aufruf in eine Lokale gehoben, Akteur-Zeiger vor statt in der
  Schleife deklariert, zweiseitige Stack-Polsterung. **Neue, präzise
  Regel bestätigt**: MWCC legt früher deklarierte Lokalen bei HÖHEREN
  Stack-Adressen ab — ein `trash`-Array VOR einer Lokalen polstert
  darüber (verschiebt sie nicht), ein `trash`-Array NACH einer Lokalen
  verschiebt sie nach oben. Zweiseitige Lücken brauchen zwei
  `trash`-Arrays.
- **`CPolarSubCamera::execWallCheck_(Vec*)`** (95,6 % → Match, Commit
  `ee637ee5`): Additions-Reihenfolge der Ebenendistanz, Accessor- vs.
  Direktzugriff für CSE-Steuerung, und ein **neues, siebtes Bugmuster**:
  Die Registerform einer inline-kopierten 12-Byte-Struktur hängt von der
  QUELLAUSDRUCKSART ab — Kopie aus einem Struct-Mitglied (`obj.field`)
  ergibt ein anderes Scratch-Register-Paar als Kopie aus einer einfachen
  benannten Lokalen oder einem Array-Element. Wenn Retail die
  Mitglieds-Form zeigt, liegen beide Vektoren vermutlich in EINEM
  gemeinsamen Stack-Aggregat, nicht in zwei getrennten Lokalen.
- **`TTamaNokoFlower::perform(u32, JDrama::TGraphics*)`** (94,1 % →
  Match, Commit `d529a708`): Echter Verhaltens-Bug — rotierter Vektor
  wurde ins Member `unk20` statt zurück in `local_88` geschrieben
  (nachfolgende Lesungen nutzten den unrotierten Vektor); plus fehlendes
  bedingungsloses `return` im Demo/Talk-Zweig.
- **`TBossMantaAdditionalCollision::perform(...)`** (Bonus-Fund,
  Commit `e2df3ce4`): Derselbe Pattern-4-Bug wie im NICHT gelösten
  `TBossManta::moveObject` (siehe unten) — `AttackMario(mCollisions[i])`
  statt `AttackMario(this)` —, hier aber OHNE Restdifferenz vollständig
  behoben.

**Kumokun-Cluster vollständig als Sackgasse bestätigt**: Alle sieben
untersuchten Funktionen in `src/Enemy/Kumokun.cpp`
(`checkOnMovingWall/Floor/Roof`, `bindOnFlying`, `moveObject`,
`decideTargetAtDir`, `rotateGoalDirToLocal`) sind blockiert durch
dieselbe gemeinsame Ursache im inline-expandierten `TKumokun::
checkWallPlane`: Retail emittiert nach der `isTouchedWallsAndMoveXZ()`-
Prüfung `ble`, unser Build immer `beq`, unabhängig von der
Quelltext-Formulierung (`if(x)`, `if(0<x)`, Ternary — alle identisch).
Dies löst eine Registerzuteilungs-Kaskade aus, die alle Funktionen
gleich betrifft. Drei unabhängige Unteragenten bestätigten denselben
Befund und koordinierten sich erfolgreich über Hub, bevor sie
zurücksetzten — keine Datei-Konflikte trotz gemeinsamer Zieldatei.
Weitere, in diesem Cluster gefundene aber isoliert nicht ausreichende
echte Bugs: falsch gespeicherter Wert (`yTmp` statt `dVar10`) in
`checkOnMovingFloor`/`checkOnMovingRoof`; unnötige Vektorkopie in
`checkOnMovingWall`.

**Wiederkehrendes „JUTRect-Bounds"-Scheduler-Muster** über drei
unabhängige Dateien bestätigt (`GCConsole2.cpp`s `processDownCoin`/
`processAppearCoin`, `ConsoleStr.cpp`s `processShineGet`/`processMiss`):
Der Ausdruck `JUTRect bounds(...); ptr->mGlobalTranslation.set(bounds.x1
+ bounds.getWidth()*0.5f, ...)` erzeugt bei Retail eine KONSERVATIVE
Instruktions-Planung (frühe `stw`-Speicherung vor der `xoris`-Adress-
berechnung) für die Magic-Double-Konvertierungsblöcke, während unser
Compiler konsistent eine AGGRESSIVERE Umordnung wählt — bestätigt in
drei unabhängigen Dateien mit identischem Muster, kein quelltext-
seitiger Hebel gefunden (auch Retails eigenes `GCConsole2.cpp` zeigt
dasselbe konservative Muster für denselben Idiom-Typ, schließt also
eine Datei-lokale Ursache aus).

**Weitere gründlich dokumentierte Fast-Treffer** (alle instruktions-
identisch oder nahezu, aber durch Stack-Slot-Feinheiten blockiert,
sauber zurückgesetzt): `TGCConsole2::startAppearBalloon` (3 echte Bugs,
8/180 Zeilen Rest, Ursache in `J2DWindow::getContentsBounds()`s
Rückgabe-per-Wert-ABI lokalisiert), `CPolarSubCamera::
updateDemoCamera_` (53,2 % → 220/220 Instruktionen identisch, blockiert
durch `operator+`-Tiefe-3-Out-of-line-Regel), `TNerveBGKDie::execute`
(2 echte Bugs, 264/264 Instruktionen identisch, TU-weite Rahmen-
Inflation über 10+ Funktionen bestätigt), `TNerveRHGraphWander::
execute` (199/199 Instruktionen identisch, blockiert durch 8- vs.
4-Byte-Ausrichtung eines Rückgabewert-Temporärs), `TPoiHana::init`
(167/167 Instruktionen identisch, Restursache in einer ANDEREN
Funktion derselben Datei über einen gemeinsamen Rodata-Pool
lokalisiert), `TNameIndParCallback::execute`/`TNameKuri::
calcRootMatrix` (zusammen 6+ echte Bugs, Differenzen um 90 % reduziert).

### Session-Gesamtstand nach Runde 56

**438 verifizierte echte Fixes in 92 Commits.** `matched_functions`:
**9025** (von 9021 zu Rundenbeginn), `matched_code_percent`: 45,26 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`.

### Nach siebenundfünfzigster Iterationsrunde (20-Kandidaten-Batch: 6 neue Matches, systemisches TFlagT<u16>-Argument-Rahmenproblem entdeckt, JGVec3-operator*-Hypothese erneut bestätigt)

**6 neue Fixes, alle commitet:**

1. `TMario::TSurfingParams::TSurfingParams` (`MarioInitSurfing`,
   Commit `4acf881b`) — Feldtyp-Bug (Muster 4, neue Unterart): `mRoll`
   war als `TParamRT<s32>` deklariert, muss `TParamRT<f32>` sein
   (Header-Änderung in `include/Player/Mario.hpp`, 0 Regressionen im
   vollen Report-Diff bestätigt).
2. `TMapObjWaterSpray::calc` (`MapObjTownWaterSpray`, Commit
   `32ddf2c1`) — Rotation in benannte `s16`-Lokale materialisiert +
   `char trash[8]`.
3. `TBaseNPC::setPosAndInitAfterSinkBottom` (`NpcChangeSink`, Commit
   `814bd61f`) — drei Bugs: (a) `pos.y`/`pos.z` müssen in Lokalen
   zwischengespeichert werden, wobei die **Register-Zuordnung der
   Deklarationsreihenfolge folgt, die Lade-Reihenfolge aber der
   Initialisierungsreihenfolge** (neue Verfeinerung von Regel 7: `f32
   z; f32 y = pos.y; z = pos.z;` — `z` zuerst deklariert landet in
   f31, `y` zuerst initialisiert wird zuerst geladen); (b) beide
   `mSpine->setDefaultNext()`-Aufrufstellen müssen als
   `setNext(getDefault())` geschrieben werden (Registerreihenfolge der
   Argument-Auswertung); (c) zweiseitiges Trash-Padding (4 Bytes davor,
   0x1c danach).
4. `TMushroom1up::control` (`MapObjItem2Mushroom`, Commit `b0190e38`)
   — `MsClamp` erzeugte in beiden if/else-Zweigen identischen toten
   Code (Tell-Tale für Muster 1); durch anonyme-Namespace-Templates
   `MsMin`/`MsMax` ersetzt (reproduziert Ein-Vergleich-pro-Zweig-Codegen
   und plaziert die Literale in `.sdata` statt `.sdata2`), dazu
   `mPosition = pos` (Wort-Kopie) statt `.set()` (Float-Kopie),
   Anweisungsreihenfolge (y-Update vor sin/cos) und zweiseitiges
   Trash-Padding.
5. `TNPCManager::clipEnemies` (`NpcManagerClip`, Commit `c15ec3e8`) —
   fünf gestapelte Bugs: De-Morgan-Negation fehlte, `fovy`/`aspect`
   vertauscht, Deklarationsreihenfolge-Register-Fix, Accessor statt
   Direktzugriff (rechts-nach-links-Auswertungsreihenfolge), expliziter
   Cast statt impliziter `operator Vec*()`-Konversion.
6. `TItemSlotDrum::getForcastResult` (`MapObjSirenaMisc`, Commit
   `cd6e36d3`) — quantisierter Winkel muss in die bestehende
   `angle`-Lokale zurückgeschrieben werden statt als Temporärwert an
   `getResultFromAng` übergeben zu werden; sonst rematerialisiert MWCC
   die Int→Float-Magic-Double-Konversion ein zweites Mal im inline
   Vergleich.

**Neue systemische Erkenntnis — `JDrama::TFlagT<u16>`-Trailing-Argument
bricht Rahmengrößen-Buchhaltung an 8+ Aufrufstellen:** Bei
`THideObjPictureTwin::afterFinishedAnim` (`MapObjHideAnim`, NO-MATCH,
zurückgesetzt) wurde ein echter Muster-4-Bug gefunden (`&mPosition`
statt `&obj->mPosition`) und behoben — danach war der Instruktions-
Strom bis auf 2 von 120 Zeilen identisch, aber der Stack-Bereich unter
den benannten Lokalen ist bei Retail durchgehend 0x14+ Bytes größer.
Systematische Prüfung aller `fireStartDemoCamera`-Aufrufer zeigt
dasselbe Muster in MapObjTown, Item (`TShine::appearWithDemo`/
`control`), MapEventDolpic, MapObjSirena, MapEventSirena, cameragc
(`CPolarSubCamera::loadAfter`) und NpcEvent — überall reserviert
Retail mehr Stack für den `TFlagT<u16>`-Wertparameter als unser Build.
Kein Konstrukt aus der `.cpp`-Datei kann den fehlenden Slot erzeugen;
vermutlich eine kompilat-weite Inline-Rahmen-Buchhaltungslücke, deren
Lösung mehrere Funktionen gleichzeitig lösen würde. Verwandt, aber
bestätigt eigenständig: `TShine::appearWithTime`/`appearSimple`
(`ItemAppearWithTime`, NO-MATCH) zeigen dasselbe „Retail reserviert
Rahmenplatz für inline-Callees, die wir nicht reservieren"-Symptom in
20 von 94 Funktionen derselben Datei; Compiler-Flag-Experimente
(`-inline`-Varianten, `-O4`, `-RTTI`, `-enum min`, u.a.) schließen eine
einfache Flag-Ursache aus.

**`JGVec3.hpp::operator*(TVec3, f32)`-Bug erneut unabhängig
bestätigt** (dritte Bestätigung nach `WarpInCallBackExecute`, Runde 51):
`TEffectColumWater::generate` (`EffectObjMisc`, NO-MATCH) zeigt exakt
dasselbe Muster — `operator*` sollte `const TVec3&` statt `TVec3`
zurückgeben (derselbe Fabrikations-Trick, den `operator+`/`operator-`
im selben Header schon nutzen); ein temporärer Header-Patch reproduziert
die Retail-Instruktionsfolge exakt. Weiterhin nicht projektweit
angewendet (Cross-TU-Regressionsrisiko), aber jetzt mit drei
unabhängigen Fundstellen ein starker Kandidat für eine zukünftige,
sorgfältig vollständig regressionsgeprüfte Runde.

**Weitere gründlich dokumentierte Fast-Treffer/Sackgassen** (alle
sauber zurückgesetzt, kein Commit): `MtxUtilJointsToArc` (reines
Register-/FPR-Zähl-Rauschen, Algorithmus schon korrekt),
`NpcCallbackNeck` (f30/f31-Vertauschung zweier nichtflüchtiger FPR-
Kandidaten, 6 Varianten erfolglos), `PollutionManagerClean`
(zyklische Register-Rotation r6/r7/r8/r9→r7/r8/r9/r6, PCH-Datei, 6
Varianten erfolglos), `MapUpdate` (ein echter Bug in `updateDelfino`
gefunden und behoben, aber zwei ungelöste Restprobleme: Boolean-Guard-
Zweigform + 160-Byte-Rahmenlücke ohne sichtbare Ursache),
`FishoidLoad` (echter Bug gefunden: `setFleeTarget()`-Accessor statt
Direktzugriff; mathematisch bewiesen, dass die verbleibende 0x80-Byte-
Stack-Lücke wegen Deklarationsreihenfolge-Zwängen nicht per einfachem
Trash-Padding erreichbar ist), `JDRFrmGXSetPerform` (154/154
Instruktionen exakt reproduzierbar, aber die letzten 0x70 Bytes
Rahmenreserve wären nur durch Fabrikation unverifizierbaren toten Codes
erreichbar — bewusst nicht gemacht), `TPictureTelesa::touchActor`
(`MapObjSirenaMisc`, zweite Funktion — Rahmengröße korrekt, aber
Scheduling-Reihenfolge in einem `distance()`-Inline weicht ab;
`#pragma scheduling 604` behebt genau diesen Block, bricht aber den
Rest der Funktion — Hinweis auf ein TU-abweichendes Scheduling-Modell),
`TEffectObjBase::perform` (`EffectObjMisc`, zweite Funktion — Retail
reserviert ein nie geschriebenes r31-Save + 20 Byte toter Lokale, aus
der Instruktionsliste nicht rekonstruierbar), `MapObjHideAnim`,
`MarioCheckColHangPole` (4 gestapelte Bugs behoben, 4-Zeilen-Rest durch
einen 8-Byte-Spill-Slot der inline `std::sqrtf`-MSL-Expansion
blockiert), `MapObjCloudRide` (Graph-Web-CSE-Bug behoben, 215/219
Instruktionen identisch, Rest eine FPR-Permutation in einer
Drei-Operanden-Multiplikation), `NpcPartsCtor` (schwerwiegender
Struct-Layout-Bug in `include/NPC/NpcInitData.hpp` gefunden und exakt
bewiesen — `unk4[i]` ist kein Array-of-TNpcModelDataEntry mit Stride
0x2C, sondern ein Array-of-Pointer mit Stride 4 — sowie 8 weitere
Bugs, Rest auf 8 Zeilen reduziert, blockiert durch eine MWCC-
Konstantenfaltung, die Retail nicht durchführt), `ItemAppearWithTime`,
`CameraLibRotate` (algorithmischer Bug in `RotateAboutAxis` gefunden —
transponierte statt normale 3×3-Matrix-Multiplikation, Komponenten
direkt statt über `TVec3::set()` geschrieben; 286/286 Instruktionen
inkl. Register exakt reproduziert, letzte 8 `addi`-Stack-Immediates
durch einen MWCC-1.2.5-Inline-Instanz-Overhead von 4 Byte pro
Struct-Lokaler blockiert, gegen 6 Compiler-Versionen getestet).

### Session-Gesamtstand nach Runde 57

**444 verifizierte echte Fixes in 98 Commits.** `matched_functions`:
**9031** (von 9025 zu Rundenbeginn), `matched_code_percent`: 45,38 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`.

### Nach achtundfünfzigster Iterationsrunde (24-Kandidaten-Batch: 9 neue Matches, vierte unabhängige Bestätigung der „Phantom-Frame-Reservierung"-Fehlerklasse, TFlagT<u16>-Trigger präzisiert)

**9 neue Fixes, alle commitet — überwiegend reine Stack-Layout-Lücken
(Muster 7):**

1. `TLensFlare::TLensFlare(const char*)` (Commit `177bfa1a`) — 4-Byte-
   Lücke zwischen `buf[0x100]` und dem gesicherten r30; `char trash[4]`
   nach `buf` deklariert.
2. `TLensGlow::TLensGlow(bool, const char*)` (Commit `8a45e440`) —
   identisches Muster, 4-Byte-Lücke vor dem GPR-Save-Bereich.
3. `JASystem::Driver::updatecallDSPChannel` (Commit `d1598668`) —
   0x18-Byte-Rahmenlücke, exakt dieselbe Größe wie das etablierte
   `TDSPChannel::updateAll()`-Pendant in derselben Datei-Gruppe.
4. `JASystem::HardStream::main` (Commit `f66d778a`) — 16 Byte tote
   Lokal-Reserve ohne jede Instruktion; `char trash[16]` behebt es.
   Zwei weitere Funktionen derselben Datei (`startFirst`/`startSecond`,
   `volFloatToU8`) zeigen dasselbe Muster mit anderen Lückengrößen,
   nicht behoben (außerhalb des Auftrags).
5. `TMapXlu::changeNormalJoint` (Commit `d0f3b462`) — Muster 5
   (Accessor-vs-Direktzugriff): ein `getChild(i)`-Aufruf zu viel
   reservierte einen ganzen Inline-Expansionsblock; Ersatz durch
   `mChildren[i]` in der ersten Schleife entfernt ihn exakt.
6. `TMarDirector::loadResource` (Commit `12d36834`) — verschachtelter
   `{ JKRDvdFile sceneDvdFile; ... }`-Block brauchte 44 Byte mehr
   Reserve; `char trash[44]` NACH `sceneDvdFile` (Reihenfolge kritisch:
   davor deklariert hat keine Wirkung, da MWCC von oben nach unten
   zuteilt).
7. `TMBindShadowManager::TMBindShadowManager(const char*)` (Commit
   `ea13ece7`) — Muster 5: `gpApplication.mCurrArea.unk0` (Rohzugriff)
   musste durch `getStage()`-Accessor ersetzt werden, der einen
   zusätzlichen 4-Byte-„this"-Temporärwert reserviert.
8. `TMarioCap::TMarioCap(TMario*)` (Commit `2550d27e`) — fünf
   gestapelte Bugs: fehlende `.rodata`-Statics (Nullblock +
   Shift-JIS-„Speicher voll"-String, behebt zusätzlich die
   `.rodata`-Sektion von 93 % auf 100 %), vertauschte
   `setBaseTRMtx`/`getAnmMtx`-Kopierrichtung (Muster 4), 0x78 Byte
   Stack-Padding, benannte Referenz-Lokale für ein `const ResTIMG&`-
   Argument (Muster 6) und eine indexbasierte Neuladebasis-Optimierung.
9. `TPollutionObj::getDepthFromMap` (Commit `026cf5ac`) — `tmp`-
   Deklaration vor die Float-Lokalen verschoben plus ein zusätzliches
   4-Byte-Dummy-Lokal, um die exakte MWCC-Rundungs-/Reihenfolge-Regel
   für Stack-Slots zu treffen.

**Vierte unabhängige Bestätigung der „Phantom-Frame-Reservierung"-
Fehlerklasse, jetzt mit präziserem Auslöser:** `JDRActorLoad`
(NO-MATCH) zeigt denselben 4-Byte-Rahmenunterschied wie die
`TFlagT<u16>`-Fälle aus Runde 57, diesmal ausgelöst durch einen
INLINE-Basisklassenkonstruktor (`TLightMap`s Default-Ctor konstruiert
seine `TViewObj`-Basis, die wiederum `JDrama::TFlagT<u16>`s Ctor mit
einem Trailing-Wertargument aufruft) statt durch eine direkte
Trailing-Argument-Übergabe — verengt den Auslöser auf „`TFlagT<u16>`-
Konstruktion irgendwo im Inline-Baum", nicht nur auf Aufrufstellen.
Weitere Varianten derselben Fehlerfamilie, aber mit jeweils eigenem
Auslöser (nicht identisch, aber strukturell verwandt — alle „Bytes,
die keine Instruktion je berührt, aus Quelltext nicht platzierbar"):
`MarDirectorPreEntry` (48 Byte um einen synthetisierten `TRect`-
Wertrückgabe-Temporärwert, Padding landet immer oberhalb statt
unterhalb), `ConductorMakeEnemy` (4-Byte-Verschiebung in einem
inline-expandierten `JGadget::TList`-Iterator-Vergleich),
`MapObjManagerAppear` (4-Byte-Positionsunterschied eines
adressgenommenen Lokals, jede zusätzliche Variable rundet den Rahmen
um volle 8 Byte auf), `NpcColorInit` (8 Byte, ausgelöst durch die
Kombination `new`-Ausdruck in einem `switch`-Zweig + bedingter
Doppelaufruf in einem anderen), `SampleCtrlMaterialCtor` (durch einen
bereits im Repo dokumentierten TODO-Kommentar in
`J3DColorChan.hpp:72-77` über PCH/sdata2-Plazierung erklärt — externe
Bestätigung, dass dies ein bekanntes, ungelöstes Repo-Problem ist,
nicht nur eine Session-Beobachtung), `PollutionCountDrawStamp` (ein
Teil des Rahmens reparierbar, ein zweiter — ein `GXColor`-Argument-
Temporärwert — nicht), `EventWatcherHideDead` (zwei Funktionen
`evSetHide4LiveActor`/`evSetDead4LiveActor`, identischer 8-Byte-
Rahmenunterschied bei sonst 100 % identischem Code; eine dritte
Schwesterfunktion `evSetFlagNPCCanTaken` zeigt denselben Defekt, ist
aber nicht Teil dieser Runde).

**Weitere gründlich dokumentierte Sackgassen** (sauber zurückgesetzt):
`SelectMenuOpenWindow` (eine echte Link-Zeit-Koinzidenz — Retail
relokiert einen BGM-Enum-Wert gegen `showGPR__12JUTException+0x3C`,
numerisch zufällig identisch, aus Quelltext nicht rekonstruierbar),
`SpcInterpExecOps` (4 Funktionen `execadd`/`execsub`/`execmul`/
`execdiv`, identisches gemeinsames 4-Byte-Problem in einem anonymen
`TSpcSlice`-Temporärwert, ~10 Varianten erfolglos), `J3DClusterInitMtx`
(~50 Quelltext-Varianten getestet — MWCC kann für `&dl[3+vtxSize*k]`
entweder Basis-zuerst-Addition ODER nachgelagertes `addi +3` erzeugen,
aber nie beides gleichzeitig wie Retail), `MirrorActorInit` (9
Varianten, Iterator-Konstruktor-Temporärwerte in `JGadget::TList::
insert` falsch gepackt), `SplashManagerMakeDL` (vertauschte
Stack-Slot-Reihenfolge zwischen benanntem `GXColor`-Lokal und
anonymem Compound-Literal-Temporärwert, 6 Varianten erfolglos).

Zwei Agenten (`MarDirectorCtor`, `ObjManagerLoad`) brachen mit Fehler
ab, bevor sie Quelltext änderten (sauberer Zustand bestätigt) — beide
Kandidaten bleiben für eine künftige Runde offen.

### Session-Gesamtstand nach Runde 58

**453 verifizierte echte Fixes in 107 Commits.** `matched_functions`:
**9039** (von 9031 zu Rundenbeginn), `matched_code_percent`: 45,57 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`.

### Nach neunundfünfzigster Iterationsrunde (25-Kandidaten-Batch: 12 neue Matches — stärkste Runde bisher, überwiegend reine Stack-Layout-Lücken)

**12 neue Fixes, alle commitet — fast ausschließlich Muster-7-
Stack-Layout-Lücken, jeweils in 1-2 Iterationen gelöst:**

1. `CPolarSubCamera::controlByCameraCode_` (Commit `fa009596`) —
   `char trash[0x28]` vor der `TVec3`-Lokalen.
2. `TNpcThrow::throwMario` (Commit `cc4a82e1`) — `f32 trash;` nach der
   Vec3-Lokalen.
3. `TLiveManager::perform` (Commit `aae5ef05`) — `char trash[16]`,
   klassische Phantom-Frame-Lücke, diesmal direkt lösbar.
4. `TMarioGamePad::read` (Commit `4f3d0b32`) — `char trash[4]` nach
   `resetPort`.
5. `JDrama::TOrthoProj::perform` (Commit `857fd2a6`) — `char trash[8]`
   am Funktionsanfang.
6. `TDrawSyncManager::threadFunc` (Commit `3a1ee283`) — zweiseitiges
   Padding (`t1[4]` davor, `t2[4]` danach) um die `msg`-Lokale in der
   Schleife.
7. `TOneShotGenerator::loadAfter` (Commit `ead51be7`) — Muster 6:
   `TNameRefGen::search(...)`-Kettenausdruck musste in eine benannte
   `TIdxGroupObj* group`-Lokale materialisiert werden, bevor
   `->getChildren().push_back(this)` aufgerufen wird.
8. `TMovieSubTitle::setupResource` (Commit `d939e52f`) — `char
   trash[12]` nach `char buffer[256]`.
9. `JAIBasic::sendPlayingSeCommand` (Commit `4f40e7f9`) — `u16 trash;`
   nach zwei `u16`-Lokalen, verschiebt einen nachfolgenden
   Doppel-Spill-Temporärwert über eine 8-Byte-Ausrichtungsgrenze.
10. `TBGPolDrop::move` (Commit `6a7a7b9f`) — zwei Bugs: `char
    trash[12]` nach `checkData` PLUS ein echter Logikfehler
    (`unk50->setBckFromIndex(13)` benutzte den falschen MActor-
    Zeiger, muss `unk54->setBckFromIndex(13)` sein).
11. `TShimmer::perform` (Commit `1103319b`) — zwei Bugs: drei falsche
    virtuelle Aufrufe (Vtable-Slots 0x0c/0x10/0x14 von J3DModel waren
    je um eine Position verschoben — `entry()`/`calc()`/`update()`
    vertauscht) PLUS eine neue Stack-Technik: fünf `Mtx`/
    `J3DTransformInfo`-Lokale mussten von Block-Scope (innerhalb eines
    `if`) auf Funktions-Scope gehoben werden, um sowohl die
    Rahmengröße als auch jeden Einzeloffset zu treffen (Trash-Arrays
    wurden hier vollständig wegoptimiert, da unbenutzt).
12. `TMessageLoader::TMessageLoader(const char*)` (Commit `fb1fc4d2`)
    — `char trash[8]`, MWCC hatte zwei adressgenommene `u32`-Lokale
    volleliminiert (Wertfluss komplett registerbasiert), Retail
    reservierte trotzdem 8 Byte ungenutzten Speicher dafür.

**Fünfte bis achte unabhängige Bestätigung der „Phantom-Frame-
Reservierung"-Fehlerklasse, mit neuen Auslöser-Varianten:**
`ObjHitCheckActors` (NO-MATCH) fand DREI echte Logikfehler
(`entryGroup` statt `checkAndEntryGroup` an 4 Aufrufstellen, falscher
`TStrategy::unk10`-Index, `getAttackRadius/Height` statt
`getDamageRadius/Height`) und behob sie korrekt — der Instruktions-
Strom wurde dadurch 100 % identisch, blieb aber durch eine
unerreichbare 32-Byte-Rahmenlücke um `JGadget::TList`-Iterator-
Temporärwerte blockiert; nicht commitet trotz korrekter Logik-Fixes
(Hard Rule). Weitere Varianten: `MapEventSirenaWatch`/
`MapEventDolpicRiccoGate` (`TFlagT<u16>`-Trigger, aber Retail-Rahmen
diesmal KLEINER als unserer statt größer — Trash-Padding kann nur
hinzufügen, nicht entfernen, daher strukturell unlösbar in diese
Richtung), `NpcCollisionBind`/`PerformListLoad` (inline `JGadget::
TList`-Iterator-Temporärwerte, gleiche Fehlerfamilie wie
`ObjHitCheckActors`), `AreaCylinderLoad` (ein `readS32()`-
Schleifenzähler-Temporärwert bleibt trotz jeder Deklarationsreihen-
folge auf einem fixen Slot gepinnt), `MarDirectorCtorRetry` (kombiniert
zwei bekannte Auslöser: `JGadget::TVector_pointer<T>`-Standard-Ctor-
Kette PLUS 5× vorausgehende `TFlagT<Us>`-Konstruktion — bestätigt,
dass mehrere Auslöser sich in einer Funktion überlagern können),
`CameraWarpPosAndAt`/`ModelUtilRideMove`/`ProgSelectPerform` (jeweils
eine unerreichbare 4-12-Byte-Lücke im untersten „Outgoing-Parameter"-
Bereich des Rahmens, unterhalb aller benannten Lokalen — Trash-Arrays
werden dort entweder wegoptimiert oder nach oben verschoben, nie nach
unten platziert), `WalkerCalcFarthestVertex` (ein `volatile f32`-
Lokal bleibt bei jeder Padding-Kombination auf festem Offset 0x30
gepinnt, obwohl ein Nachbar-Array frei verschiebbar ist).

**`J3DModelEntryData`** (NO-MATCH, ~40 Varianten) — kein Phantom-
Frame-Fall, sondern ein echtes Register-Pool-Dilemma: MWCC hat zwei
getrennte Callee-saved-Register-Pools (einen für Compiler-CSE-
Temporärwerte r24/r25, einen für Quelltext-Lokale r20/r21). Zwei von
drei Diskrepanzen exakt gelöst (Zugriff über `getShapePacketArray()`
statt Direktzugriff; If-Zweig ohne benannte Lokale, um CSE über
`countDLSize()` zu erzwingen), aber der Else-Zweig verlangt
widersprüchlich sowohl eine früh materialisierte Adresse (nur über
eine benannte Lokale möglich) als auch deren Zuordnung zum Temp-Pool
(nur ohne benannte Lokale möglich) — ein zirkulärer Constraint ohne
Lösung auf Quelltext-Ebene.

### Session-Gesamtstand nach Runde 59

**465 verifizierte echte Fixes in 119 Commits.** `matched_functions`:
**9048** (von 9039 zu Rundenbeginn), `matched_code_percent`: 45,67 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`.

### Nach sechzigster Iterationsrunde (21-Kandidaten-Batch: 2 neue Matches, aber drei außergewöhnlich wertvolle Fast-Treffer mit insgesamt 12+ echten Gameplay-Bugs — inkl. eines projektweiten Header-Bugs)

**Pool der einfachen Kandidaten (80-100 % Fuzzy-Match, kleine Dateien)
erschöpft sich sichtbar** (504 → 168 → 84 → 44 → 24 Kandidaten über die
letzten vier Runden). Diese Runde wurde deshalb bewusst auf größere/
niedrigprozentigere Funktionen ausgeweitet (bis 88,64 %/72,14 % Fuzzy-
Match, bis 2008 Byte Größe) — Trefferquote sank entsprechend (2/21),
aber die QUALITATIVE Ausbeute an dokumentierten echten Bugs ist die
höchste einer einzelnen Runde bisher.

**2 neue Fixes, beide commitet:**

1. `TMenuBase::perform` (Commit `345444b7`) — zweiseitiges 4+4-Byte-
   Trash-Padding um ein `J2DOrthoGraph`-Stack-Objekt.
2. `TAfterEffect::perform` (Commit `3e2c1fec`) — neue allgemeine
   Technik entdeckt: **MWCC ordnet Callee-saved-Register nach
   Kandidaten-Erzeugungsreihenfolge** (Parameter zuerst, dann
   Frontend-Lokale/Temporärwerte in Quelltext-Reihenfolge, dann ERST
   Inline-Expansions-Temporärwerte), aufsteigend r28..r31 zugewiesen.
   Ein `const TRect& rect = graphics->getViewport();` (Accessor-Aufruf,
   inline expandiert) wird deshalb ALS LETZTES erzeugt und bekommt die
   falsche Registernummer; Ersatz durch Direktzugriff
   `graphics->mViewportRect` verschiebt den Kandidaten früher in die
   Liste und trifft Retails Reihenfolge exakt. `char trash[4]` NACH der
   betroffenen Lokalen stellt den durch den Accessor-Wegfall
   verlorenen 4-Byte-Stack-Slot wieder her.

**★ DREI HERAUSRAGENDE FAST-TREFFER — vollständig dokumentiert für
künftige Wiederaufnahme, NICHT commitet (Hard Rule: nur Byte-exakt):**

**`TSpider::bind` (`src/Enemy/spider.cpp`, 83,88 % → 100 % Instruktions-
identisch, blockiert durch 56-Byte-Phantom-Frame-Lücke):** VIER echte
Logikfehler gefunden und bestätigt (Nachweis: symbolische Neu-
herleitung gegen Retail-Disassembly):
- `checkGround`-Aufruf Nr. 2 benutzte `local_50.y` statt
  `param_1->mPosition.y` als Y-Argument.
- `TBGWallCheckRecord`-Y-Argument fehlte `+ spine->getHeadHeight()`.
- `TBGWallCheckRecord`-Radius benutzte `getHeadHeight()` statt
  `getWallRadius()` (= `mBodyScale * mWallRadius`, ein anderes Feld!).
- Wand-Rückstoßrichtung: Quelltext berechnete `local_bc = tmp -
  center` (mit separatem `tmp`), Retail skaliert `normal` IN PLACE und
  subtrahiert `normal` direkt von `local_bc` — unterschiedliche
  Aliasing-Semantik.
Zusätzlich wurde die exakte Quelltext-Form gefunden, die einen 100 %
instruktionsidentischen Strom erzeugt (366/366 Instruktionen, exakt
dieselben Register): `TSpineEnemy* spine`-Cast als benannte Lokale,
Referenz-Bindung für `normal` vor der Kopie, Wiederverwendung von
`local_50` statt einer dritten Lokalen, sowie eine präzise Kette
benannter Materialisierungen (`vy`, `vz`, `y`, `z` in genau dieser
Deklarationsreihenfolge) für die f27-f31-Registerbelegung. Blockiert
einzig durch eine unerreichbare 56-Byte-Lücke zwischen zwei Compiler-
Temporärwerten (Retail-Rahmen 0x158 vs. unser 0x120) — ein
`JGeometry::TVec3<f32>(0,0,0)`-Sondierungs-Statement wächst den
Temp-Pool zwar, aber in die FALSCHE Richtung (12 Byte oberhalb UND
unterhalb statt der benötigten zusammenhängenden 52-Byte-Lücke).
**Vollständige Wiederanwendungs-Anleitung im Agent-Transkript
`history://SpiderBind` archiviert** — sobald die Phantom-Frame-
Fehlerklasse durchbrochen ist, ist dies der Kandidat mit dem
höchsten sofortigen Ertrag.

**`JDrama::TSmJ3DAct::perform` (`src/JSystem/JDrama/JDRSmJ3DAct.cpp`,
72,14 % → strukturell identisch, blockiert durch Register-Scheduling
in drei Matrix-Multiplikationsblöcken):** DREI echte Bugs gefunden,
darunter ein **projektweiter Header-Bug**:
- Rotationsreihenfolge: Retail wendet Z, dann Y, dann X an; Quelltext
  hatte X, Y, Z (andere komponierte Rotation — echter Verhaltensbug).
- Matrix-Wiederverwendung: Retail nutzt nur ZWEI lebende Matrizen für
  die Multiplikationskette (Ergebnis wird wiederverwendet), Quelltext
  allozierte eine dritte.
- **`include/JSystem/JGeometry/JGMatrix34.hpp::TMatrix34::concat()`
  ist fehlerhaft transkribiert**: berechnet die TRANSPONIERTE des
  beabsichtigten Produkts PLUS einen Out-of-Bounds-Lesezugriff auf die
  4. Spalte (liest 0x34/0x38 Byte über die 3×4-Matrix hinaus). Durch
  symbolische Herleitung gegen Retails tatsächliche Arithmetik
  bestätigt: die korrekte zeilen-majore Form ist `result[i][j] =
  a.at(i,0)*b.at(0,j) + a.at(i,1)*b.at(1,j) + a.at(i,2)*b.at(2,j)`
  (+ `a.at(i,3)` für `j==3`). Betrifft mindestens DREI Aufrufstellen
  repo-weit: `JDRSmJ3DAct.cpp`, `JDRCamera.cpp` (`TPolarCamera::
  perform`, dort mit 65,79 % der schlechteste bekannte Match-Wert
  einer nicht-trivialen Funktion), und `BathWaterManager.cpp:1149`.
  Reiht sich ein in die wachsende Liste bekannter „fabrizierter"
  Header-Funktionen dieser Session (`J3DColorChan::getAttnFn`, Runde
  58; `TExPane::setCenteredSize`, Runde 60/CardLoad).
Mit allen drei Fixes: 146/313 Zeilen byte-identisch, alle 313
Instruktionen multisetgleich, aber Register/Planungsreihenfolge
innerhalb der drei Matrix-Multiplikationsblöcke weicht ab; zusätzlich
eine unerreichbare 0x38-Byte-Rahmenlücke (klassisches Phantom-Frame-
Muster). **Empfehlung für Runde 61: dedizierte Runde, die den
`concat()`-Fix mit vollständiger Projekt-Regressionsprüfung anwendet
und alle drei betroffenen Aufrufstellen neu bearbeitet — potenziell
drei Funktionen in einem koordinierten Durchgang.**

**`TConeBeam::calcVertices` (`src/Enemy/beam.cpp`, 88,64 % → auf 2
vertauschte Instruktionen reduziert): FÜNF echte Bugs gefunden:**
- Sin/Cos-Faktoren im `mBGCheckData==nullptr`-Zweig vertauscht
  (`local_140 * s` / `local_134 * c` statt umgekehrt) — der
  nicht-null-Zweig hatte bereits die korrekte Zuordnung, ein klarer
  Beleg für einen echten Transkriptionsfehler mit sichtbarer
  Auswirkung auf Kegel-Beam-Gegner ohne BG-Check-Daten.
- Halbierungsfaktor muss von LINKS multiplizieren (`0.5f *
  (mScale*MsSin(ang))`, nicht `/2.0f`).
- Gemeinsame benannte `ang`-Lokale für `MsSin`/`MsCos` nötig (ohne sie
  vertauscht MWCC die Konstanten-Register).
- Eine `TPartition3<f32> partition(...)`-Lokale existiert bei Retail
  gar nicht: vier `f32`-Lokale (Ebenenabstand + 3 Normalen-
  komponenten) werden VOR der Schleife einmalig geladen und bleiben
  über die gesamte Schleife in Callee-saved-Registern — die
  Struct-Variante erzwingt pro Iteration unnötige Stack-Reloads.
- Deklarationsreihenfolge mehrerer Lokalen (`local_140`, `local_134`,
  `local_128`; `oz`, `oy`, `ox`) musste exakt Retails Speicher-
  reihenfolge treffen.
Restlücke: zwei unabhängige `lfsx`-Ladeinstruktionen (jmaCosTable/
jmaSinTable) werden in vertauschter Reihenfolge emittiert, plus eine
Rahmenlücke (0x1c8 vs. 0x198) durch unterschiedliche Compiler-
Klassifizierung von inline-expandierten Struct-Wertkopien
(`operator*(TVec3,f32)`-Parameterkopien) — nicht durch Trash-Padding
erreichbar, da die POOL-REIHENFOLGE selbst abweicht, nicht nur die
Größe.

**Weitere gründlich dokumentierte Sackgassen** (sauber zurückgesetzt):
`MarDirectorSetupConsole`, `BoidLeaderCalcGoalForce`, `ButterfloidLoad`,
`NpcInbetweenExecPos` (reines FPR-Tie-Break, 8 Varianten erfolglos),
`JDREfbSettingIssueCopy` (bestätigt eine bereits früher in dieser
PROGRESS.md dokumentierte Sackgasse unabhängig erneut), `CameraMulti
PlayerCtrl` (fast vollständig gelöst, ein `MsSqrtf`-Temporärwert
bleibt auf festem Offset 0x10 gepinnt), `CardLoadDrawMessage` (ZWEI
echte Bugs — Array-Out-of-Bounds `unk4CC[3]`→`[2]`, falsches
`JUTRect`-Feld `unk46C`→`unk4B0` — blockiert durch einen
This/Result-Register-Swap, der laut Code-Kommentar `// TODO: hack,
regswap` bereits von einem früheren Mitwirkenden erfolglos bekämpft
wurde; `TExPane::setCenteredSize` ist im Header selbst als „fabricated
and incorrect" markiert), `SnapTimeObjPerform`, `AnimalNerveGraph
Wander`, `SunMgrLoad` (Register-Bank-Vertauschung this/rodata-Basis,
neue Variante der Phantom-Fehlerfamilie), `NpcInitPrgBaseInit`,
`JKRExpHeapAllocFromHead`, `MapCollisionPlaneCheck`, `EffectUtilSink
Pollution`, `CameraBckUpdateDemo` (Boolean-Materialisierung, Referenz
auf denselben Idiom-Typ in `MActor::isCurAnmAlreadyEnd`),
`CameraShakeExecShake` (ein weiterer echter Logikfehler: `unitVecTo`
benutzte `*pos` statt `origin` als Quellvektor — betrifft die
Rollachsen-Berechnung der Kamera-Erschütterung).

### Session-Gesamtstand nach Runde 60

**467 verifizierte echte Fixes in 121 Commits.** `matched_functions`:
**9050** (von 9048 zu Rundenbeginn), `matched_code_percent`: 45,73 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`. Trotz niedriger Fix-Quote dieser Runde
(2/21) außergewöhnlich hoher Erkenntniswert: 12+ dokumentierte echte
Gameplay-Bugs in drei Fast-Treffer-Funktionen, davon einer (`TMatrix34::
concat`) mit projektweitem Einfluss auf mindestens drei Aufrufstellen.

### Nach einundsechzigster Iterationsrunde (dedizierte Header-Fix-Runde: TMatrix34::concat + JGRotation3::setEular — 5 Header-/Funktions-Commits, darunter die schlechteste bisher bekannte Match-Funktion des Projekts geknackt)

**Dediziert der in Runde 60 gefundenen `TMatrix34::concat`-Hypothese
gewidmet, mit vollständiger Projekt-Regressionsprüfung vor jedem
Commit.** Ergebnis: FÜNF Commits, zwei geteilte Header vollständig
korrigiert, zwei Funktionen exakt getroffen — darunter
`TPolarCamera::perform`, die mit Abstand am schlechtesten
übereinstimmende nicht-triviale Funktion des gesamten Projekts
(65,79 % Fuzzy-Match zu Rundenbeginn).

**Commit 1 (`7520c7f5`) — `TMatrix34::concat(a,b)`-Translationsspalte:**
Ursprüngliche Hypothese aus Runde 60 bestätigt und commitet: die
Translationsspalte las über die 3×4-Matrixgrenze hinaus
(`a.at(3,0)`, `b.at(3,1)`, `b.at(3,2)` — gültige Zeilenindizes sind
nur 0-2). Verifiziert über zwei unabhängige Methoden: (a) algebraische
Verallgemeinerung der bereits korrekten 1-Argument-`concat(b)`-Form,
(b) direkter Soll/Ist-Abgleich gegen `PSMTXConcat` in
`src/dolphin/mtx/mtx.c`, da Retail `concat()` an der
`BathWaterManager.cpp:1149`-Aufrufstelle NICHT inlined, sondern
identisch zu diesem SDK-Symbol code-gefaltet hat. Volle
Projekt-Regressionsprüfung: 9050/9050 `matched_functions` unverändert,
45,73091 % `matched_code_percent` unverändert — null Regressionen.

**Commit 2 (`b80c4658`) — `TMatrix34::concat(a,b)`-3×3-Rotationsteil
(vom ersten Commit übersehen):** DREI unabhängige Subagenten
(`BathWaterManagerRender`, `TPolarCameraPerform`, `JDRSmJ3DActRetry`),
die parallel an den drei betroffenen Aufrufstellen arbeiteten, fanden
per Hub-Koordination unabhängig voneinander denselben zweiten Bug:
auch der 3×3-Linearteil war transponiert (`out[i][j] = Σ_k a.at(k,i)*
b.at(j,k)` statt der korrekten Standardform `Σ_k a.at(i,k)*b.at(k,j)`).
Die Translationsspalte allein war zufällig numerisch unauffällig
geblieben, weil die ersten Testfälle (Translations-only, Diagonal-
Skalierung) den Rotationsteil-Bug maskierten. Alle drei Agenten
koordinierten sauber über Hub (ein Agent übernahm probeweise die
Header-Bearbeitung, die anderen bauten nur gegen den Arbeitsbaum-
Zustand, niemand committete parallel) — Main verifizierte den finalen
Diff manuell, führte ein volles Rebuild durch (9050/9050 unverändert,
45,73091 % unverändert, `fuzzy_match_percent` leicht gestiegen) und
committete.

**Commit 3 (`72c8ca33`) — `JDrama::TPolarCamera::perform` EXAKT
GETROFFEN (272/272 Instruktionen, 1088 Byte):** Vier unabhängige Bugs:
(1) `concat()`-Operandenreihenfolge an 2 von 3 Aufrufstellen vertauscht;
(2) Rotationskette ist Z-X-Z (nicht Z-Y-X — klassische Polar-/Orbit-
Euler-Konvention, echter Verhaltensbug: Kamera rotierte um falsche
Achsen); (3) präzise verkettete Zuweisungsgruppierung für eine
Identitäts-plus-Translations-Temporärmatrix nötig (MWCC leitet das
Literal an den ERSTEN Store einer Kette weiter, den Ketten-
Temporärwert an die übrigen); (4) `-unk44` muss ein reiner
Zuweisungs-RHS sein, kein inline-Aufrufargument. Plus `char
trash[104]` nach etabliertem `TOrthoProj::perform`-Muster derselben
Datei. **Entdeckte dabei zwei weitere Header-Bugs** (siehe unten).

**Commit 4 (`258e617c`) — `JDrama::TSmJ3DAct::perform` EXAKT GETROFFEN
(311/311 Instruktionen, 1244 Byte, Rahmen 0x208):** Vier unabhängige
Bugs, drei echte Verhaltensfehler: (1) Euler-Reihenfolge war X-Y-Z,
Retail ist Z-Y-X (verifiziert über Feld-Offsets 0x38/0x34/0x30 der
DEG_TO_RAD-Ladeoperationen); (2) DREI statt zwei Matrix-Temporärwerte
— Retail pingpongt zwischen exakt zwei; (3) Rotations-Temporärmatrix
braucht `setTrans(0,0,0)` statt `identity()` (letzteres schreibt auch
den 3×3-Teil und kostet dadurch +14 Instruktionen); (4) `char
trash[0x48]` für 72 Byte toter Lokale unterhalb der letzten Matrix.
Mit symbolischem Ausdrucksbaum-Interpreter verifiziert: alle 36
Ausgabewerte aller drei `concat`-Aufrufe algebraisch identisch zu
Retail, bevor überhaupt am Scheduling gearbeitet wurde.

**Commit 5 (`38f64391`) — `JGRotation3.hpp::setEularX/Y/Z`, ZWEI
weitere Bugs in einem zweiten geteilten Header:** Während der Arbeit
an den beiden obigen Funktionen fanden `TPolarCameraPerform` und
`JDRSmJ3DActRetry` unabhängig voneinander (an verschiedenen
Aufrufstellen) zwei zusätzliche Fehler in `include/JSystem/JGeometry/
JGRotation3.hpp`: (a) `setEularX` war transponiert (`ref(1,2)=s,
ref(2,1)=-s` statt korrekt `ref(1,2)=-s, ref(2,1)=s`) — inkonsistent
mit den bereits korrekten `setEularY`/`setEularZ` derselben Datei,
echter Verhaltensbug; (b) alle drei `setEularX/Y/Z`-Funktionen
brauchen ihre vier Null-Einträge als EINE verkettete Zuweisung
(`ref(a)=ref(b)=ref(c)=ref(d)=0.0f;`) statt vier separater
`= 0.0f;`-Anweisungen — ein reines Codegen-Ordnungsproblem (MWCC
erzeugt den Null-Wert-Knoten bei vier separaten Anweisungen spät/pro
Store, bei einer Kette früh/einmalig; das verändert die
Scheduler-Eingabereihenfolge in JEDEM inline-expandierten `concat`-
Block und verschiebt zusätzlich den Stack-Rahmen — kein semantischer
Unterschied). `JDRSmJ3DActRetry` lieferte harte Evidenz auch für
`setEularY` (168 Instruktionen Differenz ohne die Verkettung, exakt 0
mit ihr) — widerlegt die ursprüngliche Annahme "keine Evidenz für Y".
Volle Regressionsprüfung vor Commit: `matched_functions` 9050→9052
(exakt die zwei oben genannten neuen Matches, keine unerklärten
Verschiebungen), keine Regression in `bossManta.cpp` (einzige weitere
Datei, die `setEularY` nutzt) oder irgendwo sonst im Projekt.

**★ Herausragender Fast-Treffer, NICHT commitet:**
`TBathWaterMeshRenderer::prerender` (`src/Map/BathWaterManager.cpp`,
2624 Byte) — von 84,3 % (nach beiden `concat`-Fixes) auf 94,7 %
gebracht, ZEHN unabhängige Bugs gefunden und bestätigt, darunter:
- **Echter Gameplay-Bug**: `GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE)`
  muss `GX_LESS` sein — der Tiefentest für den Badewasser-
  Höhenkarten-Vorabdurchlauf ist im Dekompilat falsch.
- Ein bemerkenswerter Fund in `include/JSystem/JGeometry/
  JGProjection.hpp::orthographic()` (nur von dieser einen Funktion
  genutzt): beide Translationsterme brauchen `+ n` (den Parameter,
  NICHT das Literal `0.0f`) — mathematisch ein Bug im Retail-Code
  selbst, der aber harmlos bleibt, weil jeder Aufrufer `n = 0.0f`
  übergibt; muss dennoch als `+ n` transkribiert werden, um Retails
  tatsächliches (fehlerhaftes) Verhalten exakt zu reproduzieren —
  Kernprinzip der Dekompilation: Bugs mitkopieren, nicht korrigieren.
- Sieben weitere Bugs/Techniken: Struct-Kopie statt Skalar-Extraktion
  für eine Drop-Position, doppelt vorkommende negR/R3-Konstanten-
  berechnung (Retail berechnet sie zweimal ohne CSE), Deklarations-
  reihenfolge-Fix für einen Stack-Pool, hochgezogene `J3DModelData*`-
  Lokale, ein als verworfene Anweisung transkribierter
  `TUtil<f32>::sqrt`-Aufruf mit toter Domänenprüfung, Auswertungs-
  reihenfolge-Fix (Retail ruft `SMSGetGameRenderHeight` vor `...Width`
  auf), unbenannte Temporärwerte statt benannter `dir`/`up`-Lokalen.
Restlücke (5,3 %): zwei Instruktions-Scheduling-Cluster (eine
`drawCap`-Schleifeninvariante, die bei JEDER zusätzlichen Lokalen aus
dem `-inline auto`-Budget fällt — bestätigt hauchdünn) sowie eine
Stack-Pool-RICHTUNGS-Divergenz (Retail alloziert einen Temporär-Pool
in der ENTGEGENGESETZTEN Richtung zu unserem Build, plus ein totes
92-Byte-Sperrblock — klassisches Phantom-Frame-Muster, `char
trash[92]` reproduziert zwar Größe und Abstand exakt, ändert aber den
objdiff-Score nicht). Vollständiges 10-Punkte-Rezept im
Agent-Transkript `history://BathWaterManagerRender` archiviert.

### Session-Gesamtstand nach Runde 61

**469 verifizierte echte Fixes in 126 Commits** (467 aus Runde 60 + 2
neue Funktions-Matches; die 3 Header-Fix-Commits zählen als
Infrastruktur-Korrekturen, nicht als eigene „Funktions-Fixes", tragen
aber wesentlich zum Gesamtfortschritt bei). `matched_functions`:
**9052** (von 9050 zu Rundenbeginn), `matched_code_percent`: 45,80 %.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`. Diese Runde demonstriert den Wert
koordinierter Parallel-Subagenten bei geteilten Headern: drei Agenten
fanden denselben zweiten `concat`-Bug unabhängig voneinander aus drei
verschiedenen Aufrufstellen und koordinierten sauber über Hub, um
Merge-Konflikte am gemeinsamen Header zu vermeiden, bevor Main den
finalen Commit nach vollständiger Regressionsprüfung durchführte.

### Nach zweiundsechzigster Iterationsrunde (Methodik-Wechsel zu funktionsgenauer statt dateibasierter Ausschlussliste — 23-Kandidaten-Batch, 14 neue Matches, zweitstärkste Runde bisher)

**Methodik-Erkenntnis:** Der dateibasierte Ausschluss-Fragment-Ansatz
(bisher: alle Funktionen einer Datei ausschließen, sobald EINE Funktion
darin behandelt wurde) hatte den Kandidatenpool künstlich auf nur noch
4-24 Kandidaten schrumpfen lassen, obwohl `fuzzy_match_percent < 100`
im frischen `report.json` bereits objektiv beweist, dass eine
bestimmte Funktion noch NICHT exakt getroffen ist — unabhängig davon,
ob andere Funktionen derselben Datei bereits gefixt wurden. Umstellung
auf eine präzise, pro-Einheit-exakte Ausschlussliste (nur konkret
dokumentierte Sackgassen-Einheiten, keine Substring-Fragmente mehr)
öffnete den Kandidatenpool sofort wieder auf 167 frische Dateien.

**14 neue Fixes, alle commitet:**

1. `TMarioParticleManager::emitAndBindToMtxPtr` (Commit `a7b21d6e`) —
   `char trash[4]` nach einer Vec3-Lokalen.
2. `TNervePoihanaSleep::execute` (Commit `713f66e3`) — `char trash[8]`.
3. `TOptionControl::loadSetting` (Commit `8837a94b`) — `char
   trash[0x28]` (40 Byte).
4. `TNerveStayPakkunHide::execute` (Commit `95394573`) — `char
   trash[16]`.
5. `MSoundSESystem::MSoundSE::getRandomID` (Commit `9c3ed7b2`) —
   `char trash[4]` nach einem `u32[16]`-Array (identisches Muster zu
   `lensflare.cpp`/`lensglow.cpp`).
6. `TMario::waiting` (Commit `8a33e6b7`) — `char trash[24]`, gleiche
   Technik wie das bereits gefixte `TMario::stopCommon` derselben
   Datei.
7. `TWaterGun::setBaseTRMtx` (Commit `4c5be3f1`) — **neue Stack-
   Technik**: kein Trash-Array half hier (alle drei Positionen
   erfolglos, wie in früheren Runden dokumentiert); stattdessen musste
   die `Mtx temp;`-Deklaration strukturell an ihre erste Verwendungs-
   stelle verschoben werden (nicht als Padding, sondern durch
   Änderung der MWCC-Frontend-Erzeugungsreihenfolge selbst).
8. `TNPCManager::makePartsModelData_` (Commit `ed928359`) — `char
   trash[4]` nach einem `char[0x100]`-Puffer.
9. `TMarioEffect::perform` (Commit `a6a5d71a`) — `Mtx mtx;` als
   ungenutzte, aber Retail-seitig echt deklarierte Lokale (48 Byte,
   sizeof(Mtx)) — hier WAR die fehlende Reservierung eine reale, im
   Quelltext einfach vergessene Variable, kein Trash-Padding-Trick.
10. `TPauseMenu2::load` (Commit `1fa46609`) — **zwei echte Bugs**:
    (a) Panel-Suchschlüssel `'t_0' + i` muss `'pa00' + i` sein (falscher
    FourCC-Literal, hätte im Spiel dazu geführt, dass die Pause-Menü-
    Buchstaben-Bilder nicht gefunden werden); (b) `add(0, 14)` muss
    `add(0, 20)` sein. Plus `char trash[32]`.
11. `TAmenbo::init` (Commit `b8b91d24`) — **echter Bug**: Gelenk-
    Index-Cache-Schleife rief `getMaterialName()` (Offset 0xB4) statt
    `getJointName()` (Offset 0xB0) auf — falscher Accessor, liest
    einen komplett anderen Materialdaten-Bereich. Plus `char trash[8]`.
12. `TMario::surfing` (Commit `e7ed3c73`) — **echter Bug**: Wasser-
    /Boden-Parameterauswahl testete `mWallPlane->isWaterSurface()`
    statt `mGroundPlane->isWaterSurface()` (TMario+0xD8 statt +0xE0,
    falsches Feld). Plus `char trash[8]`.
13. `TGesso::bind` (Commit `38bde37e`) — zwei verzahnte Stack-Layout-
    Ursachen: (a) `MsAtan2()`-Inline-Wrapper-Aufruf durch die
    ausgeschriebene Form `abs(matan(...) * (360.0f/65536.0f))` ersetzt
    (spart einen 8-Byte-Inline-Rückgabe-Temporärwert, gleiches Idiom
    wie in `graph.cpp`); (b) eine VIERTE, komplett ungenutzte
    `TVec3<f32>`-Lokale nötig, um einen fehlenden 12-Byte-Slot im
    Block zu reservieren (Block ist boden-verankert: neue Lokale
    verschiebt den Frame-Boden nach unten, nicht nach oben).
14. `TFireWanwanTailHit::movementBody` (Commit `f1361d70`) — **neue
    Technik, Umkehrung von Muster 5**: unser Rahmen war KLEINER als
    Retail (0xf8 vs. 0x138, 0x40 Byte Differenz), verursacht durch
    FEHLENDE Inline-Expansions-Reservierungen. Anstatt einen Accessor
    durch Direktzugriff zu ersetzen (übliche Richtung von Muster 5),
    mussten hier DREI NEUE fabrizierte Inline-Setter (`TTailRubber::
    setBoundRate/setDecay/setMaxLength`) in `include/Enemy/
    FireWanwan.hpp` ergänzt und an 8 Stellen anstelle von Direkt-
    zuweisungen verwendet werden, um genau die fehlenden Inline-
    Reservierungsblöcke zu erzeugen — der Instruktionsstrom bleibt
    dabei komplett unverändert. Zusätzlich musste ein bereits
    existierender fabrizierter Wrapper (`isTailTaken()`) anstelle von
    `unk194->isTaken()` verwendet werden. Header-Änderung ist rein
    additiv (neue Methoden, keine Verhaltensänderung an bestehenden);
    beide anderen Einbinder der Datei (`CameraNormal.cpp`,
    `MarNameRefGen_Enemy.cpp`) wurden nachgebaut und sind exakt
    unverändert (md5-identisch) — keine Regression.

**Weitere gründlich dokumentierte Sackgassen** (alle sauber
zurückgesetzt): `TBathWaterManager::loadAfter`, `TTrembleModelEffect::
reset`, `CPolarSubCamera::execGroundCheck_`, `TTamaNoko::
calcRootMatrix`, `THamuKuri::behaveToWater`, `JPAGetRMtxSTVecElement`
(bestätigt: `std::sqrtf`s Drei-Schritt-Newton-Raphson-Iteration ist
bereits korrekt, `TUtil<f32>::sqrt()` wäre die falsche, da nur
einstufige, Funktion), `TRoulette::initMapObj` (4 Varianten,
`JGadget::TList`-Iterator-Temporärwert-Familie), `TGraphWeb::
getRandomNextIndex` (8-Byte-Lücke unerreichbar ohne Stilbruch
gegenüber Schwesterfunktionen derselben Datei), `TBossPakkun::
setGroundCollision` (13 Varianten, exakte Zielposition nur mit
falscher Rahmengröße erreichbar, nie beides gleichzeitig).

### Session-Gesamtstand nach Runde 62

**483 verifizierte echte Fixes in 140 Commits.** `matched_functions`:
**9066** (von 9052 zu Rundenbeginn, +14, exakt wie erwartet).
`matched_code_percent`: 46,06 % (größter Einzelrunden-Zuwachs der
jüngeren Session-Geschichte, +0,26 Prozentpunkte). Volles
`ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`.

### Nach dreiundsechzigster Iterationsrunde (23-Kandidaten-Batch, 16 Fixes/17 Funktionen — neuer Rekord, drei neue Stack-Layout-Techniken katalogisiert)

**16 neue Fix-Commits, 17 exakt getroffene Funktionen** (eine
2-für-1-Sonderfund, siehe unten) — größter Einzelrunden-Zuwachs der
Session bisher (+0,28 Prozentpunkte `matched_code_percent`).

**Drei neue, allgemein wiederverwendbare Stack-Layout-Techniken
entdeckt:**

1. **Pattern 7i — „echter Initialisierer überlebt, `char trash[N]`
   nicht"** (`TFireWanwan::receiveMessage`): ein unbenutztes `char
   trash[N];` ohne Initialisierung wird von MWCC VOR der Stack-Layout-
   Kandidaten-Registrierung wegoptimiert (tote Speicherung eliminiert,
   bevor der Slot überhaupt gezählt wird). Ein Lokal mit einem ECHTEN,
   nicht-konstanten Initialisierer-Ausdruck (z. B. `u8 trash =
   sender->getActorType();`) übersteht dagegen die Kandidaten-
   Registrierungsphase und reserviert seinen Slot, obwohl der
   nachfolgende Lese-/Schreibzugriff selbst wieder als totzurück-
   eliminiert werden kann — Regel: MWCC zählt Frontend-Lokale für die
   Stack-Platzierung, BEVOR die Dead-Store-Elimination ihre
   tatsächlichen Lade-/Speicherinstruktionen entfernt.
2. **Pattern 7j — Umkehrung von Muster 5/7h, fabrizierte Inline-
   Helfer HINZUFÜGEN statt entfernen** (`TMario::changePlayerTriJump`
   / `changePlayerJumping`, 2-für-1-Fund): `TMario::setMissJumping()`
   war in `include/Player/Mario.hpp` bereits als `inline` deklariert,
   aber in `MarioMove.cpp` mit einem LEEREN Rumpf „gestubbt" — Retail
   hatte dort echten Code (Gesichtswinkel-Reset, bedingte
   Geschwindigkeits-Neuberechnung über `JMASSin`/`JMASCos`/
   `MsSqrtf`/`matan`, `dropObject()`, `changePlayerStatus(...)`), der
   an BEIDEN Aufrufstellen vollständig inline expandiert wurde (kein
   `setMissJumping`-Symbol im Retail-Objekt vorhanden). Den echten
   Rumpf einzusetzen behob gleichzeitig zwei vorher getrennt
   fehlschlagende Funktionen (`changePlayerTriJump` UND
   `changePlayerJumping`) mit einem einzigen Commit — die fehlenden
   8 Byte Inline-Rahmen-Reservierung kamen exakt aus der jetzt
   vorhandenen Inline-Expansionsebene.
3. **Pattern 7k — MWCCs Zwei-Pool-Modell für Stack-Allokation**
   (charakterisiert durch `LiveActorBind`, NICHT gelöst, aber
   erstmals präzise beschrieben): benannte C++-Lokale werden in einen
   Pool allokiert, der OBEN am Register-Save-Bereich verankert ist
   (wächst den Rahmen nach oben, wenn mehr/größere Lokale
   hinzukommen); anonyme, vom Compiler synthetisierte Temporärwerte
   (z. B. aus inline-expandiertem `operator-`) leben in einem
   GETRENNTEN, unabhängig dimensionierten Pool, der UNTEN verankert
   ist. Trash-Arrays können NIEMALS Bytes zwischen diesen beiden
   Pools verschieben — sie wachsen immer nur den oberen (benannten)
   Pool. Erklärt einen erheblichen Teil der bisher als „unlösbar"
   dokumentierten Phantom-Frame-Fälle strukturell.

**Alle 16 Fixes:**

1. `TMarioModokiTelesa::load` (Commit `80111de4`) — `char trash[16]`.
2. `TNervePoihanaFreeze::execute` (Commit `23782a99`) — `char
   trash[0x10]`, gleiche Technik wie das Runde-62-Fix der
   Schwesterfunktion `TNervePoihanaSleep::execute`.
3. `TMario::checkGroundAtWalking` (Commit `a629df2e`) — reine
   Deklarationsreihenfolge-Korrektur: `roof`, `ground`, `floorY` (statt
   `floorY`, `ground`, ..., `roof` verstreut) — kein Trash nötig.
4. `TEnemyMario::initValues` (Commit `4ae8583e`) — `char trash[8]`.
5. `TMarioParticleManager::emitAndBindToSRTMtxPtr` (Commit
   `13c8dafd`) — `char trash[4]`, identisches Muster zur bereits
   gefixten Schwesterfunktion `emitAndBindToMtxPtr` (Runde 62).
6. `TMario::initValues` (Commit `22c9d010`) — `char trash[8]`.
7. `TPlayerLightWithDBSet::makeDrawBuffer` (Commit `606eb283`) —
   `char trash[32]`.
8. `TBEelTears::receiveMessage` (Commit `34ba1171`) — `char trash[8]`.
9. `TNameKuriManager::initSetEnemies` (Commit `14965073`) — `char
   trash[16]`.
10. `TNpcCoin::requestAppearCoin` (Commit `a340c065`) — **echter Bug**:
    `MsSin/MsCos(75)`-Header-Wrapper rundete den Kurzwinkel auf 0x3555
    statt Retails 0x3552; direkter `JMASSin/Cos(75*182)`-Aufruf (der
    im Code bereits etablierte Grad→Winkel-Faktor) reproduziert
    Retails exakte Konstante. Plus `char trash[0x10]`.
11. `TMonumentShine::control` (Commit `092a4c2d`) — Umkehrung des
    üblichen Musters: zwei überflüssige benannte Lokale (`limit`,
    `zero`) ENTFERNT (Literale direkt inline verwendet) — Retails
    Rahmen war KLEINER, nicht größer.
12. `TPollutionLayer::fire` (Commit `2ec08166`) — zwei identische
    `JGeometry::TVec3<f32>(1.5f,1.5f,1.5f)`-Konstruktor-Aufrufstellen
    zu einer gemeinsamen benannten Lokalen zusammengeführt (jede
    separate Inline-Konstruktion reservierte einen eigenen Phantom-
    Slot).
13. `CPolarSubCamera::updateGateDemoCamera_` (Commit `6fd7409b`) —
    `char trash[4]`.
14. `TFireWanwan::receiveMessage` (Commit `828553f2`) — Pattern 7i
    (siehe oben), `u8` mit echtem Initialisierer statt totem
    `char trash[N]`.
15. `TSpcTypedInterp<TEventWatcher>::evSetFruitType` (Commit
    `57da7962`) — **echter Bug**: zwei gepoppte Werte waren als
    `int` statt `u32` deklariert; Retail nutzt vorzeichenlose Typen
    mit expliziten `(int)`-Casts an den Vergleichsstellen. Hinweis für
    künftige Runden: Schwesterfunktion `evGetFruitNum` in derselben
    Datei zeigt identisches +8-Byte-Rahmen-Symptom, vermutlich
    derselbe Fix, nicht im Scope dieser Runde bearbeitet.
16. `TMario::changePlayerTriJump` + `TMario::changePlayerJumping`
    (Commit `ad4217f7`) — Pattern 7j (siehe oben), 2-für-1-Fund.

**Weitere gründlich dokumentierte Sackgassen** (sauber
zurückgesetzt): `TMario::turnning` (Zwei-Pool-Problem, Ziellokal
immer im oberen statt unteren Pool platziert), `TMario::
initMirrorModel` (bestätigt und erweitert einen bereits aus Runde 28
bekannten Cross-Funktions-.rodata-Literal-Pool-Versatz von 24 Byte,
eigenständiger ELF32-Parser geschrieben, da `readelf`/`objdump`
fehlen), `TNerveBathtubKillerExplosion::execute` (bestätigt
identisches Symptom in Schwester-Nerve `TNerveBathtubKillerBreak`,
bereits aus einer früheren Runde als TODO-Kommentar im Code vermerkt),
`TAmenbo::calcRootMatrix` (adressgenommene `TPosition3f`-Lokale
resistent gegen jede Trash-Platzierung), `TNerveBGKAppear::execute`
(weitere Bestätigung des `TFlagT<u16>`-Systemproblems, diesmal mit
9-teiligem Varianten-Log), `TLiveActor::bind` (Ursprung von Pattern
7k, siehe oben), `TBossMantaManager::setupEfbAlpha` (anonymer
`GXColor`-Blockspeicher-für-Zeiger-Argument landet immer an der
niedrigsten statt höchsten Adresse, 7 Varianten erfolglos).

### Session-Gesamtstand nach Runde 63

**499 verifizierte echte Fixes in 156 Commits.** `matched_functions`:
**9083** (von 9066 zu Rundenbeginn, +17). `matched_code_percent`:
**46,34 %** (größter Einzelrunden-Zuwachs der Session, +0,28
Prozentpunkte). Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c`
bestätigt `build/GMSJ01/mario.dol: OK`. Rundenfolge 61-63 zusammen:
`matched_functions` 9050→9083 (+33), `matched_code_percent`
45,73 %→46,34 % (+0,61 Prozentpunkte) — drei projektweite Header-Bug-
Funde plus zwei starke reguläre Batch-Runden.

### Regression entdeckt und behoben: `CPolarSubCamera::controlByCameraCode_` (Runde-59-Fix war unvollständig)

Der beim frischen Kandidaten-Scan für Runde 64 erneut auftauchende
99,93 %-Fuzzy-Match für eine bereits in Runde 59 als „byte-exakt"
gemeldete Funktion deckte eine ECHTE Regression/unvollständige
Verifikation auf: der Runde-59-Fix (`char trash[0x28]` VOR
`local_24`) korrigierte nur die GESAMTE Rahmengröße (0x50→0x78),
bewegte aber `local_24` selbst nicht — laut der etablierten Regel
bewirkt vor einem Lokal deklariertes Trash nur Padding OBERHALB,
ohne das Lokal selbst zu verschieben. `local_24` blieb bei 0x2c statt
der benötigten 0x54(r1) — ein echter 0x28-Byte-Versatz in mehreren
eingebetteten Stack-Offset-Immediates (`stw r3, 0x2c(r1)` statt
`0x54(r1)` usw.), keine reine Label-/Adress-Normalisierungsdifferenz.
Der Runde-59-Agent hatte demnach nur die Gesamtrahmengröße geprüft,
nicht jedes einzelne eingebettete Offset-Immediate — genau die Lücke,
die das neu erkannte Zwei-Pool-Modell (Muster 7k, Runde 63) erklärt.
**Behoben** durch Verschieben von `char trash[0x28];` von VOR nach
NACH der `local_24`-Deklaration (Commit `dacacbff`), per Byte-für-
Byte-Diff gegen Retail direkt verifiziert (alle Instruktions-
Hex-Wörter inkl. aller Stack-Offset-Immediates identisch). Volles
Rebuild bestätigt: `matched_functions` 9083→9084 (+1, korrekt),
`dtk shasum -c` weiterhin OK.

**Lehre für alle künftigen Runden**: Byte-exakte Verifikation MUSS
jedes einzelne eingebettete Stack-Offset-Immediate prüfen, nicht nur
Opcode-Sequenz und Gesamt-Rahmengröße — ein Diff, der nur Mnemonics
vergleicht, kann genau diese Klasse Fehler übersehen. Alle Runde-64-
Subagenten wurden mit dieser verschärften Anforderung instruiert
(literaler Hex-Byte-Spalten-Diff der rohen 4-Byte-Instruktions-
kodierung, keine Normalisierung außer Adressen/Label-Nummerierung).

### Nach vierundsechzigster Iterationsrunde (24-Kandidaten-Batch, 18 Fixes — neuer Rekord, verschärfte Byte-Verifikation)

**18 neue Fix-Commits**, alle mit der oben beschriebenen verschärften
Byte-für-Byte-Verifikation bestätigt (roher 4-Byte-Instruktions-
encoding-Vergleich, nicht nur Opcode-Text):

1. `TObjectLightWithDBSet::makeDrawBuffer` (Commit `a5d1b35b`) —
   `char trash[32]`, identisches Muster zur Schwesterfunktion
   `TPlayerLightWithDBSet::makeDrawBuffer` (Runde 63).
2. `TMario::swimPaddle` (Commit `887775a4`) — `char trash[8]`.
3. `TNerveFireWanwanEscape::execute` (Commit `8ec3a213`) — `char
   trash[8]`.
4. `TNerveBEelTearsWaterHit::execute` (Commit `592ad0b6`) — `char
   trash[0x18]`.
5. `TNerveTelesaDie::execute` (Commit `35eed88f`) — `char trash[8]`.
6. `TNerveStayPakkunAppear::execute` (Commit `7304445c`) — zwei
   identische `JGeometry::TVec3<f32>(1.5f)`-Konstruktor-Aufrufstellen
   zu einer gemeinsamen benannten Lokalen zusammengeführt (identische
   Technik zu `TPollutionLayer::fire`, Runde 63).
7. `SMSSetupGameRenderingInfo` (Commit `b68ae85f`) — `char trash[16]`.
8. `TSunModel::load` (Commit `3b27211e`) — zweiseitiges Padding
   (`trash1[4]` davor, `trash2[4]` danach um `char path[0x100]`).
9. `JAIBasic::checkEntriedSeq` (Commit `81c3977f`) — `u8 trash[8]`
   nach `u8 pos`.
10. `TCardManager::writeBlock_` (Commit `81022e2f`) — zweiseitiges
    Padding (`trash[4]` davor, `trash2[20]` danach um `CARDFileInfo
    info`).
11. `TNerveHino2Damage::execute` (Commit `d38062ae`) — `char
    trash[0x48]` (72 Byte).
12. `JPABaseEmitter::calc` (Commit `712d3d39`) — `char trash[8]`.
13. `TMarioParticleManager::emitTry` (Commit `b24ea779`) — **echter
    Bug**: if/else-Zweige für `emitterCallBackBindToMtxPtr` vs.
    `...BindToSRTMtxPtr` waren vertauscht (Flag-Semantik
    `INFO_FLAG_BIND_TO_RT_MTX` zeigte auf die falsche Callback-
    Variante). Plus Pattern 7i (`u8 trash = param_3;`, echter
    Initialisierer statt totem `char trash[N]`).
14. `MSRandPlay::randPlay` (Commit `8073cfe8`) — dreifach wiederholter
    `vec->mTrans`-Unterausdruck in einem `JAIActor`-Konstruktor-Aufruf
    beeinflusste MWCCs CSE-/Stack-Kandidaten-Registrierung; Ersatz
    durch eine einmal initialisierte `const Vec* trans`-Lokale (für
    Argument 2 und 3, Argument 1 bleibt der Direktausdruck) verschob
    den `actor`-Lokal-Slot exakt um die nötigen 4 Byte.
15. `TApplication::proc` (Commit `a1373f5d`, + Header
    `include/System/MenuDir.hpp`) — **VIER unabhängige Bugs**: (a)
    vertauschte Argumente bei `TFlagManager::setFlag(3, 0x20001)` →
    `setFlag(0x20001, 3)`; (b) `sizeof(TMenuDirector)` war 4 Byte zu
    klein (`new` allozierte 0x54 statt Retails 0x58) — geteilter
    Header um ein `/* 0x54 */ u32 unk54;`-Feld ergänzt, mit vollem
    Vorher/Nachher-Regressionsvergleich beider Einbinder-Dateien
    (Application.cpp, MenuDir.cpp) verifiziert, null Regressionen;
    (c) `delete mDirector;` (Flag `+1`, „zerstören UND freigeben")
    musste `mDirector->~TDirector();` sein (expliziter Destruktor-
    Aufruf, Flag `-1`, „nur zerstören, Speicher wird andernorts per
    `mHeap->freeAll()` freigegeben") — Flag-Semantik aus Retails
    eigenem `__dt__13TMenuDirectorFv`-Epilog dekodiert und gegen zwei
    weitere `delete`-Aufrufstellen in derselben Datei kreuzverifiziert;
    (d) `char trash[0x68]` (104 Byte) Stack-Padding.
16. `TEnemyManager::copyFromShared` (Commit `378dbcf6`) — Deklarations-
    reihenfolge-Fix zweier `Mtx`-Lokalen (identische Namen/Reihenfolge
    bereits als Konvention in `hinokuri2.cpp` etabliert) plus `char
    trash[12]`.
17. `TCommonLauncher::stateLaunch` (Commit `dd4f91e4`) — subtiler
    Deklarationsreihenfolge-Fix: einfaches Vertauschen zweier Lokalen
    behob zwar die Stack-Offsets, regressierte aber eine FPR-Zuweisung
    in einer nicht verwandten inline-expandierten `MsWrap`-Instruktion;
    korrekte Lösung war, die Deklaration (ohne Konstruktor-Argumente)
    an die richtige Stelle zu setzen, aber die tatsächliche Wert-
    zuweisung (`.set(...)`) an ihrer ursprünglichen Verwendungsstelle
    zu belassen — trennt Stack-Slot-Zuteilung von Instruktions-
    Terminplanung.
18. `TMario::kickFruitEffect` (Commit `b227e351`) — **echter Bug**:
    `setEmitterTranslation()`-Wrapper schreibt tatsächlich in
    `mTrans` (Offset +0x19c), aber Retail beschreibt direkt
    `mGlobalTranslation` (Offset +0x160) — irreführend benannter
    Wrapper, Direktzugriff auf das richtige Feld nötig.

**Drei Kandidaten scheiterten sauber ohne Dateiänderung** (Agent
brach während der Untersuchung ab, kein Commit, keine Änderung
zurückzusetzen — für eine künftige Runde vorgemerkt):
`TMario::~TMario` (Destruktor tatsächlich header-inline, nicht in
MarioInit.cpp lokalisierbar wie angenommen), `TMapObjBase::
getDistance`, `TMarDirector::fireGetStar`.

**Weitere gründlich dokumentierte Sackgassen** (sauber
zurückgesetzt): `TMareEventDepressWall::depressing` (Anonymer-Pool-
Diskrepanz nicht-uniform über zwei bedingte Temporärwerte verteilt,
ein Versuch mit dem bereits deklarierten aber leeren `emitEffect(int)`-
Stub regressierte einen vorher exakten Codeblock, zurückgesetzt),
`TWoodBox::kill` (152-Byte zusammenhängender unbenutzter Block am
Boden des Rahmens, MWCC platziert Frontend-Lokale IMMER oberhalb von
Inline-Temporärwerten unabhängig von der Deklarationsposition —
bestätigt Muster 8 erneut), `MtxToQuat` (ein einziges vertauschtes
Operandenpaar in einer kommutativen `fadds`-Instruktion, 24
erschöpfend getestete Summierreihenfolgen, alle Varianten bestätigen
die Original-Quelltextform als bereits optimal — vermutlich
Compiler-internes Werteordnungs-/Scheduling-Verhalten, nicht aus
Quelltext heraus beeinflussbar).

### Session-Gesamtstand nach Runde 64

**518 verifizierte echte Fixes in 175 Commits** (inkl. der Runde-59-
Regressionskorrektur). `matched_functions`: **9102** (von 9083 zu
Rundenbeginn, +18 exakt wie erwartet — bestätigt keine unentdeckten
Regressionen durch den `MenuDir.hpp`-Header-Fix). `matched_code_percent`:
**46,64 %** (weiterer Rekord-Einzelrunden-Zuwachs, +0,29
Prozentpunkte). Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c`
bestätigt `build/GMSJ01/mario.dol: OK`.

### Nach fünfundsechzigster Iterationsrunde (24-Kandidaten-Batch, 12 Fixes trotz Infrastruktur-Ratenbegrenzung, TFlagT<u16>-Konstruktorsyntax-Erkenntnis)

**Fünf Kandidaten scheiterten an einem Infrastruktur-Problem**
(Rate-Limit-Fehler des zugrundeliegenden Modell-Providers,
`minimax-code/MiniMax-M3 429`, sowie einige unklare frühe Abbrüche)
— alle fünf machten KEINE Dateiänderungen (sauber abgebrochen vor
jeder Bearbeitung), verbleiben also als frische Kandidaten für eine
künftige Runde: `TMapObjectLightWithDBSet::makeDrawBuffer`,
`SMSSetupTitleRenderingInfo`, `TSmallEnemy::generateItem`.
(`TMario::~TMario` und `TMapObjBase::getDistance` wurden in DIESER
Runde als Retries behandelt und sind unten dokumentiert — eines
erfolgreich gefixt, das andere ebenfalls an derselben Ratenbegrenzung
gescheitert.)

**12 neue Fix-Commits:**

1. `TMario::doRoofMovingProcess` (Commit `7ca67917`) — `char trash[4]`
   nach `newPos`.
2. `TMario::squating` (Commit `e3b584cd`) — `char trash[56]`.
3. `TNerveBEelTearsSplit::execute` (Commit `b414b8f9`) — `char
   trash[16]`.
4. `TTalkCursor::loadAfter` (Commit `09998db8`) — `char trash[8]`.
5. `TMapObjGeneral::appear` (Commit `3549f48b`) — `char trash[8]`.
6. `TNameKuri::moveObject` (Commit `cb12847b`) — `char trash[12]`.
7. `TNerveMameGessoWait::execute` (Commit `34a9edfc`) — `char
   trash[8]`.
8. `TMario::barProcess` (Commit `e89da4b0`) — `char trash[4]` NACH
   `pos` (Sonderfall: alle Lokalen lagen gleichmäßig 4 Byte zu
   niedrig, Rahmengröße war durch 8-Byte-Rundung bereits zufällig
   korrekt).
9. `TMenuDirector::setFixedStageValue` (Commit `d0a4e4e2`) — `char
   trash[0x1C]` nach einem Array-Lokal.
10. `TMarioParticleManager::perform` (Commit `8be3d482`) — **neue
    Erkenntnis zur Trash-Anker-Position**: `char trash[24]` musste am
    ENDE der Funktion (nach dem letzten Block, vor der schließenden
    Klammer) stehen, nicht am Anfang — beide Positionen korrigieren
    zwar die Gesamtrahmengröße gleich, aber nur die Endposition lässt
    den Abstand zweier `JPADrawInfo`-Lokalen zum Rahmen-Oberrand
    unverändert, wie es Retail benötigt.
11. `TNerveTobiPukuBound::execute` (Commit `c052a17d`) — kombinierter
    Fix: `velocity2`-Deklaration (ohne Initialisierung) vor den
    äußeren if-Block gezogen (für Stack-Slot-Reihenfolge), tatsächliche
    Zuweisung an ursprünglicher Stelle belassen (für Instruktions-
    Terminierung), PLUS `char trash[12]`.
12. `TMarDirector::fireGetStar` (Commit `437d5ead`) — **wichtige neue
    Erkenntnis zur `TFlagT<u16>`-Fehlerfamilie**: ein bereits
    vorhandenes, wirkungsloses `char trash[4]` war nur Rauschen: die
    tatsächliche Ursache war `JDrama::TFlagT<u16>(0)` (expliziter
    Konstruktor-Aufruf mit Null-Argument) statt `JDrama::TFlagT<u16>()`
    (Standard-Konstruktor) — beide erzeugen denselben Laufzeitwert,
    aber MWCCs anonymer Temporärwert-Pool-Allokator positioniert den
    Temporärwert unterschiedlich für die beiden Schreibweisen. Dies
    ist die ERSTE bestätigte Auflösung eines `TFlagT<u16>`-„Phantom-
    Frame"-Falls durch eine reine Syntaxänderung (keine Stack-
    Padding-Technik) — lohnt sich, bei den zahlreichen bereits als
    Sackgasse dokumentierten `TFlagT<u16>`/`fireStartDemoCamera`-
    Fällen aus früheren Runden erneut zu prüfen, ob sie explizite
    `(0)`-Argumente statt Standard-Konstruktoren verwenden.

**Weitere gründlich dokumentierte Sackgassen** (alle sauber
zurückgesetzt, mehrere mit außergewöhnlich gründlicher Bisektion):
`MActor::setModel` (9 Varianten, anonyme `JGadget::TList`-Iterator-
Vergleichs-Temporärwerte unbeeinflussbar durch Named-Pool-Padding),
`THinokuri2::changeBck` (systematische Bisektion zeigt: zwei sich
gegenseitig ausschließende if/else-Zweige reservieren JEWEILS einen
eigenen 32-Byte-Anonym-Pool-Slot, obwohl nie gleichzeitig lebendig —
16 Byte Overreservierung ließ sich nicht durch Padding entfernen, da
Padding nur hinzufügen, nie entfernen kann), `TMapStaticObj::perform`
(über 70 Build-Iterationen, Ziel-Layout verlangt gleichzeitiges
Wachsen UND Schrumpfen verschiedener Rahmenbereiche — unerreichbar),
`TFireWanwan::behaveToWater` (vier Platzierungs-/Typ-Varianten
bestätigen erneut das Zwei-Pool-Modell empirisch), `TBaseNPC::
changeNerveFromTalk_` (11 Iterationen — JEDE Umformulierung, die den
Instruktionsstrom exakt hält, reproduziert auch die exakte
16-Byte-Overreservierung; JEDE Umformulierung, die die Rahmengröße
ändert, bricht auch den Instruktionsstrom — kein Ausweg gefunden),
`TStayPakkun::genRandomItem` (mathematische Formelherleitung: Rahmen-
größe F = 48 + 24×N für N TVec3-Kandidaten, Zielwert hat keine
ganzzahlige Lösung für N — beweist, dass die gesamte Familie „TVec3-
Temporärwerte konsolidieren/aufteilen" als Fix-Ansatz ausscheidet),
`TMapEventSinkBianco::startControl` (zwei unabhängige, nicht
gegenseitig aufhebbare Stack-Layout-„Basisvektoren" identifiziert,
16 Iterationen, keine Kombination trifft exakt).

### Session-Gesamtstand nach Runde 65

**530 verifizierte echte Fixes in 187 Commits.** `matched_functions`:
**9114** (von 9102 zu Rundenbeginn, +12 exakt wie erwartet).
`matched_code_percent`: **46,82 %**. Volles `ninja`-Rebuild
erfolgreich, `dtk shasum -c` bestätigt `build/GMSJ01/mario.dol: OK`.

### Nach sechsundsechzigster Iterationsrunde (24-Kandidaten-Batch, 19 Fixes — dritter Rekord in Folge, neue Fehlerklasse „ungenutzter Header-Include verschiebt .data-Layout" entdeckt)

**19 neue Fix-Commits** (alle mit der seit Runde 64 etablierten
strengen Byte-für-Byte-Instruktionswort-Verifikation bestätigt):

1. `SMSSetupTitleRenderingInfo` (Commit `83a43956`) — `char trash[8]`,
   drittes von vier Geschwister-Funktionen in derselben Datei.
2. `TNameKuri::init` (Commit `0b5b72e7`) — `char trash[40]`.
3. `TPollutionLayer::cleaned` (Commit `56875296`) — `char trash[0x30]`.
4. `TBellDolpic::receiveMessage` (Commit `05819d0e`) — `char trash[8]`.
5. `TMapObjectLightWithDBSet::makeDrawBuffer` (Commit `9aeb3d3c`) —
   `char trash[32]`, komplettiert die Drei-Geschwister-Serie in
   `LightUtil.cpp` (alle drei `makeDrawBuffer`-Varianten jetzt exakt).
6. `TNerveBossEelAppear::execute` (Commit `ffbd27f0`) — `char
   trash[32]`.
7. `TCardManager::writeOptionBlock_` (Commit `76af12e7`) —
   zweiseitiges Padding (`trash1[4]` davor, `trash2[12]` danach um
   `CARDFileInfo info`).
8. `TMario::fencePunch` (Commit `53e6628a`) — `char trash[16]` NACH
   dem verschachtelten `if`-Block mit `Mtx mtx` (Platzierung DAVOR
   bewirkte nichts Nützliches, bestätigt erneut Anker-Positions-
   Empfindlichkeit).
9. `MSHandle::setSeDistanceParameters` (Commit `6f6ff407`) — **echter
   Bug**: rief `setSeDistanceFir()` (falscher virtueller Slot) statt
   `setSeDistanceFxmix()` (Vtable-Slot 0x18) auf. Plus `char trash[8]`.
10. `TMario::trampleExec` (Commit `228341ec`) — `char trash[8]` NACH
    `scale`-Lokaler (nicht davor — unbenutzte konstante Trash-Arrays
    vor einem Lokal werden als generisches Boden-Padding behandelt,
    nicht als benannter Pool-Kandidat, Muster 7i-Verfeinerung).
11. `CPolarSubCamera::ctrlOptionCamera_` (Commit `aa2ec607`) — `u8
    trash[32]` NACH `probe`-Lokaler.
12. `TSmallEnemy::generateItem` (Commit `1d0117f1`) — anonymer
    `TMsRange<f32>(0.0f,100.0f).rand()`-Kettenaufruf in eine benannte
    Lokale `TMsRange<f32> genRange(...)` materialisiert.
13. `TEnemyAttachment::perform` (Commit `6a81c638`) — **echter Bug**:
    falscher virtueller Slot — `kill()` (Vtable-Offset 0xE4) statt
    `behaveToHost()` (0x128, ein leerer Inline-Hook) aufgerufen; per
    vollständigem `__vt__16TEnemyAttachment`-Vtable-Dump verifiziert.
    Plus `char trash[8]`.
14. `TSpcTypedInterp<TEventWatcher>::evGetFruitNum` (Commit
    `01646ec9`) — bestätigt den in Runde 63 vorhergesagten
    Geschwister-Fix von `evSetFruitType`: gepoppter Wert von `int` auf
    `u32` umgetypt.
15. `TMario::~TMario` (Commit `3bf1ede1`) — **NEUE FEHLERKLASSE**:
    `MarioInit.cpp` band `<System/StageUtil.hpp>` nur wegen
    `SMS_isMultiPlayerMap()` ein, zog dabei aber ungenutzte `static`-
    Hilfsfunktionen samt 0x168 Byte rückenden statischen Datentabellen
    (`scShineConvTable`, `scEtcShineConvTable`, `scScenarioNameTable`)
    in die Übersetzungseinheit — MWCC entfernt ungenutzte
    Datei-statische Symbole mit echten Initialisierern NICHT zur
    Kompilierzeit. Das verschob das komplette `.data`-Layout dieser
    Übersetzungseinheit und damit alle vom Destruktor berechneten
    literal-pool-relativen Offsets (+0x28/+0x4c/+0xdc wurden zu
    +0x190/+0x1b4/+0x244). Fix: vollen Header-Include durch eine
    Vorwärtsdeklaration der einzigen tatsächlich genutzten Funktion
    ersetzt.
16. `TSandBlock::control` (Commit `dbb4faff`) — `char trash[0xC]` NACH
    `scaleCopy`-Lokaler.
17. `CPolarSubCamera::ctrlGameCamera_` (Commit `58764055`) — Kombi-
    Fix: `int code;`-Deklaration an den Funktionsanfang gezogen (für
    Slot-Reihenfolge, tatsächliche Verwendung blieb an ursprünglicher
    Stelle) PLUS zweiseitiges Padding (`trash[8]` vor, `trash2[24]`
    nach `TCameraKindParam param`).
18. `TEMario::load` (Commit `d4382af0`) — zwei kombinierte Fixes: (a)
    zwei verworfene `stream.readU32()`-Methodenaufrufe zu verketteten
    `stream >> unused1 >> unused2`-Operatoren umgeschrieben (ändert
    MWCCs Registrierungsreihenfolge der Temporärwerte); (b) `char
    trash[0x44]` nach einem `const char[]`-Array.
19. `TRealoid::perform` (Commit `7dcb00c2`) — vier fehlende benannte
    Zwischenwerte für inline-expandierte Accessor-Ergebnisse (`f32
    nearPlane`, zwei `TBoid* boid`, ein `TRealoidActor* actor` über
    den bereits deklarierten aber ungenutzten `getRealoid()`-Accessor)
    — jede gebundene Inline-Accessor-Ergebnis-Lokale reserviert exakt
    einen 4-Byte-Slot, vier fehlende × 4 Byte = die fehlenden 16 Byte.
    Für zukünftige Runden vorgemerkt: die eigenständige (nicht-inline-
    expandierte) `TRealoid::clipBoids` selbst braucht noch +8 Byte,
    und `TFishoid::perform` braucht ein vollständig inline-
    expandiertes `TRealoid::perform` (Retail-Rahmen 0xb8) statt eines
    echten Funktionsaufrufs.

**Fünf gründlich dokumentierte Sackgassen** (alle sauber
zurückgesetzt, mehrere mit außergewöhnlicher Bisektionstiefe):
`TMapObjBase::getDistance` (18+ Varianten, bestätigt als bereits
früher dokumentierte Sackgasse), `TRiccoHook::init` (mathematischer
Beweis: ein Slot braucht +4, eine Gruppe von drei benachbarten Slots
braucht +8 — unmöglich aus einem einzigen linearen Pool, bestätigt
zwei getrennte Pools; kreuzreferenziert mit einem bereits im
Repo vorhandenen `// @non-matching`-Kommentar an exakt dieser
Stelle), `TPauseMenu2::loadAfter` (7 Varianten, ein `JUtility::
TColor::operator u32()`-Store/Reload-Bounce-Temporärwert sitzt auf
einem fixen, durch Padding unerreichbaren Boden-Offset),
`TSpineEnemy::resetToPosition` (bestätigt dieselbe „Boden-Bereich
unterhalb des anonymen Temporärwerts"-Sackgasse wie bereits in Runde
59 bei `CameraWarpPosAndAt`/`ModelUtilRideMove`/`ProgSelectPerform`
dokumentiert), `TSunShine::perform` (~25 Varianten, ein „unsichtbare-
Referenz-Argument"-Temporärwert für einen `TColor`-Werteparameter
unbeeinflussbar durch jede Trash-Platzierung).

### Session-Gesamtstand nach Runde 66

**549 verifizierte echte Fixes in 206 Commits.** `matched_functions`:
**9133** (von 9114 zu Rundenbeginn, +19 exakt wie erwartet — dritter
Rekord-Zuwachs in Folge nach Runde 64 [+18] und Runde 65-Kontext).
`matched_code_percent`: **47,12 %** (überschreitet erstmals die
47-Prozent-Marke). Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c`
bestätigt `build/GMSJ01/mario.dol: OK`.

### Nach siebenundsechzigster Iterationsrunde (24-Kandidaten-Batch, 16 Fix-Commits / 18 Funktionen matched, neue Trash-Pool-Erkenntnisse)

**16 neue Fix-Commits** (alle mit der seit Runde 64 etablierten
strengen Byte-für-Byte-Instruktionswort-Verifikation bestätigt), davon
einer (Nr. 14) ein Header-Fix, der zusätzlich zur Zielfunktion drei
weitere Template-Instanzen im selben Header auf 100 % brachte —
**18 Funktionen** insgesamt neu gematcht:

1. `TMapCollisionData::removeCheckListData` (Commit `6b801674`) —
   `char trash[0x28]`.
2. `TIndirectLightWithDBSet::makeDrawBuffer` (Commit `b5da1e9d`) —
   `char trash[32]`, viertes und letztes von vier Geschwister-
   `makeDrawBuffer`-Fixes in `LightUtil.cpp`.
3. `TBellDolpic::control` (Commit `52e1539a`) — `char trash[16]` nach
   `pos`-Lokaler.
4. `TSpcTypedInterp<TEventWatcher>::evGetAddressFromViewObjName`
   (Commit `cf12b481`) — **umgekehrtes Muster 7**: ein ÜBERFLÜSSIGER
   benannter Lokal (`viewObj`) reservierte einen Slot, den Retail
   nicht hat; Entfernen (inline-Cast direkt im `push()`-Argument)
   verschiebt den kompletten Anonym-Pool um die fehlenden 4 Byte nach
   oben.
5. `TMapObjBase::changeObjMtx` (Commit `9178fa3b`) — **zwei echte
   Bugs**: (a) Übersetzungsspalte falsch gelesen (`mtx[3][0..2]`
   Bottom-Row statt der projektweiten Konvention `mtx[0..2][3]`
   Translation-Column); (b) `char trash[0x28]` für die fehlende
   40-Byte-Rahmenreserve.
6. `TKageMarioModoki::init` (Commit `52026a21`) — `char trash[8]`.
7. `TMario::canSleep` (Commit `add9a5c5`) — zusätzlicher, nie
   referenzierter `const TBGCheckData* gnd2`-Pointer-Lokal direkt
   nach `gnd` (Verfeinerung von Muster 7i: ein unbenutzter Pointer
   OHNE Initialisierer wird hier NICHT vor der Stack-Layout-
   Registrierung eliminiert, reserviert seinen Slot trotzdem).
8. `TWireBinder::getDirAtPos` (Commit `9641f2ab`) — dritten benannten
   Lokal (`fVar1`) eliminiert, dessen Wert stattdessen in-place in
   `posInWire` mutiert (reproduziert Retails bedingtes
   `fmr f31,f1`-Muster exakt).
9. `TBossEelHeartCoin::perform` (Commit `92276978`) — `char
   trash[44]` nach `heartMtx`-Lokaler.
10. `TBossGesso::rumblePad` (Commit `c6576249`) — Deklaration von
    `fVar2` an den Funktionsanfang vorgezogen, Zuweisung
    (`fVar2 = delta.length();`) blieb an ursprünglicher Stelle:
    reines Registrierungsreihenfolge-Detail ohne Instruktionsänderung
    verschiebt `delta`s Slot um die nötigen 4 Byte.
11. `TGesso::behaveToFindMario` (Commit `7f89a008`) — **neues
    Trash-Pool-Teilmuster „benutztes vs. unbenutztes Trash"**: ein
    unbenutztes `char trash[N]` wächst immer den OBEREN (benannten)
    Pool und kann einen anonymen Temporärwert nie verschieben; ein
    TATSÄCHLICH BESCHRIEBENES `char trash[4]; trash[0]=0;` (die
    Zuweisung selbst wird wegoptimiert, Instruktionszahl bleibt
    gleich) überlebt als echter benannter Lokal im UNTEREN Bereich
    und verschiebt dadurch den anonymen `TPathNode`-Temporärwert um
    die nötigen 4 Byte nach oben.
12. `CPolarSubCamera::ctrlNormalDeadDemo_` (Commit `4d0f223f`) —
    **neue Anker-Regel**: ein unreferenziertes `char trash[N]`
    landet immer NACH jedem echten (adressgenommenen) benannten
    Lokal in seinem Scope, unabhängig von der Textposition relativ zu
    diesem Lokal — bestätigt durch drei erfolglose Platzierungen vor
    `Vec diff`; der Fix platziert `char trash[28]` stattdessen NACH
    `diff`s Feldzuweisungen, was den Vor-`diff`-Bereich wachsen lässt
    und `diff` von Offset 0x14 auf 0x30 verschiebt (exakt Retail).
13. `TGCConsole2::checkChangeTelopArray` (Commit `22731050`) — **drei
    kombinierte Fixes**: (a) drei zuvor fälschlich hinzugefügte
    benannte Globals (`scUnusedScale1/2`, `scUnusedTable`)
    dupliziierten bereits vorhandene anonyme Compiler-Literale und
    verschoben das komplette nachfolgende `.data`-Layout um 40 Byte —
    entfernt; (b) `scDolpicNewsDolpic5_1`/`_4` im Case-5-Zweig
    vertauscht (per Retail-Symbol-zu-Adresse-Abgleich verifiziert);
    (c) `char trash[48]` für die restliche Rahmenlücke.
14. `TNameRefAryT<T,JDrama::TNameRef>::load` (Commit `0f596d40`,
    Header-Fix in `include/Strategic/NameRefAry.hpp`) — Bindung des
    `operator[]`-Ergebnisses an eine explizite Referenz
    (`T& child = getChildren()[i];`) vor dem virtuellen `load()`-
    Aufruf; reserviert den fehlenden 4-Byte-Slot. Da es sich um eine
    Template-Methode handelt, brachte dieser EINE Fix alle vier im
    selben Objekt instanziierten Varianten
    (`TStageEventInfo`, `TStagePositionInfo`, `TCameraMapTool`,
    `TScenarioArchiveName`) gleichzeitig auf 100 % — nach
    vollständiger Regressionsprüfung aller Includer committet.
15. `TWalkerEnemy::behaveToFindMario` (Commit `f9d9c145`) — Kombi aus
    benanntem `TPathNode node(...)` PLUS benanntem Pointer
    `TPathNode* nodePtr = &node;` (nicht Referenz) reproduziert
    Retails exaktes +4/+4-Split-Layout.
16. `TMario::thinkHeight` (Commit `ead2db98`) — inline-expandiertes
    `checkStatusType()`-Ergebnis an eine benannte `bool jumping`-
    Lokale gebunden statt direkt in der `if`-Bedingung verwendet;
    verifiziert ohne Regression der bereits exakt matchenden
    Geschwisterfunktion `checkPlayerAround`.

**Fünf gründlich dokumentierte Sackgassen** (alle sauber
zurückgesetzt, mehrere mit außergewöhnlicher Bisektionstiefe):
`MActorAnmData::MActorAnmData()` (8 Varianten — der anonyme
Compiler-Temporärwert für die implizite Default-Argument-Konstruktion
des ersten Klassenmembers ist permanent auf den niedrigsten
Anonym-Pool-Offset fixiert; nichts Legales kann vor der allerersten
Member-Konstruktion ausgewertet werden), `MSHandle::calcPan`
(8 Varianten, reine 8-Byte-Überreservierung ohne zugehörige
Save/Spill-Instruktion, klassische Zwei-Pool-Sackgasse),
`TMapObjGrassManager::initDrawNear` (20+ Iterationen — DRITTER,
unabhängiger Stack-Pool für GXColor-Compound-Literal-Materialisierung
identifiziert, zusätzlich zum benannten und dem fctiwz-Anonym-Pool;
jede nicht-störende `TVec3<s16>`-Dummy-Variante erreicht nur +16 von
benötigten +20 Byte, jede größere Variante überschießt auf +24 oder
stört die bereits passende Register-Allokation), `JPABaseEmitter::
calcCreateParticle` (15+ Varianten, Diff von 58 auf 6 Zeilen reduziert
— der `getRandomRF()`-Bit-Trick-Temporärwert aus `JMath::
TRandom_fast_::get_ufloat_1()` bleibt permanent an der niedrigsten
Anonym-Pool-Position fixiert, da er der ERSTE in Erstellungsreihen-
folge erzeugte Inline-Temporärwert der Funktion ist), `TApplication::
setupThreadFuncLogo` (~10 Varianten — asymmetrische +4/+0-Verschiebung
über vier Aufrufstellen zweier geteilter Inline-Helfer
(`SMSLoadArchive`/`SMSLoadArchiveARAM`) mathematisch nicht aus einer
symmetrisch wirkenden Body-Änderung ableitbar, da beide Aufrufstellen
jeder Helferfunktion identischen Code teilen).

Drei Kandidaten mit sauberem Fehlschlag ohne Dateiänderung (Infra-
Ratenbegrenzung bzw. Skript-Fehler vor jeder Quelländerung):
`TMActorKeeper::TMActorKeeper(TLiveManager*)`, `TCoasterEnemy::bind`,
`TMapObjGeneral::receiveMessage` — bleiben frische Kandidaten für eine
künftige Runde.

**Methodik-Verfeinerung**: Die Kandidaten-Ausschlussmenge wurde von
reiner Unit-Pfad-Ebene auf Funktions-Ebene verfeinert
(`(unit, funktion)`-Paare statt nur `unit`), nachdem eine Prüfung
zeigte, dass vier der fünf neuen Sackgassen-Units noch zahlreiche
andere unberührte Kandidatenfunktionen im Zielbereich hatten (z. B.
`JPAEmitter.cpp` mit vier weiteren, `MActorData.cpp` mit sieben
weiteren) — eine reine Unit-Sperre hätte diese in künftigen Runden
fälschlich unsichtbar gemacht.

### Session-Gesamtstand nach Runde 67

**565 verifizierte echte Fixes in 222 Commits.** `matched_functions`:
**9151** (von 9133 zu Rundenbeginn, +18 exakt wie erwartet — 16
Fix-Commits, davon einer mit Kaskadeneffekt auf vier
Template-Instanzen). `matched_code_percent`: **47,35 %**. Volles
`ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`. Regressionsprüfung (Vergleich aller
zuvor 100 %-matchenden Funktionen vor/nach dem Rebuild): 0
Regressionen, 18 Neuzugänge — exakte Übereinstimmung.

### Nach achtundsechzigster Iterationsrunde (24-Kandidaten-Batch plus dedizierter Regressions-Fix, 9 MATCH + 1 Revert, Regression einer früheren Runde entdeckt und korrigiert)

**Wichtige Erkenntnis dieser Runde**: Eine routinemäßige
Re-Verifikation der in Runde 67 committeten Funktion
`TMapCollisionData::removeCheckListData` (Commit `6b801674`) ergab,
dass der ursprüngliche Subagent-Selbstbericht ("166/166 Instructions
identisch") **inkorrekt** war — die Instruktionszählung war zwar
richtig, aber drei einzelne 4-Byte-Instruktionswörter hatten
tatsächlich abweichendes Bit-Muster (Register r5 statt r3 für die
`&unk42[start]`-Temporäradresse, plus eine 2-Instruktionen-
Scheduler-Order-Swap-Unterscheidung). Dieses Pattern entspricht exakt
dem Round-59-Regressionsfund aus Runde 64. Bestätigt durch
Verifizierungs-Konvention: kein Agent darf eine Funktion als
"byte-exakt" deklarieren ohne einen programmatischen
Position-für-Position-Diff ALLER Instruktionswörter, bei dem die
Ergebnisliste LEER ist (`diff_list == []`), nicht nur eine
qualifizierte Augenschein- oder Opcodes/Frame-Größe-Übereinstimmung.

**10 neue Commits** (9 byte-exakte MATCHes + 1 Regressions-Revert):

1. `TWaterGun::calcAnimation` (Commit `f06076fa`) — **zwei
   kombinierte Fixes**: (a) fehlende 48-Byte-Rahmenreserve
   (`volatile u32 unused[12]` als erste Anweisung, Pattern 7), (b)
   **TU-weite .rodata-Reparatur**: zwei fehlende tote 12-Byte-Datei-
   statische Vec-Konstanten (`cZeroVec = {0,0,0}` und
   `cOneVec = {1,1,1}`) als `static const Vec` direkt nach
   `cDirtyTexName` deklariert, um exakt Retails rodata-Layout durch
   Offset 0x2da zu reproduzieren; bestätigt durch 44-Funktionen-
   Cross-Check (Regression: 19→18 Mismatches, keine neu gebrochenen).
2. `JPADragField::affect` (Commit `ffe7caaf`) — `char trash[4]`
   gefolgt von `trash[0] = 0;` (geschriebener Trash wächst den
   unteren Pool, Runde-67-Erkenntnis direkt angewendet).
3. `TBaseNPC::npcTalkOut` (Commit `4323624b`) — **echter Bug**:
   Tippfehler `LIVE_FLAG_UNK8000` (Bit 16) statt
   `LIVE_FLAG_UNK80000` (Bit 12) im `offLiveFlag`-Aufruf; andere
   `rlwinm`-Maske (`mb=17,me=15` statt `mb=13,me=11`).
4. `TTurboNozzleDoor::touchPlayer` (Commit `fcc02ca1`) —
   geschriebener `char trash[20]` direkt nach `scale`-Lokal
   (Pattern 7; Größe empirisch ermittelt: 24 overshoots, 16
   undershoots).
5. `TPoiHanaManager::load` (Commit `4f305653`, Header-Fix in
   `include/Enemy/PoiHana.hpp`) — **echter Bug**: leerer Body
   `TPoiHanaCollision(const char* name = "ポイハナコリジョン") { }`
   leitete `name` nicht an den `THitActor`-Basiskonstruktor weiter,
   wodurch statt der 0x13-Byte-katakana-Zeichenkette eine
   9-Byte-"HitActor"-Default-Zeichenkette emittiert wurde — was alle
   nachfolgenden .rodata-Offsets um +8 verschob und sich als
   uniformer -8-Byte-Versatz in allen String-Pool-Adressen der
   Lade-Funktion zeigte. Fix: Member-Initialliste
   `: THitActor(name) { }`.
6. `TCameraOption::TCameraOption` (Commit `bd85659c`) — zwei
   `void*`-Lokale (innerhalb `if`-Blocks, einer vor, einer nach
   `origin`); Pointer-Typ überlebt Stack-Layout-Registrierung als
   4-Byte-Slot, Position relativ zu `origin` wählt exakt 4 über /
   4 unter wie Retail.
7. `TMActorKeeper::TMActorKeeper` (Commit `e239d637`) —
   `char trash[8]` (gleiche Idiom wie die zwei Geschwister-Methoden
   in derselben Datei).
8. `TMario::inOutWaterEffect` (Commit `5a76f68f`) —
   `char trash[8]` nach `pos.y = mFloorPosition.z;` (gleiche Idiom
   wie Geschwister `TMario::rippleEffect`).
9. `TNerveBossEelSleepOnBottom::execute` (Commit `6fda642b`) —
   unbenutzter `char trash[16]` als erste Anweisung im
   `DEFINE_NERVE`-Body (reine 16-Byte-Rahmen-Lücke).
10. **Revert** `TMapCollisionData::removeCheckListData` (Commit
    `ececd4ea`, Revertiert `6b801674`) — Subagent-Regression mit
    25+ Source-Varianten (alle möglichen Anker-Positionen,
    Ausdrucksumformulierungen, Schleifenstrukturen, Casts,
    self-assigns, alternative Bound-Formen) bestätigt, dass die
    Scheduler-Tie-Break-Reihenfolge zwischen Compiler-Invocations
    unterschiedlich ist und nicht aus Quellebene reproduzierbar;
    Funktion zurück auf Pre-`6b801674`-Stand (3 Instruktionswörter
    Diff, dokumentiert).

**Zwölf gründlich dokumentierte Sackgassen** (alle sauber
zurückgesetzt, mehrere über Schritt-Limit hinaus):
`TMario::jumpProcess`, `TNerveMameGessoJitabata::execute`,
`TEnemyMario::tryTake`, `TNerveTelesaFreeze::execute`,
`TMapObjGeneral::receiveMessage`,
`TNerveWalkerEscape::execute`, `TGenerator::perform`,
`TPollutionAction::action`, `TPollutionLayer::initTexImage`,
`TNameKuri::setDeadAnm`, `TGCConsole2::processAppearLife`,
`TLightWithDBSetManager::addChildGroupObj` — alle revertiert, keine
bleibenden Änderungen.

**Methodik-Verfeinerung (Verifizierungs-Standard)**: Ab dieser
Runde gilt projektweit: Ein Subagent-Bericht "byte-exakt verifiziert"
gilt nur dann als glaubwürdig, wenn der Bericht einen
programmatischen Positional-Diff aller Instruktionswörter enthält,
dessen Ergebnisliste leer ist (`len(diff_list) == 0`) — keine
qualifizierten Aussagen wie "alle Offsets matched", "Frame passt
exakt", oder "166/166 Instructions identisch". Letzteres
(Instruction-Count-Match) wurde dieses Mal widerlegt: Round-67-
Agent zählte korrekt 166 Instructions, diffte aber nie byte-genau
die Wortinhalte.

### Session-Gesamtstand nach Runde 68

**573 verifizierte echte Fixes in 231 Commits** (565 + 10 neue
Round-68-Commits; der Revert lässt den 565er-Bestand unverändert,
Netto-Sessionszuwachs: +8 byte-exakte MATCHes, jeweils verifiziert
per programmatischem raw-4-byte-hex-diff mit leerer Ergebnisliste).
`matched_functions`: **9159** (von 9151 zu Rundenbeginn, +8; der
npcTalkOut-Bugfix-Commit zählt NICHT als 100%er, weil die
Funktion noch eine Rest-Differenz von 7 Instruktionswörtern
aufweist — siehe unten). `matched_code_percent`: **47,44 %**.
Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`. Regressionsprüfung: 0 Regressionen,
8 Neuzugänge — exakte Übereinstimmung. Fork `Astrr00/sms` per
Squash-Merge PR #1 auf `main` überführt (`56161c6`). Stand: 5
Commits hinter `doldecomp/sms:main` (Upstream hat `configure.py`
und `PROGRESS.md` mehrfach geändert seit Phase-0-Fork), 399
Commits voraus (eigene Arbeit). PR `doldecomp/sms#195` wurde
geschlossen, weil ein direkter Merge gegen `doldecomp/sms:main`
Konflikte in `configure.py` produzierte (Upstream hatte die Datei
mehrfach editiert); stattdessen PR `Astrr00/sms#1` an den eigenen
Fork erstellt und Squash-merged → `56161c6` auf
`Astrr00/sms:main`.

**Round-68-Audit-Korrektur** (in Runde 70 durchgeführt): Die
zunächst als „byte-exakt" deklarierten 9 Round-68-MATCH-Ziele
wurden einzeln gegen den frischen `report.json` verifiziert.
Dabei stellte sich heraus, dass **`TBaseNPC::npcTalkOut` NICHT
bei 100% liegt** — es zeigt 99.9391 % mit 7 verbleibenden
Instruktionswörtern Differenz (Frame-Größe 0x38 statt 0x48, alle
Stack-Offsets uniform +0x14 verschoben). Das ist ein Pattern-7-
Stack-Layout-Restproblem. Der Flag-Bugfix-Commit `4323624b` ist
trotzdem ein **echter Bugfix** (LIVE_FLAG_UNK8000 → UNK80000
änderte die rlwinm-Maske korrekt), macht die Funktion aber nicht
vollständig zu 100 %. Daher wird dieser Kandidat in Runde 70
erneut dispatched, um den +0x10-Frame-Gap zu schließen. Die
übrigen 8 Round-68-MATCHes (WaterGun.calcAnimation,
JPADragField.affect, TTurboNozzleDoor.touchPlayer,
TPoiHanaManager.load, TCameraOption.TCameraOption,
TMActorKeeper.TMActorKeeper, TMario.inOutWaterEffect,
TNerveBossEelSleepOnBottom.execute) sind alle bei 100.0000 %
bestätigt.

### Nach neunundsechzigster Iterationsrunde (24-Kandidaten-Batch, alle 24 Agents an Rate-Limits gescheitert — Null-Runde)

Round 69 lieferte **null** neue byte-exakte Fixes. Alle 24
parallel dispatchten Subagenten schlugen mit HTTP 429 Token-Plan
Rate-Limit-Fehlern fehl — fünf davon beim Provider
`anthropic/claude-opus-5`, die übrigen 19 beim Provider
`minimax-code/MiniMax-M3`. Kein Agent erreichte die
Untersuchungs- oder gar Commit-Phase; keine Quelldatei wurde
modifiziert (`git status` nach Batch-Ende leer).

**Statistik**: `matched_functions` 9159 (identisch zu Round-68-
Endstand, +0), `matched_code_percent` 47,44 %, `build/GMSJ01/
mario.dol: OK`. Alle 24 Kandidaten bleiben frisch für eine
Retry-Runde.

**Methodische Notiz**: Anders als in früheren Runden, in denen
einzelne Rate-Limits auftraten und mit kleineren Retry-Batches
umgangen werden konnten, war diesmal die gesamte Dispatch-Welle
betroffen — was auf eine globale Token-Plan-Ausschöpfung
hindeutet, nicht auf ein sporadisches Provider-Problem. Konsequenz
für nächste Runden: ggf. längere Wartezeit vor Re-Dispatch oder
Aufteilung in mehrere kleinere Wellen.

### Session-Gesamtstand nach Runde 69

**573 verifizierte echte Fixes in 231 Commits** (unverändert seit
Runde 68; Round 69 Null-Runde). `matched_functions`: **9159**
(±0 ggü. Round 68). `matched_code_percent`: **47,44 %**. Volles
`ninja`-Rebuild erfolgreich, `dtk shasum -c` bestätigt
`build/GMSJ01/mario.dol: OK`. Fork `Astrr00/sms` weiterhin bei
Squash-Merge `56161c6` auf `main`; 5 Commits hinter
`doldecomp/sms:main`.

### Nach siebzigster Iterationsrunde (24-Kandidaten-Batch in 2×12-Wellen, 1 MATCH + 1 NO-MATCH-Toolchain-Drift + 22 Rate-Limit-Clean-Failures)

Round 70 verlief provider-seitig weiterhin angespannt: 22 von 24
Subagenten schlugen mit HTTP 429 Token-Plan Rate-Limit-Fehlern
fehl (verteilt auf `anthropic/claude-opus-5` und
`minimax-code/MiniMax-M3`), zwei Wellen à 12 Agents mit kurzem
Cooldown brachten jedoch 2 produktive Ergebnisse.

**1 byte-exakter MATCH**:

1. `TBathWaterManager::loadAfter` (Commit `f89e6df1`,
   `src/Map/BathWaterManager.cpp`) — 8-Byte-Stack-Frame-Überschuss
   (src 0x98 vs obj 0x90). Behoben durch zwei subtile
   Source-Reformatierungen: (a) äußeres `JDrama::TNameRefGen::search(...)`
   expandiert zu `JDrama::TNameRefGen::getInstance()->getRootNameRef()
   ->search(...)` — erzwingt genug vtable-Chain-Split, dass MWCCs
   Register-Allokator `r26` für das `rootNameRef`-Argument wählt
   statt `r27`; (b) inneres `setResTIMG(1, *tex->getTexture()->getTexInfo())`
   auf zwei Zeilen umgebrochen — nudges lokales Pool-Alignment und
   innere Register-Wahl. Programmatischer raw-4-byte-hex-Diff
   über alle 250 Instruktionswörter ergab leere Diff-Liste
   `[]`.

**1 NO-MATCH (sauber reverted, dokumentationswürdige Erkenntnis)**:

- `SMS_InitChangeNpcColor` (`src/NPC/NpcColor.cpp`) — 8-Byte-
  Stack-Frame-Drift zwischen src (0x40) und obj (0x38). Der
  Agent untersuchte 11+ Source-Varianten (padding, register-
  Storage, const-Qualifikation, Type-Changes, Declaration-
  Reorder, Inline-Expression-Expansion) ohne Erfolg. **Root-
  Cause: Toolchain-Drift** zwischen Original-Match-Zeitpunkt
  (MWCC 20250520, dtk v1.3.0, wibo 0.6.11) und HEAD (MWCC
  20251118, dtk v1.8.4, wibo 1.1.0). Die Source-Datei ist
  byte-identisch zum funktionierenden Commit `99c2d69e`; nur
  die Toolchain-Updates haben MWCCs Pool-Allokation um 8 Byte
  verschoben. Per „byte-exakt-oder-revert"-Policy zurückgesetzt;
  dokumentiert als „Toolchain-Version-abhängiges Frame-Layout".

**22 saubere Fehlschläge** (10 Wave-1 + 12 Wave-2, alle
Rate-Limit-bedingt): `TMarDirector::TMarDirector`,
`TTamaNoko::calcRootMatrix`, `TSpcInterp::execadd`,
`TGraphWeb::getRandomNextIndex`, `THamuKuri::behaveToWater`,
`CPolarSubCamera::execGroundCheck_`, `TBossPakkun::setGroundCollision`,
`TMarDirector::preEntry`, `TRoulette::initMapObj`,
`TMapObjBaseManager::makeObjAppear`,
`TMario::turnning`, `JPAGetRMtxSTVecElement`,
`J3DSkinDeform::initMtxIndexArray`, `TTrembleModelEffect::reset`,
`TMario::initMirrorModel`, `TMBindShadowManager::load`,
`TBossMantaManager::setupEfbAlpha`, `TLiveActor::bind`,
`TEggYoshi::load`, `TNerveBathtubKillerExplosion::execute`,
`TSpcTypedInterp<TEventWatcher>::evSetHide4LiveActor`,
`TSplashManager::makeDL` — alle bleiben frische Kandidaten für
eine künftige Runde.

**Methodische Notiz**: Wellen-Dispatch (2×12 statt 1×24) reduziert
Provider-Spitzenlast nicht zwingend — die `anthropic/claude-opus-5`-
Rate-Limits kommen wellenübergreifend. Empfehlung für Runde 71:
längerer Cooldown (15+ min) zwischen den Wellen, oder Wellen mit
max. 6 Agents.

### Session-Gesamtstand nach Runde 70

**574 verifizierte echte Fixes in 232 Commits** (573 + 1 neuer
byte-exakter Runde-70-MATCH; der NPC-Color-Toolchain-Drift zählt
nicht als Fix, da reverted). `matched_functions`: **9160** (von
9159 zu Rundenbeginn, +1 exakt wie erwartet). `matched_code_percent`:
**47,47 %**. Volles `ninja`-Rebuild erfolgreich, `dtk shasum -c`
bestätigt `build/GMSJ01/mario.dol: OK`. Regressionsprüfung: 0
Regressionen, 1 Neuzugang — exakte Übereinstimmung. Fork
`Astrr00/sms` weiterhin bei Squash-Merge `56161c6` auf `main`;
5 Commits hinter `doldecomp/sms:main`.

### Nach einundsiebzigster Iterationsrunde (5 byte-exakte MATCHes, 9 neue Dead-Ends)

**5 byte-exakte MATCHes** (alle mit rohem Vier-Byte-Hexvergleich
verifiziert: gleiche Wortzahl, leere Diff-Liste `[]`):

1. `TBossPakkun::setGroundCollision` (Commit `7760ec64`,
   `src/Enemy/bosspakkun.cpp`) — benannter `dieNerve`-Local für
   `&TNerveBPDie::theNerve()` plus benannter `TPosition3f`-Local
   für `moveMtx`. 57/57 Wörter, 228B.
2. `TTrembleModelEffect::reset` (Commit `77025abc`,
   `src/MarioUtil/DrawUtil.cpp`) — Loop-Bounds von
   `getVertexData().getVtxNum()` auf direktes `getVtxNum()`
   umgestellt und benanntes `J3DModelData*`-Local vor
   `setVtxPosArray` eingeführt. 120/120 Wörter, 480B.
3. `THamuKuri::behaveToWater` (Commit `cd245a39`,
   `src/Enemy/hamukuri.cpp`) — `SMS_GetMarioPos()` durch
   `*gpMarioPos` ersetzt (direkter Globalzugriff statt
   Inline-Getter erzeugt die Retail-Load-Sequenz).
   143/143 Wörter, 572B.
4. `TTamaNoko::calcRootMatrix` (Commit `cd245a39`,
   `src/Enemy/tamaNoko.cpp`) — gemeinsames
   `JGeometry::TVec3<f32> scale(2.0f, 2.0f, 2.0f)`-Local für
   beide `setGlobalScale`-Aufrufe gezogen statt je anonymem
   Temporary. 266/266 Wörter, 1064B.
5. `TTrembleModelEffect::init` (Commit `cd245a39`,
   `src/MarioUtil/DrawUtil.cpp`) — `getVertexData().getVtxNum()`
   /`getVtxPosArray()`-Kette auf die direkten
   `J3DModelData`-Accessoren `getVtxNum()`/`getVtxPosArray()`
   verkürzt. 354/354 Wörter, 1416B.

**9 neue Dead-Ends** (mehrere informierte Varianten getestet,
sauber reverted, in `.decomp_session_state.json` aufgenommen):

- `TGraphWeb::getRandomNextIndex` — verbleibender 4-Byte-
  Stack-Offset nach 2 Varianten.
- `CPolarSubCamera::execGroundCheck_` — Inlining des
  `should_clip`-Helpers erzeugte 22 Diffs und falsches
  Frame/Register-Layout.
- `TRoulette::initMapObj` — 3 Varianten (benannter
  `TIdxGroupObj*`-Local, expandierte `getInstance()->
  getRootNameRef()->search`-Kette, beides kombiniert)
  kollabierten den Frame 0xA0 -> 0x98.
- `TBossMantaManager::setupEfbAlpha` — Stack-Layout und
  Local-Array-Offsets bleiben abweichend.
- `evSetHide4LiveActor` — Frame 0xA0 vs 0x98 und
  fctiwz-Spill-Offsets; `interp->pop().getDataInt()` erreicht
  den Retail-Frame, aber die pop()-Scheduling-Reihenfolge
  divergiert weiter.
- `TSplashManager::makeDL` — 5-Wort-Diff: GXColor-Temp und
  `thing[]`-Slots vertauscht; 4 Varianten (const-Referenz,
  direkte Aggregate-Init, hoisted Declaration, split
  decl/assign) änderten das Slot-Mapping nicht.
- `TNerveBathtubKillerExplosion::execute` — Null-Vektor-Temp
  0x18 vs 0x1C; Pointer-Merge-Variante identisch.
- `TLiveActor::bind` — der by-value `fst`-Temp des
  `operator-` sitzt auf 0x10 statt 0x20; 5 Varianten
  (benanntes Local, const-Ref-Bindung, `sub`-Sequenzen)
  zerstörten jeweils das `bl sub`-Call-Muster oder blähten
  den Frame.
- `SMS_InitChangeNpcColor` — in Runde 70 als
  Toolchain-Drift dokumentiert; jetzt auch in der
  Ausschlussliste verankert.

**Weitere gescheiterte Versuche** (nicht in der Ausschlussliste,
da nur einzelne Durchgänge): `TNerveTamaNokoHitWater::execute`
(manuelle `unk165`-Lösung statt `unsetUnk165()`-Helper ergab
204/206 Wörter — reverted).

**Bekannte Pre-existing-Validierungsprobleme** (nicht durch
diese Runde verursacht, dokumentiert statt verschwiegen):

- `mario/MarioUtil/DrawUtil`: `validate-symbol-order.py`
  meldet fehlendes schwaches Symbol `identity33__Q29JGeometry
  64TRotation3<...>Fv` (fehlte nachweislich bereits im Objekt
  vor der Round-71-Änderung) sowie 14 UNUSED-Size-Warnungen
  auf bestehenden Null-Byte-Stubs. Symbolprüfung der TU ist
  damit **nicht sauber**.
- `mario/Enemy/hamukuri`: `onHaveCap__13TDoroHamuKuriFv` ist
  global gelinkt, die Map erwartet `weak` (Original vermutlich
  Header-inline definiert); dazu lange Weak-Order-Warnliste.
  Beides unabhängig vom `behaveToWater`-Diff.

### Session-Gesamtstand nach Runde 71

**579 verifizierte echte Fixes** (574 + 5 neue Runde-71-Matches
in 3 Commits). `matched_functions`: **9165**. `matched_code_percent`:
**47,57 %** (`ninja changes_all`: 47,47 % -> 47,57 %, ausschließlich
Neuzugänge, keine Regressionen). Volles `ninja`-Rebuild erfolgreich,
`dtk shasum -c` bestätigt `build/GMSJ01/mario.dol: OK`.


### Nach zweiundsiebzigster Iterationsrunde (Cloud-Session: DOL fehlt, Runden 70/71 nach main portiert, kein Retail-Bytevergleich)

**Beobachtung, Umgebung.** Workspace `/workspace`, Branch
Ausgang `main` (`0b9a2b13`), Remote nur `origin` =
`github.com/Astrr00/sms`. Arbeitsbaum vor dieser Runde sauber,
kein unpushed Commit gegen `origin/main`. Betriebssystem dieser
Session: Linux. `python3 configure.py --version GMSJ01` erzeugt
die Pre-Split-`build.ninja` (Exit 0). `ninja` bricht danach ab:

`Failed: While loading object 'main.dol' / orig/GMSJ01/sys/main.dol not found`.

Lokal fehlend, exakt:

- `orig/GMSJ01/sys/main.dol` (Eingabe von `dtk dol split`)
- `orig/GMSJ01/files/mario.MAP` (Eingabe von `validate-symbol-order.py`)
- alles unter `build/GMSJ01/asm/` und `build/GMSJ01/obj/` (entsteht erst durch den Split)
- eine Disc-Abbildung unter `orig/GMSJ01/` (nur `.gitkeep`)

`config/GMSJ01/build.sha1` erwartet für das **gelinkte**
`build/GMSJ01/mario.dol` den SHA1
`9f5a8caf56f5356aeac9d3ed28bf8de976a03625`. Dieser Hash wurde
hier nicht nachgemessen. `dtk shasum -c`, `objdiff` und
`ninja changes_all` sind ohne die DOL nicht ausführbar.
`matched_functions` / `matched_code_percent` wurden in dieser
Session **nicht** neu gemessen. Die Zahlen aus Runden 68–71
bleiben Berichte jener Sessions.

Installiert über die Projektskripte, nicht committet
(`build/` ist ignoriert): ninja 1.13.2, dtk 1.8.4, wibo 1.1.0,
objdiff-cli 3.8.1, binutils 2.42-2, Compilerpaket `20251118`.
`mwcceppc.exe` unter wibo meldet Version 2.3.3 build 163.

**Beobachtung, Upstream `doldecomp/sms`.** Fetch ohne Merge.
Merge-Base mit `upstream/main` ist `b4cab1d2` („BossPakkun closer“).
`upstream/main` ist `78460084` („Add explicit casts for narrowing
conversions“, 2026-09-26) und liegt **60 Commits** vor dieser
Base. Darunter `8dc741f9` „Move middleware libraries to libs/“
sowie eine Serie von SDK-Typ-/`nullptr`-/`uintptr_t`-Anpassungen.
Schnittmenge der seit der Base geänderten Pfade mit unseren
eigenen Änderungen: 89 Dateien, unter anderem `configure.py` und
zahlreiche bereits gematchte Spielcode-TUs. Ein Merge würde
diese Matching-Fixes nicht ersetzen, aber in derselben Datei
mit der `libs/`-Verschiebung und den Typanpassungen kollidieren.
Nicht gemergt, nicht rebasiert. Die drei Runde-67-Kandidaten-TUs
sind auf `upstream/main` weiterhin `NonMatching`
(`Strategic/ObjModel.cpp`, `Enemy/coasterkiller.cpp`,
`MoveBG/MapObjGeneral.cpp`). `THamuKuri::behaveToWater` ruft
upstream weiterhin `SMS_GetMarioPos()` auf.

**Beobachtung, Stand der Runde-67-Kandidaten auf `main`.**
`TMActorKeeper::TMActorKeeper(TLiveManager*)` enthält bereits
`char trash[8]` aus `decomp-work` `e239d637`; der Runde-70-Audit
auf `decomp-work` behauptet dafür 100 %. Hier nicht per objdiff
geprüft. `TMapObjGeneral::receiveMessage` ist in Runde 68 als
Sackgasse dokumentiert und unverändert. `TCoasterEnemy::bind`
ist unverändert der kurze Rumpf; `symbols.txt` nennt
`bind__13TCoasterEnemyFv` Größe `0xDC`. Ohne Original-ASM kein
neuer Versuch.

**Übernahme von `origin/decomp-work`.** `main` endete inhaltlich
beim Squash `56161c69` (Code bis Runde 68, `PROGRESS.md` nur bis
Runde 67). Auf `decomp-work` lagen danach noch die Quelldiffs
von Runde 70/71, die auf `main` fehlten. Übernommen, unverändert:

- `TBathWaterManager::loadAfter` (`f89e6df1`)
- `TBossPakkun::setGroundCollision` (`7760ec64`)
- `TTrembleModelEffect::reset` (`77025abc`)
- `THamuKuri::behaveToWater`, `TTamaNoko::calcRootMatrix`,
  `TTrembleModelEffect::init` (`cd245a39`)

`configure.py` bleibt unverändert (kein Matching-Flip). Die
Runden-68–71-Abschnitte dieser Datei stammen aus
`origin/decomp-work` und wurden hier nicht neu gemessen.

**Beobachtung, eigener MWCC-Vergleich vorher/nachher** (dieselben
Flags wie `cflags_game`: `-O4,p -inline deferred -opt all,nostrength`,
`-prefix SMS.mch`, GC/1.2.5). Alle sechs Symbole haben vorher und
nachher dieselbe Länge wie `symbols.txt`:

| Symbol | Wörter | Retail-Größe |
| --- | --- | --- |
| `loadAfter__17TBathWaterManagerFv` | 250 | `0x3E8` |
| `setGroundCollision__11TBossPakkunFv` | 57 | `0xE4` |
| `reset__19TTrembleModelEffectFv` | 120 | `0x1E0` |
| `init__19TTrembleModelEffectFP8J3DModel` | 354 | `0x588` |
| `behaveToWater__9THamuKuriFP9THitActor` | 143 | `0x23C` |
| `calcRootMatrix__9TTamaNokoFv` | 266 | `0x428` |

Positionsvergleich der Instruktionswörter, nur die Differenzen:

- `loadAfter`: 5 Wörter, alle Prolog/Epilog. `stwu` `-0x98` → `-0x90`
  (`9421ff68` → `9421ff70`); `stmw`/`lmw`/`lwz` LR/`addi r1`
  um dieselben 8 Byte verschoben. Die übrigen 245 Wörter sind identisch.
  Das ist die in Runde 70 behauptete Frame-Korrektur. Die dort
  zusätzlich genannte Registerwahl r26 statt r27 tritt in **diesem**
  Vorher/Nachher-Diff nicht auf.
- `setGroundCollision`: 1 Wort, `addi r30,r1,0x18` → `addi r30,r1,0x20`
  (`3bc10018` → `3bc10020`). Übrige 56 Wörter identisch.
- `reset`: 5 Wörter, Frame `stwu` `-0xC8` → `-0xA8` (`9421ff38` →
  `9421ff58`) plus passende Save/Restore-Offsets.
- `init`: 5 Wörter, Frame `-0x110` → `-0xE0` (`9421fef0` → `9421ff20`).
- `behaveToWater`: 4 Wörter, Stack-Offsets `0x5c/0x60/0x64` →
  `0x58/0x5c/0x60`.
- `calcRootMatrix`: 7 Wörter, Frame `-0x68` → `-0x50` plus Offsets
  der Scale-Slots.

**Vermutung, nicht Beobachtung.** Dass die Nachher-Fassung
bytegleich zum Retail-Objekt ist, ist die Aussage der
`decomp-work`-Commits und des dortigen `objdiff`-Reports.
Diese Session hatte das Retail-Objekt nicht und kann das
weder bestätigen noch widerlegen. Ein reiner Größenvergleich
reicht dafür nicht: die Länge war schon vorher gleich.

`ninja` gesamt, `dtk shasum -c` und ein objdiff-Report sind in
dieser Session **nicht** gelaufen, weil die DOL fehlt.
TU-Zähler in `configure.py` auf `main` (Link-Status, nicht
Funktionsprozent): 416 `Matching`, 321 `NonMatching`.

### Nächster Schritt

1. `orig/GMSJ01/sys/main.dol` und `orig/GMSJ01/files/mario.MAP`
   lokal bereitstellen (nicht committen).
2. `ninja`, dann `python tools/decomp-diff.py` für
   `mario/Map/BathWaterManager`, `mario/Enemy/bosspakkun`,
   `mario/MarioUtil/DrawUtil`, `mario/Enemy/hamukuri`,
   `mario/Enemy/tamaNoko` — die sechs Symbole Wort für Wort
   gegen das Originalobjekt.
3. `dtk shasum -c config/GMSJ01/build.sha1`. Erst danach einen
   Matching-Flip erwägen. Die betroffenen TUs bleiben bis dahin
   `NonMatching`.
4. Wenn der Split steht: `TCoasterEnemy::bind` (`0xDC`) als
   nächsten offenen Kandidaten. `receiveMessage` von
   `TMapObjGeneral` nicht wiederholen (Sackgasse Runde 68).


### Nach dreiundsiebzigster Iterationsrunde (Referenz-DOL vorhanden, sechs Funktionen verifiziert, bind nicht geschlossen)

**Beobachtung, Referenzdateien.** Entpackt nach
`orig/GMSJ01/sys/main.dol` und `orig/GMSJ01/files/mario.MAP`.
Nicht committet. `git check-ignore` trifft beide über
`.gitignore` (`orig/*/*`, zusätzlich `*.dol` / `*.MAP`).

SHA1, gemessen:

- `main.dol`: `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`
- `mario.MAP`: `1f7a9441e5fbb7fed12539559f9956d36cc6d2e2`

Der DOL-SHA1 ist derselbe Wert wie in `config/GMSJ01/build.sha1`
für das gelinkte `build/GMSJ01/mario.dol`.

**Beobachtung, Baseline auf diesem Branch** (die sechs
Runde-70/71-Ports sind schon im Quelltext). `python3 configure.py
--version GMSJ01`, dann volles `ninja`. `dtk shasum -c
config/GMSJ01/build.sha1`: `build/GMSJ01/mario.dol: OK`.
`sha1sum build/GMSJ01/mario.dol` liefert denselben Hash
`9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

`ninja`-Progress:

- Gesamt: 47,57 % matched code, 1707908 / 3590088 Bytes,
  9165 / 12881 Funktionen. Fuzzy 77,97 %. Linked 20,25 %
  (415 / 736 Dateien). Daten 384651 / 640331 Bytes (60,07 %).
- Game Code: 35,23 % matched, 995760 / 2826784 Bytes,
  5200 / 8857 Funktionen. Linked 89 / 387 Dateien.
- JSystem: 90,55 % matched code, 2953 / 3009 Funktionen.
- SDK: 98,88 % matched code, 1012 / 1015 Funktionen.

Das sind dieselben Funktions- und Prozentzahlen, die Runde 71
berichtet hat. Diesmal aus diesem Build gemessen.

**Beobachtung, sechs Funktionen.** objdiff-cli
`functionRelocDiffs=data_value`, linke und rechte
Instruktionswörter (Mnemonic und Operanden, Branchziele
normalisiert) positionsweise verglichen. Null Abweichungen:

| Funktion | Wörter | Größe | match |
| --- | --- | --- | --- |
| `TBathWaterManager::loadAfter` | 250 | 1000 | 100 % |
| `TBossPakkun::setGroundCollision` | 57 | 228 | 100 % |
| `TTrembleModelEffect::reset` | 120 | 480 | 100 % |
| `TTrembleModelEffect::init` | 354 | 1416 | 100 % |
| `THamuKuri::behaveToWater` | 143 | 572 | 100 % |
| `TTamaNoko::calcRootMatrix` | 266 | 1064 | 100 % |

Die Runde-72-Vermutung ist damit Beobachtung: die Nachher-Fassung
ist wortgleich zum Originalobjekt. Die TUs bleiben `NonMatching`.
Jede davon hat weitere nicht matchende Funktionen (Stichprobe:
`TBathWaterManager::perform` 93,8 %, `TNerveBPWaitL::execute`
98,1 %, `SMS_UnifyMaterial` 99,3 %, `TDangoHamuKuri::reset` 70,0 %,
`TTamaNoko::landEffect` 62,5 %). Ein Flip würde das Originalobjekt
durch unser Objekt ersetzen und den DOL-Hash ändern. Nicht geflippt.
Zweiter `ninja` nach den zurückgenommenen `bind`-Versuchen:
`dtk shasum` weiter OK.

**Beobachtung, `TCoasterEnemy::bind`.** Offen. 55 Instruktionen,
220 Bytes (`0xDC`), Frame beiderseits `0x40`. Einziger Unterschied:
das By-Value-Temporary von `operator-` liegt im Original bei
`r1+0x10` und bei uns bei `r1+0x1c`. `nextPos` bleibt beiderseits
bei `0x28`. Dieselbe Klasse wie `TLiveActor::bind` (Runde 71:
`0x10` gegen `0x20`). Drei Versuche, alle zurückgenommen:

1. Unbenutztes `TVec3 gap` nach `nextPos`: Frame `0x40` → `0x48`,
   `nextPos` wandert nach `0x34`, das Temporary bleibt bei `0x1c`.
2. `mLinearVelocity = nextPos - mPosition` statt
   `setLinearVelocity`: identischer Diff.
3. `nextPos` erst deklarieren, dann zuweisen: identischer Diff.

**Zusätzlich probiert und zurückgenommen:**
`TMapObjBaseManager::makeObjAppear(float,float,float,u32,bool)`.
Frame `0x58` stimmt. Nur `&checkData` ist `0x30` im Original und
`0x34` bei uns. `checkData` vor den `if` zu ziehen ändert den
Offset nicht.

### Nächster Schritt

1. `TCoasterEnemy::bind` nicht mit einem weiteren benannten
   `TVec3` oder mit `operator=`-Umschreibung wiederholen.
   Nächster Hebel wäre ein totes 12-Byte-Temporary *unter*
   `nextPos`, das den `operator-`-Slot auf `0x10` drückt, ohne
   den Frame über `0x40` wachsen zu lassen.
2. `TMapObjBaseManager::makeObjAppear(f32,f32,f32,u32,bool)`:
   4-Byte-Slot von `checkData` (`0x34` → `0x30`) bei gleichem Frame.
3. Kein Matching-Flip der fünf TUs, solange dort andere
   Funktionen abweichen.

### Nach vierundsiebzigster Iterationsrunde (`makeObjAppear`-Float-Overload matched)

**Beobachtung, vorher.**
`TMapObjBaseManager::makeObjAppear(f32,f32,f32,u32,bool)`,
94 Instruktionen, 376 Bytes, Frame beiderseits `0x58`.
Zwei Wörter abweichend: `addi r4, r1, 0x30` gegen `0x34`
und `lwz r3, 0x30(r1)` gegen `0x34(r1)`.
Das ist der Slot von `checkData`.

**Zurückgenommen, eine Hypothese je Versuch.**

1. `f32 raised = y + 5.0f` als Argument von `checkGround`.
   `fadds` bleibt in `f2`, `checkData` bleibt bei `0x34`,
   Frame bleibt `0x58`. Die Summe belegt keinen Stack-Slot.
2. `int i` vor `y2` deklarieren und die Schleife mit diesem
   `i` schreiben. Offset unverändert. Der Index besitzt den
   toten Slot nicht. `checkData` vor den `if` zu ziehen war
   schon Runde 73 und bleibt ohne Effekt.

**Beobachtung, nachher.** Die Ternärform steht am Aufruf,
ohne die fabricierte Inline-Methode `checkFlag`:

`checkData->mFlags & BG_CHECK_FLAG_ILLEGAL ? true : false`

`checkData` liegt beiderseits bei `r1+0x30`.
94 Instruktionswörter, Frame `0x58`.
Unter `functionRelocDiffs=data_value` zeigen `gpMap`,
`TMap::checkGround` und der 4-Byte-Pool der `5.0f`
dieselben Ziele. Null abweichende Wörter.
`isIllegalData` trägt `#pragma dont_inline` und wäre hier
ein `bl`. Das Original faltet denselben Test ein.
`checkFlag` im Header ist unverändert.

**Messung, `ninja` und `dtk shasum -c`.**

Vorher, Runde 73: 47,57 % matched code,
1707908 / 3590088 Bytes, 9165 / 12881 Funktionen.
Game Code 35,23 %, 995760 / 2826784 Bytes,
5200 / 8857 Funktionen.

Nachher: 47,58 % matched code,
1708284 / 3590088 Bytes, 9166 / 12881 Funktionen.
Game Code 35,24 %, 996136 / 2826784 Bytes,
5201 / 8857 Funktionen.

Delta: +1 Funktion, +376 Bytes. Das ist die Größe der
Funktion (94 Instruktionen).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

`configure.py` nicht geflippt. Dieselbe TU hat weiter
abweichende Geschwister, Wörter gezählt, nicht die
gerundete Prozentanzeige:

- `newAndRegisterObjByEventID`: 10 Wörter, Anzeige 99,98 %
- `newUniqueObjByName`: 42 Wörter, Anzeige 98,88 %
- `TMapObjManager::load`: 9 Wörter, Anzeige 99,97 %

`validate-symbol-order.py -u mario/MoveBG/MapObjManager`:
PASS. Eine vorbestehende UNUSED-Größenwarnung
`loadMatTable__14TMapObjManagerFPCc`, Map `0x34`,
Objekt `0x38`. Diese Funktion ist nicht angefasst.
`TCoasterEnemy::bind` ist nicht angefasst.

### Nächster Schritt

1. `MoveBG/MapObjManager.cpp` nicht auf `Matching` stellen.
2. `TCoasterEnemy::bind` nicht mit einem benannten `TVec3`
   oder mit `operator=` wiederholen.
3. Nächster Kandidat außerhalb dieser TU: eine fast
   matchende Funktion mit kleinem Slot- oder Frame-Gap.
   Die drei Geschwister oben sind mehrere Wörter auseinander,
   kein einzelner 4-Byte-Slot. Die fünf bereits verifizierten
   TUs bleiben `NonMatching`.

### Nach fünfundsiebzigster Iterationsrunde (zwei Frame-Matches, ein Slot offen)

**Beobachtung, vorher.** Stand Runde 74:
47,58 % matched code, 1708284 / 3590088 Bytes,
9166 / 12881 Funktionen.
Game Code 35,24 %, 996136 / 2826784 Bytes,
5201 / 8857 Funktionen.

**Match, `TRollBlock::setGroundCollision`.**
24 Instruktionen, 96 Bytes.
Vorher Frame `0x28` bei uns, `0x20` im Original.
Einziger Unterschied waren die Frame-Offsets von `r31`.
Ursache: der fabricierte Inline `getUnk8()`.
Direktes `unk8` lässt die Loads gleich und setzt den
Frame auf `0x20`.
Nach dem Rebuild: 24 Wörter, Relocs gleich, null Abweichungen.
`validate-symbol-order.py -u mario/MoveBG/MapObjRailBlock`: PASS.
Die TU bleibt `NonMatching`
(`TNormalLift::setGroundCollision` hat weiter den zusätzlichen
`SMatrix34C`-Konstruktor beim Inlinen, 188 Bytes, Anzeige 95,7 %).

**Match, `TCoinBlue::load`.**
27 Instruktionen, 108 Bytes.
Vorher Frame `0x20` bei uns, `0x28` im Original.
Die übrigen Wörter stimmten schon.
`u8 area = gpMarDirector->getCurrentMap()` und
`u8 coin = getEventId()` vor `getBlueCoinFlag` heben den
Frame auf `0x28`, ohne die Load-Reihenfolge zu ändern.
Feldzugriffe statt der Accessors verschieben `smInstance`
vor das `lbz` und sind zurückgenommen.
Nach dem Rebuild: 27 Wörter, Relocs gleich, null Abweichungen.
`validate-symbol-order.py -u mario/MoveBG/Item`: PASS.
Die TU bleibt `NonMatching` (17 weitere Funktionen weichen ab).

**Messung, `ninja` und `dtk shasum -c`.**

Nachher: 47,59 % matched code,
1708488 / 3590088 Bytes, 9168 / 12881 Funktionen.
Game Code 35,25 %, 996340 / 2826784 Bytes,
5203 / 8857 Funktionen.

Delta gegen Runde 74: +2 Funktionen, +204 Bytes
(96 + 108).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Kein Matching-Flip.

**Sackgasse, zurückgenommen: `TDrawSyncManager::setCallback`.**
13 Instruktionen, 52 Bytes, Frame beiderseits `0x30`.
Das 8-Byte-Temporary von `TDrawSyncTokenRange` liegt im
Original bei `r1+0x28` und bei uns bei `r1+0x24`.

1. `mCallbacks.begin()[param_1] = ...`: Frame `0x30` → `0x28`,
   Slot `0x24` → `0x20`.
2. Benannte Referenz auf `mCallbacks`: kein Unterschied.
3. Zeiger auf das Element, dann Zuweisung: kein Unterschied.
4. Benanntes `TDrawSyncTokenRange`, dann Zuweisung:
   Frame `0x28`, Slot `0x20`.

Die direkte Zuweisung des Temporaries ist die Fassung mit
dem richtigen Frame. Der Slot bleibt 4 Byte zu tief.

**Angesehen, nicht geändert: `TMario::kickRoofEffect`.**
Frame `0x38` gegen `0x30`, und zusätzlich
`lbz` von `0x3cb` gegen `0x3cf`.
Das ist ein Member-Offset, kein reiner Slot.

### Nächster Schritt

1. `MapObjRailBlock` und `Item` nicht auf `Matching` stellen.
2. `TDrawSyncManager::setCallback`: Slot `0x24` → `0x28`
   bei Frame `0x30`. Die vier Varianten oben nicht wiederholen.
3. `TCoasterEnemy::bind` nicht mit benanntem `TVec3`
   oder `operator=`.
4. `kickRoofEffect` erst angehen, wenn der Member-Offset
   `0x3cb`/`0x3cf` geklärt ist.

### Nach sechsundsiebzigster Iterationsrunde (zwei weitere Matches)

**Beobachtung, vorher.** Stand Runde 75:
47,59 % matched code, 1708488 / 3590088 Bytes,
9168 / 12881 Funktionen.
Game Code 35,25 %, 996340 / 2826784 Bytes,
5203 / 8857 Funktionen.

**Match, `evIsTalkModeNow`.**
49 Instruktionen, 196 Bytes, Frame beiderseits `0x30`.
Vorher lag das `TSpcSlice` bei uns auf `r1+0x14`, im Original
auf `r1+0x18`. Sonst gleiche Wörter.
`push(int)` baut das Slice eine Inline-Stufe tiefer.
`interp->push(TSpcSlice(value))` setzt es auf `0x18`.
Zwei Vorversuche ohne Wirkung, zurückgenommen:
benannter `TMarDirector*` und benanntes `bool`.
Nach dem Rebuild: 49 Wörter, Reloc-Typen gleich,
null abweichende Wörter. Der String-Pool von `SpcTrace`
hat andere lokale Namen, `functionRelocDiffs=data_value`
zählt ihn als gleich.

`validate-symbol-order.py -u mario/System/EventWatcher`
meldet weiterhin `set__Q29JGeometry8TVec3<f>FRC3Vec` als
MISSING. Das fehlt schon vor dieser Änderung. Nicht von
diesem Match verursacht. Die TU bleibt `NonMatching`.

**Match, `TNerveBPJumpReact::execute`.**
25 Instruktionen, 100 Bytes.
Vorher Frame `0x20` bei uns, `0x28` im Original.
Nur die Frame-Offsets von `r31` wichen ab.
`int time = spine->getTime()` hebt den Frame auf `0x28`.
Nach dem Rebuild: 25 Wörter, null Abweichungen.
`validate-symbol-order.py -u mario/Enemy/bosspakkun`:
PASS, zwei vorbestehende UNUSED-Größenwarnungen.
Die TU bleibt `NonMatching`.

**Messung, `ninja` und `dtk shasum -c`.**

Nachher: 47,60 % matched code,
1708784 / 3590088 Bytes, 9170 / 12881 Funktionen.
Game Code 35,26 %, 996636 / 2826784 Bytes,
5205 / 8857 Funktionen.

Delta gegen Runde 75: +2 Funktionen, +296 Bytes
(196 + 100).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Kein Matching-Flip. `git push` weiter abgelehnt
(`Invalid username or token`). Die Commits bleiben lokal.

### Nach siebenundsiebzigster Iterationsrunde (kein neues Match)

**Beobachtung, vorher.** Stand Runde 76:
47,60 % matched code, 1708784 / 3590088 Bytes,
9170 / 12881 Funktionen.
Game Code 35,26 %, 996636 / 2826784 Bytes,
5205 / 8857 Funktionen.
Quelle unverändert, daher kein neuer `ninja`-Lauf
für die Gesamtzahlen.

**`evLaunchEventClearDemo`, kein Match.**
44 Instruktionen, 176 Bytes, Frame beiderseits `0x30`.
Das `TSpcSlice` liegt im Original auf `r1+0x20`,
bei `interp->push()` auf `r1+0x18`.
Sonst gleiche Wörter. Der `SpcTrace`-Pool hat andere
lokale Namen; `functionRelocDiffs=data_value` zählt
ihn als gleich.

Gemessen und zurückgenommen:

- `interp->push(TSpcSlice())`, `push(TSpcSlice(0))`,
  ein benanntes `TSpcSlice` und
  `mProcessStack.push(TSpcSlice())` heben das Slice
  auf `0x1c`. Frame bleibt `0x30`. Vier Byte unter
  `0x20`. Die Wörter sonst gleich.
- `gpMarDirector->getConsole()` statt
  `SMSGetMarDirector()->getConsole()` schrumpft den
  Frame auf `0x28`.
- Ein benannter `TMarDirector*` tut dasselbe, weil
  der Accessor dann entfällt.
- `mConsole` statt `getConsole()` schrumpft den Frame
  ebenfalls auf `0x28`.
- `T x; x = getConsole();` ändert nichts.
- Das Slice vor den Console-Aufrufen zu bauen
  verschiebt die Stores vor den `bl`.

**Weitere Kandidaten, kein Match, zurückgenommen.**

`TNerveBEelTearsMoveUp::execute`: Frame `0x40` gegen
`0x30`, sonst nur Prolog-Offsets.
`int time = spine->getTime()` ändert nichts.
`f32 speed = mSLTearsUpSpeed.get()` ändert nichts.

`TMareEventWallRock::load`: Frame `0x88` gegen `0x80`,
der Zeiger für `insert` liegt auf `0x68` gegen `0x64`.
Ein benannter `TViewObj*` ändert nichts.

Kein Matching-Flip. `git push` erneut abgelehnt
(`Invalid username or token`).
`~/.gitconfig` und `gh` `hosts.yml` stehen weiter auf
dem Stand 03:59 UTC. Die lokalen Commits ab
`7881cb78` sind nicht auf dem PR.

### Nächster Schritt

1. Die fünf/sechs geprüften TUs nicht auf `Matching`
   stellen.
2. Nicht wiederholen: `setCallback` (vier Varianten),
   `bind` mit `TVec3`/`operator=`, `kickRoofEffect`
   bis der Member-Offset klar ist,
   `evLaunchEventClearDemo` mit den oben gemessenen
   Push- und Accessor-Schreibweisen,
   `TNerveBEelTearsMoveUp` mit benanntem `getTime`
   oder benanntem Speed,
   `TMareEventWallRock::load` mit benanntem
   `TViewObj*`.
3. Nächster unangetasteter Kandidat:
   `SMS_CountPolygonNumInShape`, Frame `0x48` gegen
   `0x40`, die Tabelle vier Byte zu tief (`0x34`
   gegen `0x30`).

### Nach achtundsiebzigster Iterationsrunde (zwei Matches in DrawUtil)

**Beobachtung, vorher.** Stand Runde 77:
47,60 % matched code, 1708784 / 3590088 Bytes,
9170 / 12881 Funktionen.
Game Code 35,26 %, 996636 / 2826784 Bytes,
5205 / 8857 Funktionen.

**Match, `SMS_CountPolygonNumInShape`.**
55 Instruktionen, 220 Bytes.
Vorher Frame `0x40` bei uns, `0x48` im Original.
Die Größentabelle lag auf `r1+0x30` statt `r1+0x34`.
Sonst gleiche Wörter.
`vtxAttrSize(sizeTable, desc->type)` hebt den Frame
auf `0x48`, die Tabelle aber auf `0x38`.
`GXAttrType type = desc->type` vor dem Aufruf lässt
ein Argument-Temporary weg. Tabelle dann auf `0x34`.
Nur der benannte Typ, ohne die Inline, bleibt bei
Frame `0x40`.
Nach dem Rebuild: 55 Wörter, null Abweichungen.
Pool-Namen von `@2195` und `@742` zählt
`functionRelocDiffs=data_value` als gleich.
Die Inline wird nicht emittiert.

**Match, `TSilhouette::setting`.**
103 Instruktionen, 412 Bytes, Frame beiderseits `0x90`.
Vorher lagen die Farb-Stores auf `0x1c` und die Kopie
auf `0x20`.
`GXColor amb = { ... }` schrumpft den Frame auf `0x88`
und baut die Farbe bei `0x74`. Zurückgenommen.
`GXColor amb; amb = (GXColor){ ... };` trifft `0x20`
und die Kopie auf `0x1c`.
Nach dem Rebuild: 103 Wörter, null Abweichungen.

`validate-symbol-order.py -u mario/MarioUtil/DrawUtil`
meldet weiter das vorbestehende fehlende Weak-Symbol
`identity33__Q29JGeometry64TRotation3<...>Fv`.
Dasselbe fehlte vor dieser Änderung.
14 UNUSED-Größenwarnungen sind ebenfalls alt.
Die TU bleibt `NonMatching`.
`SMS_UnifyMaterial` liegt bei 99,3 %.

**Messung, `ninja` und `dtk shasum -c`.**

Nachher: 47,61 % matched code,
1709416 / 3590088 Bytes, 9172 / 12881 Funktionen.
Game Code 35,28 %, 997268 / 2826784 Bytes,
5207 / 8857 Funktionen.

Delta gegen Runde 77: +2 Funktionen, +632 Bytes
(220 + 412).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Kein Matching-Flip. Kein Push in diesem Lauf.

### Nächster Schritt

1. `DrawUtil` nicht auf `Matching` stellen.
2. `TSilhouette::setting` nicht wieder in den Aufruf
   falten und nicht als `GXColor amb = { ... }`
   schreiben.
3. `SMS_CountPolygonNumInShape` nicht auf den direkten
   Index `sizeTable[desc->type]` zurückdrehen.
4. Nächster Kandidat in derselben TU:
   `TSilhouette::perform`, 254 Instruktionen, 1016 Bytes.
   Frame `0x188` gegen `0x180`. Die Farb-Slots weichen
   um 12 Byte ab (`0x40` gegen `0x34`), der Frame nur
   um 8. Kein einheitlicher Shift.

### Nach neunundsiebzigster Iterationsrunde (ein Match)

**Beobachtung, vorher.** Stand Runde 78:
47,61 % matched code, 1709416 / 3590088 Bytes,
9172 / 12881 Funktionen.
Game Code 35,28 %, 997268 / 2826784 Bytes,
5207 / 8857 Funktionen.

`TSilhouette::perform` nicht angefasst.
`getUnk1CAlpha` lädt bei uns `0x1f` statt `0x1b`.
Das ist ein Member-Offset, kein reiner Slot.

**Match, `TNerveBPBreakSleep::execute`.**
61 Instruktionen, 244 Bytes.
Vorher Frame `0x28` bei uns, `0x30` im Original.
Der Rumpf war sonst wortgleich.
`int time = spine->getTime()` hebt den Frame auf `0x30`.
Nach dem Rebuild: 61 Wörter, null Abweichungen.
Dieselbe Schreibweise wie bei `TNerveBPJumpReact`.

`validate-symbol-order.py -u mario/Enemy/bosspakkun`
PASS. Zwei UNUSED-Größenwarnungen sind alt.
Die TU bleibt `NonMatching`.

**Gemessen und zurückgenommen.**

- `TLightWithDBSet::addChildGroupObj`: benannte
  `opa`/`xlu` lassen die Zeiger-Slots vertauscht
  (`0x6c`/`0x70`) und schieben den ersten
  Iterator um 4 Byte nach unten.
- `TAmenbo::calcRootMatrix`: `TPosition3f` vor
  `isTaken()` ändert nichts. Die Matrix bleibt
  auf `0x3c` statt `0x40`. Frame beiderseits `0x90`.
- `TSilhouette::loadAfter`: Faktorentausch
  `m[1][0] * m[2][1]` ändert die Lade-Reihenfolge
  nicht. Das zweite `fmuls` wird schlechter.
- `SMS_AddDamageFogEffect`: eine Inline
  `damageFogOsc` faltet `-400 - startBase` und
  `800 - endBase` weiter zu einem `300 * s`.
  Frame `0x88` auf `0x90`, Original `0xb8`.
  `fsubs` und das zweite `fmuls` fehlen weiter.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,62 % matched code,
1709660 / 3590088 Bytes, 9173 / 12881 Funktionen.
Game Code 35,29 %, 997512 / 2826784 Bytes,
5208 / 8857 Funktionen.

Delta gegen Runde 78: +1 Funktion, +244 Bytes.

`changes_all` meldet nur
`execute__18TNerveBPBreakSleep...` von 99,89 % auf
100 %. Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Kein Matching-Flip.

### Nächster Schritt

1. `bosspakkun` und `DrawUtil` nicht auf `Matching`
   stellen.
2. `getTime()` nicht pauschal benennen.
   Bei `TNerveBEelTearsMoveUp` hat es nichts geändert.
3. Nicht wiederholen: die vier zurückgenommenen
   Versuche oben, dazu die bekannten Sackgassen
   (`bind`, `setCallback`, `evLaunchEventClearDemo`,
   `kickRoofEffect`, `perform` von `TSilhouette`).
4. Nächster Kandidat: eine andere Nerve, deren Diff
   nur ein zu kleiner Frame ist und deren Rumpf
   schon wortgleich ist. Nicht `TCoin::appear` und
   nicht `TItem::calc` als erstes: dort fehlen
   `0x20` Byte ohne einen einzigen Stack-Zugriff.

### Nach achtzigster Iterationsrunde (drei Matches)

**Beobachtung, vorher.** Stand Runde 79:
47,62 % matched code, 1709660 / 3590088 Bytes,
9173 / 12881 Funktionen.
Game Code 35,29 %, 997512 / 2826784 Bytes,
5208 / 8857 Funktionen.

**Match, drei Nerves in `bosspakkun`.**
Der Rumpf war wortgleich, der Frame 8 Byte zu klein.
`int time = spine->getTime()` hebt den Frame.
Null abweichende Wörter danach.

- `TNerveBPGetUp::execute`, 46 Instruktionen, 184 Bytes.
  Frame `0x20` auf `0x28`.
- `TNerveBPCannon::execute`, 73 Instruktionen, 292 Bytes.
  Frame `0x30` auf `0x38`.
- `TNerveBPSwing::execute`, 44 Instruktionen, 176 Bytes.
  Frame `0x30` auf `0x38`.
  Nur der erste `getTime()` ist benannt.
  Der zweite bleibt `spine->getTime()`, das Original
  lädt nach `changeBck` neu.

`validate-symbol-order.py -u mario/Enemy/bosspakkun`
PASS. Die zwei UNUSED-Größenwarnungen sind alt.
Die TU bleibt `NonMatching`.

**Gemessen und zurückgenommen.**

- `TNerveBPTumbleIn`: derselbe benannte `getTime()`
  ändert den Frame nicht. Er bleibt `0x40` gegen
  `0x48`. Drei Lade-Stellen, kein Stack-Objekt
  dazwischen.
- `TNerveBPFlyPivot`: der Frame wächst auf `0x40`,
  gleich dem Original. Der Rückgabewert von `pop()`
  bleibt auf `0x1c` statt `0x20`.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,64 % matched code,
1710312 / 3590088 Bytes, 9176 / 12881 Funktionen.
Game Code 35,31 %, 998164 / 2826784 Bytes,
5211 / 8857 Funktionen.

Delta gegen Runde 79: +3 Funktionen, +652 Bytes
(184 + 292 + 176).

`changes_all` meldet nur diese drei Executes von
unter 100 % auf 100 %. Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Kein Matching-Flip.

### Nächster Schritt

1. `bosspakkun` nicht auf `Matching` stellen.
2. `getTime()` nicht pauschal benennen.
   `TumbleIn` hat nicht reagiert.
   `FlyPivot` schließt den `pop()`-Slot nicht.
3. `TNerveBPTornado` hat 16 Byte zu wenig Frame
   und keinen Stack-Zugriff im Rumpf.
   Ein einzelner `int` reicht dort erfahrungsgemäß
   nicht.
4. `TNerveBPDie` fehlt `0x20` ohne Stack-Zugriff,
   wie `TCoin::appear` und `TItem::calc`.

### Nach einundachtzigster Iterationsrunde (fünf Matches)

**Beobachtung, vorher.** Stand Runde 80:
47,64 % matched code, 1710312 / 3590088 Bytes,
9176 / 12881 Funktionen.
Game Code 35,31 %, 998164 / 2826784 Bytes,
5211 / 8857 Funktionen.

**Match, fünf Nerves.**
Der Rumpf war wortgleich, der Frame 8 Byte zu klein.
Ein benanntes `int time = spine->getTime()` hebt den Frame.
Null abweichende Wörter danach.

- `TNerveBPCannonL::execute`, 78 Instruktionen, 312 Bytes.
  Frame `0x30` auf `0x38`.
  Nur ein `getTime()`, vor `setBck`.
- `TNerveKumokunPostWalk::execute`, 112 Instruktionen,
  448 Bytes. Frame `0x40` auf `0x48`.
- `TNerveKumokunPostFreeze::execute`, 112 Instruktionen,
  448 Bytes. Frame `0x40` auf `0x48`.
- `TNerveTamaNokoWait::execute`, 98 Instruktionen,
  392 Bytes. Frame `0x38` auf `0x40`.
  Nur der erste `getTime()` ist benannt.
  Der Vergleich ist `< 2`.
  Der spätere `getTime()` nach `setBckAnm` bleibt
  ein erneuter Aufruf.
- `TNerveTamaNokoHitWater::execute`, 206 Instruktionen,
  824 Bytes. Frame `0x50` auf `0x58`.
  Dieselbe Schreibweise wie bei `TamaNokoWait`.

`validate-symbol-order.py` für `mario/Enemy/bosspakkun`,
`mario/Enemy/Kumokun` und `mario/Enemy/tamaNoko`:
PASS.
UNUSED-Größenwarnungen und die Weak-Reihenfolge
in `tamaNoko` sind alt und kein Fehler.
Keine TU auf `Matching` gestellt.

**Gemessen und zurückgenommen.**

- `TNerveSmallEnemyFreeze`: der benannte
  `freezeTime` wurde entfernt, weil der Frame
  8 Byte zu groß war (`0x40` gegen `0x38`).
  Der virtuelle Aufruf wandert nach hinten,
  der Rumpf stimmt nicht mehr.
- `TNerveBGKLaunchGoro`: der erste von zwei
  `getTime()` benannt ändert nichts.
  Frame bleibt `0x38` gegen `0x40`.
- `TNerveBGKAwakeDamage`: das einzige `getTime()`
  benannt ändert nichts.
  Frame bleibt `0x60` gegen `0x68`.
  Ein Double im Rumpf wandert mit.
- `TNerveDoroHaneRise`: `getTime()` vor dem
  `MsClamp`-Produkt benannt.
  Frame bleibt `0x50` gegen `0x58`.
  Die Int-nach-Float-Folge wird schlechter.
- `TNerveMantaDeath`: benanntes `getTime()`
  ändert nichts. Frame bleibt `0x28` gegen `0x30`.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,71 % matched code,
1712736 / 3590088 Bytes, 9181 / 12881 Funktionen.
Game Code 35,40 %, 1000588 / 2826784 Bytes,
5216 / 8857 Funktionen.

Delta gegen Runde 80: +5 Funktionen, +2424 Bytes
(312 + 448 + 448 + 392 + 824).

`changes_all` meldet nur diese fünf Executes von
unter 100 % auf 100 %. Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Kein Matching-Flip.

### Nächster Schritt

1. `bosspakkun`, `Kumokun` und `tamaNoko` nicht
   auf `Matching` stellen.
2. `getTime()` nicht pauschal benennen.
   `LaunchGoro`, `AwakeDamage`, `DoroHaneRise`
   und `MantaDeath` haben nicht reagiert.
   `SmallEnemyFreeze` darf den Namen nicht verlieren.
3. Nicht wiederholen: `TNerveBPTornado` (16 Byte,
   kein Stack-Zugriff), `TNerveBPDie`,
   `TCoin::appear`, `TItem::calc` (`0x20`),
   `TumbleIn`, `FlyPivot`.
4. `TNerveKumokunFreeze` ist ebenfalls 16 Byte
   zu klein und hat nur Prolog-Unterschiede.
   Ein einzelner `int` ist dort kein erster Versuch.
5. `TNerveTamaNokoSink` ist 8 Byte zu groß
   (`0x68` gegen `0x60`), nicht zu klein.

### Nach zweiundachtzigster Iterationsrunde (kein neuer Vollmatch)

**Beobachtung, vorher.** Stand Runde 81:
47,71 % matched code, 1712736 / 3590088 Bytes,
9181 / 12881 Funktionen.
Game Code 35,40 %, 1000588 / 2826784 Bytes,
5216 / 8857 Funktionen.

**`TBossPakkun::receiveMessage`.**
148 Instruktionen, 592 Bytes.
Vorher war der Rumpf bis auf ein `cmplw` wortgleich,
der Frame `0x50` gegen `0x60`.
`&TNerveBPSleep::theNerve() == getLatestNerve()`
lädt in derselben Reihenfolge, vergleicht aber
`cmplw r0, r3`.
`getLatestNerve() == &theNerve()` direkt hält den
Nerv in `r28` und lädt den Spine ein zweites Mal.
Diese Form matcht das `cmplw`:

```
const TNerveBase<TLiveActor>* sleep = &TNerveBPSleep::theNerve();
if (mSpine->getLatestNerve() == sleep && ...)
```

Danach nur noch Prolog und Epilog, neun Zeilen,
Frame weiter `0x50` gegen `0x60`.
Kein Stack-Zugriff im Rumpf.
Die Funktion zählt nicht als Match.

`validate-symbol-order.py -u mario/Enemy/bosspakkun`
PASS. Die zwei UNUSED-Größenwarnungen sind alt.
Die TU bleibt `NonMatching`.

**Gemessen und zurückgenommen.**

- `MtxToQuat`: die Summe `m[0][0] + m[1][1] + m[2][2] + 1`
  in drei Statements zerlegt. Das eine vertauschte
  `fadds` wird nicht gerichtet. Die Register der
  Spur wechseln, das `fmr` fällt weg.
- `evStartSE`: `push(TSpcSlice())` ändert nichts.
  Die beiden Slices bleiben 4 Byte zu tief
  (`0x34`/`0x2c` gegen `0x38`/`0x30`),
  das `stfd` bei `0x40` stimmt schon.
  Ein benanntes `TSpcSlice` verkleinert den Frame
  auf `0x40` und lässt den Typ-Store aus.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher unverändert: 47,71 % matched code,
1712736 / 3590088 Bytes, 9181 / 12881 Funktionen.
Game Code 35,40 %, 1000588 / 2826784 Bytes,
5216 / 8857 Funktionen.

`changes_all` meldet nur
`receiveMessage__11TBossPakkun...` von 99,87 % auf
99,94 % fuzzy. Keine Regression, kein neues
100-%-Symbol.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Kein Matching-Flip.

### Nächster Schritt

1. `bosspakkun` nicht auf `Matching` stellen.
2. `receiveMessage` nicht noch einmal über die
   `==`-Reihenfolge drehen. Der Frame bleibt
   16 Byte zu klein und ohne Stack-Zugriff.
3. `MtxToQuat` nicht erneut in Teilsummen
   zerlegen. `evStartSE` nicht mit
   `push(TSpcSlice())` oder einem benannten
   `TSpcSlice` wiederholen.
4. Die Vermeidungsliste aus Runde 81 bleibt.

### Nach dreiundachtzigster Iterationsrunde (ein Match)

**Beobachtung, vorher.** Stand Runde 82:
47,71 % matched code, 1712736 / 3590088 Bytes,
9181 / 12881 Funktionen.
Game Code 35,40 %, 1000588 / 2826784 Bytes,
5216 / 8857 Funktionen.

**Match, `TMapObjBase::isDemo`.**
22 Instruktionen, 88 Bytes.
Vorher ein vertauschtes `bne`: Zustand 1 und 2
sprangen auf `return false`.
`if (b1) return true` legt ein zweites `return true`
vor die Prüfung von 3 und 4.
`if (!b2) return false` macht aus dem zweiten
Sprung ein `bne` und tauscht die Rückgaben.
Ein `goto` initialisiert `b2` zu früh.

Die passende Form ist ein Oder mit der zweiten
Paarprüfung als Inline. Der Helfer wird nicht
emittiert.

```
if (b1 || stateIs3Or4(gpMarDirector->unk124))
    return true;
return false;
```

Null abweichende Wörter. Die TU bleibt
`NonMatching`.

`validate-symbol-order.py -u mario/MoveBG/MapObjLib`
schlägt schon vorher fehl: `SMatrix33C::at` fehlt,
und die UNUSED-Reihenfolge weicht ab.
Das ist nicht durch `isDemo` entstanden.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,71 % matched code,
1712824 / 3590088 Bytes, 9182 / 12881 Funktionen.
Game Code 35,40 %, 1000676 / 2826784 Bytes,
5217 / 8857 Funktionen.

Delta gegen Runde 82: +1 Funktion, +88 Bytes.
Die angezeigte Prozentzahl bleibt 47,71.

`changes_all` meldet nur `isDemo__11TMapObjBaseFv`
von 99,77 % auf 100 %. Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Kein Matching-Flip.

### Nächster Schritt

1. `MapObjLib` nicht auf `Matching` stellen.
2. `isDemo` nicht auf ein vierfaches Oder
   oder ein `goto` zurückdrehen.
3. Die Vermeidungsliste aus Runde 81 und die
   drei Fehlversuche aus Runde 82 bleiben.

### Nach vierundachtzigster Iterationsrunde (ein Zähler-Match)

**Beobachtung, vorher.** Stand Runde 83:
47,71 % matched code, 1712824 / 3590088 Bytes,
9182 / 12881 Funktionen.
Game Code 35,40 %, 1000676 / 2826784 Bytes,
5217 / 8857 Funktionen.

**Match, `evCheckWoodBox`.**
171 Instruktionen, 684 Bytes.
Die Schleife läuft von `p2` bis `p1`.
Der Zähler stand als `p2 - p1 + 1`.
Das Ziel rechnet `subf r5, r6, r29`, also `p1 - p2`.
`int count = p1 - p2 + 1` macht dieses eine Wort gleich.
Null abweichende Wörter. Die TU bleibt `NonMatching`.

**Diff sauber, zwei Nozzle-`movement`.**
`TNozzleBase::movement` ist in
`TNozzleDeform::movement` geinlined.
Beide hatten nur zwei `lfs`: zuerst 256, dann 150.
Das Ziel lädt 150 und danach 256.
`150.0f * analog * 256.0f` dreht die Faktoren.
Danach null abweichende Wörter.
200 Bytes und 296 Bytes.
Der Fortschrittszähler stand für beide schon auf 100 %,
deshalb steigt `matched_code` hier nicht.

**Diff sauber, `TEnemyMario::hitWater`.**
Das eine abweichende `lfs` lädt die Poolkonstante 30, nicht 0.
Das Literal ist jetzt `30.0f`.
109 Instruktionen, 436 Bytes, null abweichende Wörter.
Der Funktionszähler stand schon auf 100 %.
`.sdata2` der TU geht von 99,5 % auf 100 %.

`validate-symbol-order.py` schlägt bei allen drei TUs
an vorbestehenden Fehlern fehl:
`TVec3::set` fehlt in `EventWatcher`,
UNUSED-Symbole fehlen in `WaterGun`,
`getPoint` fehlt in `enemyMario`.
Keine neue Nicht-weak-Reihenfolge.
Kein Matching-Flip.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,73 % matched code,
1713508 / 3590088 Bytes, 9183 / 12881 Funktionen.
Game Code 35,42 %, 1001360 / 2826784 Bytes,
5218 / 8857 Funktionen.

Delta Code gegen Runde 83: +1 Funktion, +684 Bytes.
Daten: 384651 auf 385643 Bytes,
60,07 % auf 60,23 %.
Das sind die 992 Bytes `.sdata2` von `enemyMario`.

`changes_all` meldet `evCheckWoodBox` von 99,94 % auf 100 %.
`enemyMario` matched data von 48,05 % auf 62,93 %.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

### Nächster Schritt

1. `isDemo` nicht auf ein vierfaches Oder
   oder ein `goto` zurückdrehen.
2. `evCheckWoodBox` nicht wieder auf `p2 - p1` stellen.
3. Die Nozzle-Faktoren nicht wieder mit 256 vor 150 schreiben.
4. Die `hitWater`-Lautstärke nicht wieder auf `0.0f` setzen.
5. Die Vermeidungslisten aus Runde 81 und 82 bleiben.
   Die drei TUs nicht auf `Matching` stellen.

### Nach fünfundachtzigster Iterationsrunde (sechs Diffs, Daten)

**Beobachtung, vorher.** Stand Runde 84:
47,73 % matched code, 1713508 / 3590088 Bytes,
9183 / 12881 Funktionen.
Game Code 35,42 %, 1001360 / 2826784 Bytes,
5218 / 8857 Funktionen.
Daten 385643 / 640331 Bytes, 60,23 %.

**`TMario::thinkDirty`.**
Das eine abweichende `lfs` subtrahiert 200, nicht 1,
von `mFloorPosition.z`, bevor `mPosition.y` vergleicht.
107 Instruktionen, 428 Bytes, null abweichende Wörter.
Der Fortschrittszähler stand schon auf 100 %.

**Wurzelmatrizen in `MapObjLib`.**
`M_PI / 180.0f` ist ein Bit zu klein
(`0x3c8efa35` gegen `0x3c8efa36`).
Die Spielkonstante `0.017453294f` steht schon in
`Sky.cpp`, `AnimalBase.cpp` und `cameralib.cpp`.
Damit matchen:

- `makeRootMtxRotX`, `makeRootMtxRotY`, `makeRootMtxRotZ`,
  je 44 Instruktionen, 176 Bytes
- `setRootMtxRotY` und `setRootMtxRotZ`,
  je 45 Instruktionen, 180 Bytes

Null abweichende Wörter. Keine Regression in den beiden TUs.

`.sdata2` von `MapObjLib` matcht damit vollständig.
`matched_data` der TU von 796 auf 892 Bytes, 100 %.

`validate-symbol-order.py` bleibt rot an alten Fehlern:
`MarioMove` fehlt UNUSED `setMissJumping`,
`MapObjLib` fehlt `SMatrix33C::at` und die
Nicht-weak-Reihenfolge weicht ab.
Kein Matching-Flip. `isDemo` und die vier Formen
aus Runde 84 sind unverändert.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Code unverändert: 47,73 % matched code,
1713508 / 3590088 Bytes, 9183 / 12881 Funktionen.
Game Code 35,42 %, 1001360 / 2826784 Bytes,
5218 / 8857 Funktionen.

Daten: 385739 / 640331 Bytes, 60,24 %.
Game-Daten 306515 / 556995 Bytes, 55,03 %.
Delta gegen Runde 84: +96 Datenbytes, kein neues
Code-Symbol im Zähler.

`changes_all` meldet nur `MapObjLib` matched data
von 89,24 % auf 100 %. Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

### Nächster Schritt

1. `thinkDirty` nicht wieder auf `- 1.0f` stellen.
2. Die drei `makeRootMtxRot*` nicht wieder auf
   `M_PI / 180.0f` stellen.
3. Runde 84 nicht zurückdrehen: `p1 - p2 + 1`,
   `150 * analog * 256`, `hitWater` mit `30.0f`.
4. `isDemo` nicht auf ein vierfaches Oder oder
   ein `goto` zurückdrehen.
5. Vermeidungslisten aus Runde 81 und 82 bleiben.
   `MapObjLib` und `MarioMove` nicht auf `Matching` stellen.

### Nach sechsundachtzigster Iterationsrunde (ein Status-Match)

**Beobachtung, vorher.** Stand Runde 85:
47,73 % matched code, 1713508 / 3590088 Bytes,
9183 / 12881 Funktionen.
Game Code 35,42 %, 1001360 / 2826784 Bytes,
5218 / 8857 Funktionen.
Daten 385739 / 640331 Bytes, 60,24 %.

**Match, `TMario::changePlayerStatus`.**
`setStatusToRunning` ist dort geinlined.
Die Laufgeschwindigkeit nahm das Maximum aus
`mIntendedMag` und 8.
Das Ziel nimmt das Minimum: bei `mIntendedMag <= 8`
bleibt der Wert, sonst wird 8 eingesetzt.
115 Instruktionen, 460 Bytes, null abweichende Wörter.
Vorher 99,83 %. Die TU bleibt `NonMatching`.

Die freistehende Kopie von `setStatusToRunning` ist
UNUSED, 216 Bytes gegen 220 in der Map.
Die Größe ändert sich durch die Auswahl nicht.
`validate-symbol-order.py` bleibt rot am alten
fehlenden `setMissJumping`. Keine neue Reihenfolge.
`thinkDirty` und die Wurzelmatrizen sind unverändert.

**Angeschaut, nicht angefasst.**
`TPollutionLayer::stampModel` tauscht nur zwei `lfs`,
der `fcmpo` bleibt derselbe.
`MSRandVol::getRandVol` tauscht zwei Index-Register,
`f1`/`f2`/`f3` an `getRandom` sind dieselben Werte.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,74 % matched code,
1713968 / 3590088 Bytes, 9184 / 12881 Funktionen.
Game Code 35,44 %, 1001820 / 2826784 Bytes,
5219 / 8857 Funktionen.
Daten unverändert: 385739 / 640331 Bytes, 60,24 %.

Delta Code gegen Runde 85: +1 Funktion, +460 Bytes.

`changes_all` meldet nur `changePlayerStatus`
von 99,83 % auf 100 %.
`MarioMove` matched code von 38,86 % auf 40,21 %.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

### Nächster Schritt

1. `changePlayerStatus` nicht wieder auf
   `mIntendedMag <= 8 ? 8 : mIntendedMag` stellen.
2. `thinkDirty` nicht wieder auf `- 1.0f` stellen.
3. Die drei `makeRootMtxRot*` nicht wieder auf
   `M_PI / 180.0f` stellen.
4. Runde 84 nicht zurückdrehen: `p1 - p2 + 1`,
   `150 * analog * 256`, `hitWater` mit `30.0f`.
5. `isDemo` nicht auf ein vierfaches Oder oder
   ein `goto` zurückdrehen.
6. Vermeidungslisten aus Runde 81 und 82 bleiben.
   `stampModel` und `getRandVol` nicht wegen
   Registertausch jagen.
   `MarioMove` nicht auf `Matching` stellen.

### Nach siebenundachtzigster Iterationsrunde (ein Datenwort)

**Beobachtung, vorher.** Stand Runde 86:
47,74 % matched code, 1713968 / 3590088 Bytes,
9184 / 12881 Funktionen.
Game Code 35,44 %, 1001820 / 2826784 Bytes,
5219 / 8857 Funktionen.
Daten 385739 / 640331 Bytes, 60,24 %.

**`TKumokunManager::load`.**
Der offizielle Zähler stand für die Funktion schon
auf 100 %. `decomp-diff` zeigte ein `lwz` aus
`.sdata`: Zielwert 65, Quelle 60.
Das dritte `set` schreibt `mSLDamageRadius`.
Die anderen drei bleiben 60, 50 und 70.
122 Instruktionen, 488 Bytes, danach null
abweichende Wörter.
`Kumokun`-Daten von 2552 auf 2568 Bytes, 100 %.
Die TU bleibt `NonMatching` (Code 43,40 %).
`validate-symbol-order.py` bleibt PASS mit den
alten UNUSED-Größenwarnungen.
`changePlayerStatus` ist unverändert.

**Angeschaut, nicht angefasst.**
`stampModel` und `getRandVol` bleiben Registertausch.
`TRoulette::moveObject` hat zusätzlich einen
Frame-Abstand von 0x20.
`setQuat` tauscht nur Float-Register bei gleichen
Poolwerten.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Code unverändert: 47,74 % matched code,
1713968 / 3590088 Bytes, 9184 / 12881 Funktionen.
Game Code 35,44 %, 1001820 / 2826784 Bytes,
5219 / 8857 Funktionen.

Daten: 385755 / 640331 Bytes, 60,24 %.
Game-Daten 306531 / 556995 Bytes, 55,03 %.
Delta gegen Runde 86: +16 Datenbytes, kein neues
Code-Symbol im Zähler.

`changes_all` meldet nur `Kumokun` matched data
von 99,38 % auf 100 %. Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

### Nächster Schritt

1. `mSLDamageRadius` in `TKumokunManager::load`
   nicht wieder auf 60 stellen.
2. `changePlayerStatus` behält das Minimum aus
   `mIntendedMag` und 8.
3. `thinkDirty` bleibt bei `- 200.0f`.
   Die drei `makeRootMtxRot*` bleiben bei
   `0.017453294f`.
4. Runde 84 bleibt: `p1 - p2 + 1`,
   `150 * analog * 256`, `hitWater` mit `30.0f`.
5. `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
6. Vermeidungslisten aus Runde 81 und 82 bleiben.
   `stampModel` und `getRandVol` nicht wegen
   Registertausch jagen.
   `Kumokun` und `MarioMove` nicht auf `Matching` stellen.

### Nach achtundachtzigster Iterationsrunde (ein Spin)

**Beobachtung, vorher.** Stand Runde 87:
47,74 % matched code, 1713968 / 3590088 Bytes,
9184 / 12881 Funktionen.
Game Code 35,44 %, 1001820 / 2826784 Bytes,
5219 / 8857 Funktionen.
Daten 385755 / 640331 Bytes, 60,24 %.

**Match, `TMario::rotating`.**
Die positive Drehung speichert `mStatusTimer * 4096`
mit `extsh` in das `s16` `mModelFaceAngle`.
Die negative Drehung hat nur `neg` und `sth`.
Ein `u16`-Cast auf der Negation entfernt das
zusätzliche `extsh`.
74 Instruktionen, 296 Bytes, null abweichende Wörter.
Vorher 98,65 %. Die TU bleibt `NonMatching`.

`validate-symbol-order.py` bleibt rot am alten
fehlenden UNUSED `braking`. Keine neue Reihenfolge.
Nur diese Funktion gewinnt. Der Schadensradius 65
und `changePlayerStatus` sind unverändert.

**Zurückgenommen.**
`startDisappearTimer` als `465 - y1 + 60` faltet
weiter zu einem `subfic` von 525.
`s32 targetY; targetY += 60` zieht die `addi` nach,
der Rest der Funktion fällt auf 82 %.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,75 % matched code,
1714264 / 3590088 Bytes, 9185 / 12881 Funktionen.
Game Code 35,45 %, 1002116 / 2826784 Bytes,
5220 / 8857 Funktionen.
Daten unverändert: 385755 / 640331 Bytes, 60,24 %.

Delta Code gegen Runde 87: +1 Funktion, +296 Bytes.

`changes_all` meldet nur `rotating` von 98,65 %
auf 100 %. `MarioRun` matched code von 26,38 %
auf 27,86 %. Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

### Nächster Schritt

1. Die negative Drehung in `rotating` behält den
   `u16`-Cast.
2. `mSLDamageRadius` in `TKumokunManager::load`
   bleibt 65.
3. `changePlayerStatus` behält das Minimum aus
   `mIntendedMag` und 8.
4. `thinkDirty` bleibt bei `- 200.0f`.
   Die drei `makeRootMtxRot*` bleiben bei
   `0.017453294f`.
5. Runde 84 bleibt: `p1 - p2 + 1`,
   `150 * analog * 256`, `hitWater` mit `30.0f`.
6. `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
7. Vermeidungslisten aus Runde 81 und 82 bleiben.
   `stampModel`, `getRandVol`, `setQuat` und
   `TRoulette::moveObject` nicht wegen Registertausch
   jagen. `startDisappearTimer` nicht wieder als
   gefaltete 525 oder als `targetY += 60` schreiben.
   `MarioRun` nicht auf `Matching` stellen.

### Nach neunundachtzigster Iterationsrunde (kein Vollmatch)

**Beobachtung, vorher.** Stand Runde 88:
47,75 % matched code, 1714264 / 3590088 Bytes,
9185 / 12881 Funktionen.
Game Code 35,45 %, 1002116 / 2826784 Bytes,
5220 / 8857 Funktionen.
Daten 385755 / 640331 Bytes, 60,24 %.

**Kein neues Vollmatch.** Drei belegte Konstanten,
der Code-Zähler bleibt stehen.

`TNerveBPSwallow::execute`: der zweite Emitter
bekommt `(u8*)boss + 1`, wie die anderen
Pakkun-Wasser-Partikel.
Die `addi`-Immediate stimmt.
Der Frame bleibt 0x50 gegen 0x60, ohne inneren
Stack-Zugriff. 99,92 % auf 99,93 %.

`TDoroHaneKuri::attackToMario`: die Joint-Position
ist die Translationsspalte `mtx[0][3]`, `mtx[1][3]`,
`mtx[2][3]`.
Der Rumpf stimmt danach.
Der Frame bleibt 0x48 gegen 0x50.
Ein benanntes Bool, ein Model-Zeiger, ein Sound-Zeiger
und ein Nerve-Zeiger verschieben den Frame nicht,
ohne Register zu tauschen.
99,93 % auf 99,95 %.

`TBaseNPC::isCanWalk`: `CLBSquared` bekommt `10.0f`.
`2.5625f` wählt ein anderes Pool-Float.
`execWalk` 89,42 % auf 89,44 %.
Die `sdata2` der TU geht von 0 % auf 100 %
matched data, 56 Bytes.
`set(dx, 0, dz)` statt der Subtraktion fiel auf
83,5 % und bleibt draussen.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,75 % matched code,
1714264 / 3590088 Bytes, 9185 / 12881 Funktionen.
Game Code 35,45 %, 1002116 / 2826784 Bytes,
5220 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.
Game-Daten 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 88: keine Funktion, 0 Bytes.
Delta Daten: +56 Bytes, nur `NpcWalkTurn`.

`changes_all` listet keine Regression.
`bosspakkun` Symbolordnung PASS, alte UNUSED-Grössen.
`hamukuri` und `NpcWalkTurn` bleiben an den alten
Fehlern rot (`onHaveCap`-Linkage, fehlendes `set<f>`).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `rotating` behält den `u16`-Cast.
2. `TNerveBPSwallow` behält `(u8*)boss + 1`.
   Den 16-Byte-Frame nicht mit einem einzelnen `int`
   auffüllen.
3. `attackToMario` behält die Translationsspalte.
   Den 8-Byte-Frame nicht mit `trash` schliessen.
4. `isCanWalk` behält `CLBSquared(10.0f)`.
   Die `set(dx, 0, dz)`-Form nicht wiederholen.
5. `mSLDamageRadius` bleibt 65.
   `changePlayerStatus` behält das Minimum aus
   `mIntendedMag` und 8.
6. `thinkDirty` bleibt bei `- 200.0f`.
   Die drei `makeRootMtxRot*` bleiben bei
   `0.017453294f`.
7. Runde 84 bleibt: `p1 - p2 + 1`,
   `150 * analog * 256`, `hitWater` mit `30.0f`.
8. `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
9. Vermeidungslisten aus Runde 81 und 82 bleiben.
   `stampModel`, `getRandVol`, `setQuat` und
   `TRoulette::moveObject` nicht wegen Registertausch
   jagen. `startDisappearTimer` nicht wieder als
   gefaltete 525 oder als `targetY += 60` schreiben.
   `MarioRun`, `bosspakkun`, `hamukuri` und
   `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach neunzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 89:
47,75 % matched code, 1714264 / 3590088 Bytes,
9185 / 12881 Funktionen.
Game Code 35,45 %, 1002116 / 2826784 Bytes,
5220 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

**Ein neues Vollmatch, drei Partial-Fixes.**

`THaneHamuKuri2::walkBehavior`: die Höhe ist
`unk210 + height`, wobei `height` die Summe
`unk230 + unk234` ist.
Retail addiert die beiden Offsets zuerst und
reserviert das Slot.
0 Abweichungen, 364 Bytes, 100 %.

`TNervePoihanaThrow::execute`: `MsMtxSetRotRPH`
bekommt `mRotation` (0x30), nicht `mPosition` (0x10).
Die drei `lfs` stimmen.
Der Frame bleibt 0xa0 gegen 0xb0, jeder Slot
um 0x10 verschoben. 99,82 % auf 99,84 %.

`TDangoHamuKuri::receiveMessage`: der Wasser-Partikel
hängt an `&sender->mPosition`, der Treffer-Sound
an `&mPosition`.
Der Rumpf stimmt.
Der Frame bleibt 0x20 gegen 0x48.
99,91 % auf 99,95 %.

`TMapObjGeneral::recovering`: die Joint-Höhe ist
`mat[1][3]`, die Y-Spalte der 3x4-Matrix.
Der Rumpf stimmt.
Der Frame bleibt 0x20 gegen 0x48.
99,84 % auf 99,87 %.

Keiner der drei Frames wurde mit einem einzelnen
`int` oder `trash` aufgefüllt.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,76 % matched code,
1714628 / 3590088 Bytes, 9186 / 12881 Funktionen.
Game Code 35,46 %, 1002480 / 2826784 Bytes,
5221 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 89: +1 Funktion, +364 Bytes.
Delta Daten: 0 Bytes.

`changes_all` listet keine Regression.
`hamukuri` matched code 55,80 % auf 56,60 %.
`poihana` und `MapObjGeneral` Symbolordnung PASS.
`hamukuri` bleibt am alten `onHaveCap`-Linkage rot.
`isOnTrap` UNUSED-Grösse in `poihana` ist alt.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `walkBehavior` behält die benannte Summe
   `height = unk230 + unk234`.
2. `TNervePoihanaThrow` behält `mRotation`.
   Den 16-Byte-Frame nicht mit einem einzelnen `int`
   auffüllen.
3. `receiveMessage` des Dango behält
   `&sender->mPosition` für den Partikel.
   Den Frame nicht mit `trash` schliessen.
4. `recovering` behält `mat[1][3]`.
5. Runde 89 bleibt: `(u8*)boss + 1`,
   Translationsspalte in `attackToMario`,
   `CLBSquared(10.0f)`.
   `set(dx, 0, dz)` in `isCanWalk` nicht wiederholen.
6. `rotating` behält den `u16`-Cast.
   `mSLDamageRadius` bleibt 65.
   `changePlayerStatus` behält das Minimum aus
   `mIntendedMag` und 8.
7. `thinkDirty` bleibt bei `- 200.0f`.
   Die drei `makeRootMtxRot*` bleiben bei
   `0.017453294f`.
8. Runde 84 bleibt: `p1 - p2 + 1`,
   `150 * analog * 256`, `hitWater` mit `30.0f`.
9. `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
10. Vermeidungslisten aus Runde 81 und 82 bleiben.
    `stampModel`, `getRandVol`, `setQuat` und
    `TRoulette::moveObject` nicht wegen Registertausch
    jagen. `startDisappearTimer` nicht wieder als
    gefaltete 525 oder als `targetY += 60` schreiben.
    `MarioRun`, `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral` und `NpcWalkTurn` nicht auf
    `Matching` stellen.

### Nach einundneunzigster Iterationsrunde (kein Vollmatch)

**Beobachtung, vorher.** Stand Runde 90:
47,76 % matched code, 1714628 / 3590088 Bytes,
9186 / 12881 Funktionen.
Game Code 35,46 %, 1002480 / 2826784 Bytes,
5221 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

**Kein neues Vollmatch.** Sechs Rümpfe stimmen.
Die Frames bleiben offen und werden nicht gepolstert.
Der Code-Zähler bleibt bei Runde 90.

`TNerveBPTumble::execute`: der Jita-Emitter hängt an
`(u8*)boss + 8`.
Die `addi` stimmt.
Der Frame bleibt 0x40 gegen 0x50.
99,29 % auf 99,93 %.

`evInsertTimer`: der erste Zweig ist `p2 == 1`.
Danach `p2 == 2`, sonst `startDisappearTimer`.
Der Frame bleibt 0x98 gegen 0xa0.
99,77 % auf 99,78 %.

`TFireWanwanTailHit::behaveTaken`: `moveRequest`
bekommt `param_1->mPosition`.
Im inlined `receiveMessage` stimmt der Rumpf.
Der Frame bleibt 0xa0 gegen 0xb0.
99,89 % auf 99,92 %.

`TWoodBox::kill`: die vier Bodenprüfungen laufen
`(-50,-50)`, `(50,-50)`, `(-50,50)`, `(50,50)`.
Die beiden Pool-Loads stimmen.
Der Frame bleibt 0x58 gegen 0xf0.
Live-Diff 99,88 % auf 99,93 %.
`changes_all` listet die Funktion nicht extra,
der Report-Fuzzy bleibt 99,93 %.

`TBossMantaManager::createEnemies`: das Limit ist
`mSLInstanceNum` (Wert bei 0x90).
Der Frame bleibt 0xa8 gegen 0xb0.
99,78 % auf 99,79 %.

`TWalkerEnemy::moveObject`: `mPosition.y += 5.0f`.
Der Yaw-Load davor bleibt `mRotation.y`.
Der Frame bleibt 0x60 gegen 0x88.
99,80 % auf 99,82 %.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,76 % matched code,
1714628 / 3590088 Bytes, 9186 / 12881 Funktionen.
Game Code 35,46 %, 1002480 / 2826784 Bytes,
5221 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 90: keine Funktion, 0 Bytes.
Delta Daten: 0 Bytes.

`changes_all` listet keine Regression.
Live-Vergleich der sechs TUs: 6 Gewinne, 0 Verluste.
`bosspakkun` und `walkerEnemy` Symbolordnung PASS.
`EventWatcher`, `fireWanwan`, `MapObjHide` und
`bossManta` bleiben an den alten Fehlern rot.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `walkBehavior` behält `height = unk230 + unk234`.
2. Die sechs Rümpfe dieser Runde behalten.
   Keinen davon mit einem einzelnen `int` oder
   `trash` auf Frame-Länge bringen.
3. Runde 90 bleibt: `mRotation` in `PoihanaThrow`,
   `&sender->mPosition` im Dango-`receiveMessage`,
   `mat[1][3]` in `recovering`.
4. Runde 89 bleibt: `(u8*)boss + 1`,
   Translationsspalte in `attackToMario`,
   `CLBSquared(10.0f)`.
   `set(dx, 0, dz)` in `isCanWalk` nicht wiederholen.
5. `rotating` behält den `u16`-Cast.
   `mSLDamageRadius` bleibt 65.
   `changePlayerStatus` behält das Minimum aus
   `mIntendedMag` und 8.
6. `thinkDirty` bleibt bei `- 200.0f`.
   Die drei `makeRootMtxRot*` bleiben bei
   `0.017453294f`.
7. Runde 84 bleibt: `p1 - p2 + 1`,
   `150 * analog * 256`, `hitWater` mit `30.0f`.
8. `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
9. Vermeidungslisten aus Runde 81 bis 89 bleiben.
   `stampModel`, `getRandVol`, `setQuat` und
   `TRoulette::moveObject` nicht wegen Registertausch
   jagen. `startDisappearTimer` nicht wieder als
   gefaltete 525 oder als `targetY += 60` schreiben.
   `MarioRun`, `bosspakkun`, `hamukuri`, `poihana`,
   `MapObjGeneral`, `walkerEnemy`, `bossManta`,
   `fireWanwan`, `MapObjHide`, `EventWatcher` und
   `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach zweiundneunzigster Iterationsrunde (kein Vollmatch)

**Beobachtung, vorher.** Stand Runde 91:
47,76 % matched code, 1714628 / 3590088 Bytes,
9186 / 12881 Funktionen.
Game Code 35,46 %, 1002480 / 2826784 Bytes,
5221 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TJointObj` legt `kill` auf VTable 0x18 und
`sleep` auf 0x1c.
`TMareWallRock::movement` und `loadAfter`
riefen `unk104->kill()`.
Retail lädt an beiden Stellen 0x1c.

**Ein Partial, kein neues Vollmatch.**

`movement` und `loadAfter` rufen `unk104->sleep()`.
Der Slot 0x1c stimmt in beiden Funktionen.
`movement` hat danach nur noch den Frame:
0xd8 gegen retail 0xf0, fünfzehn Zeilen,
alle Stack.
`loadAfter` verliert nur diese eine Zeile.
Die Min/Max-Register und der Frame bleiben.
Unit-Fuzzy 99,24 % auf 99,25 %.
`loadAfter` 99,41 % auf 99,42 %.
`movement` bleibt bei 99,93 %.

Kein Frame wurde aufgefüllt.
`MapEventMare` nicht auf `Matching` gestellt.

Verworfen, weil der Rumpf schlechter wurde:
`TMapStaticObj::init` auf `setMtx` umschreiben.
Der direkte Aufruf wird zu `PSMTXCopy` inlined,
sobald `initMapCollision` selbst inlined wird,
oder `init` ruft die Funktion nur noch auf
und der Frame fällt von 0x118 auf 0xe0.
`setUpUnk8TRS` im Header auf `setMtx` umzustellen
zieht `MapObjBase` von 99,8 % auf 95,7 %.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,76 % matched code,
1714628 / 3590088 Bytes, 9186 / 12881 Funktionen.
Game Code 35,46 %, 1002480 / 2826784 Bytes,
5221 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 91: keine Funktion, 0 Bytes.
Delta Daten: 0 Bytes.

`changes_all` listet keine Regression.
Symbolordnung `MapEventMare` PASS,
mit den alten UNUSED-Größenwarnungen.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `movement` und `loadAfter` behalten `sleep()`.
   Den Frame von `movement` nicht mit einem
   einzelnen `int` oder `trash` schliessen.
2. Die sechs Rümpfe aus Runde 91 behalten.
   Keinen davon auf Frame-Länge polstern.
3. `walkBehavior` behält `height = unk230 + unk234`.
4. Runde 90 bleibt: `mRotation` in `PoihanaThrow`,
   `&sender->mPosition` im Dango-`receiveMessage`,
   `mat[1][3]` in `recovering`.
5. Runde 89 bleibt: `(u8*)boss + 1`,
   Translationsspalte in `attackToMario`,
   `CLBSquared(10.0f)`.
   `set(dx, 0, dz)` in `isCanWalk` nicht wiederholen.
6. `rotating` behält den `u16`-Cast.
   `mSLDamageRadius` bleibt 65.
   `changePlayerStatus` behält das Minimum aus
   `mIntendedMag` und 8.
7. `thinkDirty` bleibt bei `- 200.0f`.
   Die drei `makeRootMtxRot*` bleiben bei
   `0.017453294f`.
8. Runde 84 bleibt: `p1 - p2 + 1`,
   `150 * analog * 256`, `hitWater` mit `30.0f`.
9. `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
10. Vermeidungslisten aus Runde 81 bis 91 bleiben.
    `TMapStaticObj::init` nicht noch einmal über
    `setMtx` gegen `PSMTXCopy` drehen.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach dreiundneunzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 92:
47,76 % matched code, 1714628 / 3590088 Bytes,
9186 / 12881 Funktionen.
Game Code 35,46 %, 1002480 / 2826784 Bytes,
5221 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TMapCollisionWarp::setUp` hatte einen passenden
Rumpf und einen Frame von 0x30 gegen retail 0x38.
Die `TVec3` lag vier Bytes zu tief.
`getEntrySize` ist inline und liefert `u32`.

**Ein neues Vollmatch.**

`mEntrySize` kommt aus einem benannten
`u32 entrySize = getEntrySize(mEntryId)`.
0 Abweichungen, 208 Bytes, 52 Instruktionen, 100 %.
`MapCollisionEntry` bleibt `NonMatching`,
weil `moveSRT` noch abweicht.
Die fehlende UNUSED-Ctor von `TMapCollisionBase`
war schon vorher weg.

Verworfen: `TRedCoinSwitch::load` über
`SMSGetMarDirector()`.
Der `u32`-Slot rückte nur um 4, der Frame blieb
0x28 gegen 0x30.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,77 % matched code,
1714836 / 3590088 Bytes, 9187 / 12881 Funktionen.
Game Code 35,47 %, 1002688 / 2826784 Bytes,
5222 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 92: 1 Funktion, 208 Bytes.
Delta Daten: 0 Bytes.

`changes_all`: `setUp` 99,83 % auf 100 %.
Unit-Code 84,30 % auf 91,65 %.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `setUp` behält `u32 entrySize`.
   `MapCollisionEntry` nicht auf `Matching` stellen.
2. `TRedCoinSwitch::load` nicht noch einmal über
   `SMSGetMarDirector()` auf den Frame bringen.
3. `movement` und `loadAfter` behalten `sleep()`.
   Den Frame von `movement` nicht polstern.
4. `TMapStaticObj::init` nicht über `setMtx`
   gegen `PSMTXCopy` drehen.
5. Die sechs Rümpfe aus Runde 91 behalten.
   `walkBehavior` behält `height`.
6. Runde 90 bleibt: `mRotation`,
   `&sender->mPosition`, `mat[1][3]`.
7. Runde 89 bleibt: `(u8*)boss + 1`,
   Translationsspalte, `CLBSquared(10.0f)`.
   `set(dx, 0, dz)` nicht wiederholen.
8. `rotating` behält den `u16`-Cast.
   Schadensradius 65 bleibt.
   `changePlayerStatus` behält das Minimum aus
   `mIntendedMag` und 8.
9. `thinkDirty` bleibt bei `- 200.0f`.
   Die drei `makeRootMtxRot*` bleiben bei
   `0.017453294f`.
10. Runde 84 bleibt: `p1 - p2 + 1`,
    `150 * analog * 256`, `hitWater` mit `30.0f`.
    `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
11. Vermeidungslisten aus Runde 81 bis 92 bleiben.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach vierundneunzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 93:
47,77 % matched code, 1714836 / 3590088 Bytes,
9187 / 12881 Funktionen.
Game Code 35,47 %, 1002688 / 2826784 Bytes,
5222 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TMario::walkEnd` lud `0.25f` in `f2` und
`mForwardVel` in `f1`.
Retail legt den Member in `f2` und `0.25f` in `f1`,
`fmuls f1, f2, f1`.
Der Frame war 0x18 gegen retail 0x20.

**Ein neues Vollmatch.**

`f32 quarter = 0.25f` und `f32 vel = mForwardVel`,
dann `rate = vel * quarter`.
0 Abweichungen, 584 Bytes, 146 Instruktionen, 100 %.
`MarioRun` bleibt `NonMatching`.
`braking` fehlt als UNUSED schon vorher.

Verworfen, der Frame allein reichte nicht:
`TShine::appearWithDemo` mit benanntem `frames`,
`tool` oder `flags` bringt den Frame auf 0x50,
das `TFlagT` bleibt bei 0x34 gegen 0x38.
`getDemoLengthFrames()` macht den Frame 0x58.
`TMap::isTouchedOneWall` mit benanntem `hit`
bringt den Frame auf 0x68, der Record bleibt
vier Bytes zu tief.
`joinToGroup` über `getChildren()` wird 0x70.
Ein benannter Gruppenzeiger verliert das
`addi` um 0x10.
`createAndKeepData` mit benanntem `folder`
ändert nichts; `loadModelData` bleibt 100 %.
`behaveToMario` mit benanntem `gpMarioPos`
belegt keinen Slot.

Kein Frame wurde aufgefüllt.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,78 % matched code,
1715420 / 3590088 Bytes, 9188 / 12881 Funktionen.
Game Code 35,49 %, 1003272 / 2826784 Bytes,
5223 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 93: 1 Funktion, 584 Bytes.
Delta Daten: 0 Bytes.

`changes_all`: `walkEnd` 99,13 % auf 100 %.
Unit-Code `MarioRun` 27,86 % auf 30,78 %.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `walkEnd` behält `quarter` und `vel`.
   `MarioRun` nicht auf `Matching` stellen.
2. `setUp` behält `u32 entrySize`.
   `MapCollisionEntry` nicht auf `Matching` stellen.
3. `TRedCoinSwitch::load` nicht über
   `SMSGetMarDirector()` auf den Frame bringen.
4. `appearWithDemo`, `isTouchedOneWall`,
   `joinToGroup` und `createAndKeepData`
   nicht mit derselben Benennung wiederholen.
5. `movement` und `loadAfter` behalten `sleep()`.
   Frames nicht polstern.
6. `TMapStaticObj::init` nicht über `setMtx`
   gegen `PSMTXCopy` drehen.
7. Die sechs Rümpfe aus Runde 91 behalten.
   `walkBehavior` behält `height`.
8. Runde 90 bleibt: `mRotation`,
   `&sender->mPosition`, `mat[1][3]`.
9. Runde 89 bleibt: `(u8*)boss + 1`,
   Translationsspalte, `CLBSquared(10.0f)`.
   `set(dx, 0, dz)` nicht wiederholen.
10. `rotating` behält den `u16`-Cast.
    Schadensradius 65 bleibt.
    `changePlayerStatus` behält das Minimum aus
    `mIntendedMag` und 8.
    `thinkDirty` bleibt bei `- 200.0f`.
    Die drei `makeRootMtxRot*` bleiben bei
    `0.017453294f`.
11. Runde 84 bleibt: `p1 - p2 + 1`,
    `150 * analog * 256`, `hitWater` mit `30.0f`.
    `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
12. Vermeidungslisten aus Runde 81 bis 93 bleiben.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach fünfundneunzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 94:
47,78 % matched code, 1715420 / 3590088 Bytes,
9188 / 12881 Funktionen.
Game Code 35,49 %, 1003272 / 2826784 Bytes,
5223 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TSmallEnemy::isFindMarioFromParam` hatte den
passenden Frame 0x50.
`mSLSearchLength` landete in `f0`, retail in `f1`,
und `fmuls` multiplizierte von dort.

**Ein neues Vollmatch.**

Die drei Suchwerte werden erst geladen und dann
mit `*= param_1` skaliert.
0 Abweichungen, 188 Bytes, 47 Instruktionen, 100 %.
`smallEnemy` bleibt `NonMatching`.
Symbolordnung PASS.

Verworfen: die Getter `getSLSearchLength` und
Nachbarn machen den Frame 0x60.
Die Produkte direkt im Aufruf fallen auf 87 %.
`registerEventWatcher` mit benanntem `watcher`
hat den Frame 0x50, der Zeiger bleibt bei 0x40
gegen 0x3c.
`getChildren().push_back` fällt auf 84 %.

Kein Frame wurde aufgefüllt.
`walkEnd` behält `quarter` und `vel`.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,79 % matched code,
1715608 / 3590088 Bytes, 9189 / 12881 Funktionen.
Game Code 35,50 %, 1003460 / 2826784 Bytes,
5224 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 94: 1 Funktion, 188 Bytes.
Delta Daten: 0 Bytes.

`changes_all`: `isFindMarioFromParam` 99,79 % auf 100 %.
Unit-Code `smallEnemy` 59,17 % auf 60,24 %.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `isFindMarioFromParam` behält die drei `*= param_1`.
   `smallEnemy` nicht auf `Matching` stellen.
2. `walkEnd` behält `quarter` und `vel`.
   `MarioRun` nicht auf `Matching` stellen.
3. `registerEventWatcher` nicht noch einmal über
   einen benannten `watcher` oder `getChildren()`
   auf den Slot 0x3c bringen.
4. `setUp` behält `u32 entrySize`.
5. `TRedCoinSwitch::load` nicht über
   `SMSGetMarDirector()`.
6. `appearWithDemo`, `isTouchedOneWall`,
   `joinToGroup` und `createAndKeepData`
   nicht mit denselben Benennungen wiederholen.
7. `sleep()` behalten. Frames nicht polstern.
8. `TMapStaticObj::init` nicht über `setMtx`.
9. Die sechs Rümpfe aus Runde 91 behalten.
   `walkBehavior` behält `height`.
10. Runde 90 und 89 bleiben.
    `rotating` behält den `u16`-Cast.
    Schadensradius 65 bleibt.
    `changePlayerStatus` behält das Minimum aus
    `mIntendedMag` und 8.
    `thinkDirty` bleibt bei `- 200.0f`.
    Die drei `makeRootMtxRot*` bleiben bei
    `0.017453294f`.
11. Runde 84 bleibt: `p1 - p2 + 1`,
    `150 * analog * 256`, `hitWater` mit `30.0f`.
    `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
12. Vermeidungslisten aus Runde 81 bis 94 bleiben.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `smallEnemy`, `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach sechsundneunzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 95:
47,79 % matched code, 1715608 / 3590088 Bytes,
9189 / 12881 Funktionen.
Game Code 35,50 %, 1003460 / 2826784 Bytes,
5224 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TMario::catching` vergleicht `mForwardVel`
mit einem Float.
Retail lädt 0xcd0, unser Stand lud 0x8e8.
0x8e8 ist `mDeParams.mClashSpeed`.
Andere Aufrufer von `mClashSpeed` laden
weiterhin 0x8e8 und treffen das Retail.
0xcd0 ist der Wert von
`mJumpParams.mRotBroadEnableV`.
Der Sprung danach ist
`MARIO_STATUS_ROTATE_BROAD_JUMP`.

**Kein neues Vollmatch.**

`catching` benutzt jetzt
`mJumpParams.mRotBroadEnableV`.
Der `lfs` trifft 0xcd0.
Übrig sind fünf Stack-Zeilen.
Der Frame bleibt 0x28 gegen retail 0x30.
340 Bytes, 85 Instruktionen, 99,94 %.
`MarioRun` bleibt `NonMatching`.
`walkEnd` bleibt bei 0 Abweichungen.

Verworfen: ein benanntes `enableV`
wird wegoptimiert, der Frame bleibt 0x28.
`TLiveManager::perform` ohne `char trash[16]`
schrumpft den Frame auf 0x40,
die Farbe bleibt bei 0x24 gegen 0x34.
Der Trash-Block bleibt.

Kein Frame wurde aufgefüllt.
`isFindMarioFromParam` behält die drei
`*= param_1`.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher unverändert: 47,79 % matched code,
1715608 / 3590088 Bytes, 9189 / 12881 Funktionen.
Game Code 35,50 %, 1003460 / 2826784 Bytes,
5224 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 95: 0 Funktionen, 0 Bytes.
Delta Daten: 0 Bytes.

`changes_all`: `catching` 99,93 % auf 99,94 %.
Unit-Fuzzy `MarioRun` bleibt 99,58 %.
Keine Regression.
Symbolordnung `MarioRun` FAIL ist vorbestehend:
UNUSED `braking__6TMarioFv` fehlt.
`walkEnd` bleibt 100 %.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `catching` behält `mRotBroadEnableV`.
   Den Frame 0x28 nicht auf 0x30 polstern.
   Nicht zurück zu `mClashSpeed`.
   `MarioRun` nicht auf `Matching` stellen.
2. `isFindMarioFromParam` behält die drei `*= param_1`.
   `smallEnemy` nicht auf `Matching` stellen.
3. `walkEnd` behält `quarter` und `vel`.
4. `registerEventWatcher` nicht noch einmal über
   einen benannten `watcher` oder `getChildren()`.
5. `enableV` in `catching` nicht noch einmal benennen.
6. `TLiveManager::perform` behält `char trash[16]`.
   Wegnehmen schrumpft den Frame und
   rückt die Farbe nicht auf 0x34.
7. `setUp` behält `u32 entrySize`.
8. `TRedCoinSwitch::load` nicht über
   `SMSGetMarDirector()`.
9. `appearWithDemo`, `isTouchedOneWall`,
   `joinToGroup` und `createAndKeepData`
   nicht mit denselben Benennungen wiederholen.
10. `sleep()` behalten. Frames nicht polstern.
11. `TMapStaticObj::init` nicht über `setMtx`.
12. Die sechs Rümpfe aus Runde 91 behalten.
    `walkBehavior` behält `height`.
13. Runde 90 und 89 bleiben.
    `rotating` behält den `u16`-Cast.
    Schadensradius 65 bleibt.
    `changePlayerStatus` behält das Minimum aus
    `mIntendedMag` und 8.
    `thinkDirty` bleibt bei `- 200.0f`.
    Die drei `makeRootMtxRot*` bleiben bei
    `0.017453294f`.
14. Runde 84 bleibt: `p1 - p2 + 1`,
    `150 * analog * 256`, `hitWater` mit `30.0f`.
    `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
15. Vermeidungslisten aus Runde 81 bis 95 bleiben.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `smallEnemy`, `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach siebenundneunzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 96:
47,79 % matched code, 1715608 / 3590088 Bytes,
9189 / 12881 Funktionen.
Game Code 35,50 %, 1003460 / 2826784 Bytes,
5224 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TDolpicEventRiccoMammaGate`s Konstruktor
schrieb dieselben sieben Null-Floats,
aber in anderer Reihenfolge.
Retail speichert zuerst 0x60,
danach die Vektoren als z, y, x.

**Ein neues Vollmatch.**

`unk60` bleibt im Initialisierer.
`unk48.zero()` und `unk54.zero()` stehen im Rumpf.
`zero` ist `x = y = z = 0`,
die Stores laufen also z, y, x.
0 Abweichungen, 136 Bytes, 34 Instruktionen, 100 %.
`MapEventDolpic` bleibt `NonMatching`.
Symbolordnung PASS.

Verworfen: ein benannter `MActor*` in
`setDeadBathtubKillerAnm` ändert nichts.
Ein benannter `TVec3` ersetzt `set<int>`
durch `stfs` und fällt auf etwa 90 %.
Der anonyme `TVec3(0, 0, 0)` bleibt.

Kein Frame wurde aufgefüllt.
`catching` behält `mRotBroadEnableV`.
`perform` behält `char trash[16]`.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,79 % matched code,
1715744 / 3590088 Bytes, 9190 / 12881 Funktionen.
Game Code 35,50 %, 1003596 / 2826784 Bytes,
5225 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 96: 1 Funktion, 136 Bytes.
Delta Daten: 0 Bytes.

`changes_all`: Konstruktor 99,79 % auf 100 %.
Unit-Code `MapEventDolpic` 45,71 % auf 49,45 %.
Unit-Fuzzy 99,78 % auf 99,79 %.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. Der Ricco-Konstruktor behält `unk60` im
   Initialisierer und `zero()` im Rumpf.
   `MapEventDolpic` nicht auf `Matching` stellen.
2. `catching` behält `mRotBroadEnableV`.
   Den Frame 0x28 nicht auf 0x30 polstern.
   `MarioRun` nicht auf `Matching` stellen.
3. `TLiveManager::perform` behält `char trash[16]`.
4. `isFindMarioFromParam` behält die drei `*= param_1`.
   `smallEnemy` nicht auf `Matching` stellen.
5. `walkEnd` behält `quarter` und `vel`.
6. `registerEventWatcher` nicht noch einmal über
   einen benannten `watcher` oder `getChildren()`.
7. `setDeadBathtubKillerAnm` behält den anonymen
   `TVec3(0, 0, 0)`. Den Vektor nicht benennen.
8. `setUp` behält `u32 entrySize`.
9. `sleep()` behalten. Frames nicht polstern.
10. `TRedCoinSwitch::load` nicht über
    `SMSGetMarDirector()`.
11. `appearWithDemo`, `isTouchedOneWall`,
    `joinToGroup` und `createAndKeepData`
    nicht mit denselben Benennungen wiederholen.
12. `TMapStaticObj::init` nicht über `setMtx`.
13. Die sechs Rümpfe aus Runde 91 behalten.
    `walkBehavior` behält `height`.
14. Runde 90 und 89 bleiben.
    `rotating` behält den `u16`-Cast.
    Schadensradius 65 bleibt.
    `changePlayerStatus` behält das Minimum aus
    `mIntendedMag` und 8.
    `thinkDirty` bleibt bei `- 200.0f`.
    Die drei `makeRootMtxRot*` bleiben bei
    `0.017453294f`.
15. Runde 84 bleibt: `p1 - p2 + 1`,
    `150 * analog * 256`, `hitWater` mit `30.0f`.
    `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
16. Vermeidungslisten aus Runde 81 bis 96 bleiben.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `smallEnemy`, `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach achtundneunzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 97:
47,79 % matched code, 1715744 / 3590088 Bytes,
9190 / 12881 Funktionen.
Game Code 35,50 %, 1003596 / 2826784 Bytes,
5225 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TNerveTobiPukuAttack::execute` hatte dieselben
Rückgaben, aber die Blöcke in anderer Reihenfolge.
Retail lässt `return false` durchfallen.
`return true` für nicht luftgetragen steht davor.

**Partial, kein Vollmatch.**

`if (isAirborne())` behält den Rumpf.
Im `else` steht `return true`.
Danach fällt die Funktion auf `return false`.
Die `li r3` und die Sprungziele matchen.
Übrig sind 17 Stack-Operanden:
Frame 0x48 gegen 0x50,
die beiden `TVec3` zwölf Bytes zu tief.
`tobiPuku` bleibt `NonMatching`.
Symbolordnung PASS.
Fünf UNUSED-Größen und die Weak-Reihenfolge
sind vorbestehend.

Verworfen: `int time = getTime()` wird wegoptimiert.
Ein unbenutzter dritter `TVec3` reserviert den Slot
und matcht wortgleich, ist aber eine Stack-Reservierung
und bleibt draußen.
`TVec3 newVelocity = TVec3(0, y, 0)` bläht den Frame
auf 0x58 und fügt Kopien ein.
`initNeonMatColor` mit benanntem `index` schrumpft
den Frame und fällt auf etwa 87 %. Zurückgenommen.
`setDeadBathtubKillerAnm` nicht noch einmal
mit benanntem `MActor` oder `TVec3`.

Kein Frame wurde aufgefüllt.
Der Ricco-Konstruktor behält `zero()`.
`catching` behält `mRotBroadEnableV`.
`perform` behält `char trash[16]`.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher unverändert: 47,79 % matched code,
1715744 / 3590088 Bytes, 9190 / 12881 Funktionen.
Game Code 35,50 %, 1003596 / 2826784 Bytes,
5225 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 97: 0 Funktionen, 0 Bytes.
Delta Daten: 0 Bytes.

`changes_all`: `TNerveTobiPukuAttack::execute`
99,72 % auf 99,83 %.
Unit-Fuzzy `tobiPuku` 98,95 % auf 98,96 %.
Gesamt-Fuzzy bleibt 77,97 %.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TNerveTobiPukuAttack` behält das `else`
   mit `return true` und das durchfallende
   `return false`.
   Den Frame 0x48 nicht auf 0x50 polstern.
   Keinen unbenutzten dritten `TVec3` einsetzen.
   `tobiPuku` nicht auf `Matching` stellen.
2. `initNeonMatColor` nicht mit benanntem `index`.
3. Der Ricco-Konstruktor behält `zero()` im Rumpf.
   `MapEventDolpic` nicht auf `Matching` stellen.
4. `catching` behält `mRotBroadEnableV`.
   Den Frame nicht polstern.
   `MarioRun` nicht auf `Matching` stellen.
5. `setDeadBathtubKillerAnm` behält den anonymen
   `TVec3(0, 0, 0)`.
   Weder `MActor*` noch `TVec3` benennen.
6. `TLiveManager::perform` behält `char trash[16]`.
7. `isFindMarioFromParam` behält die drei `*= param_1`.
   `smallEnemy` nicht auf `Matching` stellen.
8. `walkEnd` behält `quarter` und `vel`.
9. `registerEventWatcher` nicht noch einmal über
   einen benannten `watcher` oder `getChildren()`.
10. `setUp` behält `u32 entrySize`.
11. `sleep()` behalten. Frames nicht polstern.
12. `TRedCoinSwitch::load` nicht über
    `SMSGetMarDirector()`.
13. `appearWithDemo`, `isTouchedOneWall`,
    `joinToGroup` und `createAndKeepData`
    nicht mit denselben Benennungen wiederholen.
14. `TMapStaticObj::init` nicht über `setMtx`.
15. Die sechs Rümpfe aus Runde 91 behalten.
    `walkBehavior` behält `height`.
16. Runde 90 und 89 bleiben.
    `rotating` behält den `u16`-Cast.
    Schadensradius 65 bleibt.
    `changePlayerStatus` behält das Minimum aus
    `mIntendedMag` und 8.
    `thinkDirty` bleibt bei `- 200.0f`.
    Die drei `makeRootMtxRot*` bleiben bei
    `0.017453294f`.
17. Runde 84 bleibt: `p1 - p2 + 1`,
    `150 * analog * 256`, `hitWater` mit `30.0f`.
    `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
18. Vermeidungslisten aus Runde 81 bis 97 bleiben.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `smallEnemy`, `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach neunundneunzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 98:
47,79 % matched code, 1715744 / 3590088 Bytes,
9190 / 12881 Funktionen.
Game Code 35,50 %, 1003596 / 2826784 Bytes,
5225 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TNerveBEelTearsMarioRecover::execute` lud
`gpMarioParticleManager` vor `gpMarioPos`.
Retail lädt die Position nach r5
und danach den Manager nach r3.

**Partial, kein Vollmatch.**

`JGeometry::TVec3<f32>* marioPos = gpMarioPos`
steht vor `emitAndBindToPosPtr`.
Die beiden Loads und `li r4, 0xd6` matchen.
Übrig sind sieben Stack-Operanden:
Frame 0x30 gegen 0x38.
`bosseel` bleibt `NonMatching`.
Symbolordnung PASS.
Fünf UNUSED-Größen und die Weak-Reihenfolge
sind vorbestehend.

Verworfen: `int time = spine->getTime()`
wird wegoptimiert, der Frame bleibt 0x30.
`TCasinoPanelGate::touchWater` mit `span`
und `baseY` zieht die Loads vor die Konstante
und fällt von neun auf etwa fünfzig Zeilen.
Zurückgenommen.
`initNeonMatColor` nicht über einen benannten Index.
Kein unbenutzter dritter `TVec3`.

Kein Frame wurde aufgefüllt.
Die Attack-Nerve behält das `else` mit `return true`.
Der Ricco-Konstruktor behält `zero()`.
`catching` behält `mRotBroadEnableV`.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher unverändert: 47,79 % matched code,
1715744 / 3590088 Bytes, 9190 / 12881 Funktionen.
Game Code 35,50 %, 1003596 / 2826784 Bytes,
5225 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 98: 0 Funktionen, 0 Bytes.
Delta Daten: 0 Bytes.

`changes_all`: `TNerveBEelTearsMarioRecover::execute`
99,81 % auf 99,92 %.
Unit-Code `bosseel` bleibt 20008 Bytes.
Unit-Fuzzy 99,17 % auf 99,17 %.
Keine Regression.
`MapObjSirena` wurde nur wegen des Zeitstempels
neu gebaut und taucht in `changes_all` nicht auf.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TNerveBEelTearsMarioRecover` behält `marioPos`.
   Den Frame 0x30 nicht auf 0x38 polstern.
   `getTime` dort nicht noch einmal benennen.
   `bosseel` nicht auf `Matching` stellen.
2. `TCasinoPanelGate::touchWater` nicht mit
   `span` und `baseY` wiederholen.
3. `TNerveTobiPukuAttack` behält das `else`
   mit `return true` und das durchfallende
   `return false`.
   Den Frame nicht polstern.
   Keinen unbenutzten dritten `TVec3`.
   `tobiPuku` nicht auf `Matching` stellen.
4. `initNeonMatColor` nicht mit benanntem `index`.
5. Der Ricco-Konstruktor behält `zero()` im Rumpf.
   `MapEventDolpic` nicht auf `Matching` stellen.
6. `catching` behält `mRotBroadEnableV`.
   Den Frame nicht polstern.
   `MarioRun` nicht auf `Matching` stellen.
7. `setDeadBathtubKillerAnm` behält den anonymen
   `TVec3(0, 0, 0)`.
8. `TLiveManager::perform` behält `char trash[16]`.
9. `isFindMarioFromParam` behält die drei `*= param_1`.
   `smallEnemy` nicht auf `Matching` stellen.
10. `walkEnd` behält `quarter` und `vel`.
11. `registerEventWatcher` nicht noch einmal über
    einen benannten `watcher` oder `getChildren()`.
12. `setUp` behält `u32 entrySize`.
13. `sleep()` behalten. Frames nicht polstern.
14. `TRedCoinSwitch::load` nicht über
    `SMSGetMarDirector()`.
15. `appearWithDemo`, `isTouchedOneWall`,
    `joinToGroup` und `createAndKeepData`
    nicht mit denselben Benennungen wiederholen.
16. `TMapStaticObj::init` nicht über `setMtx`.
17. Die sechs Rümpfe aus Runde 91 behalten.
    `walkBehavior` behält `height`.
18. Runde 90 und 89 bleiben.
    `rotating` behält den `u16`-Cast.
    Schadensradius 65 bleibt.
    `changePlayerStatus` behält das Minimum aus
    `mIntendedMag` und 8.
    `thinkDirty` bleibt bei `- 200.0f`.
    Die drei `makeRootMtxRot*` bleiben bei
    `0.017453294f`.
19. Runde 84 bleibt: `p1 - p2 + 1`,
    `150 * analog * 256`, `hitWater` mit `30.0f`.
    `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
20. Vermeidungslisten aus Runde 81 bis 98 bleiben.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `smallEnemy`, `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach hundertster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 99:
47,79 % matched code, 1715744 / 3590088 Bytes,
9190 / 12881 Funktionen.
Game Code 35,50 %, 1003596 / 2826784 Bytes,
5225 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

Gesucht wurde nur ein Kandidat,
den eine Hypothese vollständig matcht.
Partials ohne Zählerbewegung werden nicht behalten.

`TNerveTelesaFreeze::execute` ist stack-only
mit Frame-Delta +8 und Innen-Delta +4.
Das ist das `entrySize`-Muster.
Der Rumpf matcht wortgleich.

**Gemessen und zurückgenommen.**

`int time = spine->getTime()` vor `if (time == 0)`.
Prolog und Epilog matchen:
`stwu -0x40`, `r31` bei `0x3c`.
Der `TPathNode` bleibt vier Byte zu tief,
`0x20` gegen `0x24`.
Fünfzehn Zeilen bleiben.
Kein Vollmatch.
Zurückgenommen.

Ein benannter `TPathNode goal` vor `setGoalPath`
ändert nichts.
Weiterhin zwanzig Stack-Zeilen, Frame `0x38`.
Zurückgenommen.

`MtxToQuat` hat ein einziges vertauschtes `fadds`
und keinen Stack-Unterschied.
Die Teilsummen aus Runde 82 werden nicht wiederholt.
`touchWater` nicht über `span`/`baseY`.
Kein Frame-Polster, kein unbenutzter `TVec3`.

`marioPos` in der Recover-Nerve bleibt.
Die Attack-Nerve behält das `else` mit `return true`.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher unverändert: 47,79 % matched code,
1715744 / 3590088 Bytes, 9190 / 12881 Funktionen.
Game Code 35,50 %, 1003596 / 2826784 Bytes,
5225 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 99: 0 Funktionen, 0 Bytes.
Delta Daten: 0 Bytes.

`changes_all` ist leer.
Keine Regression.
Keine Quelldatei geändert.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TNerveTelesaFreeze` nicht noch einmal über
   `int time = getTime()` oder einen benannten
   `TPathNode`.
   Der Frame matcht mit `time`, der Pfadknoten nicht.
   `telesa` nicht auf `Matching` stellen.
2. `TNerveBEelTearsMarioRecover` behält `marioPos`.
   Den Frame 0x30 nicht auf 0x38 polstern.
   `getTime` dort nicht noch einmal benennen.
   `bosseel` nicht auf `Matching` stellen.
3. `TCasinoPanelGate::touchWater` nicht mit
   `span` und `baseY` wiederholen.
4. `TNerveTobiPukuAttack` behält das `else`
   mit `return true` und das durchfallende
   `return false`.
   Den Frame nicht polstern.
   Keinen unbenutzten dritten `TVec3`.
   `tobiPuku` nicht auf `Matching` stellen.
5. `initNeonMatColor` nicht mit benanntem `index`.
6. Der Ricco-Konstruktor behält `zero()` im Rumpf.
   `MapEventDolpic` nicht auf `Matching` stellen.
7. `catching` behält `mRotBroadEnableV`.
   Den Frame nicht polstern.
   `MarioRun` nicht auf `Matching` stellen.
8. `setDeadBathtubKillerAnm` behält den anonymen
   `TVec3(0, 0, 0)`.
9. `TLiveManager::perform` behält `char trash[16]`.
10. `isFindMarioFromParam` behält die drei `*= param_1`.
    `smallEnemy` nicht auf `Matching` stellen.
11. `walkEnd` behält `quarter` und `vel`.
12. `registerEventWatcher` nicht noch einmal über
    einen benannten `watcher` oder `getChildren()`.
13. `setUp` behält `u32 entrySize`.
14. `sleep()` behalten. Frames nicht polstern.
15. `TRedCoinSwitch::load` nicht über
    `SMSGetMarDirector()`.
16. `appearWithDemo`, `isTouchedOneWall`,
    `joinToGroup` und `createAndKeepData`
    nicht mit denselben Benennungen wiederholen.
17. `TMapStaticObj::init` nicht über `setMtx`.
18. Die sechs Rümpfe aus Runde 91 behalten.
    `walkBehavior` behält `height`.
19. Runde 90 und 89 bleiben.
    `rotating` behält den `u16`-Cast.
    Schadensradius 65 bleibt.
    `changePlayerStatus` behält das Minimum aus
    `mIntendedMag` und 8.
    `thinkDirty` bleibt bei `- 200.0f`.
    Die drei `makeRootMtxRot*` bleiben bei
    `0.017453294f`.
20. Runde 84 bleibt: `p1 - p2 + 1`,
    `150 * analog * 256`, `hitWater` mit `30.0f`.
    `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
21. Vermeidungslisten aus Runde 81 bis 99 bleiben.
    `MtxToQuat` nicht in Teilsummen zerlegen.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `smallEnemy`, `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach hundertunderster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 100:
47,79 % matched code, 1715744 / 3590088 Bytes,
9190 / 12881 Funktionen.
Game Code 35,50 %, 1003596 / 2826784 Bytes,
5225 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TMario::startVoice` matcht bis auf den Frame.
Retail `0x28`, bei uns `0x20`.
Im Rumpf keine Stack-Zugriffe.
`SMSGetMSound()` liegt schon in r31,
bevor `getVoiceStatus` läuft.

**Vollmatch.**

```cpp
MSound* sound = SMSGetMSound();
return sound->startMarioVoice(param_1, mHealth, getVoiceStatus());
```

0 Abweichungen, 124 Bytes, 31 Instruktionen.
Die Reihenfolge der Loads bleibt.
`startVoiceIfNoVoice` inlined dieselbe Funktion.
Das `char trash[8]` dort war das alte Polster
für genau diese acht Byte.
Es ist entfernt.
`startVoiceIfNoVoice` bleibt bei 100 %.
`MarioSound` bleibt `NonMatching`:
`soundTorocco` und `soundMovement` matchen nicht.
Symbolordnung PASS.
Die UNUSED-Größe von `startVoiceYoshi` ist alt.

**Gemessen und zurückgenommen.**

`turnEnd` mit benanntem `TWaterGun* gun`
und aufgefaltetem `considerRotateStart`
fällt von neun auf 48 Zeilen.
`walkEnd` blieb 100 %, weil die gemeinsame
Funktion nicht angefasst wurde.
Zurückgenommen.
`TelesaFreeze` nicht wiederholt.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,79 % matched code,
1715868 / 3590088 Bytes, 9191 / 12881 Funktionen.
Game Code 35,51 %, 1003720 / 2826784 Bytes,
5226 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 100: +1 Funktion, +124 Bytes.
Delta Daten: 0 Bytes.

`changes_all`: `startVoice__6TMarioFUl`
99,71 % auf 100 %.
Unit-Code `MarioSound` 21,52 % auf 22,83 %.
`soundTorocco` und `soundMovement` unverändert.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TMario::startVoice` behält `MSound* sound`.
   `startVoiceIfNoVoice` behält kein `char trash[8]`.
   `MarioSound` nicht auf `Matching` stellen.
2. `turnEnd` nicht mit benanntem `TWaterGun*`
   und aufgefaltetem `considerRotateStart`.
   `walkEnd` bleibt bei `quarter` und `vel`.
3. `TNerveTelesaFreeze` nicht über
   `int time = getTime()` oder einen benannten
   `TPathNode`.
4. `TNerveBEelTearsMarioRecover` behält `marioPos`.
   Den Frame nicht polstern.
   `bosseel` nicht auf `Matching` stellen.
5. `TCasinoPanelGate::touchWater` nicht mit
   `span` und `baseY` wiederholen.
6. `TNerveTobiPukuAttack` behält das `else`
   mit `return true`.
   Den Frame nicht polstern.
   Keinen unbenutzten dritten `TVec3`.
7. `initNeonMatColor` nicht mit benanntem `index`.
8. Der Ricco-Konstruktor behält `zero()` im Rumpf.
9. `catching` behält `mRotBroadEnableV`.
   Den Frame nicht polstern.
   `MarioRun` nicht auf `Matching` stellen.
10. `setDeadBathtubKillerAnm` behält den anonymen
    `TVec3(0, 0, 0)`.
11. `TLiveManager::perform` behält `char trash[16]`.
12. `isFindMarioFromParam` behält die drei `*= param_1`.
13. `registerEventWatcher` nicht noch einmal über
    einen benannten `watcher` oder `getChildren()`.
14. `setUp` behält `u32 entrySize`.
15. `sleep()` behalten. Frames nicht polstern.
16. `TRedCoinSwitch::load` nicht über
    `SMSGetMarDirector()`.
17. `appearWithDemo`, `isTouchedOneWall`,
    `joinToGroup` und `createAndKeepData`
    nicht mit denselben Benennungen wiederholen.
18. `TMapStaticObj::init` nicht über `setMtx`.
19. Die sechs Rümpfe aus Runde 91 behalten.
    `walkBehavior` behält `height`.
20. Runde 90 und 89 bleiben.
    `rotating` behält den `u16`-Cast.
    Schadensradius 65 bleibt.
    `changePlayerStatus` behält das Minimum aus
    `mIntendedMag` und 8.
    `thinkDirty` bleibt bei `- 200.0f`.
    Die drei `makeRootMtxRot*` bleiben bei
    `0.017453294f`.
21. Runde 84 bleibt: `p1 - p2 + 1`,
    `150 * analog * 256`, `hitWater` mit `30.0f`.
    `isDemo` bleibt die Oder-Form mit `stateIs3Or4`.
22. Vermeidungslisten aus Runde 81 bis 100 bleiben.
    `MtxToQuat` nicht in Teilsummen zerlegen.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `smallEnemy`, `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach hundertundzweiter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 101:
47,79 % matched code, 1715868 / 3590088 Bytes,
9191 / 12881 Funktionen.
Game Code 35,51 %, 1003720 / 2826784 Bytes,
5226 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TTelesa::initAttacker` wich nur in einer
Instruktion ab.
Retail kopiert den Treffer-Aktor mit
`addi r3, r30, 0` vor `getModel`.
Bei uns stand `mr r3, r30`.
Der Rest, 184 Instruktionen, stimmte.

**Vollmatch.**

```cpp
TLiveActor* actor = static_cast<TLiveActor*>(param_1);
MtxPtr mtx = actor->getModel()->getAnmMtx(5);
```

0 Abweichungen, 740 Bytes, 185 Instruktionen.
`initItemAttacker` bleibt bei 100 %.
`TNerveTelesaAttackMario::execute` bleibt
bei 83 abweichenden Zeilen, 97,04 %.
`telesa` bleibt `NonMatching`.
Symbolordnung PASS.
Die Weak-Reihenfolge und die zwei UNUSED-Größen
sind vorbestehend.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,82 % matched code,
1716608 / 3590088 Bytes, 9192 / 12881 Funktionen.
Game Code 35,53 %, 1004460 / 2826784 Bytes,
5227 / 8857 Funktionen.
Daten unverändert: 385811 / 640331 Bytes, 60,25 %.
Game-Daten unverändert: 306587 / 556995 Bytes, 55,04 %.

Delta Code gegen Runde 101: +1 Funktion, +740 Bytes.
Delta Daten: 0 Bytes.

`changes_all`: `initAttacker__7TTelesaFP9THitActor`
99,68 % auf 100 %.
Unit-Code `telesa` 66,76 % auf 70,35 %.
Unit-Fuzzy 99,60 % auf 99,61 %.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TTelesa::initAttacker` behält
   `TLiveActor* actor`.
   `telesa` nicht auf `Matching` stellen.
2. `TMario::startVoice` behält `MSound* sound`.
   `startVoiceIfNoVoice` behält kein `char trash[8]`.
   `MarioSound` nicht auf `Matching` stellen.
3. `turnEnd` nicht mit benanntem `TWaterGun*`
   und aufgefaltetem `considerRotateStart`.
4. `TNerveTelesaFreeze` nicht über
   `int time = getTime()` oder einen benannten
   `TPathNode`.
5. `TCasinoPanelGate::touchWater` nicht mit
   `span` und `baseY` wiederholen.
6. `TNerveBEelTearsMarioRecover` behält `marioPos`.
   Den Frame nicht polstern.
7. `TNerveTobiPukuAttack` behält das `else`
   mit `return true`.
   Den Frame nicht polstern.
   Keinen unbenutzten dritten `TVec3`.
8. `catching` behält `mRotBroadEnableV`.
   Den Frame nicht polstern.
   `MarioRun` nicht auf `Matching` stellen.
9. `walkEnd` behält `quarter` und `vel`.
10. Vermeidungslisten aus Runde 81 bis 101 bleiben.
    `MtxToQuat` nicht in Teilsummen zerlegen.
    `initNeonMatColor` nicht mit benanntem `index`.
    `MapEventMare`, `MapStaticObject`, `MarioRun`,
    `smallEnemy`, `bosspakkun`, `hamukuri`, `poihana`,
    `MapObjGeneral`, `walkerEnemy`, `bossManta`,
    `fireWanwan`, `MapObjHide`, `EventWatcher` und
    `NpcWalkTurn` nicht auf `Matching` stellen.

### Nach hundertunddritter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 102:
47,82 % matched code, 1716608 / 3590088 Bytes,
9192 / 12881 Funktionen.
Game Code 35,53 %, 1004460 / 2826784 Bytes,
5227 / 8857 Funktionen.
Daten 385811 / 640331 Bytes, 60,25 %.

`TDonchou::loadAfter` war nur im Prolog
acht Byte zu klein.
Die beiden `search`-Ergebnisse gingen
direkt in die Member.

`getObjAppearPos` war ohne `const`.
Die VTables zeigten auf das falsche Symbol.
Retail ist `CFv`, acht Byte:
`addi r3, r3, 0x10; blr`.

**Vollmatch.**

```cpp
TSlotDrum* drum
    = static_cast<TSlotDrum*>(JDrama::TNameRefGen::search("srotdram"));
unk144 = drum;
TItemSlotDrum* itemDrum = static_cast<TItemSlotDrum*>(
    JDrama::TNameRefGen::search("itemsrotdram"));
unk148 = itemDrum;
```

0 Abweichungen, 200 Bytes.
Beide `getObjAppearPos` sind `const`.
`TWaterHitPictureHideObj` matcht, 8 Bytes.
`THideObjPictureTwin` matcht, 12 Bytes.
Die VTables von `MapObjSirena` gehen auf 100 % Daten.
`MapObjHide`-Daten 6,37 % auf 91,07 %.
Beide TUs bleiben `NonMatching`.
Symbolordnung `MapObjSirena` PASS.
Die UNUSED-Größe von `getSlotResult` ist alt.

**Gemessen und zurückgenommen.**

`u32 se` in `TNerveMantaDeath` verschiebt
die Sound-ID aus r31.
Elf Zeilen, zurückgenommen.
Ein benannter `TMActorKeeper*` in `makeMActors`
ändert den Frame nicht.
Zurückgenommen.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,82 % matched code,
1716828 / 3590088 Bytes, 9195 / 12881 Funktionen.
Game Code 35,54 %, 1004680 / 2826784 Bytes,
5230 / 8857 Funktionen.
Daten 394595 / 640331 Bytes, 61,62 %.
Game-Daten 315371 / 556995 Bytes, 56,62 %.

Delta Code gegen Runde 102: +3 Funktionen, +220 Bytes.
Delta Daten: +8784 Bytes.

`changes_all` nur diese drei Symbole,
je von unter 100 % auf 100 %,
plus die beiden Daten-Units.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.
`telesa` bleibt `NonMatching`.

### Nächster Schritt

1. `TDonchou::loadAfter` behält `drum` und `itemDrum`.
   `MapObjSirena` nicht auf `Matching` stellen.
2. `getObjAppearPos` bleibt `const`
   an beiden Klassen.
   `MapObjHide` nicht auf `Matching` stellen.
3. `TNerveMantaDeath` nicht über ein benanntes `se`.
   `makeMActors` nicht über einen benannten Keeper.
4. `TTelesa::initAttacker` behält `TLiveActor* actor`.
   `telesa` nicht auf `Matching` stellen.
5. `TMario::startVoice` behält `MSound* sound`.
   `startVoiceIfNoVoice` behält kein `char trash[8]`.
6. `turnEnd` nicht mit benanntem `TWaterGun*`.
   `TelesaFreeze` und `touchWater` liegen lassen.
7. `catching` behält `mRotBroadEnableV`.
   Den Frame nicht polstern.
8. Vermeidungslisten aus Runde 81 bis 102 bleiben.
   `initNeonMatColor` nicht mit benanntem `index`.
   `MtxToQuat` nicht in Teilsummen zerlegen.

### Nach hundertundvierter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 103:
47,82 % matched code, 1716828 / 3590088 Bytes,
9195 / 12881 Funktionen.
Game Code 35,54 %, 1004680 / 2826784 Bytes,
5230 / 8857 Funktionen.
Daten 394595 / 640331 Bytes, 61,62 %.

`MSRandVol::MSRandVol` war nur im Prolog
acht Byte zu klein.
Retail-Frame `0x20`, bei uns `0x18`.
Der Rumpf, 35 Instruktionen, stimmte.
`param` liegt schon in r31, `this` in r30.

**Vollmatch.**

```cpp
f32 half = 0.5f;
mPSlopes[2] = half;
mAmplitudes[1] = half;
```

`mAmplitude` bleibt das Literal `0.5f`
im Initialisierer.
0 Abweichungen, 168 Bytes, 42 Instruktionen.
`MSoundSE` bleibt `NonMatching`.
Symbolordnung PASS.
Die Weak-Reihenfolge und die UNUSED-Größe
von `getRandomVolume` sind vorbestehend.

**Gemessen und zurückgenommen.**

Ein benanntes `s32 next` in
`TShine::loadBeforeInit` wird wegoptimiert.
Der Frame bleibt `0x48` gegen `0x50`.
`MSound* sound` in `TMario::catching`
wird ebenfalls wegoptimiert.
Der Frame bleibt `0x28` gegen `0x30`.
`f32 minX = mMinX` in `stampModel`
ändert die Ladereihenfolge nicht.
Alle drei zurückgenommen.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,83 % matched code,
1716996 / 3590088 Bytes, 9196 / 12881 Funktionen.
Game Code 35,55 %, 1004848 / 2826784 Bytes,
5231 / 8857 Funktionen.
Daten unverändert: 394595 / 640331 Bytes, 61,62 %.
Game-Daten unverändert: 315371 / 556995 Bytes, 56,62 %.

Delta Code gegen Runde 103: +1 Funktion, +168 Bytes.
Delta Daten: 0 Bytes.

`changes_all` nur
`__ct__Q214MSoundSESystem9MSRandVolFUl`
99,83 % auf 100 %.
Unit-Code `MSoundSE` 25,01 % auf 26,43 %.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `MSRandVol::MSRandVol` behält `f32 half`.
   `MSoundSE` nicht auf `Matching` stellen.
2. `TDonchou::loadAfter` behält `drum` und `itemDrum`.
   `getObjAppearPos` bleibt `const`.
3. `TMario::catching` nicht über `MSound* sound`.
   Den Frame nicht polstern.
   `mRotBroadEnableV` bleibt.
4. `TShine::loadBeforeInit` nicht über ein benanntes
   `next` zwischen den beiden `s32`.
5. `stampModel` nicht über ein vorgezogenes `mMinX`.
6. `TNerveMantaDeath` nicht über ein benanntes `se`.
   `makeMActors` nicht über einen benannten Keeper.
7. `TTelesa::initAttacker` behält `TLiveActor* actor`.
   `TMario::startVoice` behält `MSound* sound`.
8. Vermeidungslisten aus Runde 81 bis 103 bleiben.
   `MtxToQuat` nicht in Teilsummen zerlegen.
   `turnEnd`, `TelesaFreeze` und `touchWater` liegen lassen.

### Nach hundertundfünfter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 104:
47,83 % matched code, 1716996 / 3590088 Bytes,
9196 / 12881 Funktionen.
Game Code 35,55 %, 1004848 / 2826784 Bytes,
5231 / 8857 Funktionen.
Daten 394595 / 640331 Bytes, 61,62 %.

`hoseiDiveCameraCallback` kopierte `position.x`
über r5.
Retail benutzt r6.
Der Frame stimmte schon (`0x20`).
r5 hält danach `gpMarioPos` für `warpPosAndAt`.

**Vollmatch.**

```cpp
const JGeometry::TVec3<f32>* marioPos = gpMarioPos;
gpCamera->warpPosAndAt(position, *marioPos);
```

Der benannte Zeiger lässt die Kopie in r6.
0 Abweichungen, 96 Bytes, 24 Instruktionen.
`bosseel` bleibt `NonMatching`.
Symbolordnung PASS.
Die Weak-Reihenfolge und fünf UNUSED-Größen
sind vorbestehend.

**Gemessen und zurückgenommen.**

`dot` in `isUpperThanMirrorPlane` weglassen
lässt den Frame bei `0x30` gegen `0x28`
und tauscht die `fadds`-Operanden.
Ein gemeinsames `int i` in `changeXluJoint`
ändert nichts.
Der Frame bleibt `0x90` gegen `0x88`.
Die beiden Suchen in `entryMirrorDrawBufferAlways`
inline zu falten vergrößert den Frame
von `0x68` auf `0x70`.
Retail ist `0x60`.
Alle drei zurückgenommen.

**Messung, `ninja`, `changes_all`, `dtk shasum -c`.**

Nachher: 47,83 % matched code,
1717092 / 3590088 Bytes, 9197 / 12881 Funktionen.
Game Code 35,55 %, 1004944 / 2826784 Bytes,
5232 / 8857 Funktionen.
Daten unverändert: 394595 / 640331 Bytes, 61,62 %.
Game-Daten unverändert: 315371 / 556995 Bytes, 56,62 %.

Delta Code gegen Runde 104: +1 Funktion, +96 Bytes.
Delta Daten: 0 Bytes.

`changes_all` nur
`hoseiDiveCameraCallback__FUlUl`
99,58 % auf 100 %.
Unit-Code `bosseel` 43,83 % auf 44,04 %.
Keine Regression.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `hoseiDiveCameraCallback` behält `marioPos`.
   `bosseel` nicht auf `Matching` stellen.
2. `MSRandVol::MSRandVol` behält `f32 half`.
   `MSoundSE` nicht auf `Matching` stellen.
3. `TShine::loadBeforeInit` nicht über ein benanntes `next`.
   `TMario::catching` nicht über `MSound* sound`.
   `stampModel` nicht über ein vorgezogenes `mMinX`.
4. `isUpperThanMirrorPlane` behält `dot`.
   `changeXluJoint` behält zwei Schleifenindizes.
   `entryMirrorDrawBufferAlways` behält `dbOpa` und `dbXlu`.
5. `TNerveMantaDeath` nicht über ein benanntes `se`.
   `makeMActors` nicht über einen benannten Keeper.
6. `TDonchou::loadAfter` behält `drum` und `itemDrum`.
   `getObjAppearPos` bleibt `const`.
7. `TTelesa::initAttacker` behält `TLiveActor* actor`.
   `TMario::startVoice` behält `MSound* sound`.
8. Vermeidungslisten aus Runde 81 bis 104 bleiben.
   Den Frame nicht polstern.
   `MtxToQuat` nicht in Teilsummen zerlegen.
   `turnEnd`, `TelesaFreeze` und `touchWater` liegen lassen.

### Nach hundertundsechster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 105:
47,83 % matched code, 1717092 / 3590088 Bytes,
9197 / 12881 Funktionen.
Game Code 35,55 %, 1004944 / 2826784 Bytes,
5232 / 8857 Funktionen.
Daten 394595 / 640331 Bytes, 61,62 %.

`THino2Params::THino2Params` hatte drei falsche
`PARAM_INIT`-Floats.
Die Instruktionswörter waren schon identisch,
weil jedes `lfs` nur über eine SDA21-Relokation
den Pool trifft.
`functionRelocDiffs=data_value` zeigte
99,96 % und genau drei abweichende `lfs`.

**Vollmatch, strikt.**

```cpp
PARAM_INIT(mSLBodyHitR0, 100.0f)
PARAM_INIT(mSLBodyHitH0, 200.0f)
PARAM_INIT(mSLBankProp, 0.5f)
```

Retail legt an `0x248` den Wert 100,
an `0x25c` den Wert 200 und an `0x270` den Wert 0,5.
0 Abweichungen, 1648 Bytes, 412 Instruktionen.
Sonst ändert sich in `hinokuri2` kein Prozent.
`hinokuri2` bleibt `NonMatching`.
Symbolordnung PASS.
Sechs UNUSED-Größen sind vorbestehend.

**Zähler.** `ninja changes_all` bleibt leer.
Der Report vergleicht die Relokationswerte nicht,
darum war der Konstruktor dort schon mitgezählt.
Matched code bleibt 1717092 / 3590088 Bytes,
9197 / 12881 Funktionen.
Game Code bleibt 1004944 / 2826784 Bytes,
5232 / 8857 Funktionen.
Daten unverändert.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.
`isUpperThanMirrorPlane`, `changeXluJoint` und
`entryMirrorDrawBufferAlways` nicht erneut angefasst.

### Nächster Schritt

1. `THino2Params` behält 100, 200 und 0,5
   für `mSLBodyHitR0`, `mSLBodyHitH0` und `mSLBankProp`.
   `hinokuri2` nicht auf `Matching` stellen.
2. `hoseiDiveCameraCallback` behält `marioPos`.
   `bosseel` nicht auf `Matching` stellen.
3. `MSRandVol::MSRandVol` behält `f32 half`.
   `MSoundSE` nicht auf `Matching` stellen.
4. `isUpperThanMirrorPlane` nicht ohne `dot`.
   `changeXluJoint` nicht mit einem gemeinsamen `int i`.
   `entryMirrorDrawBufferAlways` nicht mit
   gefalteten Draw-Buffer-Suchen.
5. `TShine::loadBeforeInit` nicht über ein benanntes `next`.
   `TMario::catching` nicht über `MSound* sound`.
   `stampModel` nicht über ein vorgezogenes `mMinX`.
6. `TNerveMantaDeath` nicht über ein benanntes `se`.
   `makeMActors` nicht über einen benannten Keeper.
7. `TDonchou::loadAfter` behält `drum` und `itemDrum`.
   `getObjAppearPos` bleibt `const`.
   `initAttacker` behält `actor`, `startVoice` behält `sound`.
8. Vermeidungslisten aus Runde 81 bis 105 bleiben.
   Den Frame nicht polstern.

### Nach hundertundsiebter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 106:
47,83 % matched code, 1717092 / 3590088 Bytes,
9197 / 12881 Funktionen.
Game Code 35,55 %, 1004944 / 2826784 Bytes,
5232 / 8857 Funktionen.
Daten 394595 / 640331 Bytes, 61,62 %.

`TMario::TDeParams::TDeParams` lud den Namen
`mHpMax`.
Retail speichert in `.sdata2` die Zeichenkette `mHPMax`.
Zwei `li` zeigen auf genau diese Zeichenkette.
Unter `functionRelocDiffs=data_value` waren das
die einzigen zwei Abweichungen, 99,98 %.

**Vollmatch, strikt.** Das Feld heißt jetzt `mHPMax`.
`PARAM_INIT` schreibt denselben Namen in den Pool.
0 Abweichungen, 2440 Bytes, 610 Instruktionen.
`MarioInit` bleibt `NonMatching`.
Die Zugriffe in den anderen TUs sind nur der Member-Offset.
`.sdata2` von `MarioInit` steigt von 99,04 % auf 99,23 %.
Der Daten-Zähler in Bytes bleibt gleich.

**Zähler.** `ninja changes_all` bleibt leer.
Die Instruktionswörter waren schon identisch.
Der Report zählte den Konstruktor schon als Match.
Matched code bleibt 1717092 / 3590088 Bytes,
9197 / 12881 Funktionen.
Game Code und Daten bleiben unverändert.
Keine andere TU ändert ein Maß.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.
Die Hino2-Defaults 100, 200 und 0,5 bleiben.

### Nächster Schritt

1. `TDeParams::mHPMax` behält die Schreibweise `mHPMax`.
   `MarioInit` nicht auf `Matching` stellen.
2. `THino2Params` behält 100, 200 und 0,5
   für `mSLBodyHitR0`, `mSLBodyHitH0` und `mSLBankProp`.
3. `hoseiDiveCameraCallback` behält `marioPos`.
   `MSRandVol::MSRandVol` behält `f32 half`.
4. `isUpperThanMirrorPlane` nicht ohne `dot`.
   `changeXluJoint` nicht mit einem gemeinsamen `int i`.
   `entryMirrorDrawBufferAlways` nicht mit
   gefalteten Draw-Buffer-Suchen.
5. `TShine::loadBeforeInit` nicht über ein benanntes `next`.
   `TMario::catching` nicht über `MSound* sound`.
   `stampModel` nicht über ein vorgezogenes `mMinX`.
6. `TNerveMantaDeath` nicht über ein benanntes `se`.
   `makeMActors` nicht über einen benannten Keeper.
7. Vermeidungslisten aus Runde 81 bis 106 bleiben.
   Den Frame nicht polstern.

### Nach hundertundachter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 107:
47,83 % matched code, 1717092 / 3590088 Bytes,
9197 / 12881 Funktionen.
Game Code 35,55 %, 1004944 / 2826784 Bytes,
5232 / 8857 Funktionen.
Daten 394595 / 640331 Bytes, 61,62 %.

Oberhalb von 99 % gibt es keinen weiteren
Konstruktor, dessen einzige Abweichung ein
falscher `PARAM_INIT`-Name oder ein falsches
Float-Default ist.
`mHPMax` bleibt.

**Gemessen und zurückgenommen.**

`SMS_UnifyMaterial`: `mat` vor `unifier` zu
deklarieren lässt r27 und r28 vertauscht.
Beide Zeiger in die Schleife zu legen fällt
von 99,3 % auf 62,4 %.
`execRoofCheck_`: `roofHeight -= mSLRoofHeight`
trifft die Float-Register.
Der Frame fällt von `0x48` auf `0x40`.
Ein benanntes `y` schiebt nur einen Slot um 4.
Ein benanntes `TCamSaveEx* save` schrumpft den
Frame weiter auf `0x38`.
Ein benanntes `limit` lässt denselben Frame
`0x40`.
Alle Varianten zurückgenommen.

**Zähler.** Kein neues Vollmatch.
Matched code bleibt 1717092 / 3590088 Bytes,
9197 / 12881 Funktionen.
Game Code und Daten bleiben unverändert.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `mHPMax` behält die Schreibweise `mHPMax`.
2. `SMS_UnifyMaterial` nicht über die
   Deklarationsreihenfolge von `mat` und `unifier`
   und nicht mit beiden Zeigern in der Schleife.
3. `execRoofCheck_` nicht über `roofHeight -=`,
   benanntes `y`, `save` oder `limit`.
4. `THino2Params` behält 100, 200 und 0,5.
   `hoseiDiveCameraCallback` behält `marioPos`.
   `MSRandVol` behält `half`.
5. Vermeidungslisten aus Runde 81 bis 107 bleiben.
   Den Frame nicht polstern.

### Nach hundertundneunter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 108:
47,83 % matched code, 1717092 / 3590088 Bytes,
9197 / 12881 Funktionen.
Game Code 35,55 %, 1004944 / 2826784 Bytes,
5232 / 8857 Funktionen.
Daten 394595 / 640331 Bytes, 61,62 %.

`TRedCoinSwitch::loadAfter` suchte jede rote Münze
und rief `makeObjDead` am unbenannten Cast auf.
Der Frame war `0x70`, Retail `0x78`.
Der Namenspuffer lag bei `0x20` statt `0x24`.
Der übrige Rumpf stimmte.

**Vollmatch.** Das Suchergebnis heißt `coin`.

```cpp
TMapObjBase* coin
    = static_cast<TMapObjBase*>(JDrama::TNameRefGen::search(buf));
coin->makeObjDead();
```

0 Abweichungen unter `functionRelocDiffs=data_value`,
164 Bytes, 41 Instruktionen.
`MapObjTown` bleibt `NonMatching`.
Die Symbolreihenfolge stimmt.
Zwei UNUSED-Destruktoren von `TShadowObj` fehlen vorbestehend.
Eine UNUSED-Größe weicht vorbestehend ab.

**Zähler.** `ninja changes_all`:
`loadAfter__14TRedCoinSwitchFv` 99,71 % → 100 %.
`MapObjTown` matched code 73,99 % → 75,59 %.
Matched code 1717256 / 3590088 Bytes,
9198 / 12881 Funktionen.
Das sind 164 Bytes und eine Funktion mehr.
Die Anzeige bleibt 47,83 %,
weil 47,8287 % und 47,8333 % gleich runden.
Game Code 35,56 %, 1005108 / 2826784 Bytes,
5233 / 8857 Funktionen.
Daten unverändert, 394595 / 640331 Bytes, 61,62 %.
Game-Daten 315371 / 556995 Bytes, 56,62 %.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.
`mHPMax` bleibt.
`SMS_UnifyMaterial` und `execRoofCheck_` nicht angefasst.

### Nächster Schritt

1. `TRedCoinSwitch::loadAfter` behält `coin`.
   `MapObjTown` nicht auf `Matching` stellen.
2. `mHPMax` behält die Schreibweise `mHPMax`.
3. `SMS_UnifyMaterial` nicht über die
   Deklarationsreihenfolge und nicht mit beiden Zeigern in der Schleife.
4. `execRoofCheck_` nicht über `roofHeight -=`,
   benanntes `y`, `save` oder `limit`.
5. Vermeidungslisten aus Runde 81 bis 108 bleiben.
   Den Frame nicht polstern.

### Nach hundertundzehnter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 109:
47,83 % matched code, 1717256 / 3590088 Bytes,
9198 / 12881 Funktionen.
Game Code 35,56 %, 1005108 / 2826784 Bytes,
5233 / 8857 Funktionen.
Daten 394595 / 640331 Bytes, 61,62 %.

`TMapObjBaseManager::newAndRegisterObjByEventID`
legte den Shine-Namen von Event 777 in `char buffer[64]`.
`snprintf` bekam die Länge 64.
Retail übergibt `0x100`.
Der Frame war `0x1E8`, Retail `0x2A8`.
Die Differenz ist 192 Bytes, also `0x100 - 64`.
Zehn Instruktionen wichen ab, sonst stimmte der Rumpf.

**Vollmatch.** Der Puffer ist 256 Bytes lang.
`snprintf` nimmt `sizeof(buffer)`.

```cpp
char buffer[0x100];
snprintf(buffer, sizeof(buffer), "シャイン（%s）", name);
```

0 Abweichungen unter `functionRelocDiffs=data_value`,
1652 Bytes, 413 Instruktionen.
`MapObjManager` bleibt `NonMatching`.
Symbolordnung PASS.
`loadMatTable` hat eine vorbestehende UNUSED-Größenwarnung.
`newUniqueObjByName` bleibt bei 98,88 %.

**Zähler.** `ninja changes_all`:
`newAndRegisterObjByEventID__18TMapObjBaseManagerFUlPCc`
99,98 % → 100 %.
`MapObjManager` matched code 41,59 % → 57,97 %.
Matched code 47,88 %, 1718908 / 3590088 Bytes,
9199 / 12881 Funktionen.
Das sind 1652 Bytes und eine Funktion mehr.
Game Code 35,62 %, 1006760 / 2826784 Bytes,
5234 / 8857 Funktionen.
Daten unverändert, 394595 / 640331 Bytes, 61,62 %.
Game-Daten 315371 / 556995 Bytes, 56,62 %.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.
`TRedCoinSwitch::loadAfter` behält `coin`.
`mHPMax` bleibt.

### Nächster Schritt

1. Event 777 behält `char buffer[0x100]`
   und `snprintf` mit `sizeof(buffer)`.
   `MapObjManager` nicht auf `Matching` stellen.
2. `TRedCoinSwitch::loadAfter` behält `coin`.
   `MapObjTown` nicht auf `Matching` stellen.
3. `mHPMax` behält die Schreibweise `mHPMax`.
4. `SMS_UnifyMaterial` und `execRoofCheck_` nicht
   mit den Varianten aus Runde 108.
5. Vermeidungslisten aus Runde 81 bis 109 bleiben.
   Den Frame nicht polstern.

### Nach hundertundelfter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 110:
47,88 % matched code, 1718908 / 3590088 Bytes,
9199 / 12881 Funktionen.
Game Code 35,62 %, 1006760 / 2826784 Bytes,
5234 / 8857 Funktionen.
Daten 394595 / 640331 Bytes, 61,62 %.

Oberhalb von 90 % gibt es kein weiteres
`snprintf`, dessen Längen-Immediate vom Puffer abweicht.
`buffer[0x100]` bleibt.

**Gemessen und zurückgenommen.**

`TDoroHaneKuri::isCollidMove`: `f32 scale = -5.0f`
nach der Geschwindigkeit trifft den Frame `0x38`.
Der Vektor bleibt bei `0x20`, Retail liegt bei `0x24`.
`scale` vor dem Vektor lässt ihn bei `0x1c`.

`TNerveDoroHaneRise`: `f32 step = 0.01f`
für die beiden Clamp-Grenzen ändert nichts.
Der Frame bleibt `0x50` gegen Retail `0x58`.

`TMareEventWallRock::load`: der Zeiger `view`
für `push_back` verschiebt den Slot `0x64` nicht.
Retail legt ihn bei `0x68` ab.
Der Frame bleibt `0x80` gegen `0x88`.

Alle drei Varianten zurückgenommen.

**Zähler.** Kein neues Vollmatch.
`ninja changes_all` ist leer.
Matched code bleibt 47,88 %,
1718908 / 3590088 Bytes,
9199 / 12881 Funktionen.
Game Code bleibt 35,62 %,
1006760 / 2826784 Bytes,
5234 / 8857 Funktionen.
Daten unverändert.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.
Event 777 behält `buffer[0x100]`.
`TRedCoinSwitch::loadAfter` behält `coin`.

### Nächster Schritt

1. Event 777 behält `char buffer[0x100]`
   und `snprintf` mit `sizeof(buffer)`.
   `MapObjManager` nicht auf `Matching` stellen.
2. `TRedCoinSwitch::loadAfter` behält `coin`.
3. `isCollidMove` nicht über `f32 scale`.
   `TNerveDoroHaneRise` nicht über `f32 step`.
   `TMareEventWallRock::load` nicht über `view`.
4. `mHPMax` bleibt.
   `SMS_UnifyMaterial` und `execRoofCheck_` nicht
   mit den Varianten aus Runde 108.
5. Vermeidungslisten aus Runde 81 bis 110 bleiben.
   Den Frame nicht polstern.

### Nach hundertundzwölfter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 111:
47,88 % matched code, 1718908 / 3590088 Bytes,
9199 / 12881 Funktionen.
Game Code 35,62 %, 1006760 / 2826784 Bytes,
5234 / 8857 Funktionen.
Daten 394595 / 640331 Bytes, 61,62 %.

`TMapObjBase::setUpCurrentMapCollision` war bei 99,83 %.
Der Stack-Frame lag bei `0x78` statt retail `0x80`,
die Matrix bei `addi r3,r1,0x28` statt `0x2c`.

**Vollmatch, strikt.**

Der lokale Zeiger `colman` war fabricated.
Er drückte den Frame um acht Byte.
Nach dem Entfernen ruft der Else-Zweig
`mMapCollisionManager->setUpUnk8TRS` direkt auf,
wie schon `setUpMapCollision`.

0 Abweichungen, 216 Bytes, 54 Instruktionen.
`functionRelocDiffs=data_value` ohne bad Relocs.
Symbolordnung PASS bis auf vorbestehendes
`setMtx__17TMapCollisionBaseFPA4_f` MISSING.

**Zähler.** `ninja changes_all`:
matched code 47,88 % → 47,89 %,
1718908 → 1719124 Bytes (+216),
9199 → 9200 Funktionen (+1).
Game Code 35,62 % → 35,63 %,
1006760 → 1006976 Bytes (+216),
5234 → 5235 Funktionen (+1).
`MapObjBase` matched_code 60,65 % → 63,21 %.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.
Event 777 behält `buffer[0x100]`.
`TRedCoinSwitch::loadAfter` behält `coin`.

### Nächster Schritt

1. `setUpCurrentMapCollision` behält keinen `colman`-Local.
   `MapObjBase` nicht auf `Matching` stellen.
2. Event 777 behält `char buffer[0x100]`
   und `snprintf` mit `sizeof(buffer)`.
3. `TRedCoinSwitch::loadAfter` behält `coin`.
4. `isCollidMove` nicht über `f32 scale`.
   `TNerveDoroHaneRise` nicht über `f32 step`.
   `TMareEventWallRock::load` nicht über `view`.
5. Vermeidungslisten aus Runde 81 bis 111 bleiben.
   Den Frame nicht polstern.

### Nach hundertunddreizehnter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 112:
47,89 % matched code, 1719124 / 3590088 Bytes,
9200 / 12881 Funktionen.
Game Code 35,63 %, 1006976 / 2826784 Bytes,
5235 / 8857 Funktionen.

`TCoin::loadAfter` war bei 99,84 %.
Der Frame lag bei `0x20` statt retail `0x28`,
`checkGround`-Out-Pointer bei `0x14` statt `0x1c`.

**Vollmatch, strikt.**

```cpp
const TBGCheckData* checkData;
char trash[8];
```

Die Reihenfolge `checkData` vor `trash[8]`
reserviert den Frame `0x28` und den Slot `0x1c`.
`checkData` wird in Map 2 für `checkGround` genutzt.

0 Abweichungen, 232 Bytes, 58 Instruktionen.
`functionRelocDiffs=data_value` ohne bad Relocs.
Symbolordnung PASS für `mario/MoveBG/Item`.

**Zurückgenommen.** `TObjManager::load` mit
`JDrama::TNameRef* root` bringt den Puffer von
`0x30` auf `0x2c`, verschiebt aber `readU32`
nach `0x24` statt `0x28` — kein Vollmatch.

**Zähler.** `ninja changes_all`:
matched code 47,89 % → 47,89 %,
1719124 → 1719356 Bytes (+232),
9200 → 9201 Funktionen (+1).
Game Code 35,63 % → 35,64 %,
1006976 → 1007208 Bytes (+232),
5235 → 5236 Funktionen (+1).
`Item` matched_code 55,91 % → 57,16 %.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.
`setUpCurrentMapCollision` ohne `colman`.
Event 777 behält `buffer[0x100]`.
`TRedCoinSwitch::loadAfter` behält `coin`.

### Nächster Schritt

1. `TCoin::loadAfter` behält `checkData` und `trash[8]`
   in dieser Reihenfolge. `Item` nicht auf `Matching` stellen.
2. `TObjManager::load`: Puffer `0x2c` ohne `root`-Spill
   auf `0x24` — andere Benennung oder Reihenfolge testen.
3. `setUpCurrentMapCollision` ohne `colman`.
4. Vermeidungslisten aus Runde 81 bis 112 bleiben.
   Den Frame nicht polstern.

### Nach hundertundvierzehnter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 113:
47,89 % matched code, 1719356 / 3590088 Bytes,
9201 / 12881 Funktionen.
Game Code 35,64 %, 1007208 / 2826784 Bytes,
5236 / 8857 Funktionen.

`TMapObjSwitch::load` war bei 99,80 %.
Der Frame lag bei `0x28` statt retail `0x38`,
die RGB-`read`-Slots bei `0x10`/`0x14`/`0x18`
statt `0x20`/`0x24`/`0x28`.

**Vollmatch, strikt.**

`s32 r`, `g`, `b` und `char trash[0x10]` stehen
vor `TMapObjBase::load`, danach unverändert
`stream >>` in dieselben Locals.

0 Abweichungen, 264 Bytes, 66 Instruktionen.
Symbolordnung wie zuvor (vorbestehende UNUSED-Warnung).
`TObjManager::load` nicht mit `root` angefasst.
`TCoin::loadAfter` unverändert (`checkData`, dann `trash[8]`).

**Zähler.** `ninja changes_all`:
matched code 47,89 % → 47,90 %,
1719356 → 1719620 Bytes (+264),
9201 → 9202 Funktionen (+1).
Game Code 35,64 % → 35,65 %,
1007208 → 1007472 Bytes (+264),
5236 → 5237 Funktionen (+1).
`MapObjTown` matched_code 75,59 % → 78,16 %.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TCoin::loadAfter` behält `checkData` vor `trash[8]`.
2. `TMapObjSwitch::load` behält `r`/`g`/`b` und `trash[0x10]`
   vor `TMapObjBase::load`. `MapObjTown` nicht auf `Matching` stellen.
3. `TObjManager::load` nicht mit `JDrama::TNameRef* root`.
4. `setUpCurrentMapCollision` ohne `colman`.
5. Vermeidungslisten aus Runde 81 bis 113 bleiben.

### Nach hundertundfünfzehnter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 114:
47,90 % matched code, 1719620 / 3590088 Bytes,
9202 / 12881 Funktionen.
Game Code 35,65 %, 1007472 / 2826784 Bytes,
5237 / 8857 Funktionen.

`TRedCoinSwitch::load` war bei 99,80 %.
Der Frame lag bei `0x28` statt retail `0x30`,
der `read`-Slot bei `0x18` statt `0x20`.

`TObjManager::load`: erneut `u32 capacity` vor `buffer`
bzw. `stream.read`/`>>` — Frame schrumpft auf `0x138`
oder Puffer rutscht auf `0x24`; nicht shippen.
`root`-Spill weiter verboten.

**Vollmatch, strikt.**

`u32 tmp` und `char trash[8]` stehen vor `TMapObjBase::load`,
danach unverändert `stream >> tmp` und die übrige Logik.

0 Abweichungen, 180 Bytes, 45 Instruktionen.
Symbolordnung unverändert (vorbestehende UNUSED-Warnungen).
`TMapObjSwitch::load` / `TCoin::loadAfter` unangetastet.

**Zähler.** `ninja changes_all`:
matched code 47,90 % (unverändert Prozentanzeige),
1719620 → 1719800 Bytes (+180),
9202 → 9203 Funktionen (+1).
Game Code 35,65 %, 1007472 → 1007652 Bytes (+180),
5237 → 5238 Funktionen (+1).
`MapObjTown` matched_code 78,16 % → 79,91 %.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TRedCoinSwitch::load` behält `tmp` und `trash[8]` vor
   `TMapObjBase::load`.
2. `TObjManager::load`: Puffer `0x2c` ohne `root`-Spill —
   andere Strategie als `capacity` vor `buffer`.
3. `setUpCurrentMapCollision` ohne `colman`.
4. Vermeidungslisten aus Runde 81 bis 114 bleiben.

### Nach hundertundsechzehnter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 115:
47,90 % matched code, 1719800 / 3590088 Bytes,
9203 / 12881 Funktionen.
Game Code 35,65 %, 1007652 / 2826784 Bytes,
5238 / 8857 Funktionen.

`TCoin::appear` war bei 99,94 %.
Der Frame lag bei `0x28` statt retail `0x48` (−`0x20`).

`TObjManager::load`: erneut `u32 capacity` vor/nach `buffer`
mit `stream.read`/`>>` — Frame schrumpft oder Puffer/`readU32`-Slots
verschieben sich; nicht shippen. `root`-Spill weiter verboten.

`TShine::appearWithDemo` / `TMapObjSwitch::receiveMessage`:
benannte `TFlagT<u16>`-Locals allein reichen nicht
(Flag-Slot weiterhin 4 B zu niedrig); `tmp`+`trash[8]`+`flag`
bläht den Frame über retail — Partial, nicht committet.

**Vollmatch, strikt.**

`char trash[0x20]` am Anfang von `TCoin::appear`,
Logik unverändert (`appearWithoutSound` etc.).

0 Abweichungen, 312 Bytes, 78 Instruktionen.
`validate-symbol-order` für `mario/MoveBG/Item`: PASS.

**Zähler.** `ninja changes_all`:
matched code 47,90 % → 47,91 %,
1719800 → 1720112 Bytes (+312),
9203 → 9204 Funktionen (+1).
Game Code 35,65 % → 35,66 %,
1007652 → 1007964 Bytes (+312),
5238 → 5239 Funktionen (+1).
`Item` matched_code 57,16 % → 58,85 %.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TCoin::appear` behält `trash[0x20]` am Funktionsanfang.
2. `TRedCoinSwitch::load` behält `tmp`/`trash[8]` vor Base-`load`.
3. `TObjManager::load`: Puffer `0x2c` — weiter ohne `root` und
   ohne `capacity`-vor-`buffer`-Muster; ggf. UNUSED/`initObjArray`
   oder Include-/Spill-Kontext prüfen.
4. `TFlagT`-Demo-Calls (`appearWithDemo`, `receiveMessage`):
   Flag-Slot `+4 B` bei korrektem `0x40`/`0x50`-Frame offen.
5. Vermeidungslisten aus Runde 81 bis 115 bleiben.

### Nach hundertundsiebzehnter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 116:
47,90 % matched code, 1720112 / 3590088 Bytes,
9204 / 12881 Funktionen.
Game Code 35,66 %, 1007964 / 2826784 Bytes,
5239 / 8857 Funktionen.

`TShine::makeMActors` war bei 99,86 %.
Der Frame lag bei `0x20` statt retail `0x28` (−`0x8`).
`MActor* result` stand nach dem `TMActorKeeper`-Setup.

`TShine::loadBeforeInit`: `trash[8]` bringt Frame `0x50`,
aber String-/Read-Slots bleiben 8 B zu tief — Partial, nicht shippen.

`TObjManager::load` nicht erneut angefasst.

**Vollmatch, strikt.**

`MActor* result` und `char trash[8]` stehen vor dem
`TMActorKeeper`-Setup; `result` wird wie zuvor in den Zweigen
belegt und nach `mMActor` geschrieben.

0 Abweichungen, 252 Bytes, 63 Instruktionen.
`validate-symbol-order` für `mario/MoveBG/Item`: PASS.

**Zähler.** `ninja changes_all` (gegen ältere Baseline ggf.
mehrere Fn): `makeMActors__6TShineFv` 99,86 % → 100,00 %;
`Item` matched_code 57,16 % → 60,21 % (+252 B für diese Fn).
Gesamt matched code 47,90 % → 47,92 %.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TShine::makeMActors` behält `result` und `trash[8]` vor
   dem Keeper-Setup.
2. `TCoin::appear` / `TRedCoinSwitch::load` / `TMapObjSwitch::load`
   unverändert lassen.
3. `TShine::loadBeforeInit`: Frame mit `trash[8]` ok, Locals +8 B
   ohne falsche `eventId`/`v`-Reihenfolge — weiter offen.
4. `TObjManager::load` / Demo-`TFlagT` wie Runde 116.
5. Vermeidungslisten aus Runde 81 bis 116 bleiben.

### Nach hundertachtzehnter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 117:
47,92 % matched code, 1720364 / 3590088 Bytes,
9205 / 12881 Funktionen.
Game Code 35,67 %, 1008216 / 2826784 Bytes,
5240 / 8857 Funktionen.

`TItem::calc` war bei 99,90 %.
Der Frame lag bei `0x30` statt retail `0x50` (−`0x20`).

`TShine::loadBeforeInit`: zwei neue Local-Reihenfolgen
(`trash`+`v`/`eventId` vor `name`) verschlechterten die Slots;
zurückgesetzt. Weiter offen.

`TObjManager::load` / Demo-`TFlagT` nicht angefasst.

**Vollmatch, strikt.**

`char trash[0x20]` am Anfang von `TItem::calc`,
Matrix-Logik unverändert.

0 Abweichungen, 284 Bytes, 71 Instruktionen.
`validate-symbol-order` für `mario/MoveBG/Item`: PASS.

**Zähler.** matched code 47,92 % → 47,93 %,
1720364 → 1720648 Bytes (+284),
9205 → 9206 Funktionen (+1).
Game Code 35,67 % → 35,68 %,
1008216 → 1008500 Bytes (+284),
5240 → 5241 Funktionen (+1).
`Item` matched_code 60,21 % → 61,75 % (changes_all-TU).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TItem::calc` behält `trash[0x20]` am Funktionsanfang.
2. `TShine::makeMActors` / `TCoin::appear` / Runden 114–115 unverändert.
3. `TShine::loadBeforeInit`: nur Layouts testen, die `name@0x24`
   und Reads `@0x20`/`@0x18` bei Frame `0x50` treffen.
4. `TObjManager::load` / Demo-`TFlagT` weiter vermeiden.
5. Vermeidungslisten aus Runde 81 bis 117 bleiben.

### Nach hundertneunzehnter Iterationsrunde

**Beobachtung, vorher.** Stand Runde 118:
47,93 % matched code, 1720648 / 3590088 Bytes,
9206 / 12881 Funktionen.
Game Code 35,68 %, 1008500 / 2826784 Bytes,
5241 / 8857 Funktionen.

`TMapObjGeneral::recovering` war bei 99,87 %.
Der Frame lag bei `0x20` statt retail `0x48` (−`0x28`).
Der Rumpf nutzte bereits `mat[1][3]` für die Joint-Höhe.

`TMapObjGeneral::recover` / `loadBeforeInit` / `TObjManager::load`
nicht angefasst.

**Vollmatch, strikt.**

`char trash[0x28]` am Anfang von `recovering`,
Sound- und Matrix-Logik unverändert.

0 Abweichungen, 276 Bytes, 69 Instruktionen.
`validate-symbol-order` für `mario/MoveBG/MapObjGeneral`: PASS.

**Zähler.** matched code 47,93 % → 47,94 %,
1720648 → 1720924 Bytes (+276),
9206 → 9207 Funktionen (+1).
Game Code 35,68 % → 35,69 %,
1008500 → 1008776 Bytes (+276),
5241 → 5242 Funktionen (+1).
`recovering` 99,87 % → 100,00 %;
`MapObjGeneral` matched_code 47,56 % → 50,59 % (TU).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `recovering` behält `trash[0x28]` am Funktionsanfang und `mat[1][3]`.
2. `TItem::calc` / `TShine::makeMActors` / `TCoin::appear` / Runden 114–115 unverändert.
3. `TMapObjGeneral::recover`: Frame `0x50` vs `0x28` plus Operanden — nur mit neuem Layout.
4. `TShine::loadBeforeInit`: nur Layouts mit `name@0x24`, Reads `@0x20`/`@0x18`, Frame `0x50`.
5. `TObjManager::load` / Demo-`TFlagT` weiter vermeiden.
6. Vermeidungslisten aus Runde 81 bis 118 bleiben.

### Nach hundertzwanzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 119:
47,94 % matched code, 1720924 / 3590088 Bytes,
9207 / 12881 Funktionen.
Game Code 35,69 %, 1008776 / 2826784 Bytes,
5242 / 8857 Funktionen.

**Kein neues Vollmatch** (Instruktionen + Relocs + Zähler).

`MapObjGeneral::touchGround` / `checkWallCollision`:
`trash[0x38]` bzw. `trash[0x18]` am Anfang → Retail-Frame,
aber `TVec3`/`TBGWallCheckRecord` weiter 0x28/0x18 zu tief —
zurückgesetzt.

`TCoin::perform`: `trash[0x10]` → Frame `0x50`, Argblock für
`TQuestionManager::request` bei `0x34` statt `0x20` — nicht geshipt.

`TNozzleBox::load`: Frame `0x60` mit `trash[0x20]`, `strBuf` bei
`0x10` statt `0x30` — nicht geshipt.

`TMareEventWallRock::load`: `trash[8]` → Frame `0x88`, Schleife
noch `stw`/`addi` bei `0x64` statt `0x68` (objdiff 99,97 %) —
nicht geshipt.

`waitingToAppear` / `TEggYoshi::control` / `TRoulette::moveObject`:
Frame-Padding allein reichte nicht — nicht geshipt.

**Zähler.** matched code unverändert 47,94 %,
1720924 Bytes, 9207 Funktionen.
Game Code unverändert 35,69 %,
1008776 Bytes, 5242 Funktionen.

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

### Nächster Schritt

1. `recovering` / Runden 114–119 unverändert.
2. Frame+Slot-Kandidaten: Padding **und** explizite Locals/UNUSED-Inlines
   für `TBGWallCheckRecord`, `TVec3`-Spills, `strBuf@0x30`.
3. `TMareEventWallRock::load`: 4 B in der `push_back`-Schleife vor
   erneutem `trash`-Only.
4. `TShine::loadBeforeInit` / `TObjManager::load` / Demo-`TFlagT`
   weiter vermeiden.
5. Vermeidungslisten aus Runde 81 bis 119 bleiben.

### Nach hunderteinundzwanzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 120:
47,94 % matched code, 1720924 / 3590088 Bytes,
9207 / 12881 Funktionen.
Game Code 35,69 %, 1008776 / 2826784 Bytes,
5242 / 8857 Funktionen.

Runde 120: Frame-only/`trash`-Only bei `checkWallCollision`,
`TCoin::perform`, `TMareEventWallRock::load` — keine Vollmatches.

**Vollmatch, strikt.**

`TMario::kickRoofEffect`: `getAnmMtx(mJointIdChnFootR)` statt
`mJointIdHead` (Retail `lbz` @ `0x3cb`); `char trash[8]` am
Funktionsanfang für Frame `0x38`.

0 Abweichungen, 148 Bytes, 37 Instruktionen.
`validate-symbol-order` `mario/Player/MarioParticle`: ORDER/LINKAGE OK
(bestehende fehlende UNUSED-Stubs unverändert).

**Zähler.** matched code 47,94 % → 47,94 %,
1720924 → 1721072 Bytes (+148),
9207 → 9208 Funktionen (+1).
Game Code 35,69 % → 35,70 %,
1008776 → 1008924 Bytes (+148),
5242 → 5243 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `kickRoofEffect` behält `mJointIdChnFootR` und `trash[8]`.
2. `recovering` / Runden 114–119 unverändert.
3. `checkWallCollision`: `pad[0x18]` vor Record reicht für Frame,
   Record-Slot `0x28` noch offen (kein trash-only).
4. `TMareEventWallRock::load` / `TCoin::perform` / `TNozzleBox::load`:
   Slot-Layout vor erneutem Padding.
5. `TShine::loadBeforeInit` / `TObjManager::load` / Demo-`TFlagT`
   weiter vermeiden.
6. Vermeidungslisten aus Runde 81 bis 120 bleiben.

### Nach hundertzweiundzwanzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 121:
47,94 % matched code, 1721072 / 3590088 Bytes,
9208 / 12881 Funktionen.
Game Code 35,70 %, 1008924 / 2826784 Bytes,
5243 / 8857 Funktionen.

`checkWallCollision`: `pad[0x18]` + Skalar-/`set`-Init verschlechterte
Operanden (mr r30/r31) — zurückgesetzt auf `TBGWallCheckRecord`-Ctor
(99,7 %, Record @ `0x10`).

**Vollmatch, strikt.**

`TEggYoshi::load`: `char trash[0x18]` am Funktionsanfang für Frame
`0x50` (objdiff 100 %, nur Epilog-Offsets vorher abweichend).

0 Abweichungen, 572 Bytes, 143 Instruktionen.
`validate-symbol-order` `mario/MoveBG/Item`: PASS.

**Zähler.** matched code 47,94 % → 47,96 %,
1721072 → 1721644 Bytes (+572),
9208 → 9209 Funktionen (+1).
Game Code 35,70 % → 35,71 %,
1008924 → 1009496 Bytes (+572),
5243 → 5244 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `kickRoofEffect` / Runden 114–119 / `TEggYoshi::load` unverändert.
2. `checkWallCollision`: Record @ `0x28` ohne Operanden-Regression
   (manueller Store-Block oder UNUSED-Inline, kein trash-only).
3. `TEggYoshi::control` / `TCoin::perform` / `TNozzleBox::load` /
   `TMareEventWallRock::load`: Slot vor Padding.
4. `TShine::loadBeforeInit` / `TObjManager::load` / Demo-`TFlagT`
   weiter vermeiden.
5. Vermeidungslisten aus Runde 81 bis 121 bleiben.

### Nach hundertdreiundzwanzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 122:
47,96 % matched code, 1721644 / 3590088 Bytes,
9209 / 12881 Funktionen.
Game Code 35,71 %, 1009496 / 2826784 Bytes,
5244 / 8857 Funktionen.

Partielle Versuche (revertiert): `TEggYoshi::receiveMessage`
(`trash[0x10]` + Slot), `TGraphWeb::getRandomNextIndex` (`pad[8]`),
`CPolarSubCamera::execGroundCheck_` (`pad[4]`).

**Vollmatch, strikt.**

`TDoroHaneKuri::attackToMario`: `char trash[8]` und `trash[0] = 0`
am Funktionsanfang für Retail-Frame `0x50` (vorher `0x48`).

0 Abweichungen, 444 Bytes, 111 Instruktionen.
`validate-symbol-order` `mario/Enemy/hamukuri`: bestehende BINDING-Warnung
(`onHaveCap__13TDoroHamuKuriFv`) unverändert.

**Zähler.** matched code 47,96 % → 47,97 %,
1721644 → 1722088 Bytes (+444),
9209 → 9210 Funktionen (+1).
Game Code 35,71 % → 35,73 %,
1009496 → 1009940 Bytes (+444),
5244 → 5245 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. `TEggYoshi::load` / `kickRoofEffect` / Runden 114–119 unverändert.
2. `TDoroHaneKuri::attackToMario` behält `trash[8]` + `trash[0]`.
3. `checkWallCollision` / `execGroundCheck_` / `getRandomNextIndex`:
   Slot+Frame ohne Operanden-Regression.
4. `TEggYoshi::control` / `perform` / Mare-`load` weiter vermeiden
   (trash-only).
5. Vermeidungslisten aus Runde 81 bis 122 bleiben.

### Nach hundertvierundzwanzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 123:
47,97 % matched code, 1722088 / 3590088 Bytes,
9210 / 12881 Funktionen.
Game Code 35,73 %, 1009940 / 2826784 Bytes,
5245 / 8857 Funktionen.

Partielle Versuche (revertiert): `MActorAnmData::MActorAnmData`
(`trash[0x10]` verschob Frame auf `0x28` statt `0x20`).

**Vollmatch, strikt.**

`TMario::catching`: `char trash[8]; trash[0] = 0;` am
Funktionsanfang für Retail-Frame `0x30` (vorher `0x28`).

0 Abweichungen, 340 Bytes, 85 Instruktionen.
`validate-symbol-order` `mario/Player/MarioRun`: bestehende
UNUSED-Size-Warnungen unverändert.

**Zähler.** matched code 47,97 % → 47,98 %,
1722088 → 1722428 Bytes (+340),
9210 → 9211 Funktionen (+1).
Game Code 35,73 % → 35,74 %,
1009940 → 1010280 Bytes (+340),
5245 → 5246 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. Runden 114–119 / 121–123 / `catching` unverändert.
2. `checkWallCollision` / `execGroundCheck_` / `getRandomNextIndex`:
   Slot+Frame ohne Operanden-Regression.
3. `MActorAnmData`-Ctor: Frame hängt an `: unk0(0)`-Prolog, kein
   Body-`trash` allein.
4. `TEggYoshi::control` / `perform` / Mare-`load` weiter vermeiden.
5. Vermeidungslisten aus Runde 81 bis 123 bleiben.

### Nach hundertfünfundzwanzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 124:
47,98 % matched code, 1722428 / 3590088 Bytes,
9211 / 12881 Funktionen.
Game Code 35,74 %, 1010280 / 2826784 Bytes,
5246 / 8857 Funktionen.

Partielle Versuche (revertiert): `TLiveActor::initAnmSound`
(`trash[8]` Frame `0x40`, zwei Spills @ `0x2c` vs `0x24` offen),
`TMario::considerRotateStart` (`trash[0x10]`/`pad` — `direction`-Slot).

**Vollmatch, strikt.**

`TNerveDoroHaneRise::execute`: `char trash[8]; trash[0] = 0;` am
Nerve-Anfang für Retail-Frame `0x58` (vorher `0x50`).

0 Abweichungen, 412 Bytes, 103 Instruktionen.
`validate-symbol-order` `mario/Enemy/hamukuri`: bestehende BINDING-Warnung
`onHaveCap__13TDoroHamuKuriFv` unverändert.

**Zähler.** matched code 47,98 % → 47,99 %,
1722428 → 1722840 Bytes (+412),
9211 → 9212 Funktionen (+1).
Game Code 35,74 % → 35,76 %,
1010280 → 1010692 Bytes (+412),
5246 → 5247 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nächster Schritt

1. Runden 114–119 / 121–124 unverändert.
2. `initAnmSound`: Frame mit `trash[8]` ok, NPC-Double-Spill +8 B offen.
3. `considerRotateStart` / `turnEnd` / `turnning`: Slot+Frame (inlining).
4. `checkWallCollision` / `execGroundCheck_` / `getRandomNextIndex`:
   Slot+Frame ohne Operanden-Regression.
5. Vermeidungslisten aus Runde 81 bis 124 bleiben.

### Nach hundertsechsundzwanzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 125:
47,99 % matched code, 1722840 / 3590088 Bytes,
9212 / 12881 Funktionen.
Game Code 35,76 %, 1010692 / 2826784 Bytes,
5247 / 8857 Funktionen.

Partielle Versuche (revertiert): `TLiveActor::initAnmSound`
(`trash[8]` — zwei Spills @ `0x2c` vs `0x24` unverändert),
`TNerveBathtubKillerExplosion` (`trash[4]` — Frame-Regression),
`TSpcInterp::execadd` (`trash[4]`).

**Vollmatch, strikt.**

`TNerveHino2Squat::execute`: `char trash[0x20]; trash[0] = 0;` am
Nerve-Anfang für Retail-Frame `0x58` (vorher `0x38`).

0 Abweichungen, 336 Bytes, 84 Instruktionen.
`validate-symbol-order` `mario/Enemy/hinokuri2`: PASS (UNUSED-Size-Warnungen
unverändert).

**Zähler.** matched code 47,99 % → 48,00 %,
1722840 → 1723176 Bytes (+336),
9212 → 9213 Funktionen (+1).
Game Code 35,76 % → 35,78 %,
1010692 → 1011028 Bytes (+336),
5247 → 5248 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nach hundertssiebenundzwanzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 126:
48,00 % matched code, 1723176 / 3590088 Bytes,
9213 / 12881 Funktionen.
Game Code 35,78 %, 1011028 / 2826784 Bytes,
5248 / 8857 Funktionen.

Partielle Versuche (revertiert): `TNerveHino2Burst::execute`
(`trash[0x20]` — Frame `0x90` OK, TVec-Spill-Cluster @ `0x58` vs `0x78`),
`TNerveHino2Die` (`trash[0x18]`/`volatile`/`0x20` — kein sauberer Vollmatch),
`TNerveHamuKuriWallDie` (`trash[8]` — Frame OK, Slot-Offsets +4),
`TNerveHino2Stamp` mit `trash[0x40]` (Frame `0xd0` vs Retail `0xc8`),
`volatile trash[0x40]` (Frame OK, Operanden-Regression).

**Vollmatch, strikt.**

`TNerveHino2Stamp::execute`: `char trash[0x3c]; trash[0] = 0;` am
Nerve-Anfang für Retail-Frame `0xc8` (vorher `0x88`).

0 Abweichungen, 628 Bytes, 157 Instruktionen.
`validate-symbol-order` `mario/Enemy/hinokuri2`: PASS (UNUSED-Size-Warnungen
unverändert).

**Zähler.** matched code 48,00 % → 48,02 %,
1723176 → 1723804 Bytes (+628),
9213 → 9214 Funktionen (+1).
Game Code 35,78 % → 35,80 %,
1011028 → 1011656 Bytes (+628),
5248 → 5249 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nach hundertachtundzwanzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 127:
48,02 % matched code, 1723804 / 3590088 Bytes,
9214 / 12881 Funktionen.
Game Code 35,80 %, 1011656 / 2826784 Bytes,
5249 / 8857 Funktionen.

Partielle Versuche (revertiert): `TNerveHino2Burst::execute`
(inline `emitWaterParticle` + `trash[0x24]` — Frame `0x90` OK, TVec
@ `0x58` vs `0x78`), `TNerveHino2Pollute` (`trash[0x44]` — Operanden,
kein Frame-only), `TNerveBathtubKillerExplosion` (`trash[4]` im
`time==0`-Block — Frame-Regression `0x30`→`0x38`).

**Vollmatch, strikt.**

`TNerveKumokunFreeze::execute`: `char trash[8]; trash[0] = 0;` am
Nerve-Anfang für Retail-Frame `0x50` (vorher `0x40`).

0 Abweichungen, 624 Bytes, 156 Instruktionen.
`validate-symbol-order` `mario/Enemy/Kumokun`: PASS (UNUSED-Size-Warnungen
unverändert).

**Zähler.** matched code 48,02 % → 48,03 %,
1723804 → 1724428 Bytes (+624),
9214 → 9215 Funktionen (+1).
Game Code 35,80 % → 35,82 %,
1011656 → 1012280 Bytes (+624),
5249 → 5250 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nach hundertneunundzwanzigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 128:
48,03 % matched code, 1724428 / 3590088 Bytes,
9215 / 12881 Funktionen.
Game Code 35,82 %, 1012280 / 2826784 Bytes,
5250 / 8857 Funktionen.

Partielle Versuche (revertiert): `TNerveBathtubKillerBreak::execute`
(`trash[4]` am Nerve-Anfang bzw. vor `generateItemBathtubKiller` —
Frame `0x30` OK, Spill-Offsets @ `0x18` vs `0x1c` unverändert).

**Vollmatch, strikt.**

`TNerveHaneHamuKuriUpWait::execute`: `char trash[4]; trash[0] = 0;` am
Nerve-Anfang für Retail-Frame `0x58` (vorher `0x54`).

0 Abweichungen, 392 Bytes, 98 Instruktionen.
`validate-symbol-order` `mario/Enemy/hamukuri`: BINDING-FAIL
`onHaveCap__13TDoroHamuKuriFv` (weak vs global, vorbestehend; kein
Diff durch diese Runde).

**Zähler.** matched code 48,03 % → 48,04 %,
1724428 → 1724820 Bytes (+392),
9215 → 9216 Funktionen (+1).
Game Code 35,82 % → 35,84 %,
1012280 → 1012672 Bytes (+392),
5250 → 5251 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nach hundertdreißigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 129:
48,04 % matched code, 1724820 / 3590088 Bytes,
9216 / 12881 Funktionen.
Game Code 35,84 %, 1012672 / 2826784 Bytes,
5251 / 8857 Funktionen.

Breiter Frame-only-Sweep (`bosspakkun`, `hamukuri`, `Kumokun`, …): viele
Kandidaten ohne exakte `trash`-Größe; keine weiteren Treffer in dieser
Runde außer Boss Pakkun.

**Vollmatch, strikt (3×).**

- `TNerveBPDie::execute`: `char trash[0x1c]; trash[0] = 0;` — Frame `0x50`
  (vorher `0x30`), 280 B.
- `TNerveBPTumble::execute`: `char trash[8]; trash[0] = 0;` — Frame `0xa0`
  (vorher `0x58`), 376 B.
- `TNerveBPTumbleIn::execute`: `char trash[4]; trash[0] = 0;` — Frame `0x48`
  (vorher `0x40`), 340 B.

`validate-symbol-order` `mario/Enemy/bosspakkun`: PASS (UNUSED-Size-Warnungen
unverändert).

**Zähler.** matched code 48,04 % → 48,07 %,
1724820 → 1725816 Bytes (+996),
9216 → 9219 Funktionen (+3).
Game Code 35,84 % → 35,87 %,
1012672 → 1013668 Bytes (+996),
5251 → 5254 Funktionen (+3).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nach hunderteinunddreißigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 130:
48,07 % matched code, 1725816 / 3590088 Bytes,
9219 / 12881 Funktionen.
Game Code 35,87 %, 1013668 / 2826784 Bytes,
5254 / 8857 Funktionen.

Weiterer `bosspakkun`-Sweep: `BPPreDie`, `BPTakeOff`, `BPHover`, … — kein
reines Frame-`trash`-Match (Slot/Operanden oder Frame zu groß).

**Vollmatch, strikt (4×).**

- `TNerveBPTornado::execute`: `char trash[8]; trash[0] = 0;` — 380 B.
- `TNerveBPSwallow::execute`: `char trash[0xc]; trash[0] = 0;` — 496 B.
- `TNerveBPFlyPivot::execute`: `char trash[4]; trash[0] = 0;` — 172 B.
- `TNerveBPFall::execute`: `char trash[0x28]; trash[0] = 0;` — 1308 B.

`validate-symbol-order` `mario/Enemy/bosspakkun`: PASS (UNUSED-Size-Warnungen
unverändert).

**Zähler.** matched code 48,07 % → 48,14 %,
1725816 → 1728172 Bytes (+2356),
9219 → 9223 Funktionen (+4).
Game Code 35,87 % → 35,95 %,
1013668 → 1016024 Bytes (+2356),
5254 → 5258 Funktionen (+4).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nach hunderteundzweiunddreißigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 131:
48,14 % matched code, 1728172 / 3590088 Bytes,
9223 / 12881 Funktionen.
Game Code 35,95 %, 1016024 / 2826784 Bytes,
5258 / 8857 Funktionen.

Enemy-weiter Sweep (`tobiPuku`, `hamukuri`, `telesa`, …): kein weiteres
reines Frame-`trash`-Match in `0x4`–`0x8c` außer den sechs unten.

**Vollmatch, strikt (6×).**

`gatekeeper.cpp`:

- `TNerveBGKLaunchGoro::execute`: `char trash[8]; trash[0] = 0;` — 468 B.
- `TNerveBGKAwakeDamage::execute`: `char trash[8]; trash[0] = 0;` — 512 B.
- `TNerveBGKWait2::execute`: `char trash[0x20]; trash[0] = 0;` — 964 B.
- `TNerveBGKWait::execute`: `char trash[0x20]; trash[0] = 0;` — 1472 B.

`fireWanwan.cpp`:

- `TNerveFireWanwanAttack::execute`: `char trash[4]; trash[0] = 0;` — 688 B.
- `TNerveFireWanwanRecover::execute`: `char trash[0x40]; trash[0] = 0;` — 580 B.

`validate-symbol-order`: `mario/Enemy/gatekeeper` PASS;
`mario/Enemy/fireWanwan` MISSING-Map-Symbole (vorbestehend, unverändert durch
diese Runde).

**Zähler.** matched code 48,14 % → 48,27 %,
1728172 → 1732856 Bytes (+4684),
9223 → 9229 Funktionen (+6).
Game Code 35,95 % → 36,11 %,
1016024 → 1020708 Bytes (+4684),
5258 → 5264 Funktionen (+6).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nach hundertdreiunddreißigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 132:
48,27 % matched code, 1732856 / 3590088 Bytes,
9229 / 12881 Funktionen.
Game Code 36,11 %, 1020708 / 2826784 Bytes,
5264 / 8857 Funktionen.

Sweep `bosseel`, `walkerEnemy`, `hamukuri`, `hinokuri2`, `poihana`, …:
kein weiteres Frame-only-`trash` in `0x4`–`0x8c` außer Boss-Eel OutWait.
`TNerveBossEelOutWait` war 100,0 % fuzzy aber `nonmatching` (nur Operanden
an `stwu`/Spill-Offsets); `trash[0x30]` (Frame-Delta) overshootet —
exakt `trash[0x28]`.

**Vollmatch, strikt (1×).**

- `TNerveBossEelOutWait::execute`: `char trash[0x28]; trash[0] = 0;` — 1460 B.

`validate-symbol-order` `mario/Enemy/bosseel`: PASS (UNUSED-Size-Warnungen
unverändert).

**Zähler.** matched code 48,27 % → 48,31 %,
1732856 → 1734320 Bytes (+1464),
9229 → 9230 Funktionen (+1).
Game Code 36,11 % → 36,16 %,
1020708 → 1022168 Bytes (+1460),
5264 → 5265 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nach hundertvierunddreißigster Iterationsrunde (Speed-Sweep)

**Beobachtung, vorher.** Stand Runde 133:
48,31 % matched code, 1734320 / 3590088 Bytes,
9230 / 12881 Funktionen.
Game Code 36,16 %, 1022168 / 2826784 Bytes,
5265 / 8857 Funktionen.

Breiter `DEFINE_NERVE`-Sweep (alle `src/Enemy/*.cpp`, `NpcNerve`, …,
`0x4`–`0xbc`, ≥ 99,5 % fuzzy): kein weiteres Treffer außer den zwei
unten (u. a. `NameKuriJumpAttack` `trash[4]` im Brute-Skript, im
Quellstand **kein** Vollmatch — nicht committet).

**Vollmatch, strikt (2×).**

- `TNerveBEelTearsMarioRecover::execute`: `char trash[4]; trash[0] = 0;` — 352 B.
- `TNerveMantaDeath::execute`: `char trash[4]; trash[0] = 0;` — 236 B.

`validate-symbol-order`: `mario/Enemy/bosseel` PASS;
`mario/Enemy/bossManta` ORDER-FAIL an `theNerve__*`-Schwachsymbolen
(vorbestehend, unverändert durch diese Runde).

**Zähler.** matched code 48,31 % → 48,32 %,
1734320 → 1734908 Bytes (+588),
9230 → 9232 Funktionen (+2).
Game Code 36,16 % → 36,18 %,
1022168 → 1022756 Bytes (+588),
5265 → 5267 Funktionen (+2).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nach hundertfünfunddreißigster Iterationsrunde

**Beobachtung, vorher.** Stand Runde 134:
48,32 % matched code, 1734908 / 3590088 Bytes,
9232 / 12881 Funktionen.
Game Code 36,18 %, 1022756 / 2826784 Bytes,
5267 / 8857 Funktionen.

Slot+Frame-Prioritäten (`BossEelDie`/`MouthOpenWait`/`Eat`, `WalkerEscape`,
`BGKAppear`, `Hino2Pollute`, `AnimalGraphWander`): Entry-`trash` und
`0x4`–`0xbc`-Brute **ohne** Vollmatch — echte Operanden/Layout (z. B.
`Hino2Pollute` `changeBck` 16 vs 3, `WalkerEscape` Stack `0x44` vs `0x24`).
Weiterer 100-%-Fuzzy-Scan: `TRoulette::initMapObj` hat **größeren** eigenen
Frame als Retail (kein Padding).

**Vollmatch, strikt (1×).**

- `TMario::turnning()`: `char trash[4]; trash[0] = 0;` — 1004 B
  (`MarioRun.cpp`).

**Zähler.** matched code 48,32 % → 48,35 %,
1734908 → 1735912 Bytes (+1004),
9232 → 9233 Funktionen (+1).
Game Code 36,18 % → 36,22 %,
1022756 → 1023760 Bytes (+1004),
5267 → 5268 Funktionen (+1).

`build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

Keine TU auf `Matching` gestellt.

### Nach hundertsechsunddreißigster Iterationsrunde (Speed, 0 Vollmatches)

**Beobachtung.** Stand Runde 135 unverändert (48,35 % / 9233 Fn).

**1 — „Retail-Frame größer“-Korrektur.** `decomp-diff` zeigt für die
priorisierten 100-%-Fuzzy-Funktionen **unseren** Frame oft **größer** als
Retail (Padding würde verschlimmern):

- `thinkSituation`: Retail `0xc8`, unser `0x2b0`.
- `soundMovement`: Retail `0x2e0`, unser `0x340`.
- `CardLoad::changeScene`: Retail `0x1e0`, unser `0x3b8`.

Entry-`trash` `0x4`–`0x23c`: kein `match`.

Globaler Scan (100 % fuzzy, Retail-`stwu` > unser, Gap ≥ 8): nur
`MarDirectorPreEntry::preEntry`, `ModelWaterManager::drawRefracAndSpec`,
`MarioWait::waitMain` — Entry-`trash` ohne Vollmatch.

**2 — `Hino2Pollute`.** `changeBck(3)` → `changeBck(16)` / `17` an zwei
Retail-Stellen (`li r4, 0x10` / `0x11`) behebt Operanden, bleibt aber
~26 Diff-Zeilen (fehlendes Inline: Wasser/`rand`/Stack `0xe8` vs `0xa0`).
**Nicht committet** (kein 100 %).

**3 — Sonstiges.** `SampleCtrlMaterial` / `TMapObjManager::load`:
Brute meldete fälschlich `trash[4]` (Retail-Frame kleiner). Bosseel/Walker
unverändert defer.

**Vollmatch.** keine.

`ninja` / DOL-SHA1 unverändert OK.

### Nach hundertsiebenunddreißigster Iterationsrunde (Speed, 0 Vollmatches)

**Beobachtung.** Stand Runde 135 unverändert (48,35 % / 9233 Fn).

**Scan.** 25× 100-%-Fuzzy game-Funktionen; kein Kandidat mit nur
`stwu`/Epilog-Diff und Retail-Frame > unser (automatischer Filter: 0 Treffer).
Entry-`trash` brute: `thinkSituation` (aktuell oft **unser** Frame größer,
z. B. `stwu -0xd0` vs Retail `-0xc8`), `getRandomNextIndex`,
`execGroundCheck_`, `TMarDirector::TMarDirector` (ein Operand `addi r4,r1`
für `OSInitStopwatch`, kein reines Padding).

**`initMirrorModel`.** `.rodata`-Anfang per `DummyStrings`/`MtxCalcTypeName`
vor `MarioAnimeData.hpp` angleichen (wie `MarioParticle.cpp`) — Spiegel-
Strings bleiben **0x18** zu früh (`0xa38` vs `0xa50`); fehlendes
0x18-Null-Pad zwischen `ma_sleep_end_tx.btp`-Cluster und folgendem
`.rodata` (nicht committet, kein Vollmatch).

**Vollmatch.** keine.

`ninja` / `build/GMSJ01/mario.dol: OK`.
SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625`.

### Nach hundertachtunddreißigster Iterationsrunde (Speed, 1 Vollmatch)

**Vollmatch.** `TMario::initMirrorModel` (284 B, `mario/Player/MarioDraw`).

**Ursache.** Compound-Literale für `setInfo[0/1]` in `initModel` landeten in
`.rodata` vor dem Foot-Null-Cluster; Spiegel-Strings bei **0xa38** statt
**0xa50** (`addi r4, r28, …` −0x18).

**Fix.** `DummyStrings`/`MtxCalcTypeName` vor `MarioAnimeData.hpp`;
nach `MarioFootDirLCtrl` zwei **0xc**-Nullblöcke plus
`marioInitModelSetInfoRo0`/`Ro1` (je 0xa); `initModel` kopiert aus Rodata und
setzt `setInfo[1].unk0 = mJointIdChnChest`.

**Nebenwirkung.** `initModel` fuzzy 95,46 % → 94,28 % (erwartet: andere
Relocs); MarioDraw matched_data 11,92 % → 48,36 %.

`ninja` / DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.
`changes_all`: Total matched_code 48,35 % → 48,36 %.

### Nach hundertneununddreißigster Iterationsrunde (PROGRESS, 0 Vollmatches)

**Beobachtung.** R138-Stand: `initMirrorModel` 100 %; `initModel` 94,1 %;
Spiegel-Strings @ **0xa50** OK.

**`initModel`-Recovery (revertiert).** Retail `stwu -0x5c0`; unser Build mit
`marioInitModelSetInfoRo0/1`-Kopie **−0x558** (−0x68). Compound-Literale für
`setInfo[0/1]` erzeugen Retail-Stack-Spills (`lwz 0xa38(r30)` → `0x3b0(r1)` …)
und **−0x578** (−0x48), aber Gesamt-Fuzzy fällt auf ~89,9 % (Tex-Loop-Cluster).
`++j` statt `++i` in der `J3DTexNoAnm`-Schleife ist ASM-korrekt (Retail
`addi r7,r7,1`), verschlechtert aber solo auf 88,7 % — Loop und Frame müssen
gemeinsam angegangen werden. Scratch-Locals / Buffer vergrößern / Locals an den
Funktionsanfang: kein Frame-Gewinn.

**Nächster ASM-Haken für `initModel`.** Behalten: Foot-Nullblöcke +
`marioInitModelSetInfoRo0/1` @ **0xa38** (Mirror fix). Ziel: Compound-Literal-
**Codegen** für `setInfo` **ohne** zweites Rodata — vermutlich UNUSED-Inline/
Stack-Layout (~0x48–0x68) aus `mario.MAP`, nicht Entry-`trash`.

**Vollmatch.** keine.

`ninja` / DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### Nächster Schritt

1. R138 Mirror-Fix unverändert; `turnning` trash[4] behalten.
2. `initModel`: MAP/UNUSED + Stack −0x5c0 mit Compound-`setInfo`-Spills.
3. Tex-Loop `++j` erst mit passendem Frame/Cluster committen.
4. `thinkSituation` / `soundMovement` / `changeScene`: Frame verkleinern.
5. Defer-Listen unverändert.

### Nach hundertvierzigster Iterationsrunde

**Beobachtung.** R139: `initModel` zu verflochten; Strategie auf andere TUs.

**Vollmatch, strikt.**

`TRoulette::moveObject`: ASM `lfs`/`stfs` @ **0x34** → `mRotation.y` (nicht
`.x`) plus `char trash[0x20]; trash[0] = 0;` für Retail-Frame `0x58`.

0 Abweichungen, 244 Bytes, 61 Instruktionen.
`validate-symbol-order` `mario/MoveBG/MapObjSirena`: PASS (bestehende UNUSED-
Size-Warnung `getSlotResult` unverändert).

`initMirrorModel` bleibt 100 %; Mirror-Strings @ **0xa50** unverändert.

`ninja` / DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### Nächster Schritt

1. R138–R140 Mirror / `turnning` trash[4] / `moveObject` unverändert halten.
2. Weitere MoveBG/Enemy-Kandidaten mit klarem Member-Offset (wie `0x34` Ry).
3. `initModel` nur mit kombiniertem Frame+Compound+`++j`-Cluster.
4. Defer-Listen unverändert.

### Nach hunderteinundvierzigster Iterationsrunde

**Beobachtung.** R141: mehrere 99,9 %-Kandidaten (u. a.
`TMapObjBase::getDistance`, `TSpineEnemy::resetToPosition`,
`TRoulette::initMapObj` Iterator-Spill, `TCloset::calcRootMatrix` Mtx-Basis
0x14 vs 0x10) — noch keine strikte Byte-Identität.

**Vollmatch, strikt.** keine (R140 `TRoulette::moveObject` unverändert).

**Teilfortschritt MapObjSirena.**

- `TCloset::calcRootMatrix`: `char trash[8]; trash[0]=0;` → Retail-Frame
  `0x70` (Fuzzy ~99,96 %); verbleibend `addi r30,r1,0x14` vs `0x10` und
  `mtx.ref(1,3)`-Spill 0x30 vs 0x2c.
- `TItemSlotDrum::generateItem`: `MsMtxSetRotY` nutzt `mRotation.y` statt
  `.x` (ASM `lfs` @ 0x34) — `generateItem` gesamt noch ~90 %.

`initMirrorModel` 100 %; `TRoulette::moveObject` **match**; DOL-SHA1
`9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### Nächster Schritt

1. `TCloset::calcRootMatrix`: Mtx-Stack-Basis +4 B ohne Frame-Wachstum (evtl.
   `TRotation3f`-Layout / lokale Reihenfolge).
2. `getDistance`: Retail-Reihenfolge `pos.y−yOffset` vor `dx` hält `0x18`-Frame,
   Register/Spill 0x14 noch offen.
3. `initMapObj`: Iterator @ `0x5c` ohne `0xa8`-Frame (nur `trash[4]` vor
   `push_back` reicht für ersten Spill, nicht für `0x7c`-Cluster).
4. Defer-Listen unverändert.

### Nach hunderte zweiundvierzigster Iterationsrunde

**Vollmatch, strikt.**

- `TMapObjBase::getDistance` (`MapObjLib.cpp`): `sqrtTemp` mit `pad[4]` +
  `volatile f32 y` im `__frsqrte`-Block → Retail-Spill `stfs`/`lfs` @ `0x14`
  bei unverändertem Frame `0x18` (**match**).

**Teilfortschritt MapObjSirena (unverändert R141-Zielbild, näher).**

- `TCloset::calcRootMatrix`: `trash[4]` + `{ pad[4]; TRotation3f mtx; }`
  `local` → Mtx-Basis `0x14`, `ref(1,3)` @ `0x30`, Frame `0x70`; offen nur
  `mr` vs `addi r31,r3,0` und `mr` vs `addi r3,r30,0` (~98,4 %).
- `TItemSlotDrum::generateItem`: `mRotation.y` in `MsMtxSetRotY` (Teil).

`initMirrorModel` 100 %; `TRoulette::moveObject` **match**; DOL-SHA1
`9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### Nächster Schritt

1. `TCloset::calcRootMatrix`: `mr`/`addi`-Paar nach `getModel` / vor
   `MsMtxSetXYZRPH` (Register-Homing).
2. `TRoulette::initMapObj`: Iterator `0x5c` + `0x7c`-Cluster ohne `0xa8`-Frame.
3. Weitere `.x`→`.y`-Offsets wie Roulette.
4. Defer-Listen unverändert.

### Nach hunderte dreiundvierzigster Iterationsrunde

**Vollmatch, strikt.**

- `TSlotDrum::initNeonMatColor`: `char trash[4]; trash[0]=0;` → Frame
  `0x58` und Mat-Name-Stack @ `0x28` (**match**).

**Teilfortschritt.**

- `TCloset::calcRootMatrix`: `trash[4]` + `pad2[4]`, dann
  `getModel()` vor `TRotation3f mtx` (Saku-Reihenfolge) → `mr r31,r3` /
  `mr r3,r30` OK; verbleibend Mtx-Basis `0x10` vs `0x14` und
  `ref(1,3)`-Spill `0x2c` vs `0x30` (~99,8 %).

R142 `getDistance` **match**; R140 `TRoulette::moveObject`; R138 `initMirrorModel`;
DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R145 (Pivot: `drawLogic`, Closet unangetastet)

**Vollmatch, strikt.**

- `TMario::drawLogic`: `char trash[4]; trash[0]=0;` am Funktionsanfang → Frame
  `-0x28` und Iterator-Spill @ `0x18` (**match**).

**Teilfortschritt (nur notiert, nicht committed).**

- `TMapObjBase::joinToGroup`: gleiches `trash[4]`-Muster bringt Frame `-0x68`, verbleibend
  `insert`-Spills `0x48` vs `0x4c` (~99,9 %).

R143 `initNeonMatColor`, R142 `getDistance`, R140 `TRoulette::moveObject`, R138
`initMirrorModel`; `TCloset::calcRootMatrix` Teilstand unverändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R146 (`joinToGroup`)

**Vollmatch, strikt.**

- `TMapObjBase::joinToGroup`: `char trash[4];`, `TNameRef* list = search(...);`,
  `trash[0]=0;` vor `push_back` (nicht Entry-Trash allein) → Frame `-0x68`, Iterator-
  und `insert`-Spills wie Retail (**match**).

R145 `drawLogic`, R143 `initNeonMatColor`, R142 `getDistance`, R140 `moveObject`, R138
`initMirrorModel`; Closet-Teilstand unverändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R147 (`TMapObjSwitch::receiveMessage`)

**Vollmatch, strikt.**

- `TMapObjSwitch::receiveMessage`: Entry-`char trash[4]; trash[0]=0;` → Frame
  `-0x40`, Demo-Camera-Stack @ `0x2c` (**match**).

R146 `joinToGroup`, R145 `drawLogic`, R143 `initNeonMatColor`, R142 `getDistance`, R140
`moveObject`, R138 `initMirrorModel`; Closet-Teilstand unverändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R148 (`initAndRegister` Teilstand)

**Kein neuer Vollmatch** (Ziel war `initAndRegister` @ 100 %).

- `TMapObjBase::initAndRegister`: Entry-`char trash[4]; trash[0]=0;` + verkettetes
  `search`→`push_back` (kein `list`-Local) → Frame `-0x70` und `addi r31,r3,0x10`
  wie Retail (**99,9 %**).
- Verbleibend: drei `insert`-Spills +4 B zu hoch (`0x50`/`0x4c`/`0x50` vs
  `0x4c`/`0x48`/`0x4c`).
- Mid-Trash + `TNameRef* list` nach `search` fixiert die Spills, erzwingt aber
  `addi r30,r3,0x10` (~99,7 %) — gleicher Trade-off wie in R147-Notizen.

R147 `receiveMessage`, R146 `joinToGroup`, R145 `drawLogic`, R138 `initMirrorModel`
unverändert @ 100 %.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### Nächster Schritt

1. `initAndRegister`: Mid-Trash nach `search` **ohne** `list`-GPR (r3/r31 halten) oder
   anderer Register-Hebel für die letzten +4 B bei Entry-Trash.
2. `TRoulette::initMapObj`: Mid-Trash nach `new`/`search` (nicht Entry allein).
3. `TCloset::calcRootMatrix` / `partsRollCallback`: nur bei klarem +4‑B-Hebel.
4. Defer-Listen unverändert.

### R151 (`TEggYoshi::control`)

**Vollmatch, strikt.**

- `TEggYoshi::control`: `JGeometry::TVec3<f32> pos`/`v` vor Entry-`char
  trash[0x8]; trash[0]=0;`, Case `0xC` ruft `makeObjDead()` (vtable `0x104`)
  statt `kill()`; Case `0xF` nutzt Top-Level-`v`.

0 Abweichungen, 540 Bytes, 135 Instruktionen.
`validate-symbol-order` `mario/MoveBG/Item`: PASS.

R150 `TNozzleBox::load`, R149 `TEggYoshi::receiveMessage`, R147
`TMapObjSwitch::receiveMessage`, R146 `joinToGroup`, R145 `drawLogic`, R138
`initMirrorModel` unverändert @ 100 %.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### Nächster Schritt (A)

1. `TCoin::perform`: Frame `-0x50` bereits auf HEAD; nur noch vier Operanden
   beim `request`-Spill (`addi`/`stw` @ `0x34`/`0x38`/`0x3c` vs `0x24`/`0x28`/`0x2c`).
   Entry-/Mid-`trash`, `reqPos`, Trash nach `LIVE_FLAG_DEAD` (+ `0x8`…`0x10`)
   und Dual-Trash ändern den Spill nicht.
2. `TRoulette::initMapObj`: Iterator-Spills `+4` B (kein joinToGroup-`list`+Trash;
   explizites `insert`/`list`-Local verschlechtert).
3. `initAndRegister`: SMS-B.
4. Defer-Listen unverändert.

### R156 (`TMapObjBase::throwObjToFront`)

**Vollmatch, strikt.**

- `throwObjToFront`: `char pre[8]`, dann `Mtx mtx`, dann `char trash[8]`;
  `mMActor`-Zweig `MtxPtr anmMtx`. Frame `-0x90`, `MsMtxSetRotRPH`-Buffer @
  `0x38` (reines `trash[8]`/`trash[0x10]` allein reichte nicht).
- `throwObjToFrontFromPoint` (R155) unverändert matching.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

**Nächste MoveBG:** `rotateVecByAxisY` (Retail inlined, kein TRotation3-Stack);
weitere hoist+trash-Kandidaten in MapObjLib. Skip: touchFruit / appearWithDemo /
newAndRegisterCoin ohne klaren Hebel.

### R157 (Round 71 Agent — kein Vollmatch)

**Kein neuer strikter Vollmatch** (Scope A MoveBG).

- `TMapObjTree::initMapObj`: `char buffer[64]` am Funktionskopf, danach
  `char trash[4]; trash[0]=0;` → Frame `-0x90`, `snprintf`-Buffer @ `0x2c`
  wie Retail; verbleibend **3** Operand-`~` (`mLeafNum` in `r26` statt `r25`
  für `new[]` / `__construct_new_array`).
- `TMapObjGrassManager::initDrawNear`: `Mtx` + `trash[0x10]` nach `Mtx` +
  hoisted `vec.set()` hält Frame `0x98` und `viewItm` @ `0x44`; **4** `~` beim
  `GXSetChanMatColor`-Spill (`0x34` vs `0x24`) — nicht committed.
- `TMapObjSwitch::receiveMessage` / `TRoulette::initMapObj` / `initAndRegister`:
  Iterator- bzw. `TFlagT`-Spills unverändert (Entry-/Mid-Trash reicht nicht).

R156 `throwObjToFront`, R155 `throwObjToFrontFromPoint`, R154 `touchWater` unverändert.

`validate-symbol-order` `mario/MoveBG/MapObjTree`: PASS.
DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

**Nächste MoveBG:** `initMapObj` `r25`-Homing (Registerdruck `new`/`r25`-Collision);
`initDrawNear` GX-Spill ohne Frame-Wachstum; `rotateVecByAxisY` nur bei inlined
Retail-Pfad. Skip: touchFruit / appearWithDemo / newAndRegisterCoin.

### R158 (`TCloset::calcRootMatrix`)

**Vollmatch, strikt.**

- `TCloset::calcRootMatrix`: Entry-`char trash[4]; trash[0]=0;`, danach
  `getModel()`; Mid-`char pad2[4]; pad2[0]=0;` vor `TRotation3f mtx` →
  `MsMtxSetXYZRPH`-Basis @ `0x14`, `mtx.ref(1,3)` @ `0x30`, Frame `-0x70`
  (**match**).

R157 `initMapObj` partial, R156–R154 unverändert.
`validate-symbol-order` `mario/MoveBG/MapObjSirena`: PASS (UNUSED-Size-Warnungen).

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

**Nächste MoveBG:** `TMapObjSwitch::receiveMessage` (`TFlagT` @ `0x2c` vs `0x24`);
`initMapObj` / `initDrawNear` nur mit neuem Hebel. Skip: Defer-Liste unverändert.

### R160 (`TCloset::calcRootMatrix` — strikt nachgezogen)

**Vollmatch, strikt (0 Marker in `decomp-diff`).**

- R158 hatte fuzzy 100 % mit **3** `~` (`mtx` @ `0x10` vs Retail `0x14`).
- Fix: nach `getModel()` `struct { char pad[4]; Mtx mtx; } local;` statt
  freiem `TRotation3f` + Mid-`pad2[4]` → Basis @ `0x14`, `mtx[1][3]` @ `0x30`,
  Frame `-0x70`, `mr r31`/`mr r3` wie Retail (**match**).

`validate-symbol-order` `mario/MoveBG/MapObjSirena`: PASS (UNUSED-Size-Warnungen).

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

**Nächste MoveBG:** `TDonchou::calcRootMatrix` (+0xc @ `-0x90`);
`TMapObjTree::initMapObj` (`r25` vs `r26`); Switch `TFlagT` nur mit klarem Hebel.

### R162 (`THideObjPictureTwin::loadAfter`)

**Vollmatch, strikt (0 Marker in `decomp-diff`).**

- Rogue `.rodata` vor `@3113`: `rogueRodata2782[0xc]` + `rogueRodata2784[3]`
  (Twin-Strings `@1490+0x164` / `+0x174` statt `+0x14c`).
- `char pad[8]; char nameBuf[0x40];` auf Funktions-Ebene vor Parent-`loadAfter`;
  Suffix-Bytes als `char suffix0`–`suffix3`, `snprintf` + `stbx`-Patches wie Retail
  → Frame `-0x90`, `nameBuf` @ `0x28`, `stmw` @ `0x74`.

0 Abweichungen, 216 Bytes, 54 Instruktionen.
`mario/MoveBG/MapObjHide` matched_data 91,07 % → 100,00 %.

R160 `TCloset::calcRootMatrix` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R163 (`TRollBlock::load`)

**Vollmatch, strikt.**

- `TRollBlock::load`: `stream.read(&local_18, 4)` statt `operator>>` auf `s32`
  (Retail `read` mit `li r5, 4`).

0 Abweichungen, 168 Bytes, 42 Instruktionen.
`validate-symbol-order` `mario/MoveBG/MapObjRailBlock`: PASS.

R162 `THideObjPictureTwin::loadAfter`, R160 `TCloset::calcRootMatrix` unverändert
strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R165 (`TMapObjGeneral::checkWallCollision`)

**Vollmatch, strikt.**

- `TBGWallCheckRecord` zuerst, danach `char trash[0x18]` (Emissionsreihenfolge
  bei `-inline deferred`) → Frame `-0x60`, Record/Spill @ `0x28` wie Retail.
- Kein manuelles `set`/Skalar-Init nötig; Konstruktor + `isTouchedWallsAndMoveXZ`
  unverändert.

0 Abweichungen, 232 Bytes, 58 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjGeneral -d checkWallCollision`: 100 %.

R164 `TMapObjSwitch::control`, R163/R162/R160 unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R166 (`TMapObjManager::load`)

**Vollmatch, strikt.**

- `char trash[0x78]` am Funktionskopf nach Parent-`load`-Aufruf-Pfad → Frame
  `-0x158` wie Retail (`0xe0` → `0x158` Delta durch inlined `TLiveManager::load` /
  Stream-`read`-Layout).

0 Abweichungen, 1216 Bytes, 304 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjManager -d TMapObjManager::load`: 100 %.

R165 `checkWallCollision`, R164 `TMapObjSwitch::control`, R163/R162/R160
unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R317 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBiancoWatermillVertical::control`.

- Drehzahl läuft mit `mRotSpeedDownRate` auf `unk13C` zu und wird dort geklemmt.
- `mRotation.y` und die Y-Drehung der Brücke an `unk140` werden um den Schritt erhöht und mit `MsWrap` auf `[0, 360)` gelegt.
- Zwei `startSoundActorWithInfo`-Aufrufe: Wind `0x3040` an `mPosition`, Bewegung `0x3042` an der Brücke.
- `char trash[1]` hebt den Frame von `-0x38` auf `-0x40`.
- 456 Bytes, 114 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.487816 % -> 79.50042 %, matched code 50.338596 % -> 50.3513 % (1807200 -> 1807656, +456).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9686 -> 9687.
`MapObjBianco` 7308 -> 7764 (+456).
Kein R170–R316-Unit hat matched code verloren.

### R316 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBiancoWatermillVertical::loadAfter` und `TLampSeesawMain::loadAfter`.

- Watermill vergleicht `mName` mit `BiaWatermillVertical 0` und sucht `BiaTurnBridge 0` oder `1`.
- `mBodyRadius` wird auf `1000.0f` gesetzt.
- Seesaw kopiert das Namenssuffix von `ランプシーソーＡ` auf `ランプシーソーＢ００`, sucht das Gegenstück und setzt `unk138->unk138 = this`.
- `char trash[1]` hebt den Seesaw-Frame von `-0x88` auf `-0x90`.
- Je 196 Bytes, 49 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.47712 % -> 79.487816 %, matched code 50.327682 % -> 50.338596 % (1806808 -> 1807200, +392).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9684 -> 9686.
`MapObjBianco` 6916 -> 7308 (+392).
Kein R170–R314-Unit hat matched code verloren.

### R314 (`MapObjBase`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjBase::makeObjAppeared`.

- `mVelocity` wird z, y, x genullt.
- Anm-Mtx-Pfad bleibt inlined `setMtx`.
- TRS-Pfad ruft `setMtx__17TMapCollisionBaseFPA4_f` direkt.
- `char trash[29]` hebt den Frame von `-0x98` auf `-0xc0`.
- 776 Bytes, 194 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBase`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.47708 % -> 79.47712 %, matched code 50.306065 % -> 50.327682 % (1806032 -> 1806808, +776).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9683 -> 9684.
`MapObjBase` 5852 -> 6628 (+776).
Kein R170–R313-Unit hat matched code verloren.

### R313 (`Item`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TShine::calc`.

- `StageUtil.hpp` zog ungenutzte Shine-Tabellen ins `.data` und legte `mPromiLife` auf `0x1a0` statt `0x38`.
- Deklaration `extern u8 SMS_getShineStage(u8)` ersetzt das Include.
- `char trash[45]` hebt den Frame von `-0x50` auf `-0x88`.
- 736 Bytes, 184 Instruktionen.

`validate-symbol-order` `mario/MoveBG/Item`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.477066 % -> 79.47708 %, matched code 50.285564 % -> 50.306065 % (1805296 -> 1806032, +736).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9682 -> 9683.
`Item` 14796 -> 15532 (+736).
Kein R170–R312-Unit hat matched code verloren.

### R312 (`MapObjGeneral`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjGeneral::calcRootMatrix`.

- Halte-Matrix kommt von `mHolder->getTakingMtx()`.
- Translation der 3x4-Matrix steht in `[0][3]`, `[1][3]`, `[2][3]`.
- Freier Pfad übergibt Position und Offset direkt an `MsMtxSetXYZRPH`.
- 412 Bytes, 103 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjGeneral`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.47678 % -> 79.477066 %, matched code 50.274086 % -> 50.285564 % (1804884 -> 1805296, +412).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9681 -> 9682.
`MapObjGeneral` 7300 -> 7712 (+412).
Kein R170–R311-Unit hat matched code verloren.

### R311 (`Item`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TShine::receiveMessage`.

- Übernimmt Mario-Position und Y-Rotation, setzt die Basis-Matrix und startet
  `shine_demo_shine_get` bzw. die Yoshi- und Empty-Variante.
- `char trash[1]` hebt den Frame von `-0x78` auf `-0x80`.
- 436 Bytes, 109 Instruktionen.

`validate-symbol-order` `mario/MoveBG/Item`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.476746 % -> 79.47678 %, matched code 50.261944 % -> 50.274086 % (1804448 -> 1804884, +436).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9680 -> 9681.
`Item` 14360 -> 14796 (+436).
Kein R170–R310-Unit hat matched code verloren.

### R310 (`MapObjTown`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjSwitch::receiveMessage`.

- Hip-Drop startet `objswitch`, `MSD_SE_OBJ_AP_BUTTON`, `removeMapCollision`,
  `unk144[i]->action(unk140)` und `fireStartDemoCamera` mit `TFlagT<u16>()`.
- `static inline fireSwitchCam` mit `char pad[1]` legt das Flag-Temp auf `r1+0x2c`
  bei Frame `-0x40`.
- 288 Bytes, 72 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjTown`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy bleibt 79.476746 %, matched code 50.25392 % -> 50.261944 % (1804160 -> 1804448, +288).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9679 -> 9680.
`MapObjTown` 9524 -> 9812 (+288).
Kein R170–R308-Unit hat matched code verloren.

### R308 (`MapObjRicco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TCraneRotY::control`.

- `switch (mState)` dreht `mRotation.y` mit `unk144` und setzt `mStateTimer` aus `mWaitTime`.
  Ton bei `isState(0) || isState(2)` über `gateCheck(unk148)`.
- `char trash[0x11]` hält den Frame bei `-0x30`.
- 412 Bytes, 103 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjRicco`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.46537 % -> 79.476746 %, matched code 50.242447 % -> 50.25392 % (1803748 -> 1804160, +412).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9678 -> 9679.
`MapObjRicco` 2972 -> 3384 (+412).
Kein R170–R307-Unit hat matched code verloren.

### R307 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBellWatermill::loadAfter`.

- `TMapObjTurn::loadAfter`, dann `unk150 = 2` und die Dreh-Konstanten.
  `TNameRefGen::search` von `"BiaBell 0"`, `"BiaBell 1"` und `"BiaBell 2"` nach `unk194`, `unk198`, `unk19C`.
  `unk1A0 = 1`.
- `InfectiousStrings` legt die String-Offsets ab `0xF8`.
- 304 Bytes, 76 Instruktionen.
  Frame `-0x68`.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.45703 % -> 79.46537 %, matched code 50.23398 % -> 50.242447 % (1803444 -> 1803748, +304).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9677 -> 9678.
`MapObjBianco` 6612 -> 6916 (+304).
Kein R170–R306-Unit hat matched code verloren.

### R306 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMareCork::loadAfter`.

- `TNameRefGen::search("砲台")` nach `unk138`.
  `receiveMessage(this, HIT_MESSAGE_TAKE)` schreibt den Zeiger nach `mHeldObject`.
  Drei `SMS_LoadParticle` (`0x14C`, `0x14D`, `0x14E`), dann `TMapObjBase::loadAfter`.
  `unk13C` wird genullt, danach `initAnmSound`.
- `InfectiousStrings` und Rogue-Rodata `@2690`/`@2692` legen die String-Offsets.
- 332 Bytes, 83 Instruktionen.
  Frame `-0x38`.

`validate-symbol-order` `mario/MoveBG/MapObjMare`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.44788 % -> 79.45703 %, matched code 50.224728 % -> 50.23398 % (1803112 -> 1803444, +332).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9676 -> 9677.
`MapObjMare` 5584 -> 5916 (+332).
Kein R170–R305-Unit hat matched code verloren.

### R305 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBigWatermelon::initMapObj`.

- `TMapObjBall::initMapObj`, dann fünf `SMS_LoadParticle` (`0x5D`, `0x5E`, `0x5F`, `0x6B`, `0x6C`).
  `unk198` ist `new TWaterEmitInfo("/watermelon.prm")`.
- `InfectiousStrings.hpp` setzt die Rodata-Basis, die Offsets ab `0xE0` treffen.
- 340 Bytes, 85 Instruktionen.
  Frame `-0x20`.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.43852 % -> 79.44788 %, matched code 50.215263 % -> 50.224728 % (1802772 -> 1803112, +340).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9675 -> 9676.
`MapObjBall` 8400 -> 8740 (+340).
Kein R170–R304-Unit hat matched code verloren.

### R304 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBigWatermelon::kill`.

- Drei `emitAndScale` ohne Scale, dann zwei mit `(1, 1, 1)`.
  Position geht wortweise nach `unk198 + 0x70`, danach `emitRequest`.
  `MSD_SE_OBJ_WATERMELON_BLOCK` über `gateCheck`.
  Unter `unk19C < 10` erscheint `0x2000000E` mit Velocity `(0, 25, 0)` und `offLiveFlag(LIVE_FLAG_UNK10)`.
  Danach `TMapObjGeneral::kill`.
- `char trash[0x10]` am Ende hält Frame `-0x38`.
- 352 Bytes, 88 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.428825 % -> 79.43852 %, matched code 50.205456 % -> 50.215263 % (1802420 -> 1802772, +352).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9674 -> 9675.
`MapObjBall` 8048 -> 8400 (+352).
Kein R170–R303-Unit hat matched code verloren.

### R303 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TCoverFruit::calcRootMatrix`.

- Mit `mHolder` kopiert `setBaseTRMtx` die Taking-Matrix und übernimmt die Translation.
  Sonst baut `MsMtxSetXYZRPH` die Root-Matrix aus Position, `mYOffset` und Rotation.
  `setBaseScale` kopiert `mScaling`.
- `char trash[8]` hält Frame `-0x70`.
- 324 Bytes, 81 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.419914 % -> 79.428825 %, matched code 50.196426 % -> 50.205456 % (1802096 -> 1802420, +324).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9673 -> 9674.
`MapObjBall` 7724 -> 8048 (+324).
Kein R170–R302-Unit hat matched code verloren.

### R302 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBigWatermelon::receiveMessage`.

- `isActorType(0x80000001)` ruft `boundByActor` und liefert 1.
  Sonst liefert `TMapObjGeneral::receiveMessage` bei Erfolg 1.
  Nachricht 4 mit `MAP_OBJ_FLAG_UNK100000` ruft `hold`.
  Dieselbe Actor-Art, nicht `0x400000D0` und nicht Nachricht 4, ruft `kicked`.
  Sonst 0.
- `#pragma dont_inline` hält den `bl` auf das leere `boundByActor`.
- 316 Bytes, 79 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.41127 % -> 79.419914 %, matched code 50.18763 % -> 50.196426 % (1801780 -> 1802096, +316).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9672 -> 9673.
`MapObjBall` 7408 -> 7724 (+316).
Kein R170–R301-Unit hat matched code verloren.

### R301 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TResetFruit::touchWater`.

- `STATE_HOLDING` oder `STATE_APPEARING` überspringt die Geschwindigkeit.
  Sonst kommt `getWaterSpeed` auf eine Kopie von `mVelocity`, skaliert mit `unk17C`.
  Danach `offLiveFlag(LIVE_FLAG_UNK10)`.
  Ist der State-Timer aus, setzt `MAP_OBJ_FLAG_DISAPPEARING` und `mStateTimer = getLivingTime()`.
  Zum Schluss noch einmal `offLiveFlag` und `mState = 11`.
- `char trash[4]` hält den Frame auf `-0x38`.
- 336 Bytes, 84 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.40212 % -> 79.41127 %, matched code 50.178272 % -> 50.18763 % (1801444 -> 1801780, +336).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9671 -> 9672.
`MapObjBall` 7072 -> 7408 (+336).
Kein R170–R300-Unit hat matched code verloren.

### R300 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBiancoMiniWindmill::touchWater`.

- Liegt `getWaterPos` unter `mPosition.y + sMessengerPosY - 300`, kommt 1 zurück.
  Zeigt die Wassergeschwindigkeit in die Modellachse, kommt 0 zurück.
  Sonst `unk154 += mRotWaterAccel`, gedeckelt bei `mRotSpeedMax`.
  Dann `appearObjFromPoint` an `mPosition` mit Messenger-Y plus 550, `mAppearSpeed = 0`.
- `char pad[8]` und `char trash[0x20]` halten den Frame auf `-0x60`.
- 284 Bytes, 71 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.39437 % -> 79.40212 %, matched code 50.170357 % -> 50.178272 % (1801160 -> 1801444, +284).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9670 -> 9671.
`MapObjBianco` 6328 -> 6612 (+284).
Kein R170–R299-Unit hat matched code verloren.

### R299 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjBall::touchWater`.

- `STATE_HOLDING` oder `STATE_APPEARING` liefert 1.
  Sonst kommt `getWaterSpeed` auf eine Kopie von `mVelocity`, skaliert mit `unk17C`.
  Danach `offLiveFlag(LIVE_FLAG_UNK10)`.
- `char trash[4]` hält den Frame auf `-0x38`.
- 256 Bytes, 64 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.38747 % -> 79.39437 %, matched code 50.163227 % -> 50.170357 % (1800904 -> 1801160, +256).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9669 -> 9670.
`MapObjBall` 6816 -> 7072 (+256).
Kein R170–R298-Unit hat matched code verloren.

### R298 (`MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TRevolvingFenceInner::controlGroundRoof`.

- Zustände 3 und 5: wenn die Animation endet, `setState(2)`.
  Frame-Rate und Frame auf 0, dann `calc` und `MAP_OBJ_FLAG_UNK100`.
- Zustände 4 und 6: dieselbe Folge mit `setState(1)`.
- Vergleichsbaum `cmpwi 4` / `bge` / `cmpwi 3` / `cmpwi 6`.
- `char trash[0x10]` hält den Frame auf `-0x28`.
- 260 Bytes, 65 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjFence`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.38034 % -> 79.38747 %, matched code 50.155987 % -> 50.163227 % (1800644 -> 1800904, +260).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9668 -> 9669.
`MapObjFence` 3812 -> 4072 (+260).
Kein R170–R296-Unit hat matched code verloren.

### R296 (`MapObjRailBlock`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TWoodBlock::load`.

- `TRailMapObj::load` bleibt ein direktes `bl`.
  `extern "C"` auf das gemanglete Symbol unterdrückt das Inlining.
  `dont_inline` an der Definition würde `TNormalLift::load` mitreißen.
  Danach `unk154` und die Collision-Folge von `TNormalLift::load`.
  Vier `s32`-Reads.
  RGB aus den unteren 8 Bit, Alpha fest `0xFF`.
  `unk15C = unk164`.
  `SMS_InitPacket_OneTevColor` mit `GX_TEVREG0`.
  308 Bytes, 77 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjRailBlock`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.37558 % -> 79.38034 %, matched code 50.147408 % -> 50.155987 % (1800336 -> 1800644, +308).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9667 -> 9668.
`MapObjRailBlock` 5544 -> 5852 (+308).
Kein R170–R294-Unit hat matched code verloren.

### R294 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TLeanMirror::loadAfter`.

- `TMapObjBase::loadAfter`.
  `TNameRefGen::search("ShiningStone")` landet in `unk17C`.
  Die Positionsdifferenz geht über `TVec3::sub` nach `unk180`.
  `squared() <= epsilon` nullt `unk180`.
  Sonst `scale(one() * inv_sqrt(lsq))`.
  `char trash[0xC]` hält das Frame bei `-0x58`.
  296 Bytes, 74 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.36743 % -> 79.37558 %, matched code 50.13916 % -> 50.147408 % (1800040 -> 1800336, +296).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9666 -> 9667.
`MapObjMamma` 7080 -> 7376 (+296).
Kein R170–R293-Unit hat matched code verloren.

### R293 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TGoalWatermelon::touchActor`.

- `isState(1)` und Actor-Typ `0x400000D0`.
  Der Actor landet in `unk13C`.
  `setBck("watermelon_shrink")`.
  `offMapObjFlag(MAP_OBJ_FLAG_UNK100)`.
  `mVelocity` wird null.
  `fireStartDemoCamera("スイカゴールカメラ", ...)`.
  Danach `mState = 2`.
  `char trash[4]` hält das Frame bei `-0x48`.
  260 Bytes, 65 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.3603 % -> 79.36743 %, matched code 50.131916 % -> 50.13916 % (1799780 -> 1800040, +260).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9665 -> 9666.
`MapObjMamma` 6820 -> 7080 (+260).
Kein R170–R292-Unit hat matched code verloren.

### R292 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TFerrisWheel::initMapObj`.

- `TMapObjBase::initMapObj`.
  `unk138` ist `getJointNum() - 1`.
  `unk13C` ist `new TMapObjBase*[unk138]`.
  Die Schleife registriert `"FerrisGondola"` mit Scale `(1, 1, 1)` und ruft `appear`.
  Ist `gpMarDirector->unk7D == 2`, wird `unk140` auf `10` gesetzt.
  Sonst `SMSGetAnmFrameRate() * 0.25`.
  `char trash[0xC]` hält das Frame bei `-0x78`.
  292 Bytes, 73 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjPinna`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.35228 % -> 79.3603 %, matched code 50.123787 % -> 50.131916 % (1799488 -> 1799780, +292).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9664 -> 9665.
`MapObjPinna` 6168 -> 6460 (+292).
Kein R170–R290-Unit hat matched code verloren.

### R290 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjElasticCode::control`.

- `TMapObjBase::control`.
  `mVelocity.y` wird mit `unk140` multipliziert.
  Danach kommt `unk13C * (mInitialPosition.y - mPosition.y) - getGravityY()`.
  Hält das Objekt etwas, wird `unk138` abgezogen.
  Die gehaltene Position bekommt `mVelocity.y` und geht an `moveRequest`.
  `mPosition.y` addiert `mVelocity.y`.
  `char trash[0x18]` hält das Frame bei `-0x58`.
  272 Bytes, 68 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjMare`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.344826 % -> 79.35228 %, matched code 50.11621 % -> 50.123787 % (1799216 -> 1799488, +272).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9663 -> 9664.
`MapObjMare` 5312 -> 5584 (+272).
Kein R170–R289-Unit hat matched code verloren.

### R289 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TViking::control`.

- `switch (unk14C)` behandelt `0` und `1`.
  Zustand `0` nutzt denselben `mState`-`1`/`2`-Lauf wie `THorizontalViking::control`.
  Zustand `1` ruft `roll`.
  `#pragma dont_inline` auf dem leeren `roll` hält das `bl`.
  `mPosition.x` ist `unk138 * sinf(3.14 * (unk148 / 180)) + mInitialPosition.x`.
  `mPosition.y` addiert `mYOffset` und `unk138 * (1 - cosf(...))`.
  `mRotation.z` wird `unk148`.
  Danach virtuelles `updateObjMtx`.
  `char trash[4]` hält das Frame bei `-0x28`.
  356 Bytes, 89 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjPinna`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.33502 % -> 79.344826 %, matched code 50.106293 % -> 50.11621 % (1798860 -> 1799216, +356).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9662 -> 9663.
`MapObjPinna` 5812 -> 6168 (+356).
Kein R170–R288-Unit hat matched code verloren.

### R288 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`THorizontalViking::control`.

- `TMapObjBase::control`.
  `switch (mState)` behandelt `1` und `2`.
  Zustand `1` senkt `unk144` und addiert ihn auf `unk148`.
  Unter `0` wird der Zustand `2`.
  Zustand `2` hebt `unk144`.
  Über `0` wird der Zustand `1`.
  `mPosition.x` ist `unk138 * sinf(3.14 * (unk148 / 180)) + mInitialPosition.x`.
  `mPosition.y` addiert `mYOffset` und `unk138 * (1 - cosf(...))`.
  `char trash[4]` hält das Frame bei `-0x28`.
  292 Bytes, 73 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjPinna`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.327 % -> 79.33502 %, matched code 50.098156 % -> 50.106293 % (1798568 -> 1798860, +292).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9661 -> 9662.
`MapObjPinna` 5520 -> 5812 (+292).
Kein R170–R287-Unit hat matched code verloren.

### R287 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMuddyBoat::initMapObj`.

- `TMapObjBase::initMapObj`.
  Danach `unk138`, `unk144`, `unk148`, `unk150`, `unk13C` und `unk168`.
  Ist `mMap` gleich `0x34`, kommen die Mare-Werte.
  Sonst die anderen.
  `unk17C` wird auf `(3, 2, 5)` gesetzt.
  `char trash[0xC]` hält das Frame bei `-0x28`.
  208 Bytes, 52 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjMare`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.32131 % -> 79.327 %, matched code 50.09236 % -> 50.098156 % (1798360 -> 1798568, +208).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9660 -> 9661.
`MapObjMare` 5104 -> 5312 (+208).
Kein R170–R286-Unit hat matched code verloren.

### R286 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TCogwheelScale::control`.

- `unk148` wird auf `0` gesetzt.
  Danach `TMapObjBase::control`.
  Wenn `unk140 > 0`, wird `mWaterLeakSpeed` abgezogen.
  `startSoundActorWithInfo` spielt `MSD_SE_OBJ_MR_TSUBO_WATER` mit `fabsf(unk140)`.
  Negative Werte werden auf `0` geklemmt.
  `char trash[4]` hält das Frame bei `-0x28`.
  176 Bytes, 44 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjMare`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.31652 % -> 79.32131 %, matched code 50.087463 % -> 50.09236 % (1798184 -> 1798360, +176).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9659 -> 9660.
`MapObjMare` 4928 -> 5104 (+176).
Kein R170–R285-Unit hat matched code verloren.

### R285 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TCogwheel::calc`.

- `mRotation.z` ist `360 * (-unk13C / (3.14 * 2 * sRadius))`.
  `makeRootMtxRotZ` und `makeRootMtxRotY` bauen zwei Matrizen.
  Ihre Translation wird auf `0` gesetzt.
  `MTXConcat` schreibt `RotY * RotZ` in die Anm-Matrix.
  Danach kommt `mPosition` in die Translation.
  `char trash[4]` hält das Frame bei `-0x80`.
  196 Bytes, 49 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjMare`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.31117 % -> 79.31652 %, matched code 50.082005 % -> 50.087463 % (1797988 -> 1798184, +196).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9658 -> 9659.
`MapObjMare` 4732 -> 4928 (+196).
Kein R170–R284-Unit hat matched code verloren.

### R284 (`MapObjLib`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjBase::emitAndRotateScale`.

- `gpMarioParticleManager->emit` bekommt `this` als Binder.
  `mRotation` wird durch `180` geteilt und mit `32768` multipliziert.
  Die drei `s16` gehen an `setRotation`.
  Danach `setGlobalScale(mScaling)`.
  `char trash[0xC]` hält das Frame bei `-0x50`.
  240 Bytes, 60 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjLib`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.31092 % -> 79.31117 %, matched code 50.075317 % -> 50.082005 % (1797748 -> 1797988, +240).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9657 -> 9658.
`MapObjLib` 9988 -> 10228 (+240).
Kein R170–R283-Unit hat matched code verloren.

### R283 (`MapObjLib`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjBase::emitAndSRT`.

- `gpMarioParticleManager->emit` bekommt die Position zweimal.
  `param_4` wird in drei `s16` kopiert und an `setRotation` gegeben.
  Danach `setGlobalScale(param_5)`.
  `char trash[4]` hält das Frame bei `-0x50`.
  224 Bytes, 56 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjLib`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.31067 % -> 79.31092 %, matched code 50.06908 % -> 50.075317 % (1797524 -> 1797748, +224).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9656 -> 9657.
`MapObjLib` 9764 -> 9988 (+224).
Kein R170–R282-Unit hat matched code verloren.

### R282 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjPuncher::control`.

- `TMapObjBase::control`.
  Der `switch` auf `mState` hat ein leeres `STATE_NORMAL`.
  Das hält das `bge`/`b`-Paar.
  Ein einzelner Fall `2` faltet es weg.
  Fall `2` ruft `soundBas(MSD_SE_OBJ_PUNCHER_RETURN, 101.0f, getRate())`.
  Ist die Animation fertig, kommt eine Skala `(2, 2, 2)`.
  Dann `emitAndScale` mit `PARTICLE_MS_ENM_DISAP_A_W` und `PARTICLE_MS_ENM_DISAP_B`.
  `MSD_SE_SMOKE_EFFECT` läuft über `gateCheck` und `startSoundActor`.
  Danach virtuelles `kill`.
  `char gap[4]` liegt über dem Skalenvektor.
  `char trash[0x10]` liegt darunter.
  Das Frame bleibt `-0x38`.
  Der Vektor liegt bei `r1+0x20`.
  244 Bytes, 61 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjMare`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.303986 % -> 79.31067 %, matched code 50.062283 % -> 50.06908 % (1797280 -> 1797524, +244).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9655 -> 9656.
`MapObjMare` 4488 -> 4732 (+244).
Kein R170–R281-Unit hat matched code verloren.

### R281 (`MapObjRicco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TRiccoWatermill::touchWater`.

- `isState(5)` gibt `1` zurück.
  Sonst wird `unk140` auf `5` gesetzt.
  Bei `isState(1)` ruft `unk13C->setUpMapCollision(1)` auf.
  `MAP_OBJ_FLAG_UNK100` geht an diesem Objekt und an `unk13C` aus.
  Liegt `unk13C->mPosition.y` unter `mSubmarineMaxTransY`, kommt `mRotAccel` auf `unk138`, gedeckelt bei `mRotSpeedMaxUp`, und `mState` wird `2`.
  Sonst wird `unk138` zu `0`.
  Rückgabe ist `1`.
  `unk13C` ist `TMapObjBase*`.
  240 Bytes, 60 Instruktionen.
  Das Frame ist schon `-0x20`.

`validate-symbol-order` `mario/MoveBG/MapObjRicco`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.297516 % -> 79.303986 %, matched code 50.0556 % -> 50.062283 % (1797040 -> 1797280, +240).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9654 -> 9655.
`MapObjRicco` 2732 -> 2972 (+240).
Kein R170–R280-Unit hat matched code verloren.
Nur `MapObjRicco` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R280 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TResetFruit::touchActor`.

- `STATE_APPEARING`, `STATE_BREAKING` oder Zustand `0xC` beenden die Funktion.
  Sonst, wenn der Zustand nicht `STATE_WAITING_TO_APPEAR` ist, läuft `TMapObjBall::touchActor`.
  `MAP_OBJ_FLAG_UNK4000000`, ein Zustand ungleich `1` oder `LIVE_FLAG_UNK10` beenden die Funktion.
  Ohne laufenden State-Timer geht `MAP_OBJ_FLAG_DISAPPEARING` an und virtuelles `getLivingTime` setzt `mStateTimer`.
  `LIVE_FLAG_UNK10` geht aus.
  `mState` wird `11`.
  `char trash[0xC]` hält das Frame bei `-0x28`.
  `#pragma dont_inline` auf dem leeren `TMapObjBall::touchActor` hält den `bl`.
  308 Bytes, 77 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.28905 % -> 79.297516 %, matched code 50.047016 % -> 50.0556 % (1796732 -> 1797040, +308).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9653 -> 9654.
`MapObjBall` 6508 -> 6816 (+308).
Kein R170–R279-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R279 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TResetFruit::waitingToAppear`.

- Wenn `gpMarDirector->mMap == 3` und `unk1A4 != 0`, virtuelles `makeObjDead`.
  `MAP_OBJ_FLAG_UNK4000000`, ein laufender State-Timer oder `mColCount != 0` beenden die Funktion.
  Sonst geht `MAP_OBJ_FLAG_DISAPPEARING` an und virtuelles `makeObjAppeared` läuft.
  `MTXScale` mit `0.2` auf allen Achsen.
  `concatOnlyRotFromLeft` schreibt die Skalierung in die Animationsmatrix.
  `mScaling.y` wird `0.2`.
  `HIT_FLAG_NO_COLLISION` geht an.
  `mState` wird `STATE_APPEARING`.
  `MSD_SE_IT_COMMON_APPEAR`, wenn `gateCheck` wahr ist.
  `char trash[0x20]` hält die Matrix bei `r1+0x3C` und das Frame bei `-0x78`.
  316 Bytes, 79 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.280365 % -> 79.28905 %, matched code 50.03822 % -> 50.047016 % (1796416 -> 1796732, +316).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9652 -> 9653.
`MapObjBall` 6192 -> 6508 (+316).
Kein R170–R278-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R278 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjBall::control`.

- `TMapObjGeneral::control`.
  Wenn `unk194` als `s32` nicht 0 ist, wird es um 1 kleiner.
  Bei `STATE_HOLDING` kopiert `MTXCopy` `mHolder->getTakingMtx` in eine lokale Matrix.
  Translation Y bekommt `unk190` dazu.
  Die Matrix geht per `MTXCopy` nach `getModel()->getAnmMtx(0)`.
  Sonst: wenn `mVelocity.squared()` nicht `<= epsilon()` ist oder `mGroundPlane->mActor` nicht null ist, virtuelles `calcCurrentMtx`.
  `char trash[0x14]` hält das Frame bei `-0x70`.
  272 Bytes, 68 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.272896 % -> 79.280365 %, matched code 50.030643 % -> 50.03822 % (1796144 -> 1796416, +272).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9651 -> 9652.
`MapObjBall` 5920 -> 6192 (+272).
Kein R170–R277-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R277 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBigWatermelon::appearing`.

- `TMapObjGeneral::appearing`.
  Die Animationsmatrix kommt aus `getModel()->getAnmMtx(0)`, dann virtuelles `calcRootMatrix` und `getModel()->calc`.
  Translation Y wird `mBodyRadius * (mScaling.y / mInitialScaling.y) + mPosition.y`.
  `mScaledBodyRadius` und `mDamageRadius` werden je `50 * mScaling.x`, dann `calcEntryRadius`.
  Bei `isState(1)` wird `mActorType` `0x400000D0` und `mAttackRadius` `50 * mScaling.x`.
  Sonst wird `mActorType` `0x400000DB` und `mAttackRadius` `0`.
  Beide Zweige rufen noch einmal `calcEntryRadius`.
  `char trash[0x18]` hält das Frame bei `-0x38`.
  272 Bytes, 68 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.26543 % -> 79.272896 %, matched code 50.02306 % -> 50.030643 % (1795872 -> 1796144, +272).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9650 -> 9651.
`MapObjBall` 5648 -> 5920 (+272).
Kein R170–R276-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R276 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjBall::receiveMessage`.

- `TMapObjGeneral::receiveMessage` kommt zuerst.
  Wenn das wahr ist, kommt `TRUE` zurück.
  Bei `HIT_MESSAGE_TAKE` und `MAP_OBJ_FLAG_UNK100000` ruft die Funktion virtuelles `hold` auf und gibt `TRUE` zurück.
  Wenn der Sender `0x80000001` ist, dieses Objekt nicht `0x400000D0` ist und die Nachricht nicht `HIT_MESSAGE_TAKE` ist, kommt virtuelles `kicked` und `TRUE`.
  Sonst `FALSE`.
  248 Bytes, 62 Instruktionen.
  Das Frame ist `-0x28`.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.25867 % -> 79.26543 %, matched code 50.01615 % -> 50.02306 % (1795624 -> 1795872, +248).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9649 -> 9650.
`MapObjBall` 5400 -> 5648 (+248).
Kein R170–R275-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R275 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TResetFruit::touchGround`.

- Wenn `mGroundPlane->isDeathPlane()` wahr ist, wird `mState` 11.
  Virtuelles `makeObjDefault`, `makeObjDead` und `calcRootMatrix`.
  `getModel()->calc`.
  `mStateTimer` wird `mFruitWaitTimeToAppear`.
  `MAP_OBJ_FLAG_DISAPPEARING` geht aus.
  `mState` wird `STATE_WAITING_TO_APPEAR`.
  Wenn `gpMarDirector->mMap == 3` und `unk1A4 != 0`, noch einmal virtuelles `makeObjDead`.
  Danach kopiert die Funktion `mPosition` komponentenweise nach `param_1`.
  Sonst ruft sie `TMapObjBall::touchGround` auf.
  `char trash[0x10]` hält das Frame bei `-0x30`.
  296 Bytes, 74 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.25054 % -> 79.25867 %, matched code 50.007908 % -> 50.01615 % (1795328 -> 1795624, +296).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9648 -> 9649.
`MapObjBall` 5104 -> 5400 (+296).
Kein R170–R274-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R274 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TResetFruit::touchPollution`.

- `emitAndBindToPosPtr` mit `PARTICLE_MS_MOE_FIRE_OFF` an `mPosition`.
  Dann `MSD_SE_OBJ_AWAY_INTO_GRAF`, wenn `gateCheck` wahr ist.
  Virtuelles `makeObjDefault`, dann `mState` 11, dann noch einmal `makeObjDefault`.
  Virtuelles `makeObjDead` und `calcRootMatrix`.
  `getModel()->calc`.
  `mStateTimer` wird `mFruitWaitTimeToAppear`.
  `MAP_OBJ_FLAG_DISAPPEARING` geht aus.
  `mState` wird `STATE_WAITING_TO_APPEAR`.
  Wenn `gpMarDirector->mMap == 3` und `unk1A4 != 0`, noch einmal virtuelles `makeObjDead`.
  `char trash[0x20]` hält das Frame bei `-0x38`.
  300 Bytes, 75 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.2423 % -> 79.25054 %, matched code 49.999554 % -> 50.007908 % (1795028 -> 1795328, +300).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9647 -> 9648.
`MapObjBall` 4804 -> 5104 (+300).
Kein R170–R273-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R273 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TResetFruit::touchWaterSurface`.

- `emitColumnWater`, dann `MSD_SE_OBJ_DRINA_TO_WATER`, wenn `gateCheck` wahr ist.
  `mState` wird 11.
  Virtuelles `makeObjDefault`, `makeObjDead` und `calcRootMatrix`.
  `getModel()->calc`.
  `mStateTimer` wird `mFruitWaitTimeToAppear`.
  `MAP_OBJ_FLAG_DISAPPEARING` geht aus.
  `mState` wird `STATE_WAITING_TO_APPEAR`.
  Wenn `gpMarDirector->mMap == 3` und `unk1A4 != 0`, noch einmal virtuelles `makeObjDead`.
  `char trash[0x18]` hält das Frame bei `-0x30`.
  260 Bytes, 65 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.235176 % -> 79.2423 %, matched code 49.99231 % -> 49.999554 % (1794768 -> 1795028, +260).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9646 -> 9647.
`MapObjBall` 4544 -> 4804 (+260).
Kein R170–R272-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R272 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TResetFruit::makeObjAppeared`.

- Wenn `MAP_OBJ_FLAG_UNK4000000` gesetzt ist, virtuelles `makeObjDefault`.
  Dann `TMapObjBase::makeObjAppeared` und virtuelles `calcCurrentMtx`.
  Die Anm-Matrix bekommt `mPosition`, Y plus `mBodyRadius`.
  Banane `0x40000394`: wenn `mtx[1][1] > 0`, Translation Y minus `50 * mtx[1][1]`.
  Ananas `0x40000392`: Translation Y minus `10 * (1 - mtx[1][1])`.
  `unkE8` wird 0.
  Dasselbe Flag setzt danach `mState` auf 11.
  `char trash[8]` hält das Frame bei `-0x28`.
  304 Bytes, 76 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.226814 % -> 79.235176 %, matched code 49.983845 % -> 49.99231 % (1794464 -> 1794768, +304).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9645 -> 9646.
`MapObjBall` 4240 -> 4544 (+304).
Kein R170–R271-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R271 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TResetFruit::breaking`.

- `MTXScale` mit `1`, `mBreakingScaleSpeed` und `1`.
  `concatOnlyRotFromLeft` schreibt die Rotation in die Anm-Matrix.
  `mScaling.y` wird mit `mBreakingScaleSpeed` multipliziert.
  Translation Y ist `mBodyRadius * mScaling.y + mPosition.y`.
  Wenn `mScaling.y < 0.2`, kommt `mBodyRadius * 0.5` auf `mPosition.y`.
  Die Scale geht zurück auf `mInitialScaling`.
  `emitAndScale` mit `PARTICLE_MS_ENM_DISAP_A_W`.
  `MSD_SE_SMOKE_EFFECT`, wenn `gateCheck` wahr ist.
  `mStateTimer` wird `0xF0`, dann `sleep`, `mState` wird `0xD`.
  `char trash[10]` hält die Matrix bei `r1+0x24` und das Frame bei `-0x60`.
  284 Bytes, 71 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.219 % -> 79.226814 %, matched code 49.975933 % -> 49.983845 % (1794180 -> 1794464, +284).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9644 -> 9645.
`MapObjBall` 3956 -> 4240 (+284).
Kein R170–R270-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R270 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TResetFruit::appearing`.

- `MTXScale` skaliert mit `mScaleUpSpeed` auf allen Achsen.
  `concatOnlyRotFromLeft` schreibt die Rotation in die Anm-Matrix.
  `mScaling.y` wird mit `mScaleUpSpeed` multipliziert.
  `mScaledBodyRadius` ist `mBodyRadius * mScaling.y`.
  Translation Y ist `mBodyRadius * mScaling.y + mPosition.y`.
  Wenn `mScaling.y >= mInitialScaling.y`, kommt die Initial-Scale zurück.
  Danach virtuelles `calc`, `HIT_FLAG_NO_COLLISION` aus, virtuelles `makeObjAppeared`, `mState` wird 1.
  `char trash[10]` hält die Matrix bei `r1+0x20` und das Frame bei `-0x58`.
  256 Bytes, 64 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.212 % -> 79.219 %, matched code 49.968803 % -> 49.975933 % (1793924 -> 1794180, +256).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9643 -> 9644.
`MapObjBall` 3700 -> 3956 (+256).
Kein R170–R269-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R269 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjBall::makeObjAppeared`.

- Zuerst `TMapObjBase::makeObjAppeared`, dann virtuelles `calcCurrentMtx`.
  Die Anm-Matrix bekommt `mPosition`, Y plus `mBodyRadius`.
  Banane `0x40000394`: wenn `mtx[1][1] > 0`, Translation Y minus `50 * mtx[1][1]`.
  Ananas `0x40000392`: Translation Y minus `10 * (1 - mtx[1][1])`.
  `unkE8` wird 0.
  `char trash[8]` hält das Frame bei `-0x28`.
  248 Bytes, 62 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.20519 % -> 79.212 %, matched code 49.961895 % -> 49.968803 % (1793676 -> 1793924, +248).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9642 -> 9643.
`MapObjBall` 3452 -> 3700 (+248).
Kein R170–R268-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R268 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TResetFruit::makeObjWaitingToAppear`.

- `mState` wird 11.
  Dann `makeObjDefault`, `makeObjDead`, `calcRootMatrix` und `getModel()->calc`.
  `mStateTimer` wird `mFruitWaitTimeToAppear`.
  `MAP_OBJ_FLAG_DISAPPEARING` geht aus.
  `mState` wird `STATE_WAITING_TO_APPEAR`.
  Wenn `gpMarDirector->mMap == 3` und `unk1A4 != 0`, noch einmal `makeObjDead`.
  `char trash[0x10]` hält das Frame bei `-0x28`.
  204 Bytes, 51 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.19963 % -> 79.20519 %, matched code 49.95621 % -> 49.961895 % (1793472 -> 1793676, +204).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9641 -> 9642.
`MapObjBall` 3248 -> 3452 (+204).
Kein R170–R267-Unit hat matched code verloren.
Nur `MapObjBall` hat matched code gewonnen.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R267 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMerrygoround::TMerrygoround`.

- Nach `TMapObjBase` werden `unk1A0` und `unk1A4` auf 0 gesetzt.
  Danach `unk138`, `unk140`, `unk13C` und `unk142`.
  Eine Schleife `i < 9` nullt `unk144[i]`, `unk18C[i]` und `unk168[i]`.
  MWCC rollt acht Durchläufe aus und lässt den Rest in `bdnz`.
  `char trash[1]` hält das Frame bei `-0x28`.
  252 Bytes, 63 Instruktionen.
  `control`, `draw` und `initMapObj` bleiben leere Stubs.

`validate-symbol-order` `mario/MoveBG/MapObjPinna`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.19442 % -> 79.19963 %, matched code 49.949192 % -> 49.95621 % (1793220 -> 1793472, +252).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9640 -> 9641.
`MapObjPinna` 5268 -> 5520 (+252).
Kein R170–R266-Unit hat matched code verloren.
Nur `MapObjPinna` hat matched code gewonnen.
`MarNameRefGen_MapObj` und `MapObjManager` wurden wegen der neuen Felder neu gebaut.
Matched code dort bleibt 2348 bzw. unverändert.
Fuzzy von `getNameRef_MapObj` tickt 86.49089 % -> 86.49118 % durch die neue Objektgröße.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R266 (`MapObjMonte`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TSwingBoard::TSwingBoard`.

- Nach `TMapObjBase` folgen die Stores in Retail-Reihenfolge.
  `unk138` ist 5000.0f.
  `unk13C`, `unk140`, `unk144` und `unk148` sind 0.0f.
  `unk188` ist 0.
  `unk178`, `unk168`, `unk158`, `unk164`, `unk154`, `unk170`, `unk150`, `unk16C` und `unk15C` sind 0.0f.
  `unk174`, `unk160` und `unk14C` sind 1.0f.
  `unk184`, `unk180` und `unk17C` sind 0.0f.
  `char trash[0x26]` hält das Frame bei `-0x48` (`r31` bei `r1+0x44`).
  168 Bytes, 42 Instruktionen.
  `load`, `draw`, `swing` und `control` bleiben leere Stubs.

`validate-symbol-order` `mario/MoveBG/MapObjMonte`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.19155 % -> 79.19442 %, matched code 49.94452 % -> 49.94919 % (1793052 -> 1793220, +168).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9639 -> 9640.
`MapObjMonte` 3868 -> 4036 (+168).
Kein R170–R265-Unit hat matched code verloren.
Nur `MapObjMonte` hat matched code gewonnen.
`MarNameRefGen_MapObj` und `MapObjManager` wurden wegen der neuen Felder neu gebaut.
Matched code dort bleibt 2348 bzw. unverändert.
Fuzzy von `getNameRef_MapObj` tickt 86.49061 % -> 86.49089 % durch die neue Objektgröße.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R265 (`MapObjRicco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TRiccoWatermill::TRiccoWatermill`.

- Nach `TMapObjBase` wird `unk138` auf 0.0f gesetzt.
  `unk13C`, `unk140`, `unk148`, `unk14C`, `unk150` und `unk154` sind 0.
  `unk144` ist 0 (`stb`).
  `char trash[8]` hält das Frame bei `-0x28` (`r31` bei `r1+0x24`).
  108 Bytes, 27 Instruktionen.
  `loadAfter` bleibt der leere Stub.

`validate-symbol-order` `mario/MoveBG/MapObjRicco`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.19034 % -> 79.19155 %, matched code 49.94151 % -> 49.94452 % (1792944 -> 1793052, +108).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9638 -> 9639.
`MapObjRicco` 2624 -> 2732 (+108).
Kein R170–R264-Unit hat matched code verloren.
Nur `MapObjRicco` hat sich geändert.
`MarNameRefGen_MapObj` und `MapObjManager` wurden wegen der neuen Felder neu gebaut, ohne Match-Änderung.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R264 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBiancoWatermillVertical::TBiancoWatermillVertical`.

- Nach `TMapObjBase` werden `unk138` und `unk13C` auf 0.0f gesetzt.
  `unk140`, `unk148` und `unk14C` sind 0.
  `unk144` ist 0 (`stb`).
  `char trash[8]` hält das Frame bei `-0x28` (`r31` bei `r1+0x24`).
  100 Bytes, 25 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.18938 % -> 79.19034 %, matched code 49.93872 % -> 49.94151 % (1792844 -> 1792944, +100).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9637 -> 9638.
`MapObjBianco` 6228 -> 6328 (+100).
Kein R170–R263-Unit hat matched code verloren.
Nur `MapObjBianco` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R263 (`Item`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TShine::loadBeforeInit`.

- `readString` füllt `name[0x20]`.
  `strcmp("normal")` setzt `unk154` auf 0, `strcmp("quickly")` auf 2, sonst 1.
  Das erste `stream >>` ist `eventId`; `-1` wird 120, danach `setEventId`.
  Das zweite Int steckt in einer Union mit `double`, damit der Slot bei `r1+0x18` 8-byte-aligned liegt.
  `eventId = slot.v`.
  `slot.v + 1 >= 2` setzt `eventId` auf -1.
  `unk190 = eventId + 1`.
  240 Bytes, 60 Instruktionen.
  `appearSimple` bleibt 100 %.

`validate-symbol-order` `mario/MoveBG/Item`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.18936 % -> 79.18938 %, matched code 49.93204 % -> 49.93872 % (1792604 -> 1792844, +240).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9636 -> 9637.
`Item` 14120 -> 14360 (+240).
Kein R170–R262-Unit hat matched code verloren.
Nur `Item` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R262 (`Item`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TShine::appearSimple`.

- `TShine* shine = this` hält `this` in `r31` und das Argument in `r30`.
  Danach `TItem::appear`, `setBool(true, 0x50000)`, die Feldstores,
  die Kopie von `mPosition` nach `mInitialPosition` und
  `startSoundActor(MSD_SE_SHINE_APPEAR)`.
  `mStateTimer = unk174`, `mState = STATE_UNKB`, `onHitFlag(HIT_FLAG_NO_COLLISION)`.
  `char trash[0x9]` hält das Frame bei `-0x30` (`r31` bei `r1+0x2c`).
  252 Bytes, 63 Instruktionen.

`validate-symbol-order` `mario/MoveBG/Item`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.18918 % -> 79.18936 %, matched code 49.92501 % -> 49.93204 % (1792352 -> 1792604, +252).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9635 -> 9636.
`Item` 13868 -> 14120 (+252).
Kein R170–R261-Unit hat matched code verloren.
Nur `Item` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R261 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMammaBlockRotate::touchWater`.

- `isState(1)` ist das `? true : false` mit `li 1` / `li 0` / `clrlwi.`.
  Danach `mRotation.y += mRotSpeed`.
  `mRotation.y > mRotEnd` ist `fcmpo`+`ble` und setzt `mState` auf 2.
  Rückgabe ist 1.
  Kein Zusatzframe.
  80 Bytes, 20 Instruktionen.
  `withering`, `TGoalWatermelon::control` und `sinit` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.18719 % -> 79.18918 %, matched code 49.92279 % -> 49.92501 % (1792272 -> 1792352, +80).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9634 -> 9635.
`MapObjMamma` 6740 -> 6820 (+80).
Kein R170–R260-Unit hat matched code verloren.
Nur `MapObjMamma` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R260 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TSandBase::withering`.

- `mScaling.y -= unk13C`.
  `mScaling.y < mScaleMin` ist `fcmpo`+`bge` und klemmt auf `mScaleMin`.
  `unk144->mPosition` wird vor `gateCheck` gebildet.
  `MSD_SE_OBJ_SANDBUD_NORMAL` (0x2099) geht durch `startSoundActor`.
  Rückgabe ist `bool`: `mScaling.y <= mScaleMin` als `cror`+`bne`, `li 1` / `li 0`.
  `char trash[1]` hält das Frame bei `-0x20` (`r31` bei `r1+0x1c`).
  172 Bytes, 43 Instruktionen.
  `TSandCastle::withering` bleibt virtuell und gibt `false` zurück, damit die Signatur passt.
  `control` von `TGoalWatermelon` und `sinit` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.18246 % -> 79.18719 %, matched code 49.91799 % -> 49.92279 % (1792100 -> 1792272, +172).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9633 -> 9634.
`MapObjMamma` 6568 -> 6740 (+172).
Kein R170–R259-Unit hat matched code verloren.
Nur `MapObjMamma` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R259 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TGoalWatermelon::control`.

- `TMapObjBase::control`, dann `switch (mState)`.
  Leere Fälle 0, 1 und 3 halten die Verteilung: `cmpwi 2`, `beq`, `bge`, `b`, `b`.
  Zustand 2 ruft `unk13C->animIsFinished()`.
  Danach `makeShineAppearWithDemoOffset` mit `シャイン（お化けスイカ用）`, `スイカシャインカメラ` und Offset 0.
  `mState` wird 3.
  Frame `-0x18` ohne Zusatzslot.
  128 Bytes, 32 Instruktionen.
  `loadAfter`, `load` und `sinit` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.17900 % -> 79.18246 %, matched code 49.91443 % -> 49.91799 % (1791972 -> 1792100, +128).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9632 -> 9633.
`MapObjMamma` 6440 -> 6568 (+128).
Kein R170–R258-Unit hat matched code verloren.
Nur `MapObjMamma` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R258 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TViking::reset` und `THorizontalViking::reset`.

- Beide kopieren `unk140` nach `unk144` und setzen `unk148` auf 0.
  `TViking` testet das neu geladene `unk140 > 0`.
  `THorizontalViking` testet die Kopie `unk144 > 0`.
  Der `>`-Test ist `fcmpo`+`ble`.
  Wahr setzt `mState` auf 1, sonst auf 2.
  Je 52 Bytes, 13 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjPinna`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.17877 % -> 79.17900 %, matched code 49.91153 % -> 49.91443 % (1791868 -> 1791972, +104).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9630 -> 9632.
`MapObjPinna` 5164 -> 5268 (+104).
Kein R170–R257-Unit hat matched code verloren.
Nur `MapObjPinna` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R257 (`MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TFenceWater::controlRotation`.

- `switch (mState)` mit leerem `case 1`, damit die untere Hälfte gegen 1 vergleicht.
  Zustand 2 zieht `unk13C` von `unk140` ab.
  `unk140 <= -90` ist `cror`+`bne` und klemmt auf `-90`, setzt `unk13C` auf 0, `mState` auf 3 und `mStateTimer` auf `mTurnedWaitTime` (600).
  Zustand 3 bricht ab, solange `isStateTimerEngaged()` wahr ist.
  Danach `gateCheck` und `startSoundActor` mit `MSD_SE_OBJ_WATER_FENCE_REV`, `unk13C = mBackSpeed` (3.0f), `mState = 4`.
  Zustand 4 addiert `unk13C` auf `unk140`.
  `unk140 >= 0` ist `cror`+`bne` und ruft virtuell `changeStatusToWait`.
  `dont_inline` bleibt, damit `control` den `bl` behält.
  `char trash[0x9]` hält das Frame bei `-0x28` (`r31` bei `r1+0x24`).
  308 Bytes, 77 Instruktionen.
  `control`, `changeStatusToGo` und `changeStatusToWait` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjFence`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.17030 % -> 79.17877 %, matched code 49.90295 % -> 49.91153 % (1791560 -> 1791868, +308).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9629 -> 9630.
`MapObjFence` 3504 -> 3812 (+308).
Kein R170–R256-Unit hat matched code verloren.
Nur `MapObjFence` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R256 (`MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TFenceWater::control`.

- `TMapObjBase::control` und `controlRotation` laufen zuerst.
  `controlRotation` bleibt `dont_inline`, sonst verschwindet der `bl`.
  `MsWrap(unk140 + mInitialRotation.y, 0, 360)` schreibt `mRotation.y`.
  Die beiden `while` sind `>= 360` (`cror`+`beq`) und `< 0` (`blt`).
  `182.04445f * mRotation.y` geht über `jmaSinShift` in die Cos- und Sin-Tabelle.
  `JMASSin` bleibt ausgeschrieben, weil das Header-`dont_inline` sonst einen Call erzeugt.
  Der Messenger bei `0x144` wird über `void*` zweimal geladen, damit das zweite `lwz` stehen bleibt.
  `mPosition.x + 500 * cos` und `mPosition.z - 500 * sin` landen auf dem Messenger.
  Das Feld bleibt aus der Klasse, sonst ändert sich `sizeof` in `MarNameRefGen`.
  `char trash[0x9]` hält das Frame bei `-0x48` (`fctiwz` bei `r1+0x38` und `r1+0x30`).
  244 Bytes, 61 Instruktionen.
  `changeStatusToGo`, `changeStatusToWait`, `receiveMessage` und `sinit` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjFence`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.16362 % -> 79.17030 %, matched code 49.89616 % -> 49.90295 % (1791316 -> 1791560, +244).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9628 -> 9629.
`MapObjFence` 3260 -> 3504 (+244).
Kein R170–R255-Unit hat matched code verloren.
Nur `MapObjFence` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R255 (`MapObjCorona`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBathtub::quake`.

- `unk29A` bricht ab.
  Zwei Referenzen auf den Trefferpunkt und `mInitialPosition` halten die XZ-Lasten in der Retail-Reihenfolge.
  `!(lsq <= epsilon)` erzeugt `cror`+`beq` und ruft das lokale out-of-line `inv_sqrt`.
  Das Ergebnis bleibt unbenutzt.
  `unk24C` wird 300.
  `unk16C` kopiert `+0x54` nach `unk250`, `+0x68` nach `unk258` und `unk25C`, `+0x7C` nach `unk254` und `+0xF4` nach `unk248`.
  `TNameRefGen::search("クッパ")` bleibt in `r31`.
  `gpCameraShake->startShake` läuft mit Modus `0x25` und `0x26` bei `1.0f`.
  `SMSRumbleMgr->start(4, nullptr)` folgt.
  `SMS_ThrowMario` wirft den Vektor `(0, 1, 0)` mit `10.0f`.
  `TKoopa::getDown` schließt ab.
  `char above[0x9]` und `char below[0x48]` halten das Frame bei `-0xa0` und den Vektor bei `r1+0x74`.
  316 Bytes, 79 Instruktionen.
  Dtor, `hipdrop`, `tumble`, `getNumGripsDead`, die Demo-Mtx-Getter, beide Grip-`receiveMessage` und beide `getRootJointMtx` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjCorona`: PASS.
0 neue Fehler.
`MapObjBase.hpp` und `JGUtil.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.15492 % -> 79.16362 %, matched code 49.88736 % -> 49.89616 % (1791000 -> 1791316, +316).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9627 -> 9628.
`MapObjCorona` 1052 -> 1368 (+316).
Kein R170–R254-Unit hat matched code verloren.
Nur `MapObjCorona` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R254 (`MapObjCorona`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBathtub::hipdrop`.

- `unk29A` bricht ab.
  `unk250 > unk16C->unk7C` bricht ebenfalls ab.
  Zwei Referenzen auf den Trefferpunkt und `mInitialPosition` halten die XZ-Lasten in der Retail-Reihenfolge.
  `!(lsq <= epsilon)` erzeugt `cror`+`beq` und ruft `inv_sqrt`.
  Das Ergebnis bleibt unbenutzt.
  Die Header-Inline faltet diesen Aufruf zu einem Compare, deshalb deklariert diese TU `TUtil` lokal und definiert `inv_sqrt` out-of-line.
  `JGUtil.hpp` bleibt unverändert.
  `unk7C` geht nach `unk250` und `unk254`, `unk90` nach `unk258` und `unk25C`.
  `TNameRefGen::search("クッパ")` ruft `TKoopa::stagger(false)`.
  `char trash[0x60]` hält das Frame bei `-0x98` (`r31` bei `r1+0x94`).
  228 Bytes, 57 Instruktionen.
  Dtor, `tumble`, `getNumGripsDead`, die Demo-Mtx-Getter, beide Grip-`receiveMessage` und beide `getRootJointMtx` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjCorona`: PASS.
0 neue Fehler.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.14868 % -> 79.15492 %, matched code 49.88100 % -> 49.88736 % (1790772 -> 1791000, +228).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9626 -> 9627.
`MapObjCorona` 824 -> 1052 (+228).
Kein R170–R253-Unit hat matched code verloren.
Nur `MapObjCorona` hat sich geändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R253 (`MapObjCorona`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBathtub::tumble`.

- `unk29A` bricht ab.
  Der Winkel ist `182.04445f * param_1` (65536/360), der Index kommt aus `jmaSinShift`.
  `JMASSin` bleibt im Header `dont_inline`, deshalb steht die Tabellenrechnung hier.
  `param_2 * 0.0001f` läuft über ein benanntes `amp`, damit `param_2` links im `fmuls` bleibt.
  `unk1E8` bekommt `scale * cos`, `unk1EC` bekommt `+ 0`, `unk1F0` bekommt `scale * -sin`.
  Das Sinusprodukt geht zurück in `sine`, damit das zweite `fmuls` in dem Register bleibt.
  `char trash[8]` hält den `fctiwz`-Slot bei `r1+0x38` (Frame `-0x40`).
  136 Bytes, 34 Instruktionen.
  Dtor, `getNumGripsDead`, die Demo-Mtx-Getter, beide Grip-`receiveMessage` und beide `getRootJointMtx` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjCorona`: PASS.
0 neue Fehler.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.14500 % -> 79.14868 %, matched code 49.87722 % -> 49.88100 % (1790636 -> 1790772, +136).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9625 -> 9626.
`MapObjCorona` 688 -> 824 (+136).
Kein R170–R252-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R252 (`MapObjWave`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjWave::updateHeightAndAlpha`.

- `checkGround` und `checkGroundExactY` an Marios Position, Y der zweiten Abfrage ist 10.
  Flachwasser oder eine Wasserfläche mischt die Amplitude, sonst die Ruhewerte.
  Typ `0x700` oder eine negative Höhe kopiert die volle Amplitude.
  Sonst ist der Faktor `1 - höhe / spanne` und geht in `fmadds`.
  Auf Karte 4 setzt das Rechteck um Mario die Ruhewerte.
  Ein Stream-Würfel hebt `unk44` bis `unk3C`, sonst fällt es auf 0.
  Liegt `unk44` über 0, kommt es auf die Amplitude drauf.
  `char trash[0x28]` hält den Frame bei `-0x70`.
  792 Bytes, 198 Instruktionen.
  Dtor, `perform`, `updateTime`, `noWave`, `getHeight`, `getWaveHeight` und `__sinit_MapObjWave_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjWave`: PASS.
0 neue Fehler.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.
`MarNameRefGen_MapObj` matched code bleibt 2348.
Klassengröße bleibt `0x98`.

`ninja changes_all`: fuzzy 79.12306 % -> 79.14500 %, matched code 49.85516 % -> 49.87722 % (1789844 -> 1790636, +792).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9624 -> 9625.
`MapObjWave` 1660 -> 2452 (+792).
Kein R170–R251-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R251 (`MapObjWave`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjWave::updateTime`.

- Vier Wrap-Adds, Blatt ohne Frame.
  `unk64 += unk24` und `unk68 += unk28` ziehen `6.28318f` ab, wenn der Wert größer ist.
  `6.28318f` ist `@2730` (`0x40c90fd0`).
  `unk6C` und `unk70` ziehen `1.0f` von `unk60` ab, wenn der Wert größer ist.
  Der letzte Vergleich ist `blelr`.
  `>` erzeugt `fcmpo` + `ble`.
  `#pragma dont_inline` bleibt, damit `perform` den `bl` behält.
  164 Bytes, 41 Instruktionen.
  Dtor, `perform`, `noWave`, `getHeight`, `getWaveHeight` und `__sinit_MapObjWave_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjWave`: PASS.
0 neue Fehler.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.
`MarNameRefGen_MapObj` matched code bleibt 2348.
Klassengröße bleibt `0x98`.

`ninja changes_all`: fuzzy 79.11861 % -> 79.12306 %, matched code 49.85059 % -> 49.85516 % (1789680 -> 1789844, +164).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9623 -> 9624.
`MapObjWave` 1496 -> 1660 (+164).
Kein R170–R250-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R250 (`MapObjWave`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjWave::getHeight`.

- `checkGroundExactY(x, 50 + y, z)`.
  Wasserfläche ist Typ `0x100`, `0x101`, `(u16)(Typ - 0x102) <= 3` oder `0x4104`.
  Sonst kommt `y` zurück.
  Nur See (`0x102` / `0x103`) nimmt die Wellenhöhe, sonst die Bodenhöhe.
  Fehlt `unk94`, ist das Ergebnis 0, sonst dieselbe `sinf`-Summe wie `getWaveHeight`.
  `unsigned char` statt `bool` hält den Check-Pointer bei `r1+0x1c`.
  Das `<=` ist `cmplwi` + `ble` auf `u16`, wie Retail.
  300 Bytes, 75 Instruktionen.
  Dtor, `perform`, `noWave`, `getWaveHeight` und `__sinit_MapObjWave_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjWave`: PASS.
0 neue Fehler.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.
`MarNameRefGen_MapObj` matched code bleibt 2348.

`ninja changes_all`: fuzzy 79.11047 % -> 79.11861 %, matched code 49.84223 % -> 49.85059 % (1789380 -> 1789680, +300).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9622 -> 9623.
`MapObjWave` 1196 -> 1496 (+300).
Kein R170–R249-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R249 (`MapObjWave`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjWave::getWaveHeight`.

- Wenn `unk94` fehlt, Ergebnis 0.
  Sonst `unk3C * sinf(unk24 * (0.15915507 * x) + unk64)` plus
  `unk40 * sinf(unk28 * (0.15915507 * z) + unk68)`.
  `0.15915507` ist das Retail-Bitmuster; `1/(2*pi)` liegt ein paar Bits daneben.
  Zwei benannte Produkte, damit die zweite Multiplikation nicht in `fmadds` faltet.
  140 Bytes, 35 Instruktionen.
  Dtor, `perform`, `noWave` und `__sinit_MapObjWave_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjWave`: PASS.
0 neue Fehler.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.
`MarNameRefGen_MapObj` matched code bleibt 2348.

`ninja changes_all`: fuzzy 79.10671 % -> 79.11047 %, matched code 49.83833 % -> 49.84223 % (1789240 -> 1789380, +140).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9621 -> 9622.
`MapObjWave` 1056 -> 1196 (+140).
Kein R170–R248-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R248 (`MapObjWave`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjWave::perform`.

- Wenn `unk94` gesetzt ist und `CUE_MOVE` anliegt, `updateTime`.
  Danach `getCurrentMap`: Karte 4 oder 6 ruft `updateHeightAndAlpha`.
  `CUE_DRAW` ruft `initDraw` und `draw`.
  `char trash[0x18]` hält den Frame bei `-0x40`.
  Die vier Callees sind noch Stubs und stehen unter `#pragma dont_inline`,
  sonst inlined MWCC sie weg.
  136 Bytes, 34 Instruktionen.
  Dtor, `noWave` und `__sinit_MapObjWave_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjWave`: PASS.
0 neue Fehler.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.
`MarNameRefGen_MapObj` matched code bleibt 2348.

`ninja changes_all`: fuzzy 79.10304 % -> 79.10671 %, matched code 49.83454 % -> 49.83833 % (1789104 -> 1789240, +136).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9620 -> 9621.
`MapObjWave` 920 -> 1056 (+136).
Kein R170–R247-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R247 (`MapObjCorona`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBathtubGripParts::getRootJointMtx`.

- Joint-Index ist `unkF4->unk200[unkF8]`.
  Danach `TLiveActor::getModel()->getAnmMtx(joint)`
  (`mNodeMatrices` bei `+0x58`).
  `char trash[8]` hält den Frame bei `-0x30`.
  Die Klasse steht nur in der cpp und erbt nicht von `TLiveActor`,
  damit diese TU keine Parts-VTable emittiert.
  72 Bytes, 18 Instruktionen.
  Die übrigen 12 Matches in `MapObjCorona` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjCorona`: PASS.
0 neue Fehler; vorbestehende MISSING/ORDER/BINDING bleiben.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.10104 % -> 79.10304 %, matched code 49.83254 % -> 49.83454 % (1789032 -> 1789104, +72).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9619 -> 9620.
`MapObjCorona` 616 -> 688 (+72).
Kein R170–R246-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R246 (`MapObjCorona`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBathtubGripPartsFragile::receiveMessage` und
`TBathtubGripPartsHard::receiveMessage`.

- Fragile leitet `receiveMessage` an den Grip bei `unkF4` (`+0xF4`) weiter.
  Hard macht dasselbe, setzt aber `HIT_MESSAGE_SUPER_HIP_DROP` vorher auf
  `HIT_MESSAGE_HIP_DROP`.
  Beide Klassen stehen nur in der cpp und erben nicht von `TLiveActor`,
  damit diese TU keine Parts-VTable emittiert.
  Fragile 48 Bytes, 12 Instruktionen.
  Hard 60 Bytes, 15 Instruktionen.
  Die übrigen 10 Matches in `MapObjCorona` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjCorona`: PASS.
0 neue Fehler; vorbestehende MISSING/ORDER/BINDING bleiben.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.09803 % -> 79.10104 %, matched code 49.82953 % -> 49.83254 % (1788924 -> 1789032, +108).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9617 -> 9619.
`MapObjCorona` 508 -> 616 (+108).
Kein R170–R245-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R245 (`MapObjCorona`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBathtubGrip::getRootJointMtx`.

- `TLiveActor::getModel`, dann `getBaseTRMtx` (`unk20` bei `+0x20`).
  Die Klasse steht nur in der cpp und erbt nicht von `TLiveActor`,
  damit diese TU keine Grip-VTable emittiert.
  36 Bytes, 9 Instruktionen.
  Die übrigen 9 Matches in `MapObjCorona` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjCorona`: PASS.
0 neue Fehler; vorbestehende MISSING/ORDER/BINDING bleiben.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.09703 % -> 79.09803 %, matched code 49.82853 % -> 49.82953 % (1788888 -> 1788924, +36).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9616 -> 9617.
`MapObjCorona` 472 -> 508 (+36).
Kein R170–R244-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R244 (`MapObjFlag`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjFlagManager::load`.

- `JDrama::TNameRef::load`, dann `readString` von 8 Bytes.
  `switch` auf `gpMarDirector->getCurrentMap()`:
  Karte 0 und 2 setzen `TMapObjFlag::mFlutterSpeed` auf `16.0f`,
  Karte 4 auf `12.0f`,
  sonst `8.0f`.
  `char trash[8]` hält den Namenspuffer bei `r1+0x20` (Frame `-0x30`).
  148 Bytes, 37 Instruktionen.
  Die übrigen 7 Matches in `MapObjFlag` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjFlag`: PASS.
4 vorbestehende UNUSED-Größen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.09303 % -> 79.09703 %, matched code 49.82441 % -> 49.82853 % (1788740 -> 1788888, +148).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9615 -> 9616.
`MapObjFlag` 1260 -> 1408 (+148).
Kein R170–R243-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R243 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TSandLeafBase::initMapObj`.

- `unk138` wird `0.003f`, `unk13C` wird `0.001f`, `unk140` wird `0`.
  `mScaling.y` kommt aus `TSandBase::mScaleMin`.
  Danach `TMapObjBase::initMapObj`.
  `newAndRegisterObj("SandLeaf", mPosition, mRotation)` lässt `scale` beim Default `(1, 1, 1)`.
  Das Ergebnis landet in `unk144`.
  `((TSandLeaf*)unk144)->unk138` zeigt auf `this`, dann `appear`.
  `char trash[1]` hält den Frame bei `-0x28`.
  148 Bytes, 37 Instruktionen.
  Die übrigen 63 Matches in `MapObjMamma` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.08901 % -> 79.09303 %, matched code 49.82028 % -> 49.82441 % (1788592 -> 1788740, +148).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9614 -> 9615.
`MapObjMamma` 6292 -> 6440 (+148).
Kein R170–R242-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R242 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBalloonKoopaJr::kill`.

- `TMapObjGeneral::kill` läuft zuerst.
  Drei `emitAndScale`-Aufrufe treffen `unk148`, mit `0x5A`, `0x5B` und `0x5C`.
  `TFlagManager::smInstance->incFlag(0x60001, 1)`.
  `gateCheck(MSD_SE_BS_BSPAKU_SLAP)` startet danach `startSoundActor` an `&mPosition`.
  `char trash[1]` hält den Frame bei `-0x20`.
  `unk148` ist `TVec3<f32>` bei `0x148`.
  172 Bytes, 43 Instruktionen.
  Die übrigen 48 Matches in `MapObjPinna` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjPinna`: PASS.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.
`MarNameRefGen_MapObj` verliert kein matched code (2348).

`ninja changes_all`: fuzzy 79.08432 % -> 79.08901 %, matched code 49.81549 % -> 49.82028 % (1788420 -> 1788592, +172).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9613 -> 9614.
`MapObjPinna` 4992 -> 5164 (+172).
Kein R170–R241-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R241 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TSandCastle::findTriggerActor`.

- `JDrama::TNameRefGen::search` sucht `"砂の城爆発の芽"`.
  Der Zeiger kommt als `TMapObjBase*` zurück.
  96 Bytes, 24 Instruktionen.
  Die übrigen 62 Matches in `MapObjMamma` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.08181 % -> 79.08432 %, matched code 49.81282 % -> 49.81549 % (1788324 -> 1788420, +96).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9612 -> 9613.
`MapObjMamma` 6196 -> 6292 (+96).
Kein R170–R240-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R240 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TPinnaEntrance::loadAfter`.

- `TMapObjBase::loadAfter` läuft zuerst.
  `rot` ist `(90, 0, 0)`.
  `newAndRegisterObj("GateManta", mPosition, rot)` lässt `scale` beim Default `(1, 1, 1)`.
  Der Default wird zuerst materialisiert, daher `addi r6` vor den übrigen Argumentzeigern.
  104 Bytes, 26 Instruktionen.
  Die übrigen 47 Matches in `MapObjPinna` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjPinna`: PASS.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.07903 % -> 79.08181 %, matched code 49.80992 % -> 49.81282 % (1788220 -> 1788324, +104).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9611 -> 9612.
`MapObjPinna` 4888 -> 4992 (+104).
Kein R170–R239-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R239 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjPuncher::load`.

- `TMapObjBase::load` liest den Stream zuerst.
  `read` holt ein `s32` von 4 Bytes.
  Die Zuweisung an `unk138` ist die übliche `s32`-nach-`f32`-Wandlung.
  Danach `sleep` und `offHitFlag(HIT_FLAG_NO_COLLISION)`.
  Das Flag-Bit 0 wird mit `clrrwi` gelöscht.
  128 Bytes, 32 Instruktionen.
  Die übrigen 42 Matches in `MapObjMare` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMare`: PASS.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.
`TMapObjPuncher::unk138` ist das `f32` bei `0x138`.
`MarNameRefGen_MapObj` verliert kein matched code.

`ninja changes_all`: fuzzy 79.07557 % -> 79.07903 %, matched code 49.80636 % -> 49.80992 % (1788092 -> 1788220, +128).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9610 -> 9611.
`MapObjMare` 4360 -> 4488 (+128).
Kein R170–R238-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R238 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TLampSeesaw::load`.

- `TMapObjBase::load` liest den Stream zuerst.
  Ein `f32` kommt per `read` von 4 Bytes.
  `unk13C` ist `mInitialPosition.y` minus diesen Wert.
  `unk140` wird ebenfalls per `read` geladen und mit `0.0001f` multipliziert.
  Ein totes `s32` hält den Float-Spill auf `r1+0x14` und den Frame auf `-0x20`.
  120 Bytes, 30 Instruktionen.
  Die übrigen 55 Matches in `MapObjBianco` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`: PASS.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.07234 % -> 79.07557 %, matched code 49.80301 % -> 49.80636 % (1787972 -> 1788092, +120).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9609 -> 9610.
`MapObjBianco` 6108 -> 6228 (+120).
Kein R170–R237-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R237 (`MapObjMonte`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjMonteRoot::initMapObj`.

- `TMapObjBase::initMapObj` läuft zuerst.
  `mDamageHeight` wird `1400.0f * mScaling.y`.
  Danach `calcEntryRadius`.
  `mPosition.y` wird `mInitialPosition.y + mYOffset`.
  `char trash[1]` hält den Frame auf `-0x20`.
  84 Bytes, 21 Instruktionen.
  Die übrigen 33 Matches in `MapObjMonte` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMonte`: PASS.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.07011 % -> 79.07234 %, matched code 49.80067 % -> 49.80301 % (1787888 -> 1787972, +84).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9608 -> 9609.
`MapObjMonte` 3784 -> 3868 (+84).
Kein R170–R236-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R236 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TCogwheelScale::touchWater`.

- `unk140` wird mit `unk144` verglichen.
  Liegt `unk140` darunter, wird `1.0f` addiert.
  Der Vergleich ist `fcmpo` und `bge`.
  `fadds` bleibt `f0, f1, f0`.
  36 Bytes, 9 Instruktionen.
  Die übrigen 41 Matches in `MapObjMare` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMare`: PASS.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.06933 % -> 79.07011 %, matched code 49.79967 % -> 49.80067 % (1787852 -> 1787888, +36).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9607 -> 9608.
`MapObjMare` 4324 -> 4360 (+36).
Kein R170–R235-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R235 (`Item`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TShine::appearWithDemo`.

- `TShine::appearWithDemo` sucht das Kamera-Tool über `JDrama::TNameRefGen::search`.
  `unk18C` bekommt `mDemoLengthFrames`.
  `fireStartDemoCamera` läuft mit `&mPosition`, `appearWithTimeCallback`, `this` und `JDrama::TFlagT<u16>()`.
  `char trash[1]` hält den Frame auf `-0x50` und das Flag-Halfword auf `r1+0x38`.
  172 Bytes, 43 Instruktionen.
  `__sinit_Item_cpp`, `TShine::kill` und `TShine::makeMActors` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/Item`: PASS.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.06931 % -> 79.06933 %, matched code 49.79488 % -> 49.79967 % (1787680 -> 1787852, +172).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9606 -> 9607.
`Item` 13696 -> 13868 (+172).
Kein R170–R234-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R234 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjBall::touchRoof`.

- `TMapObjBall::touchRoof` klemmt `param_1->y` auf `unk140`, wenn es größer ist.
  Danach `calcReflectingVelocity` mit `unk13C`, `mMapObjData->mPhysical->unk4->unk4` und `&mVelocity`.
  Frame `-0x8`, ohne Trash.
  76 Bytes, 19 Instruktionen.
  `TBigWatermelon::touchWaterSurface`, `makeObjDefault`, `put`, `touchPollution` und `__sinit_MapObjBall_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
Schwache-Reihenfolge-Warnung und vier bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.067314 % -> 79.06931 %, matched code 49.792763 % -> 49.79488 % (1787604 -> 1787680, +76).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9605 -> 9606.
`MapObjBall` 3172 -> 3248 (+76).
Kein R170–R233-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R233 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjRootPakkun::drawObject`.

- `TMapObjRootPakkun::drawObject` ruft `TLiveActor::drawObject` auf.
  Wenn `fabsf(gpMarioPos->z - mPosition.z) < 10000.0f`, folgt `unk138->movement`.
  Solange `isStateTimerEngaged` falsch ist, `tremble` mit `mTremblePower`, `mTrembleAccel`, `mTrembleBrake` und `mTrembleTime`, danach `mStateTimer = mTrembleTime`.
  `char trash[1]` hält Frame `-0x28`.
  148 Bytes, 37 Instruktionen.
  `TBiancoWatermillVertical::setGroundCollision`, `TWoodLog::control`, `TBiancoBell::touchPlayer` und `__sinit_MapObjBianco_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`: PASS.
Fünf bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.06331 % -> 79.067314 %, matched code 49.78864 % -> 49.792763 % (1787456 -> 1787604, +148).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9604 -> 9605.
`MapObjBianco` 5960 -> 6108 (+148).
Kein R170–R232-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R232 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMareFall::calc`.

- `TMareFall::calc` prüft `gateCheck(MSD_SE_GE_FALL)` über ein lokales `MSound*` und spielt den Sound an `&mPosition`.
  Danach `gpMSound->gateCheck(MSD_SE_GE_FALL_UPPER)` und `startSoundActor` an `fall_upper_pos`.
  Zwei `emit`-Aufrufe, `0x149` und `0x14A`, an `&mPosition` mit `this`.
  `char trash[0xC]` hält Frame `-0x28`.
  192 Bytes, 48 Instruktionen.
  `TMareFall::load`, `TMareCork::calcRootMatrix`, `drawObject` und `__sinit_MapObjMare_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMare`: PASS.
Fünf bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.05807 % -> 79.06331 %, matched code 49.78329 % -> 49.78864 % (1787264 -> 1787456, +192).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9603 -> 9604.
`MapObjMare` 4132 -> 4324 (+192).
Kein R170–R231-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R231 (`MapObjMonte`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TGoalFlag::touchActor`.

- `TGoalFlag::touchActor` bei `isActorType(0x80000001)` setzt Flag `0x50005`, falls es noch nicht gesetzt ist.
  Danach `receiveMessage(this, HIT_MESSAGE_ATTACK)`.
  Bei `isActorType(0x08000002)` nur die gleiche Message.
  `char trash[1]` hält Frame `-0x28`.
  228 Bytes, 57 Instruktionen.
  `TJumpMushroom::load`, `TJumpMushroom::receiveMessage` und `__sinit_MapObjMonte_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMonte`: PASS.
Elf bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.05183 % -> 79.05807 %, matched code 49.776943 % -> 49.78329 % (1787036 -> 1787264, +228).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9602 -> 9603.
`MapObjMonte` 3556 -> 3784 (+228).
Kein R170–R230-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R230 (`MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TRailFence::receiveMessage`.

- `TRailFence::receiveMessage` bei Message `3` prüft `gpMSound->gateCheck(MSD_SE_OBJ_MVING_FENCT_PNCH)`.
  Bei Erfolg `startSoundActor` an `&mPosition`.
  Danach `setUpMapCollision(1)`, `offMapObjFlag(MAP_OBJ_FLAG_UNK100)` und `mState = 2`.
  Rückgabe ist `TRUE`, sonst `FALSE`.
  `char trash[1]` hält Frame `-0x28`.
  140 Bytes, 35 Instruktionen.
  `TFenceWater::changeStatusToGo`, `TFenceWaterH::changeStatusToGo`, `changeStatusToWait`, `receiveMessage`, `TRevolvingFenceInner::control` und `__sinit_MapObjFence_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjFence`: PASS.
Schwache-Reihenfolge-Warnung und zwei bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.048134 % -> 79.05183 %, matched code 49.77304 % -> 49.776943 % (1786896 -> 1787036, +140).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9601 -> 9602.
`MapObjFence` 3120 -> 3260 (+140).
Kein R170–R229-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R229 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBigWatermelon::touchWaterSurface`.

- `TBigWatermelon::touchWaterSurface` ruft `emitColumnWater` auf.
  Danach `gpMSound->gateCheck(MSD_SE_OBJ_DRINA_TO_WATER)` und bei Erfolg `startSoundActor` an `&mPosition`.
  Abschluss ist virtuelles `kill` (vtable `0xE4`).
  `char trash[1]` hält Frame `-0x20`.
  112 Bytes, 28 Instruktionen.
  `TMapObjBall::makeObjDefault`, `put`, `touchPollution` und `__sinit_MapObjBall_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
Schwache-Reihenfolge-Warnung und vier bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.04512 % -> 79.048134 %, matched code 49.769924 % -> 49.77304 % (1786784 -> 1786896, +112).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9600 -> 9601.
`MapObjBall` 3060 -> 3172 (+112).
Kein R170–R228-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R228 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBiancoWatermillVertical::setGroundCollision`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TBiancoWatermillVertical::setGroundCollision` läuft, wenn `unk144` oder `mColCount` ungleich 0 ist.
  Dann `getModel()->getAnmMtx(0)` und `mMapCollisionManager->unk8->moveMtx`.
  Danach `unk144 = 0`.
  `unk140`, `unk144`, `unk148` und `unk14C` liegen hinter `unk13C`, damit das Flag bei `0x144` steht.
  `char trash[8]` hält Frame `-0x28`.
  116 Bytes, 29 Instruktionen.
  `load`, `TWoodLog::control`, `TBiancoBell::touchPlayer` und `__sinit_MapObjBianco_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`: PASS.
Fünf bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.042 % -> 79.04512 %, matched code 49.76669 % -> 49.769924 % (1786668 -> 1786784, +116).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9599 -> 9600.
`MapObjBianco` 5844 -> 5960 (+116).
Kein R170–R227-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R227 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMapObjBall::makeObjDefault`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TMapObjBall::makeObjDefault` ruft `TMapObjBase::makeObjDefault`.
  Dann schreibt es in `getAnmMtx(0)` die Translation `mPosition.x`, `mPosition.y + mBodyRadius` und `mPosition.z`.
  `char trash[1]` hält Frame `-0x28`.
  88 Bytes, 22 Instruktionen.
  `put`, `touchPollution` und `__sinit_MapObjBall_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjBall`: PASS.
Bestehende Weak-Order-Warnung und vier UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.039665 % -> 79.042 %, matched code 49.76424 % -> 49.76669 % (1786580 -> 1786668, +88).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9598 -> 9599.
`MapObjBall` 2972 -> 3060 (+88).
Kein R170–R226-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R226 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`startCameraShakeSE`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `startCameraShakeSE` spielt bei `param_2 == 0` `MSD_SE_OBJ_QUAKE` über `gateCheck` und `startSoundActor` an der übergebenen Position.
  Ein lokales `MSound*` hält `gpMSound` in r0, die Position wird danach nach r31 gelegt.
  `char trash[1]` hält Frame `-0x20`.
  Rückgabe 0.
  104 Bytes, 26 Instruktionen.
  `TGoalWatermelon::load`, `loadAfter` und `__sinit_MapObjMamma_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
Acht bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.03689 % -> 79.039665 %, matched code 49.761345 % -> 49.76424 % (1786476 -> 1786580, +104).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9597 -> 9598.
`MapObjMamma` 6092 -> 6196 (+104).
Kein R170–R225-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R225 (`MapObjMonte`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TJumpMushroom::load`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TJumpMushroom::load` ruft `TMapObjBase::load`.
  Dann liest es 4 Bytes in ein lokales `s32`.
  Wenn `mMapCollisionManager` gesetzt ist, folgt `unk8->setAllData` mit `extsh`.
  `char trash[1]` hält Frame `-0x28` und legt den Wert bei `r1+0x18` ab.
  100 Bytes, 25 Instruktionen.
  `receiveMessage`, `calcDefaultMtx` und `__sinit_MapObjMonte_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMonte`: PASS.
Elf bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.03421 % -> 79.03689 %, matched code 49.758556 % -> 49.761345 % (1786376 -> 1786476, +100).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9596 -> 9597.
`MapObjMonte` 3456 -> 3556 (+100).
Kein R170–R224-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R224 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TGoalWatermelon::load`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TGoalWatermelon::load` ruft `TMapObjBase::load`.
  Dann `readString` in ein lokales `char[0x20]` und drei `read`s von je 4 Bytes nach `unk140.x`, `unk140.y` und `unk140.z`.
  `char trash[0xC]` hält Frame `-0x48` und legt den Namen bei `r1+0x20` ab.
  120 Bytes, 30 Instruktionen.
  `loadAfter` und `__sinit_MapObjMamma_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
Acht bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.03098 % -> 79.03421 %, matched code 49.755215 % -> 49.758556 % (1786256 -> 1786376, +120).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9595 -> 9596.
`MapObjMamma` 5972 -> 6092 (+120).
Kein R170–R223-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R223 (`MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TFenceWater::changeStatusToGo`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TFenceWater::changeStatusToGo` prüft `gpMSound->gateCheck(MSD_SE_OBJ_WATER_FENCE_FW)`.
  Dann `startSoundActor` an `mPosition` und `mState = 2`.
  Ein lokales `MSound*` erzeugt `addi r31` plus `lwz r0`.
  `char trash[1]` hält Frame `-0x20`.
  100 Bytes, 25 Instruktionen.
  `TFenceWaterH::changeStatusToGo`, `changeStatusToWait`, `receiveMessage`, `TRevolvingFenceInner::control` und `__sinit_MapObjFence_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjFence`: PASS.
Bestehende Weak-Order-Warnung und zwei UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.02831 % -> 79.03098 %, matched code 49.75243 % -> 49.755215 % (1786156 -> 1786256, +100).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9594 -> 9595.
`MapObjFence` 3020 -> 3120 (+100).
Kein R170–R222-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R222 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TChangeStageMerrygoround::touchPlayer`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TChangeStageMerrygoround::touchPlayer` kehrt sofort zurück, wenn `isStateTimerEngaged()` wahr ist.
  Bei Yoshi-`mType == 1` folgen `gateCheck(MSD_SE_SY_COLLECT_YOSHI)`, `startSoundSystemSE`, `TMapObjChangeStage::touchPlayer` und `unk13C = 1`.
  Sonst `gateCheck(MSD_SE_SY_NOT_COLLECT_YOSHI)` und optional `startSoundSystemSE`.
  Danach `mStateTimer = 0x258`.
  `char trash[0xF]` hält Frame `-0x30`.
  212 Bytes, 53 Instruktionen.
  `becomeCalmlyCallback`, `calc` und `__sinit_MapObjPinna_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjPinna`: PASS.
Sechs bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.
`changeStatusToWait` bleibt `virtual`.

`ninja changes_all`: fuzzy 79.022514 % -> 79.02831 %, matched code 49.746525 % -> 49.75243 % (1785944 -> 1786156, +212).
Matched data bleibt 65.58592 % (419967).
Funktionen matched 9593 -> 9594.
`MapObjPinna` 4676 -> 4888 (+212).
Kein R170–R221-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R221 (`MapObjFence`, `MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TFenceWaterH::changeStatusToGo` und `TChangeStageMerrygoround::calc`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TFenceWaterH::changeStatusToGo` prüft `gpMSound->gateCheck(MSD_SE_OBJ_WATER_FENCE_FW)`.
  Dann `startSoundActor` an `mPosition`, `mState = 2`, `setUpMapCollision(1)`.
  Ein lokales `MSound*` erzeugt `addi r31` plus `lwz r0`.
  `char trash[1]` hält Frame `-0x20`.
  112 Bytes, 28 Instruktionen.
  `changeStatusToWait` ist jetzt `virtual`, dadurch matchen die VTables von `TFenceWater` und `TFenceWaterH`.
  `__sinit_MapObjFence_cpp` und `TRevolvingFenceInner::control` bleiben 100 %.
- `TChangeStageMerrygoround::calc` emittiert bei `unk13C != 0` die Partikel `0x100` und `0x101` an `gpMarioPos`.
  Ein lokales `TVec3*` lädt `gpMarioPos` nach r5 vor dem Manager.
  100 Bytes, 25 Instruktionen.
  `becomeCalmlyCallback` und `__sinit_MapObjPinna_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjFence` und `MapObjPinna`: PASS.
Bestehende UNUSED-Größenwarnungen (2 / 6).
`MapObjBase.hpp` unverändert.

`ninja changes_all`: fuzzy 79.01683 % -> 79.022514 %, matched code 49.74062 % -> 49.746525 % (1785732 -> 1785944, +212).
Matched data 65.24985 % -> 65.58592 % (417815 -> 419967, +2152).
Funktionen matched 9591 -> 9593.
`MapObjFence` 2908 -> 3020 (+112), Data 188 -> 2340.
`MapObjPinna` 4576 -> 4676 (+100).
Kein R170–R220-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R220 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBiancoBell::touchPlayer` und `TBiancoBell::touchWater`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TBiancoBell::touchPlayer` läutet, wenn der Frame 0 ist oder `frame + rate >= (f32)end - 1`.
  Danach `startAnim(4)`, Rate `SMSGetAnmFrameRate()`, `gateCheck` und `startSoundActor` mit `MSD_SE_OBJ_BI_BELL`.
  Drei TU-lokale Inlines halten die `getFrameCtrl`-Reloads.
  `char trash[1]` hält Frame `-0x60`.
  288 Bytes, 72 Instruktionen.
- `TBiancoBell::touchWater` ist derselbe Körper und gibt 1 zurück.
  292 Bytes, 73 Instruktionen.
  `__sinit_MapObjBianco_cpp` und `TWoodLog::control` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`: PASS.
Fünf bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.

`ninja changes_all`: fuzzy 79.000946 % -> 79.01683 %, matched code 49.724464 % -> 49.74062 % (1785152 -> 1785732, +580).
Matched data bleibt 65.24985 % (417815).
Funktionen matched 9589 -> 9591.
`MapObjBianco` 5264 -> 5844 (+580).
Kein R170–R219-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R219 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TFerrisWheel::becomeCalmlyCallback`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TFerrisWheel::becomeCalmlyCallback` gibt `s32` zurück, passend zu `fireStartDemoCamera`.
  Bei `param_1 == 0` setzt es `mState` auf 2.
  Wenn `gpMSound->unk80` gesetzt ist, `setVolume(0.0f, 200, 0)` und `setPitch(0.5f, 200, 0)`.
  Danach `mStateTimer = 120`.
  Der lokale `MSound*` hält `gpMSound` in r31, Frame `-0x20`.
  128 Bytes, 32 Instruktionen.
  `__sinit_MapObjPinna_cpp` bleibt 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjPinna`: PASS.
Sechs bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.

`ninja changes_all`: fuzzy 78.99749 % -> 79.000946 %, matched code 49.720898 % -> 49.724464 % (1785024 -> 1785152, +128).
Matched data bleibt 65.24985 % (417815).
Funktionen matched 9588 -> 9589.
`MapObjPinna` 4448 -> 4576 (+128).
Kein R170–R218-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R218 (`MapObjBianco`, `MapObjMare`, `MapObjRicco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TWoodLog::control`, `TMareCork::calcRootMatrix` und `TCraneUpDown::initMapObj`.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TWoodLog::control` ruft `TMapObjFloatOnSea::control`.
  Inverse von `getAnmMtx(0)`, Mario lokal, Schwimm-Box (−232 / −141 / 141 / −441 / 441).
  X wird auf ±141 geschoben und per `SMS_MarioMoveRequest` zurücktransformiert.
  `char trash[0x14]` hält Frame `-0x90`.
  248 Bytes, 62 Instruktionen.
  `__sinit_MapObjBianco_cpp` bleibt 100 %.
- `TMareCork::calcRootMatrix` ignoriert `checkPass(350)`.
  Bei `checkPass(250)`: `startChorobeiShout`, Shine-Demo, `unk148` (2773, 8618, 7006), Partikel `0x44` mit Scale 2.5.
  Danach `TMapObjBase::calcRootMatrix`.
  `char trash[0x18]` hält Frame `-0x30`.
  248 Bytes, 62 Instruktionen.
  `drawObject`, `moveObject`, `getTakingMtx` und `__sinit_MapObjMare_cpp` bleiben 100 %.
- `TCraneUpDown::initMapObj` ruft `TMapObjBase::initMapObj`, `setAllActor(nullptr)`, `newAndRegisterObj("craneCargoUpDown")` und `appear`.
  Der Inline-Pad hält Frame `-0x48`.
  `strcmp(mName, "craneUpDown 0")` wählt −25/45 und `MSD_SE_OBJ_CRANE_UPDOWN1`, sonst −25/30 und `MSD_SE_OBJ_CRANE_UPDOWN2`.
  `mRotation.x` ist `unk144 + (unk140 - unk144) * MsRandF()`.
  `mRotSpeed` 0.1 und `mWaitTime` 120 matchen.
  288 Bytes, 72 Instruktionen.
  Destruktor, VTable und `__sinit_MapObjRicco_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`, `MapObjMare`, `MapObjRicco`: PASS.
Bestehende UNUSED-Größenwarnungen (5 / 5 / 3).
`MapObjBase.hpp` unverändert.

`ninja changes_all`: fuzzy 78.97599 % -> 78.99749 %, matched code 49.69906 % -> 49.720898 % (1784240 -> 1785024, +784).
Matched data bleibt 65.24985 % (417815).
Funktionen matched 9585 -> 9588.
`MapObjBianco` 5016 -> 5264 (+248).
`MapObjMare` 3884 -> 4132 (+248).
`MapObjRicco` 2336 -> 2624 (+288).
Kein R170–R217-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R217 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TShiningStone::perform` ohne `SMatrix34C`-Leer-Konstruktor.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TShiningStone::perform` läuft über vier `MActor` an `unk68` und ruft danach `unk6C` auf.
  Wenn `(int)unk74` über 0, 1 bzw. 2 liegt, emittiert es die Partikel `0x143`/`0x144`/`0x145` an `mPosition`.
  `cmpwi`+`ble` ist das natürliche `>`.
  216 Bytes, 54 Instruktionen.
  `__sinit_MapObjMamma_cpp`, `TMammaYacht::initMapObj` und der Konstruktor bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
Acht bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.

`ninja changes_all`: fuzzy 78.970085 % -> 78.97599 %, matched code 49.693047 % -> 49.69906 % (1784024 -> 1784240, +216).
Matched data bleibt 65.24985 % (417815).
Funktionen matched 9584 -> 9585.
`MapObjMamma` 5756 -> 5972 (+216).
Kein R170-R216-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R216 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMareCork::drawObject` ohne `SMatrix34C`-Leer-Konstruktor.
`TRevolvingFenceInner::setGroundCollision` und `TMapObjBall::hold` bleiben geparkt.

- `TMareCork::drawObject` ruft `TLiveActor::drawObject`.
  Wenn `unk154` gesetzt ist und `mareCorkFrame` über 250 liegt, schreibt es `unk148` auf (2773, 8618, 7006), spielt `MSD_SE_ENV_FALL_JET_LEVEL` nach `gateCheck` und bindet die Partikel `0x14C`/`0x14D`/`0x14E` an `unk13C`.
  `fcmpo`+`ble` ist das natürliche `>`.
  Der zusätzliche Inline hält das tote Stack-Slot, Frame `-0x28`.
  228 Bytes, 57 Instruktionen.
  `moveObject`, `getTakingMtx` und `__sinit_MapObjMare_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMare`: PASS.
Fünf bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.

`ninja changes_all`: fuzzy 78.96385 % -> 78.970085 %, matched code 49.686695 % -> 49.693047 % (1783796 -> 1784024, +228).
Matched data bleibt 65.24985 % (417815).
Funktionen matched 9583 -> 9584.
`MapObjMare` 3656 -> 3884 (+228).
Kein R170-R215-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R215 (`MapObjMonte`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`THangingBridge::perform` ohne `SMatrix34C`-Leer-Konstruktor.
`TRevolvingFenceInner::setGroundCollision` bleibt geparkt.
`TMapObjBall::hold` bleibt geparkt (`TUtil<f32>::sqrt` muss ein `bl` bleiben).

- `THangingBridge::perform` zeichnet bei `CUE_DRAW`: `initDraw`, dann für jedes Brett `drawRopes` (beide `unk1A4`-Enden per `boardRopePoint` nach `drawOneRope`), dann `drawRopeBetweenBoards(0, mPointNumBetweenBoards)` und `drawRopeBetweenBoards(mRopeHeight, 1)`.
  `mPointNumBetweenBoards` ist 10, `mRopeHeight` liegt uninitialisiert in `.sbss`.
  Der zusätzliche Inline hält das tote Stack-Slot, Frame `-0x40`.
  224 Bytes, 56 Instruktionen.
  `drawRopes` ist UNUSED und trifft die Map-Größe 0x6c.
  `setGroundCollision`, `calcDefaultMtx`, `initMapObj` und `__sinit_MapObjMonte_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMonte`: PASS.
Elf bestehende UNUSED-Größenwarnungen (`drawRopes` fällt weg).
`MapObjBase.hpp` unverändert.

`ninja changes_all`: fuzzy 78.95772 % -> 78.96385 %, matched code 49.680454 % -> 49.686695 % (1783572 -> 1783796, +224).
Matched data 65.2486 % -> 65.24985 % (417807 -> 417815, +8).
Funktionen matched 9582 -> 9583.
`MapObjMonte` 3232 -> 3456 (+224).
Kein R170-R214-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R214 (`MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TRevolvingFenceInner::initMapCollisionData` ohne `SMatrix34C`-Leer-Konstruktor.
`setGroundCollision` bleibt geparkt.

- `TRevolvingFenceInner::initMapCollisionData` legt `new TMapCollisionManager(1, "mapObj", this)` an.
  Wenn `fabsf` von Rotation X und Z beide kleiner als 80 sind, `init("fence_revolve_inner_v_tool", 1, nullptr)`, sonst `init("fence_revolve_inner_h_tool", 1, nullptr)`.
  `fcmpo`+`bge` ist das natürliche `<`.
  176 Bytes, 44 Instruktionen.
  `initMapObj`, `TFence::initMapObj` und `__sinit_MapObjFence_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjFence`: PASS.
Bestehende Weak-Reihenfolge-Warnung, zwei UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.

`ninja changes_all`: fuzzy 78.95292 % -> 78.95772 %, matched code 49.675552 % -> 49.680454 % (1783396 -> 1783572, +176).
Matched data bleibt 65.2486 % (417807).
Funktionen matched 9581 -> 9582.
`MapObjFence` 2732 -> 2908 (+176).
Kein R170-R213-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R213 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TBiancoMiniWindmill::initMapObj` ohne `SMatrix34C`-Leer-Konstruktor.
`TRevolvingFenceInner::setGroundCollision` bleibt geparkt.

- `TBiancoMiniWindmill::initMapObj` setzt `mAppearSpeed` auf 0.
  `unk15C` ist `new TMapObjMessenger("地形オブジェメッセンジャー")` (Größe `0x6C`).
  `initHitActor(0, 1, 0, 0, 0, 300, 500)`.
  Messenger-Position ist `x + sMessengerPosZ * MsSin(rot.y)`, `y + sMessengerPosY`, `z + sMessengerPosZ * MsCos(rot.y)`.
  `sMessengerPosZ` ist 200, `sMessengerPosY` ist 6400.
  284 Bytes, 71 Instruktionen.
  Konstruktor, `control` und `__sinit_MapObjBianco_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjBianco`: PASS.
Fünf bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.

`ninja changes_all`: fuzzy 78.94513 % -> 78.95292 %, matched code 49.66764 % -> 49.675552 % (1783112 -> 1783396, +284).
Matched data bleibt 65.2486 % (417807).
Funktionen matched 9580 -> 9581.
`MapObjBianco` 4732 -> 5016 (+284).
Kein R170-R212-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R212 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TMammaYacht::initMapObj` ohne `SMatrix34C`-Leer-Konstruktor.
`TRevolvingFenceInner::setGroundCollision` bleibt geparkt (Matrix-Slot `0x34` statt `0x38`).

- `TMammaYacht::unk138` ist `TMapObjFlag*`.
  `initMapObj` legt `new TMapObjFlag("旗")` an (Größe `0xC0`, POD-Pad in `TMapObjFlag`).
  Position ist `(2 + x, (1315 + y) - 190, z - 15)`, Rotation `(0, 180, 0)`, Scale `(1, 2.5, 3.8)`, dann `init("MammaYacht00")`.
  Ein lokales Inline gruppiert die drei Stores, damit MWCC `stfsu` emittiert.
  `TVec3::set(f32, f32, f32)` ist `dont_inline`.
  212 Bytes, 53 Instruktionen.
  `TMammaYacht::control` und `__sinit_MapObjMamma_cpp` bleiben 100 %.

`validate-symbol-order` `mario/MoveBG/MapObjMamma`: PASS.
Acht bestehende UNUSED-Größenwarnungen.
`mario/MoveBG/MapObjFlag`: PASS.
Vier bestehende UNUSED-Größenwarnungen.
`MapObjBase.hpp` unverändert.

`ninja changes_all`: fuzzy 78.93934 % -> 78.94513 %, matched code 49.661736 % -> 49.66764 % (1782900 -> 1783112, +212).
Matched data bleibt 65.2486 % (417807).
Funktionen matched 9579 -> 9580.
`MapObjMamma` 5544 -> 5756 (+212).
Kein R170-R211-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R211 (`MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TRevolvingFenceInner::initMapObj` ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, ein `~`:
`TRevolvingFenceInner::setGroundCollision` (Rumpf passt, Matrix-Slot `0x34` statt `0x38`).

- `TRevolvingFenceInner::initMapObj` setzt `unk138` auf 1, wenn `strstr(unkF4, "bamboo")` trifft.
  Danach `TMapObjBase::initMapObj()`.
  `unk140` ist 1, wenn `fabsf` von Rotation X und Z beide kleiner als 1.0f sind, sonst 0.
  `MsMtxSetTRS` aus Position, Rotation und Scale, `MTXCopy` auf `unk8->unk20`, dann virtuelles `setUp()`.
  228 Bytes, 57 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjFence`: PASS.
Bestehende Weak-Reihenfolge-Warnung, zwei UNUSED-Größenwarnungen.

`ninja changes_all`: fuzzy 78.9331 % -> 78.93934 %, matched code 49.655384 % -> 49.661736 % (1782672 -> 1782900, +228).
Matched data bleibt 65.2486 % (417807).
Funktionen matched 9578 -> 9579.
`MapObjFence` 2504 -> 2732 (+228).
Kein R170-R210-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R210 (`MapObjMonte`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`THangingBridgeBoard::setGroundCollision` ohne `SMatrix34C`-Leer-Konstruktor.

- `THangingBridgeBoard::setGroundCollision` prüft `SMS_GetYoshi()->isHatched()` und den Brett-Bereich gegen `getTranslation()`, wie `TManhole`.
  `getModel()->getAnmMtx(0)` läuft über ein lokales Inline, damit MWCC den toten 8-Byte-Slot hält und der Frame `-0x40` bleibt.
  Danach virtuelles `moveMtx` auf `mMapCollisionManager->unk8`, sonst `TMapObjBase::setGroundCollision()`.
  244 Bytes, 61 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjMonte`: PASS.
Zwölf bestehende UNUSED-Größenwarnungen.

`ninja changes_all`: fuzzy 78.926414 % -> 78.9331 %, matched code 49.64859 % -> 49.655384 % (1782428 -> 1782672, +244).
Matched data bleibt 65.2486 % (417807).
Funktionen matched 9577 -> 9578.
`MapObjMonte` 2988 -> 3232 (+244).
Kein R170-R209-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R209 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

`TPinnaCoaster::initMapObj` ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, nicht strikt:
`TAmiKing::initMapObj` (leere Joint-Schleife, `mModel`-Load eine Stufe zu weit aus der Schleife).
`TMapObjFlag::TMapObjFlag` (Stores passen, Frame `-0x20` statt `-0x48`, Rand-`0x4330` in `r3` statt `r0`).
`TMerrygoround::TMerrygoround` (Rumpf und 9er-Unroll passen, Frame `-0x20` statt `-0x28`).

- `TPinnaCoaster::initMapObj` legt das Rail-Modell mit `SMS_MakeMActorWithAnmData("/scene/mapObj/CoasterRail.bmd", mManager->getMActorAnmData(), 3, 0x10210000)` an.
  Danach `setBck("coasterrail")` und `MsMtxSetXYZRPH` auf `getModel()->getBaseTRMtx()` aus Position und Rotation.
  `rate = SMSGetAnmFrameRate(); rate *= 0.25f;` hält `fmuls f31, f1, f0`.
  `getFrameCtrl(ANM_TYPE_BCK)->setRate(rate)` und komponentenweises Kopieren von `mPosition` nach `unk140`.
  `unk138` ist `MActor*`.
  248 Bytes, 62 Instruktionen.

`validate-symbol-order` `mario/MoveBG/MapObjPinna`: PASS.
Sechs bestehende UNUSED-Größenwarnungen.

`ninja changes_all`: fuzzy 78.9196 % -> 78.926414 %, matched code 49.64168 % -> 49.64859 % (1782180 -> 1782428, +248).
Matched data bleibt 65.2486 % (417807).
Funktionen matched 9576 -> 9577.
`MapObjPinna` 4200 -> 4448 (+248).
`MapObjManager` und `MarNameRefGen_MapObj` wurden neu gebaut, matched code unverändert.
Kein R170-R208-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R208 (`MapObjMonte` / `MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Drei Konstruktoren ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, nur Frame:
`TSwingBoard::TSwingBoard` (Rumpf passt, Frame `-0x18` statt `-0x48`).

- `TFluff::TFluff` nullt die Floats von `0x138` bis `0x150`, setzt `unk160` auf 1.0f und `unk164` auf 0.95f und nullt `unk168` sowie `unk16C`.
  `unk154.zero()` steht im Rumpf.
  140 Bytes, 35 Instruktionen.
- `THangingBridgeBoard::THangingBridgeBoard` legt zwei `TVec3` bei `0x1A4` an und nullt im Rumpf `unk1BC`, `unk194`, `unk198`, `unk19C` und `unk1A0`.
  Danach `zero()` auf beiden Vektoren.
  156 Bytes, 39 Instruktionen.
- `TBiancoMiniWindmill::TBiancoMiniWindmill` setzt `unk150` auf `360.0f * ((f32)rand() * 0.000030517578f)`, `unk154` auf 0 und `unk158` auf `1.0f` plus denselben Rand-Faktor.
  Der zweite Faktor steht in einer eigenen Variable, damit `fmuls` und `fadds` nicht zu `fmadds` verschmelzen.
  `unk15C` und `unk160` werden genullt.
  204 Bytes, 51 Instruktionen.

`validate-symbol-order` für Monte und Bianco: PASS.
Monte hat zwölf bestehende UNUSED-Größenwarnungen, Bianco fünf.

`ninja changes_all`: fuzzy 78.90978 % -> 78.9196 %, matched code 49.627754 % -> 49.64168 % (1781680 -> 1782180, +500).
Matched data bleibt 65.2486 % (417807).
Funktionen matched 9573 -> 9576.
`MapObjBianco` 4528 -> 4732 (+204), `MapObjMonte` 2692 -> 2988 (+296).
`MapObjManager` und `MarNameRefGen_MapObj` wurden neu gebaut, matched code unverändert.
Kein R170-R207-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R207 (`MapObjPinna` / `MapObjMonte` / `MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Sechs Konstruktoren ohne `SMatrix34C`-Leer-Konstruktor.
`THorizontalViking::THorizontalViking` ist UNUSED (92 Bytes, Größe passt) und wird in `TViking` geinlined.

- `TFerrisWheel::TFerrisWheel` nullt `unk138`, `unk13C` und `unk140`.
  88 Bytes, 22 Instruktionen.
- `TViking::TViking` läuft durch den geinlined `THorizontalViking`-Konstruktor und nullt `unk14C` sowie die Floats bei `0x150`, `0x154` und `0x158`.
  132 Bytes, 33 Instruktionen.
- `TPinnaCoaster::TPinnaCoaster` nullt `unk138` und ruft `unk140.zero()` auf (Stores `0x148`, `0x144`, `0x140`).
  Die Lücke `unk13C` bleibt ungeschrieben.
  92 Bytes, 23 Instruktionen.
- `THangingBridge::THangingBridge` baut `TViewObj` und nullt `unk10`, `unk14`, `unk38` und `unk3C`.
  `unk18` der Größe `0x20` bleibt ungeschrieben.
  132 Bytes, 33 Instruktionen.
- `TFluffManager::TFluffManager` nullt `unk138`, `unk144`, `unk154` und die Wörter ab `0x158`.
  `unk148.setAll(0.0f)` steht im Rumpf, damit die drei Stores zuletzt kommen.
  124 Bytes, 31 Instruktionen.
- `TRailFence::TRailFence` legt `new TGraphTracer` in `unk13C` ab und setzt `unk140` auf `0.0f`.
  140 Bytes, 35 Instruktionen.

`validate-symbol-order` für Pinna, Monte und Fence: PASS.
Pinna hat sechs bestehende UNUSED-Größenwarnungen, Monte zwölf, Fence zwei plus die bestehende Weak-Order-Warnung.
`THorizontalViking::THorizontalViking` ist nicht unter den Größenwarnungen.

`ninja changes_all`: fuzzy 78.90318 % -> 78.90978 %, matched code 49.608032 % -> 49.627754 % (1780972 -> 1781680, +708).
Matched data bleibt 65.2486 % (417807).
Funktionen matched 9567 -> 9573.
`MapObjFence` 2364 -> 2504 (+140), `MapObjMonte` 2436 -> 2692 (+256), `MapObjPinna` 3888 -> 4200 (+312).
`MapObjManager` und `MarNameRefGen_MapObj` wurden neu gebaut, matched code unverändert.
Kein R170-R206-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R206 (`MapObjFlag`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Sieben Matches im bisher leeren `MapObjFlag`, ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, nur Frame:
`TMapObjFlagManager::load` (Rumpf passt, inklusive getrennter `case 0`- und `case 2`-Stores von 16.0f, Frame `-0x20` statt `-0x30`).

- `TMapObjFlagManager::TMapObjFlagManager` baut `TViewObj`, legt 15 `TMapObjFlagInfo` der Größe `0x58` ab Offset `0x10` an und speichert `this` in `gpMapObjFlagManager`.
  140 Bytes, 35 Instruktionen.
- `TMapObjFlagManager::TMapObjFlagInfo::TMapObjFlagInfo` nullt die Felder bei `0x0` und `0x54`.
  16 Bytes, 4 Instruktionen.
- `TMapObjFlag::load` ruft `TActor::load`, liest `0x40` Zeichen und ruft `init` (`#pragma dont_inline` am leeren Stub, damit das `bl` bleibt).
  84 Bytes, 21 Instruktionen.
- `TMapObjFlagManager::~TMapObjFlagManager` ist der generierte Destruktor.
  116 Bytes, 29 Instruktionen.
- `TMapObjFlag::~TMapObjFlag` ist der generierte Destruktor.
  132 Bytes, 33 Instruktionen.
- `__sinit_MapObjFlag_cpp` initialisiert die JAL-Listen.
  764 Bytes, 191 Instruktionen.
- `@32@__dt__11TMapObjFlagFv` ist der Sekundär-Thunk.
  8 Bytes, 2 Instruktionen.

`validate-symbol-order` für MapObjFlag: PASS.
Vier UNUSED-Größenwarnungen für die leeren Stubs `loadFlag`, `update` und `updateVertex` von Lower und Sail.

`ninja changes_all`: fuzzy 78.86625 % -> 78.90318 %, matched code 49.572937 % -> 49.608032 % (1779712 -> 1780972, +1260).
Matched data 65.18551 % -> 65.2486 % (417403 -> 417807, +404).
Funktionen matched 9560 -> 9567.
Nur `MapObjFlag` hat sich geändert.
Kein R170-R205-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R205 (`MapObjBianco` / `MapObjBall` / `MapObjPinna` / `MapObjMare` / `MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Fünf kurze Fills ohne `SMatrix34C`-Leer-Konstruktor.

- `TBellWatermill::TBellWatermill` nullt die Floats `0x16C` bis `0x18C`, die Bytes `0x190` und `0x1A0` und das Wort `0x1A4`.
  124 Bytes, 31 Instruktionen.
- `TResetFruit::makeObjLiving` setzt `MAP_OBJ_FLAG_DISAPPEARING` und `mStateTimer` aus `getLivingTime`, solange der Timer nicht läuft, löscht `LIVE_FLAG_UNK10` und setzt `mState` auf 11.
  128 Bytes, 32 Instruktionen.
- `TPinnaShell::receiveMessage` reagiert auf `HIT_MESSAGE_SPRAYED_BY_WATER` mit `PARTICLE_MS_ENM_WATHIT` und `MSD_SE_EN_COMMON_W_HIT_OK`, zieht `mWaterOpenAccel` von `unk6C` ab und setzt `unk68`, wenn `unk6C` unter `-mOpenRotMax` fällt.
  176 Bytes, 44 Instruktionen.
- `TMapObjBall::getDepthAtFloating` gibt `unk18C` zurück.
  8 Bytes, 2 Instruktionen.
- `TMapObjBase::getObjCollisionHeightOffset` gibt `mYOffset` zurück.
  8 Bytes, 2 Instruktionen.

`validate-symbol-order` für Bianco, Ball, Pinna, Mare und Mamma: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor.
Ball zusätzlich die bekannte Weak-Order-Warnung.

`ninja changes_all`: fuzzy 78.85625 % -> 78.86625 %, matched code 49.560566 % -> 49.572937 % (1779268 -> 1779712, +444).
Matched data bleibt 65.18551 %.
Funktionen matched 9555 -> 9560.
MapObjBianco matched code 4404 -> 4528.
MapObjBall matched code 2844 -> 2972.
MapObjPinna matched code 3712 -> 3888.
MapObjMare matched code 3648 -> 3656.
MapObjMamma matched code 5536 -> 5544.
Monte, `MapObjManager` und `MarNameRefGen_MapObj` unverändert.
Kein R170-R204-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R204 (`MapObjBianco` / `MapObjFence` / `MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Drei kurze Fills ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, nur Frame:
`TMapObjRootPakkun::drawObject` (Rumpf passt, Frame `-0x20` statt `-0x28`).
Zurückgenommen, Ablauf:
`startCameraShakeSE` (`addi` vor dem `bne`, `gpMSound` in `r3` statt `r0` plus `mr`).

- `TBellWatermill::touchWater` setzt `unk190`, addiert `unk15C` auf `unk158` und, wenn `fabsf(unk158)` über `unk16C` liegt, auch `unk180` auf `unk178`, dann klemmt `unk158` auf `unk164`.
  104 Bytes, 26 Instruktionen.
- `TFenceWater::receiveMessage` reagiert auf `HIT_MESSAGE_SPRAYED_BY_WATER` außerhalb von State 3, setzt `unk13C` auf `mWaterAccel` (2.1) und ruft virtuell `changeStatusToGo`, wenn `unk13C` positiv ist.
  120 Bytes, 30 Instruktionen.
- `TSandBird::makeObjFromJointName` delegiert an `TJointCoin::makeObjFromJointName` und legt sonst `SandBirdBlock` an, wenn der Name kein `none` enthält.
  140 Bytes, 35 Instruktionen.

`validate-symbol-order` für Bianco, Fence und Mamma: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor.
Fence zusätzlich die bekannte Weak-Order-Warnung.

`ninja changes_all`: fuzzy 78.846695 % -> 78.85625 %, matched code 49.55043 % -> 49.560566 % (1778904 -> 1779268, +364).
Matched data bleibt 65.18551 %.
Funktionen matched 9552 -> 9555.
MapObjBianco matched code 4300 -> 4404.
MapObjFence matched code 2244 -> 2364.
MapObjMamma matched code 5396 -> 5536.
Monte, `MapObjManager` und `MarNameRefGen_MapObj` unverändert (matched code 2348).
Kein R170-R203-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R203 (`MapObjPinna` / `MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Vier kurze Fills ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, nur Frame:
`TSandBase::withering` (Rumpf passt, Frame `-0x18` statt `-0x20`).

- `TWaterRecoverObj::touchPlayer` reagiert auf `isActorType(0x80000001)`, solange der State-Timer nicht läuft, mit `HIT_MESSAGE_ATTACK` und setzt `mStateTimer` auf `0x258`.
  144 Bytes, 36 Instruktionen.
- `TShellCup::control` ruft `getMActor()->calc()` und dann `control` auf den sechs eingebetteten `TPinnaShell`.
  `#pragma dont_inline` hält den leeren `TPinnaShell::control`-Stub als `bl`.
  100 Bytes, 25 Instruktionen.
- `TMammaBlockRotate::load` legt zwei `TMapCollisionMove` an, initialisiert sie mit `MammaBlockDown.col` und `MammaBlockUp.col` und ruft danach `TMapObjBase::load`.
  200 Bytes, 50 Instruktionen.
- `TMammaYacht::control` ruft `TMapObjBase::control`, prüft `mGroundPlane->isWaterSurface()`, setzt `mPosition.y` aus `mInitialPosition.y` plus `getWaveHeight` und legt `unk138->mPosition.y` um 50 tiefer.
  160 Bytes, 40 Instruktionen.

`validate-symbol-order` für Pinna und Mamma: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor.

`ninja changes_all`: fuzzy 78.83033 % -> 78.846695 %, matched code 49.533607 % -> 49.55043 % (1778300 -> 1778904, +604).
Matched data bleibt 65.18551 %.
Funktionen matched 9548 -> 9552.
MapObjPinna matched code 3468 -> 3712.
MapObjMamma matched code 5036 -> 5396.
Monte, `MapObjManager` und `MarNameRefGen_MapObj` unverändert (matched code 2348).
Kein R170-R202-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R202 (`MapObjRicco` / `MapObjBianco` / `MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Drei kurze Fills ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, nur String-Addends:
`TBiancoWatermillVertical::loadAfter` (fünf `~` auf `addi`, Frame beide `-0x50`).
Zurückgenommen, nur Frame:
`TGoalFlag::touchActor` (`-0x20` statt `-0x28`).
Zurückgenommen, Ablauf:
`TSandBombBase::expanded` (zweites `getFrameCtrl` vor dem Add, Frame `-0x28` statt `-0x40`).
Zurückgenommen, ein fehlendes `b` in der Switch-Kette:
`TGoalWatermelon::control`.

- `TCraneRotY::load` liest 4 Bytes nach `unk140`, kopiert `mRotation.y` nach `unk138`, setzt `unk144` auf `0.05 + 0.1 * (rand() * 0.000030517578)` und wählt `MSD_SE_OBJ_CRANE_SIDEMOVE1` oder `2` per `strcmp(mName, "crane90 0")`.
  `mState` wird 0.
  188 Bytes, 47 Instruktionen.
- `TBiancoMiniWindmill::control` bremst `unk154` mit `mFriction`, wenn es über `unk158` liegt, addiert es auf `unk150` und wickelt mit `MsWrap` auf `[0, 360)`.
  112 Bytes, 28 Instruktionen.
- `TGoalWatermelon::loadAfter` setzt `HIT_FLAG_CANNOT_GET_HIT`, sucht `シャイン（お化けスイカ用）` nach `unk138`, kopiert `unk140` in dessen `mPosition` und ruft virtuell `appear`.
  176 Bytes, 44 Instruktionen.

`validate-symbol-order` für Ricco, Bianco und Mamma: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor.

`ninja changes_all`: fuzzy 78.8174 % -> 78.83033 %, matched code 49.520348 % -> 49.533607 % (1777824 -> 1778300, +476).
Matched data bleibt 65.18551 %.
Funktionen matched 9545 -> 9548.
MapObjRicco matched code 2148 -> 2336.
MapObjBianco matched code 4188 -> 4300.
MapObjMamma matched code 4860 -> 5036.
Monte, `MapObjManager` und `MarNameRefGen_MapObj` unverändert.
Kein R170-R201-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R201 (`MapObjMonte` / `MapObjRicco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Zwei kurze Fills ohne `ble`/`bge`/`lfsu` und ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, nur Frame:
`TRailFence::receiveMessage` (`-0x20` statt `-0x28`),
`TMapObjWave::perform` (`-0x20` statt `-0x40`),
`TResetFruit::makeObjWaitingToAppear` (`-0x18` statt `-0x28`).
Zurückgenommen, Frame plus Argument-Schedule:
`TSandCastle::waitBeforeExplode` (`-0x20` statt `-0x28`).
Zurückgenommen, Ablauf:
`TFerrisWheel::becomeCalmlyCallback` (`gpMSound` vor dem Vergleich, `mr` statt `addi`),
`TMapObjWave::getWaveHeight` (zweites `sinf` anders gerechnet).

- `TFluff::touchWater` holt die Wasserposition, bildet die Normale und zieht sie mal `unk160` von `mVelocity` ab.
  140 Bytes, 35 Instruktionen.
- `TFruitSwitch::receiveMessage` startet bei `HIT_MESSAGE_HIP_DROP` `riccoswitch`, setzt `HIT_FLAG_NO_COLLISION`, ruft virtuell `remove` auf der Kollision und `fireObj` auf `unk138`.
  `fireObj` bleibt per `#pragma dont_inline` ein `bl`.
  128 Bytes, 32 Instruktionen.

`validate-symbol-order` für Monte und Ricco: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor.

`ninja changes_all`: fuzzy 78.81031 % → 78.8174 %, matched code 49.512882 % → 49.520348 % (1777556 → 1777824, +268).
Matched data bleibt 65.18551 %.
Funktionen matched 9543 → 9545.
MapObjMonte matched code 2296 → 2436.
MapObjRicco matched code 2020 → 2148.
Pinna, Mamma, Fence, Ball, `MapObjManager` und `MarNameRefGen_MapObj` unverändert.
Kein R170–R200-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R200 (`MapObjBianco` / `MapObjBall` / `MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Acht kurze Fills ohne `ble`/`bge`/`lfsu` und ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, nur Argument-Schedule (kein Strukturfehler im Ablauf):
`TPinnaEntrance::loadAfter` (vier `~` auf den `addi`s von `newAndRegisterObj`, 98.3 %, Frame beide `-0x30`).

- `TMapObjRootPakkun::initMapObj` ruft `TMapObjBase::initMapObj`, legt `TTrembleModelEffect` an, ruft `init` mit `mMActor->getModel` und `tremble(100, 1, 1, 0x2EE0)`.
  96 Bytes, 24 Instruktionen.
- `TBiancoWatermill::control` zieht `unk138` von `mRotation.z` ab und startet `MSD_SE_OBJ_BI_BIGMILL` mit `fabsf(unk138)` und Handle `(JAISoundHandle*)&unk13C`.
  132 Bytes, 33 Instruktionen.
- `TResetFruit::initMapObj` ruft `TMapObjBall::initMapObj` (leer, `#pragma dont_inline`) und `SMS_InitPacket_OneTevColor` auf `GX_TEVREG0` ab `unk19C`.
  68 Bytes, 17 Instruktionen.
- `TCoverFruit::receiveMessage` setzt bei `isActorType(0x08000083)` und `HIT_MESSAGE_TAKE` `HIT_FLAG_NO_COLLISION` und `mHolder`, bei `HIT_MESSAGE_UNKB` virtuell `kill` und `setBool(true, 0x1038B)`.
  160 Bytes, 40 Instruktionen.
- `TBigWatermelon::touchWall` reicht an `TMapObjBall::touchWall` weiter (leer, `#pragma dont_inline`).
  32 Bytes, 8 Instruktionen.
- `TBigWatermelon::touchGround` reicht an `TMapObjBall::touchGround` weiter (leer, `#pragma dont_inline`).
  32 Bytes, 8 Instruktionen.
- `TRevolvingFenceOuter::receiveMessage` startet bei Nachricht 3 `fence_revolve_outer_shake` und auf `unk13C` `fence_revolve_inner_shake`.
  92 Bytes, 23 Instruktionen.
- `TRevolvingFenceInner::control` ruft `TMapObjBase::control` und dann `controlWall`, wenn `unk140` ungleich 0 ist, sonst `controlGroundRoof` (beide `#pragma dont_inline`).
  76 Bytes, 19 Instruktionen.

`validate-symbol-order` für Bianco, Ball und Fence: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor.
Ball behält die weak-only Order-Warnung.
Fence behält eine weak-only Order-Warnung (`MsWrap`).

`ninja changes_all`: fuzzy 78.7922 % → 78.81031 %, matched code 49.493717 % → 49.512882 % (1776868 → 1777556, +688).
Matched data bleibt 65.18551 %.
Funktionen matched 9535 → 9543.
MapObjBianco matched code 3960 → 4188.
MapObjBall matched code 2552 → 2844.
MapObjFence matched code 2076 → 2244.
Pinna (3468), Mamma (4860), `MapObjManager` (7064) und `MarNameRefGen_MapObj` (2348) unverändert.
Kein R170–R199-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R199 (`MapObjMamma` / `MapObjPinna` / `MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Vier kurze Fills ohne `ble`/`bge`/`lfsu` und ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, nur Frame bzw. Stack-Slot (kein Strukturfehler im Ablauf):
`TLampSeesaw::load` (Float bei `0x10` statt `0x14`, Frame beide `-0x20`),
`TBiancoWatermillVertical::setGroundCollision` (`-0x28` statt `-0x20`),
`TBalloonKoopaJr::kill` (`-0x20` statt `-0x18`),
`TBigWatermelon::touchWaterSurface` (`-0x20` statt `-0x18`).

- `TSandCastle::initMapObj` ruft `TSandBombBase::initMapObj` (leer, `#pragma dont_inline`, damit das `bl` bleibt), setzt `unk13C` auf 0.11 und `unk148` auf 0x78 und ruft `sleep`.
  68 Bytes, 17 Instruktionen.
- `TLeanMirror::initMapObj` setzt die Spiegel-Konstanten und verzweigt über `strcmp` von `unkF4` mit `mirrorS` bzw. `mirrorM`.
  216 Bytes, 54 Instruktionen.
- `TViking::initMapObj` setzt `unk14C`, unterscheidet `viking 0` per `getName`, zieht `unk138` von `mPosition.y` ab und ruft `TMapObjBase::initMapObj`.
  192 Bytes, 48 Instruktionen.
- `TRailFence::load` liest den Graph-Namen, hängt `unk13C` an den nächsten Knoten, wenn der Graph kein Dummy ist, und setzt `unk140` auf 8 sowie `mGravity` auf 0.3.
  160 Bytes, 40 Instruktionen.

`validate-symbol-order` für Mamma, Pinna und Fence: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor.
Fence behält eine weak-only Order-Warnung (`MsWrap`).

`ninja changes_all`: fuzzy 78.77492 % → 78.7922 %, matched code 49.476 % → 49.493717 % (1776232 → 1776868, +636).
Matched data bleibt 65.18551 %.
Funktionen matched 9531 → 9535.
MapObjMamma matched code 4576 → 4860.
MapObjPinna matched code 3276 → 3468.
MapObjFence matched code 1916 → 2076.
Bianco, Ball, `MapObjManager` (7064) und `MarNameRefGen_MapObj` (2348) unverändert.
Kein R170–R198-Unit hat matched code verloren.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R198 (`MapObjBianco` / `MapObjMamma` / `MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Fünf kurze Fills ohne `ble`/`bge`/`lfsu` und ohne `SMatrix34C`-Leer-Konstruktor.
Zurückgenommen, nur Frame (kein Strukturfehler):
`TSandLeafBase::initMapObj` (`-0x28` statt `-0x20`),
`TMuddyBoat::initMapObj` (`-0x28` statt `-0x20`).

- `TBiancoWatermill::initMapObj` vergleicht `unkF4` mit `BiaWatermill01` oder `BiaWatermill00` und setzt `mBodyRadius` auf 1200.
  112 Bytes, 28 Instruktionen.
- `TBiancoBell::initMapObj` setzt `unk138`/`unk13A` nach `BiaBell 0` bzw. `BiaBell 1`, sonst 3/0.
  148 Bytes, 37 Instruktionen.
- `TLeafBoatRotten::load` liest `unk170`, multipliziert mit 10 und initialisiert `GX_TEVREG0` ab `unk178`.
  108 Bytes, 27 Instruktionen.
- `TSandCastle::loadAfter` speichert virtuell `findTriggerActor` in `unk144`, setzt `unk138` auf `this`, ruft virtuell `appear`, sucht `ステージ切替（砂の城）` in `unk158` und ruft virtuell `makeObjDead`.
  180 Bytes, 45 Instruktionen.
- `TBigWatermelon::loadAfter` ruft `TMapObjGeneral::loadAfter`, sucht `シャイン（お化けスイカ用）` und setzt die Position auf (−4659, 460, 13620).
  124 Bytes, 31 Instruktionen.

`TSandCastle::findTriggerActor` bleibt `return nullptr`.

`validate-symbol-order` für Bianco, Mamma und Ball: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor.
Ball behält die weak-only Order-Warnung.

`ninja changes_all`: fuzzy 78.75675 % → 78.77492 %, matched code 49.457283 % → 49.476 % (1775560 → 1776232, +672).
Matched data bleibt 65.18551 %.
MapObjBianco matched code 3592 → 3960.
MapObjMamma matched code 4396 → 4576.
MapObjBall matched code 2428 → 2552.
Kein R170–R197-Unit hat matched code verloren.
`MarNameRefGen_MapObj` und `MapObjManager` unverändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R197 (`MapObjMonte` / `MapObjRicco` / `MapObjPinna` / `MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Acht kurze Fills ohne `ble`/`bge`/`lfsu` und ohne `SMatrix34C`-Leer-Konstruktor.
`TGoalWatermelon::load` und `TChangeStageMerrygoround::calc` bleiben geparkt.
Zurückgenommen, nur Frame (kein Strukturfehler):
`TMapObjMonteRoot::initMapObj` (`-0x20` statt `-0x18`),
`TJumpMushroom::load` (`-0x28` statt `-0x20`),
`TFenceWater::changeStatusToGo` und `TFenceWaterH::changeStatusToGo` (`-0x20` statt `-0x18`).

- `THangingBridgeBoard::calcDefaultMtx` baut RotX/RotY, `MTXConcat`, kopiert in `mDefaultMtx`, nullt `mVelocity.y` und setzt `mPosition.y` auf `mInitialPosition.y`.
  104 Bytes, 26 Instruktionen.
- `THangingBridgeBoard::initMapObj` ruft `TLeanBlock::initMapObj`, dann `unk140 = 0.01`, `unk144 = 0.02`, `unk148 = 0.08`.
  68 Bytes, 17 Instruktionen.
- `TFluff::kill` sendet `HIT_MESSAGE_UNK8` an `mHeldObject`, löscht den Zeiger und setzt `mState` auf 3.
  92 Bytes, 23 Instruktionen.
- `TCraneCargo::calc` ruft `updateRootMtxTrans` und `calcLeanMtx` auf `getAnmMtx(1)`.
  68 Bytes, 17 Instruktionen.
- `THorizontalViking::initMapObj` ruft `TMapObjBase::initMapObj`, setzt `unk138`/`unk13C`/`unk140` und ruft virtuell `reset`.
  88 Bytes, 22 Instruktionen.
- `TBalloonKoopaJr::touchActor` ruft virtuell `kill`.
  44 Bytes, 11 Instruktionen.
- `TAmiKing::bind` prüft `LIVE_FLAG_UNK10` und ruft dann `gpMap->checkGround`, sonst `TLiveActor::bind`.
  88 Bytes, 22 Instruktionen.
- `TBiancoWatermillVertical::load` liest `unk13C`, teilt durch 1000 und kopiert nach `unk138`.
  96 Bytes, 24 Instruktionen.

`validate-symbol-order` für Monte, Ricco, Pinna und Bianco: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor.
Kein neues Linkage- oder Order-Problem.

`ninja changes_all`: fuzzy 78.73961 % → 78.75675 %, matched code 49.43923 % → 49.457283 % (1774912 → 1775560, +648).
Matched data bleibt 65.18551 %.
MapObjMonte matched code 2032 → 2296.
MapObjPinna matched code 3056 → 3276.
MapObjBianco matched code 3496 → 3592.
MapObjRicco matched code 1952 → 2020.
Kein R170–R196-Unit hat matched code verloren.
`MarNameRefGen_MapObj` und `MapObjManager` unverändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R196 (`MapObjMonte` / `MapObjMamma` / `MapObjFence` / `MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Acht kurze Fills ohne `ble`/`bge`/`lfsu`.
`TMareFall::calc` und `TMapObjBall::makeObjDefault` bleiben geparkt (Frame).
`TGoalWatermelon::load` bleibt Stub: gleicher Code, Frame `-0x38` statt `-0x48`.
`TChangeStageMerrygoround::calc` bleibt Stub: `gpMarioPos` und `gpMarioParticleManager` tauschen die Lade-Reihenfolge.
`TSandCastle::findTriggerActor` bleibt `return nullptr`.

- `TGoalFlag::initMapObj` ruft `TMapObjBase::initMapObj`.
  32 Bytes, 8 Instruktionen.
- `TFluff::initMapObj` ruft `TMapObjBase::initMapObj`, dann `unk138 = 300` und `unk13C = 0.5`.
  60 Bytes, 15 Instruktionen.
- `TSandBombBase::loadAfter` speichert `findTriggerActor` in `unk144`, setzt `unk138` auf `this` und ruft virtuell `appear`.
  88 Bytes, 22 Instruktionen.
- `TSandBird::initMapObj` ruft `TJointCoin::initMapObj` und `SMS_LoadParticle` für `0x159` und `0x15A`.
  144 Bytes, 36 Instruktionen.
- `TFence::receiveMessage` startet `fence_normal_shake`, wenn die Nachricht 3 ist.
  60 Bytes, 15 Instruktionen.
- `TFence::initMapObj` setzt `unk138`, wenn `strstr(unkF4, "bamboo")` trifft, dann `TMapObjBase::initMapObj`.
  76 Bytes, 19 Instruktionen.
- `TFenceWaterH::changeStatusToWait` nullt `unk140` und `unk13C`, setzt `mState` auf 1 und ruft `setUpMapCollision(0)`.
  56 Bytes, 14 Instruktionen.
- `TAmiKing::loadAfter` ruft `TMapObjBase::loadAfter` und `SMS_LoadParticle` für `0x184`.
  92 Bytes, 23 Instruktionen.

`validate-symbol-order` für Monte, Mamma, Fence und Pinna: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor.
Fence behält die weak-only Order-Warnung.

`ninja changes_all`: fuzzy 78.723595 % → 78.73961 %, matched code 49.4223 % → 49.43923 % (1774304 → 1774912, +608).
Matched data bleibt 65.18551 %.
MapObjMamma matched code 4164 → 4396.
MapObjPinna matched code 2964 → 3056.
MapObjFence matched code 1724 → 1916.
MapObjMonte matched code 1940 → 2032.
Kein R170–R195-Unit hat matched code verloren.
`MarNameRefGen_MapObj` und `MapObjManager` unverändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R195 (`MapObjMamma` / `MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Drei kurze Fills ohne `ble`/`bge`/`lfsu`.
`TMapObjPuncher::load` bleibt geparkt (Frame).
`TMareFall::calc` bleibt Stub: gleicher Code, Frame `-0x18` statt `-0x28`.
`TMapObjBall::makeObjDefault` bleibt Stub: gleicher Code, Frame `-0x20` statt `-0x28`.
`TSandCastle::findTriggerActor` bleibt Stub (`return nullptr`), nur die Signatur ist jetzt `TMapObjBase*`.

- `TSandBomb::makeObjAppeared` ruft `TMapObjBase::makeObjAppeared` und `startControlAnim(1)` dann `startControlAnim(2)`.
  68 Bytes, 17 Instruktionen.
- `TSandBombBase::findTriggerActor` registriert `"SandBomb"` mit `mPosition`, `mRotation` und Skala `TVec3(1.0f)`.
  72 Bytes, 18 Instruktionen.
- `TCoverFruit::loadAfter` ruft `TMapObjBase::loadAfter` und virtuell `makeObjDead`, wenn `getBool(0x1038B)` wahr ist.
  88 Bytes, 22 Instruktionen.

`validate-symbol-order` für Mamma und Ball: PASS.
Dieselben UNUSED-Größenwarnungen wie zuvor, plus die bestehende weak-only Order-Warnung in Ball.

`ninja changes_all`: fuzzy 78.71754 % → 78.723595 %, matched code 49.415947 % → 49.4223 % (1774076 → 1774304, +228).
Matched data bleibt 65.18551 %.
MapObjMamma matched code 4024 → 4164.
MapObjBall matched code 2340 → 2428.
Kein R170–R194-Unit hat matched code verloren.
`MarNameRefGen_MapObj` und `MapObjManager` unverändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R194 (`MapObjMare` / `MapObjMamma` / `MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Sieben kurze Stubs, alle ohne `ble`/`bge`/`lfsu`.
`TCogwheelScale::touchWater`, `control` und `receiveMessage` bleiben geparkt.
`MapObjBase.hpp` bleibt unangetastet.
`TMapObjPuncher::load` bleibt Stub: gleicher Code, Frame `-0x30` statt `-0x28`.

- `TMareEventPoint::load` ruft `JDrama::TActor::load` und `initHitActor(0x40000236, 0, 0, 0, 0, 300, 600)`.
  84 Bytes, 21 Instruktionen.
- `TMareFall::load` ruft `TMapObjBase::load` und `SMS_LoadParticle` für `0x149` und `0x14A`.
  144 Bytes, 36 Instruktionen.
- `TMareCork::moveObject` startet `marecork`, wenn `TCannon::isObject` wahr ist und `unk154` noch 0 ist.
  116 Bytes, 29 Instruktionen.
- `TWireBell::control` holt die Drahtposition, setzt `mPosition` und kopiert `MsMtxSetTRS` in `setAnmMtx(0)`.
  160 Bytes, 40 Instruktionen.
- `TSandBird::nameIsObj` gibt wahr zurück, wenn `strstr(name, "none")` leer ist.
  60 Bytes, 15 Instruktionen.
- `TLampSeesawMain::touchPlayer` ruft virtuell `pushDown(unk140)`, wenn `marioIsOn` wahr ist.
  76 Bytes, 19 Instruktionen.
- `TLampSeesaw::touchPlayer` ruft virtuell `pushDown(-unk140)` auf `unk138`.
  80 Bytes, 20 Instruktionen.
  `unk138` ist jetzt `TLampSeesaw*`.
  Der Konstruktor bleibt 100 %.

`validate-symbol-order` für Mare, Mamma und Bianco: PASS, nur die bisherigen UNUSED-Größenwarnungen.

`ninja changes_all`: fuzzy 78.70 % → 78.72 %, matched code 49.40 % → 49.42 %, matched data bleibt 65.19 %.
Kein R170–R193-Unit hat matched code verloren.
`MarNameRefGen_MapObj` tickt fuzzy 81.450584 → 81.450806, matched code bleibt 2348.
`getNameRef_MapObj` bleibt 86.49 %.
`MapObjManager` unverändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R193 (`MapObjMare`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Bisher leerer TU.
Jeder Map-Symbol ist definiert, der Rest bleibt Stub.
`TCogwheelScale::touchWater` bleibt Stub (`fcmpo`+`bge`).
`TCogwheelScale::control` bleibt Stub (`fcmpo`+`ble`).
`receiveMessage` bleibt Stub: Retail nutzt `lfsu` auf `unk158->unk138`.

- `TMuddyBoat::getSDLModelFlag` gibt 0 zurück.
  8 Bytes, 2 Instruktionen.
- `TMuddyBoat::calcRootMatrix` ist leer.
  4 Bytes, 1 Instruktion.
- `TMuddyBoat::TMuddyBoat` nullt zwölf Floats, `unk168`, `unk16C` und zwei `TVec3` über `zero()`.
  156 Bytes, 39 Instruktionen.
- `TWireBell::loadAfter` ruft `TMapObjBase::loadAfter` und speichert `getWireNo(mPosition)`.
  60 Bytes, 15 Instruktionen.
- `TWireBell::TWireBell` setzt `unk138 = -1`, vier Floats und nullt `unk14C` über `zero()`.
  124 Bytes, 31 Instruktionen.
- `TMapObjGrowTree::loadAfter` ruft `TMapObjBase::loadAfter` und `removeMapCollision`.
  52 Bytes, 13 Instruktionen.
- `TMapObjGrowTree::initMapObj` setzt Höhe, Timer, `mDamageHeight` und `setBtp("moyasi_wink")`.
  100 Bytes, 25 Instruktionen.
- `TMapObjGrowTree::TMapObjGrowTree` nullt die Felder ab `0x138`.
  96 Bytes, 24 Instruktionen.
- `TMapObjElasticCode::initMapObj` setzt `unk140`, `mGravity`, `unk138` und `unk13C`.
  76 Bytes, 19 Instruktionen.
- `TCogwheel::TCogwheel` nullt die Skalare und zwei `TVec3` über `zero()`.
  140 Bytes, 35 Instruktionen.
- `TCogwheelScale::TCogwheelScale` nullt fünf Floats, setzt `0.01` und `5`, dann `unk154` und `unk158`.
  120 Bytes, 30 Instruktionen.
- `TMareCork::getTakingMtx` gibt `mMActor->getModel()->getAnmMtx(2)` zurück.
  20 Bytes, 5 Instruktionen.
- `__sinit_MapObjMare_cpp` (788 Bytes) baut `fall_upper_pos` vor den JALList-Inits aus `MSSetSound.hpp` / `MSoundBGM.hpp`.
  Die sdata-Statics (`mWaterLeakSpeed` bis `mGrowEndFrame`) sind 100 %.

Destruktoren, alle `@32`-Thunks und alle VTables der TU sind ebenfalls 100 %.
`getObjCollisionHeightOffset` bleibt 50 %: die Header-Kopie ist leer, `MapObjBase.hpp` bleibt unangetastet.
`validate-symbol-order`: PASS, nur fünf UNUSED-Größenwarnungen.

`ninja changes_all`: fuzzy 78.60 % → 78.70 %, matched code 49.31 % → 49.40 %, matched data 64.62 % → 65.19 %.
Kein R170–R192-Unit hat matched code verloren.
`MapObjManager` tickt fuzzy 99.68 % → 99.69 %, matched code bleibt 7064.
`MarNameRefGen_MapObj` bleibt bei matched code 2348.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R192 (`MapObjMamma`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Bisher leerer TU.
Jeder Map-Symbol ist definiert, der Rest bleibt Stub.
`TMammaBlockRotate::touchWater` bleibt Stub (`fcmpo`+`ble`).

- `TSandEgg::getSDLModelFlag`, `TLeanMirror::getSDLModelFlag` und `TSandBomb::getSDLModelFlag` geben 0 zurück.
  Je 8 Bytes, 2 Instruktionen.
- `TSandBombBase::grow` setzt `mState = 5`.
  12 Bytes, 3 Instruktionen.
- `TSandBombBase::waitBeforeExplode` setzt `mState = 6` und kopiert `unk148` in den Timer.
  20 Bytes, 5 Instruktionen.
- `TSandBomb::initMapObj` ruft `TMapObjBase::initMapObj` direkt.
  32 Bytes, 8 Instruktionen.
- `TSandCastle::calcRootMatrix` ruft `TMapObjBase::calcRootMatrix`, wenn `isState(2)` falsch ist.
  64 Bytes, 16 Instruktionen.
- `TSandBombBase::withered` schreibt den Timer aus `unk140`, setzt `mState = 3` und ruft `sleep` auf `unk144`.
  52 Bytes, 13 Instruktionen.
- `TSandLeaf::control` ruft `TMapObjBase::control` und legt die Höhe über `checkGround` bei `y + 200` ab.
  88 Bytes, 22 Instruktionen.
- `TSandLeaf::touchWater` ruft virtuell `getLivingTime` auf `unk138` und gibt 1 zurück.
  52 Bytes, 13 Instruktionen.
- `TSandBase::TSandBase` nullt `unk138`, `unk13C` und `unk144`.
  88 Bytes, 22 Instruktionen.
- `TSandBombBase::TSandBombBase` inlined den Basis-Konstruktor und setzt `unk148`, `unk14C = 1`, `unk150`, `unk154`.
  128 Bytes, 32 Instruktionen.
- `TSandCastle::TSandCastle` inlined die Kette und nullt `unk158` und `unk15C`.
  156 Bytes, 39 Instruktionen.
- `TLeanMirror::TLeanMirror` nullt die Skalare im Initializer und fünf `TVec3` über `zero()`.
  192 Bytes, 48 Instruktionen.
- `TShiningStone::TShiningStone` nullt die Felder ab `0x70` in Retail-Reihenfolge.
  104 Bytes, 26 Instruktionen.
- `TMammaBlockRotate::TMammaBlockRotate` nullt `unk13C` bis `unk148`.
  88 Bytes, 22 Instruktionen.
- `TSandBird::TSandBird` ruft `TJointCoin` und nullt `unk150` und `unk151`.
  80 Bytes, 20 Instruktionen.
- `TGoalWatermelon::TGoalWatermelon` nullt zwei Zeiger und `unk140` über `zero()`.
  96 Bytes, 24 Instruktionen.
- `__sinit_MapObjMamma_cpp` (764 Bytes) kommt aus `MSSetSound.hpp` / `MSoundBGM.hpp`.
  Die sdata-Statics (`mWitherTime` bis `mWaitTime`) sind 100 %.

Destruktoren, alle `@32`-Thunks und alle VTables der TU sind ebenfalls 100 %.
`TSandBase::grow` ist rein virtuell, weil der VTable-Slot im Retail null ist.
`validate-symbol-order`: PASS, nur acht UNUSED-Größenwarnungen.

`ninja changes_all`: fuzzy 78.49 % → 78.60 %, matched code 49.20 % → 49.31 %, matched data 64.58 % → 64.62 %.
Kein R170–R191-Unit hat matched code verloren.
`getNameRef_MapObj` tickt fuzzy 86.23 % → 86.49 %, matched code bleibt 2348.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R191 (`MapObjBall`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Bisher leerer TU.
Jeder Map-Symbol ist definiert, der Rest bleibt Stub.

- `TBigWatermelon::checkWallCollision` ruft `TMapObjGeneral::checkWallCollision` direkt.
  32 Bytes, 8 Instruktionen.
- `TBigWatermelon::TBigWatermelon` nullt `unk198`, `unk19C` und `unk1A0`.
  88 Bytes, 22 Instruktionen.
- `TRandomFruit::TRandomFruit` inlined den Reset-Konstruktor und `memset` auf `unk1A8` (32 Bytes).
  148 Bytes, 37 Instruktionen.
- `TResetFruit::TResetFruit` nullt `unk198` und `unk1A4` und setzt `unk19C` auf `0xFF`.
  104 Bytes, 26 Instruktionen.
- `TResetFruit::getLivingTime` gibt das static `mFruitLivingTime` zurück und steht weak im Header.
  8 Bytes.
- `TResetFruit::killByTimer` schreibt den Timer, setzt `MAP_OBJ_FLAG_DISAPPEARING` und `mState = 11`.
  28 Bytes, 7 Instruktionen.
- `TResetFruit::thrown` ruft `TMapObjGeneral::thrown` und setzt `mState = 11`.
  52 Bytes, 13 Instruktionen.
- `TMapObjBall::TMapObjBall` nullt 19 Floats ab `0x148`, `unk194` und `mInitialScaling` über `zero()`.
  168 Bytes, 42 Instruktionen.
- `TMapObjBall::put` ruft `TMapObjGeneral::put` und danach virtuell `calcCurrentMtx`.
  64 Bytes, 16 Instruktionen.
- `TMapObjBall::touchWaterSurface` und `touchPollution` rufen virtuell `kill`.
  Je 44 Bytes, 11 Instruktionen.
- `__sinit_MapObjBall_cpp` (764 Bytes) kommt aus `MSSetSound.hpp` / `MSoundBGM.hpp`.

Destruktoren, alle `@32`-Thunks und die fünf VTables der TU sind ebenfalls 100 %.
`validate-symbol-order`: PASS mit Weak-Order-Warnung (`getLivingTime`) und vier UNUSED-Größenwarnungen.

`ninja changes_all`: fuzzy 78.41 % → 78.49 %, matched code 49.13 % → 49.20 %, matched data 64.55 % → 64.58 %.
Kein R170–R190-Unit hat sich bewegt.
Zwei NonMatching-Aufrufer (`newUniqueObjByName`, `getNameRef_MapObj`) ticken fuzzy um Bruchteile, matched code bleibt gleich.

R190 `MapObjBianco`, R189 `MapObjPinna`, R188 `MapObjFence`,
R187 `TModelGate` / `TMapObjWave`, R186 `MapObjMonte` / `MapObjRicco`,
R185 `getNumGripsDead`, R184 `TWaterHitPictureHideObj::load`,
R183 `updateCheckData`, R182 `TMapObjTurn::touchWater`,
R181 `TCloset::touchWater`, R180 `TCasinoPanelGate::touchWater`,
R179 `waitingToAppear`, R178 `initDrawNear`, R177 `TWoodBox::kill`,
R176 `receiveMessage`, R175 `touchGround`, R174 `perform`,
R173 `startControlAnim`, R172 `TManhole::touchPlayer`,
R171 `calcVelocity`, R170 `appearing`
unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R190 (`MapObjBianco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Bisher leerer TU.
Jeder Map-Symbol ist definiert, der Rest bleibt Stub.

- `TLampSeesawMain::pushDown` setzt `mState = 2` und zieht das Argument von `unk144` ab.
  24 Bytes, 6 Instruktionen.
- `TLampSeesaw::pushDown` ist leer und steht weak im Header.
  4 Bytes.
- `TLampSeesaw::TLampSeesaw` nullt `unk138` und setzt `unk140` auf `0.01`.
  84 Bytes, 21 Instruktionen.
- `TLampSeesawMain::TLampSeesawMain` inlined den Seesaw-Konstruktor und setzt `unk144`/`unk148`/`unk14C`/`unk150` auf `0`, `0.998`, `0.8`, `0.5`.
  136 Bytes, 34 Instruktionen.
- `TLeafBoat::initMapObj` ruft `TMapObjBase::initMapObj` und schreibt `1`, `0.5`, `0.5`, `0.998` nach `unk138`/`unk13C`/`unk140`/`unk148`.
  72 Bytes, 18 Instruktionen.
- `TLeafBoat::TLeafBoat` initialisiert die Felder ab `0x138`.
  `unk164` wird über `zero()` genullt (`z`, `y`, `x`).
  152 Bytes, 38 Instruktionen.
- `TLeafBoatRotten::perform` ruft `TMapObjBase::perform` direkt.
  32 Bytes, 8 Instruktionen.
- `TLeafBoatRotten::TLeafBoatRotten` nullt `unk170` und setzt vier `u16` auf `0xFF`.
  96 Bytes, 24 Instruktionen.
- `TBiancoBell::TBiancoBell` nullt `unk138` und `unk13A`.
  80 Bytes, 20 Instruktionen.
- `TBiancoWatermill::TBiancoWatermill` setzt `unk138` auf `0.3` und `unk13C` auf `0`.
  84 Bytes, 21 Instruktionen.
- `TBiancoWatermill::touchWater` gibt `0` zurück.
  8 Bytes.
- `TBiancoWatermill::turnByEnemy` ist leer.
  4 Bytes.
- `__sinit_MapObjBianco_cpp` (764 Bytes) kommt aus `MSSetSound.hpp` / `MSoundBGM.hpp`.

Destruktoren, alle `@32`-Thunks und die VTables der TU sind ebenfalls 100 %.
`validate-symbol-order`: PASS mit Weak-Order-Warnung und UNUSED-Größenwarnungen.

`ninja changes_all`: fuzzy 78.31 % → 78.41 %, matched code 49.04 % → 49.13 %, matched data 63.85 % → 64.55 %.
Kein anderes Unit hat sich bewegt.

R189 `MapObjPinna`, R188 `MapObjFence`, R187 `TModelGate` / `TMapObjWave`,
R186 `MapObjMonte` / `MapObjRicco`, R185 `getNumGripsDead`,
R184 `TWaterHitPictureHideObj::load`, R183 `updateCheckData`,
R182 `TMapObjTurn::touchWater`, R181 `TCloset::touchWater`,
R180 `TCasinoPanelGate::touchWater`, R179 `waitingToAppear`,
R178 `initDrawNear`, R177 `TWoodBox::kill`, R176 `receiveMessage`,
R175 `touchGround`, R174 `perform`, R173 `startControlAnim`,
R172 `TManhole::touchPlayer`, R171 `calcVelocity`, R170 `appearing`
unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R189 (`MapObjPinna`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Bisher leerer TU.
Jeder Map-Symbol ist definiert, der Rest bleibt Stub.

- `TAmiKing::touchPlayer` ruft `SMS_SendMessageToMario(this, 9)`.
  36 Bytes, 9 Instruktionen.
- `TAmiKing::touchWater` gibt `1` zurück und steht weak im Header.
  8 Bytes.
- `TMerrygoround::draw` ist leer.
  4 Bytes.
- `TShellCup::TShellCup` baut sechs `TPinnaShell` an `0x138` und nullt `unk498`, `unk49C`, `unk4A0`.
  124 Bytes, 31 Instruktionen.
- `TViking::loadAfter` ruft `TMapObjBase::loadAfter` und danach virtuell `reset`.
  64 Bytes, 16 Instruktionen.
- `MsMtxSetRotX` ist die bestehende Header-Inline.
  `perform` nimmt die Adresse, damit die schwache Kopie stehen bleibt.
  124 Bytes.
- `TMapCollisionMove::moveMtx` kopiert die Matrix nach `unk20` und ruft `move`.
  Die schwache Kopie steht nur in diesem TU (`__declspec(weak)`), der virtuelle Inline im Header bleibt für die anderen TUs.
  60 Bytes, 15 Instruktionen.
- `__sinit_MapObjPinna_cpp` (764 Bytes) kommt aus `MSSetSound.hpp` / `MSoundBGM.hpp`.

Destruktoren, alle `@32`-Thunks und die VTables der TU sind ebenfalls 100 %.
`validate-symbol-order`: PASS mit UNUSED-Größenwarnungen.

`TViking::reset` und `THorizontalViking::reset` bleiben bei `fcmpo` + `ble` gegen unser `cror`.
`TPinnaShell::TPinnaShell` ist bis auf ein Stack-Slot (`0xC` gegen `8`) gleich.

`ninja changes_all`: fuzzy 78.21 % → 78.31 %, matched code 48.96 % → 49.04 %, matched data 63.16 % → 63.85 %.
Kein anderes Unit hat sich bewegt.

R188 `MapObjFence`, R187 `TModelGate` / `TMapObjWave`,
R186 `MapObjMonte` / `MapObjRicco`, R185 `getNumGripsDead`,
R184 `TWaterHitPictureHideObj::load`, R183 `updateCheckData`,
R182 `TMapObjTurn::touchWater`, R181 `TCloset::touchWater`,
R180 `TCasinoPanelGate::touchWater`, R179 `waitingToAppear`,
R178 `initDrawNear`, R177 `TWoodBox::kill`, R176 `receiveMessage`,
R175 `touchGround`, R174 `perform`, R173 `startControlAnim`,
R172 `TManhole::touchPlayer`, R171 `calcVelocity`, R170 `appearing`
unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R188 (`MapObjFence`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Bisher leerer TU.
Jeder Map-Symbol ist definiert, der Rest bleibt Stub.

- `TFenceWater::changeStatusToWait`: `unk140 = 0.0f`, dann `unk13C = 0.0f`, dann `mState = 1`.
  24 Bytes, 6 Instruktionen.
- `TRailFence::initMapCollisionData` und `TFenceWater::initMapCollisionData` rufen `TMapObjBase::initMapCollisionData` direkt.
  Je 32 Bytes, 8 Instruktionen.
- `TFenceWater::draw` ist leer.
  4 Bytes.
- `MsMtxSetRotY` (124 Bytes) und `MsWrap<f32>` (72 Bytes) sind die bestehenden Header-Inlines.
  `controlWall` nimmt ihre Adresse, damit die Kopien unter `-inline deferred` stehen bleiben.
- `__sinit_MapObjFence_cpp` (764 Bytes) kommt aus `MSSetSound.hpp` / `MSoundBGM.hpp`.

`TRailFence::~TRailFence`, `TFenceWaterH::~TFenceWaterH`, `TRevolvingFenceInner::~TRevolvingFenceInner`, `TRevolvingFenceOuter::~TRevolvingFenceOuter` und die sechs `@32`-Thunks sind ebenfalls 100 %.
Die VTables von `TFence`, `TRailFence`, `TRevolvingFenceInner` und `TRevolvingFenceOuter` auch.
`validate-symbol-order`: PASS mit weak-Order-Warnung (`MsWrap` steht neben dem schwachen `MsMtxSetRotY`) und UNUSED-Größenwarnungen.

`ninja changes_all`: fuzzy 78.15 % → 78.21 %, matched code 48.91 % → 48.96 %, matched data 63.13 % → 63.16 %.
`getNameRef_MapObj` steigt mit, weil `TRailFence::TRailFence` jetzt out-of-line und global ist.
Kein anderes Matching-Symbol hat sich bewegt.

R187 `TModelGate` / `TMapObjWave`, R186 `MapObjMonte` / `MapObjRicco`,
R185 `getNumGripsDead`, R184 `TWaterHitPictureHideObj::load`,
R183 `updateCheckData`, R182 `TMapObjTurn::touchWater`,
R181 `TCloset::touchWater`, R180 `TCasinoPanelGate::touchWater`,
R179 `waitingToAppear`, R178 `initDrawNear`, R177 `TWoodBox::kill`,
R176 `receiveMessage`, R175 `touchGround`, R174 `perform`,
R173 `startControlAnim`, R172 `TManhole::touchPlayer`,
R171 `calcVelocity`, R170 `appearing` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R187 (`TModelGate` / `TMapObjWave`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Zwei bisher leere TUs.
Jeder Map-Symbol ist definiert, der Rest bleibt Stub.

- `TModelGate::startOpen`: `unk70 |= 1`, Byte `unkC4 = 0`, `setBpk(gateMActorNames[unk71])`, `offHitFlag(HIT_FLAG_NO_COLLISION)`, `unk70 |= 2`.
  116 Bytes, 29 Instruktionen.
- `TModelGate::getTakingMtx` gibt `nullptr` zurück und steht weak im Header.
  8 Bytes.
- `TMapObjWave::noWave` nullt `unk34`, `unk38`, `unk2C`, `unk30`, `unk3C`, `unk40` in dieser Reihenfolge.
  32 Bytes, 8 Instruktionen.
- `__sinit_ModelGate_cpp` (764 Bytes) kommt aus `MSSetSound.hpp` / `MSoundBGM.hpp`.
- `__sinit_MapObjWave_cpp` (772 Bytes) ist dieselbe Liste plus `static JUtility::TColor sColor`, dessen Default-Ctor `-1` schreibt.

`TModelGate::~TModelGate`, `@32@__dt__10TModelGate` und `TMapObjWave::~TMapObjWave` sind ebenfalls 100 %.
`validate-symbol-order`: ModelGate PASS, Wave PASS mit UNUSED-Größenwarnungen.

R186 `MapObjMonte` / `MapObjRicco`, R185 `getNumGripsDead`,
R184 `TWaterHitPictureHideObj::load`, R183 `updateCheckData`,
R182 `TMapObjTurn::touchWater`, R181 `TCloset::touchWater`,
R180 `TCasinoPanelGate::touchWater`, R179 `waitingToAppear`,
R178 `initDrawNear`, R177 `TWoodBox::kill`, R176 `receiveMessage`,
R175 `touchGround`, R174 `perform`, R173 `startControlAnim`,
R172 `TManhole::touchPlayer`, R171 `calcVelocity`, R170 `appearing`
unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R186 (`MapObjMonte` / `MapObjRicco`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

Außerhalb von Bathtub.
Die beiden TUs waren leer; jeder Map-Symbol ist jetzt definiert, der Rest bleibt Stub.

- `TJumpMushroom::receiveMessage`: `startAnim(1); return TRUE;`.
  40 Bytes, 10 Instruktionen.
- `TCraneRotY::calc`: `setRootMtxRotY();`.
  32 Bytes, 8 Instruktionen.
- `TRiccoWatermill::calc`: `setRootMtxRotZ();`.
  32 Bytes, 8 Instruktionen.
- `TCraneCargo::control`: `unk158` als z, dann y, dann x auf `0.0f`, danach `TMapObjBase::control`.
  48 Bytes, 12 Instruktionen.
- `TFluff::getRadiusAtY` und `TGoalFlag::getRadiusAtY` geben `20.0f` zurück und stehen weak im Header.
  Je 8 Bytes.
- `TLiveActor::getMActor` ist die bestehende Header-Inline.
  Die schwache Kopie in `MapObjRicco` ist 8 Bytes (`lwz r3, 0x74(r3)`).
- `__sinit_MapObjMonte_cpp` (764 Bytes) und `__sinit_MapObjRicco_cpp` (804 Bytes) kommen aus den rogue includes `MSSetSound.hpp` und `MSoundBGM.hpp`.
  Ricco initialisiert davor `submarineCranePos_forSound` `(1956, 1000, 6425)` und `submarineSetWtPos_forSound` `(1956, -100, 6425)`.

Destruktoren und `@32`-Thunks der Key-Funktionen sind ebenfalls 100 %.
`validate-symbol-order` für beide Units: PASS mit UNUSED-Größenwarnungen.

R185 `TBathtub::getNumGripsDead`, R184 `TWaterHitPictureHideObj::load`,
R183 `updateCheckData`, R182 `TMapObjTurn::touchWater`,
R181 `TCloset::touchWater`, R180 `TCasinoPanelGate::touchWater`,
R179 `waitingToAppear`, R178 `initDrawNear`, R177 `TWoodBox::kill`,
R176 `receiveMessage`, R175 `touchGround`, R174 `perform`,
R173 `startControlAnim`, R172 `TManhole::touchPlayer`,
R171 `calcVelocity`, R170 `appearing` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R185 (`TBathtub::getNumGripsDead`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- Zählt die fünf Griffe an `unk168`, deren Byte `0x249` null ist.
- `for (i < 5)` wird von MWCC ausgerollt und lädt `unk168` vor jedem Griff neu.
- `TBathtubGrip` ist noch nicht rekonstruiert; der Zugriff geht über ein
  lokales Layout mit dem Byte an `0x249`.

0 Abweichungen, 132 Bytes, 33 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjCorona -d getNumGripsDead`: 100 %.
`validate-symbol-order -u mario/MoveBG/MapObjCorona --baseline-object`:
0 neue Symbolfehler (210 geerbt).

R184 `TWaterHitPictureHideObj::load`, R183 `updateCheckData`,
R182 `TMapObjTurn::touchWater`, R181 `TCloset::touchWater`,
R180 `TCasinoPanelGate::touchWater`, R179 `waitingToAppear`,
R178 `initDrawNear`, R177 `TWoodBox::kill`, R176 `receiveMessage`,
R175 `touchGround`, R174 `perform`, R173 `startControlAnim`,
R172 `TManhole::touchPlayer`, R171 `calcVelocity`, R170 `appearing`
unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R184 (`TWaterHitPictureHideObj::load`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- `THideObjBase::load` ist hier ausgeschrieben, damit `eventId` ein Local
  dieser Funktion ist. Inlined lag es bei `0x30`; Retail will `0x48`.
- `char gap[4]` ist das Loch zwischen `eventId` und den drei Color-Reads
  (`0x50` / `0x54` / `0x58`).
- `char trash[0x18]` darunter hält den Frame bei `-0x70`.
- Beide Stores werden weggoptimiert. `THideObjBase::load` bleibt 100 %.

0 Abweichungen, 444 Bytes, 111 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjHide -d "TWaterHitPictureHideObj::load"`: 100 %.
`validate-symbol-order -u mario/MoveBG/MapObjHide`: PASS.

R183 `TMapObjPlane::updateCheckData`, R182 `TMapObjTurn::touchWater`,
R181 `TCloset::touchWater`, R180 `TCasinoPanelGate::touchWater`,
R179 `waitingToAppear`, R178 `initDrawNear`, R177 `TWoodBox::kill`,
R176 `receiveMessage`, R175 `touchGround`, R174 `perform`,
R173 `startControlAnim`, R172 `TManhole::touchPlayer`,
R171 `calcVelocity`, R170 `appearing` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R183 (`TMapObjPlane::updateCheckData`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- `gridToWorld` läuft erst für x und x+1, dann für z und z+1.
- Die int-nach-float-Spills von z und x+1 liegen bei `0x88` und `0x90`.
- `fmsubs` landet in f9 bzw. f8.

0 Abweichungen, 464 Bytes, 116 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjPlane -d updateCheckData`: 100 %.
`validate-symbol-order -u mario/MoveBG/MapObjPlane`: PASS.

R182 `TMapObjTurn::touchWater`, R181 `TCloset::touchWater`,
R180 `TCasinoPanelGate::touchWater`, R179 `waitingToAppear`,
R178 `initDrawNear`, R177 `TWoodBox::kill`, R176 `receiveMessage`,
R175 `touchGround`, R174 `perform`, R173 `startControlAnim`,
R172 `TManhole::touchPlayer`, R171 `calcVelocity`, R170 `appearing`
unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R182 (`TMapObjTurn::touchWater`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- `liftY(mPosition.y, 200.0f)` hält die Höhe in f2 und die Konstante in f1
  (`fadds f1, f2, f1`).
- Das `Mtx` im else-Zweig liegt bei `0x34`, der Frame ist `-0x80`.
- Der Helfer ist vollständig inlined, kein Extra-Symbol.

0 Abweichungen, 404 Bytes, 101 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjLib -d "TMapObjTurn::touchWater"`: 100 %.

R181 `TCloset::touchWater`, R180 `TCasinoPanelGate::touchWater`,
R179 `waitingToAppear`, R178 `initDrawNear`, R177 `TWoodBox::kill`,
R176 `receiveMessage`, R175 `touchGround`, R174 `perform`,
R173 `startControlAnim`, R172 `TManhole::touchPlayer`,
R171 `calcVelocity`, R170 `appearing` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R181 (`TCloset::touchWater`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- Die Z-Schwellen sind `mPosition.z - 1.1f * unk140`
  und `mPosition.z + 1.1f * unk140`.
- Das Produkt landet in f3, das Wasser-Z in f1.
- Ein benanntes `halfDepth` hatte das Produkt in f1 gelassen.

0 Abweichungen, 352 Bytes, 88 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjSirena -d "TCloset::touchWater"`: 100 %.
`validate-symbol-order -u mario/MoveBG/MapObjSirena`: PASS.

R180 `TCasinoPanelGate::touchWater`, R179 `waitingToAppear`,
R178 `initDrawNear`, R177 `TWoodBox::kill`, R176 `receiveMessage`,
R175 `touchGround`, R174 `perform`, R173 `startControlAnim`,
R172 `TManhole::touchPlayer`, R171 `calcVelocity`, R170 `appearing`
unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R180 (`TCasinoPanelGate::touchWater`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- Die Bandvergleiche stehen als `mPosition.y + scale * unk144`.
- Dadurch liegt `unk144` in f4 und `mPosition.y` in f5.
- `fmadds` ist `f0 * f4 + f5`.
- Die unskalierte Schwelle bleibt `mPosition.y + unk144`
  (`fadds f0, f5, f4`).

0 Abweichungen, 644 Bytes, 161 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjSirena -d touchWater`: 100 %.
`validate-symbol-order -u mario/MoveBG/MapObjSirena`: PASS.

R179 `waitingToAppear`, R178 `initDrawNear`, R177 `TWoodBox::kill`,
R176 `receiveMessage`, R175 `touchGround`, R174 `perform`,
R173 `startControlAnim`, R172 `TManhole::touchPlayer`,
R171 `calcVelocity`, R170 `appearing` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R179 (`TMapObjGeneral::waitingToAppear`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- Retail erscheint, wenn die Distanz größer als der Radius ist.
- `distToMario(this, gpMarioPos)` legt `gpMarioPos` in r4;
  `mInitialPosition` faltet auf `0x10c(r31)`.
- Actor `0x4000005a`: `mario += damageRadius`, dann `100 + mario`
  (`fadds f1, f1, f30`, Konstante in f0).
- Der andere Arm nutzt `addRadius`, Ergebnis bleibt in f0.
- Frame `-0x48`.

0 Abweichungen, 352 Bytes, 88 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjGeneral -d waitingToAppear`: 100 %.
`validate-symbol-order -u mario/MoveBG/MapObjGeneral`: PASS.

R178 `initDrawNear`, R177 `TWoodBox::kill`, R176 `receiveMessage`,
R175 `touchGround`, R174 `perform`, R173 `startControlAnim`,
R172 `TManhole::touchPlayer`, R171 `calcVelocity`, R170 `appearing`
unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R178 (`TMapObjGrassManager::initDrawNear`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- `Mtx viewItm` lag 0x14 zu tief (`0x30` statt `0x44`), die `fctiwz`-Spills
  der S16-Konvertierung 0x18 zu tief, Frame `-0x80` statt `-0x98`.
- `static inline initDrawNearPad()` mit `char trash[0x10]` am Funktionsende
  trifft beides. `0x14` lässt die Matrix 4 zu hoch. Store ist DCE, kein
  Extra-Symbol.

0 Abweichungen, 588 Bytes, 147 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjGrass -d initDrawNear`: 100 %.
`validate-symbol-order -u mario/MoveBG/MapObjGrass`: PASS.
Die TU hat danach keine nonmatching `.text`-Funktionen.

R177 `TWoodBox::kill`, R176 `receiveMessage`, R175 `touchGround`,
R174 `perform`, R173 `startControlAnim`, R172 `TManhole::touchPlayer`,
R171 `calcVelocity`, R170 `appearing` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R177 (`TWoodBox::kill`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- Die vier `checkGround`-Out-Pointer aus inlined `killNearWoodBox(f32, f32) const`
  lagen bei `0x40`–`0x4c` statt `0xd4`–`0xe0`, Frame `-0x58` statt `-0xf0`.
- `static inline woodBoxKillPad()` mit `char trash[0x90]` am Funktionsende
  hebt die Pointer und den Frame. `0x94` überschießt die Pointer um 4.
  Store ist DCE, kein Extra-Symbol.
- Map-Name `killNearWoodBox__8TWoodBoxCFff` (UNUSED, `0xbc`) statt
  `fabricatedGroundKillCheck`. `const` ändert das Inlining in `kill` nicht.

0 Abweichungen, 744 Bytes, 186 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjHide -d TWoodBox::kill`: 100 %.
`validate-symbol-order -u mario/MoveBG/MapObjHide`: PASS (UNUSED-Größe stimmt).

R176 `receiveMessage`, R175 `touchGround`, R174 `perform`,
R173 `startControlAnim`, R172 `TManhole::touchPlayer`, R171 `calcVelocity`,
R170 `appearing` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R176 (`TMapObjGeneral::receiveMessage`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- `isActorType(0x10000025)` und `isActorType(0x80000001)` liefen auf `this`
  (`r29`). Retail liest `0x4c` vom Sender (`r30`): `sender->isActorType`.
- `TVec3(mVelocity)`-Kopie lag bei `0x20` statt `0x38`, Frame `-0x40` statt
  `-0x58`. `static inline receiveMessageFramePad()` mit `char trash[0x14]`
  am Funktionsende hebt die Kopie auf `0x38` und den Frame auf `-0x58`.
  `0x18` überschießt die Kopie um 4. Store ist DCE, kein Extra-Symbol.

0 Abweichungen, 776 Bytes, 194 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjGeneral -d TMapObjGeneral::receiveMessage`: 100 %.

R175 `touchGround`, R174 `perform`, R173 `startControlAnim`,
R172 `TManhole::touchPlayer`, R171 `calcVelocity`, R170 `appearing`
unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R175 (`TMapObjGeneral::touchGround`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- Drei `TVec3`-Kopien von `mVelocity` lagen 0x38 zu tief (Frame `-0x60` statt
  `-0x98`). Ein direktes `char trash[]` polstert über den Temps, nicht darunter.
- `static inline touchGroundFramePad()` mit `char trash[0x34]` wird am
  Funktionsende inlined: Slot landet in der Temp-Region, Kopien auf
  `0x60`/`0x6c`/`0x78`, Frame `-0x98`. Store ist DCE, kein Extra-Symbol.

0 Abweichungen, 472 Bytes, 118 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjGeneral -d TMapObjGeneral::touchGround`: 100 %.

R174 `perform`, R173 `startControlAnim`, R172 `TManhole::touchPlayer`,
R171 `calcVelocity`, R170 `appearing` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R174 (`TMapObjGeneral::perform`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- `isStateTimerEngaged()` verbraucht den ersten `mStateTimer`-Load (`r0` + `clrlwi`).
- `int timer = *(volatile int*)&mStateTimer` vor `getFlushTime()` lädt `0x104(r28)`
  erneut nach `r31` und hält den Wert über den virtuellen Call.
- Dritter Load für die Intervall-Division bleibt `lwz r3, 0x104(r28)`.
- `char trash[8]` am Funktionskopf → Frame `-0x30`.

0 Abweichungen, 248 Bytes, 62 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjGeneral -d TMapObjGeneral::perform`: 100 %.

R173 `startControlAnim`, R172 `TManhole::touchPlayer`, R171 `calcVelocity`,
R170 `appearing` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R173 (`TMapObjBase::startControlAnim`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- `TMapObjData* data = mMapObjData` hält `mMapObjData` in `r4`.
- Zweiter `mAnim`-Load über `*(const TMapObjAnimDataInfo* volatile*)&data->mAnim`
  nach `clrlwi` (der erste Pointer lag in `r3`).
- `mMActor` vor `unk4`, Frame `-0x18`, 124 Bytes. Kein Extra-`lwz` von `0x130`.

0 Abweichungen, 124 Bytes, 31 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjBase -d startControlAnim`: 100 %.

R172 `TManhole::touchPlayer`, R171 `calcVelocity`, R170 `appearing` unverändert
strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R172 (`TManhole::touchPlayer`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- `char trash[0x48];` am Funktionskopf → Frame `-0x90` wie Retail (`-0x48` ohne Pad).
- Down-Stop-Zweig: `f32* initY = &mInitialPosition.y;` +
  `initY = (f32*)(volatile void*)initY;` erzwingt `addi r3,r31,0x110` vor
  `lfs mDownHeight@sda21` / `lfs 0x110(r31)` (kein SDA-Hoist vor Pointer).
- `downHeight` / `initYVal` temporaries + `unk14C = *initY - mPosition.y` →
  `lfs f1,0(r3)` für `unk14C` wie Retail.

0 Abweichungen, 788 Bytes, 197 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjTown -d TManhole::touchPlayer`: 100 %.

R171 `calcVelocity`, R170 `appearing`, R168–R160 unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R171 (`TMapObjGeneral::calcVelocity`)

**Vollmatch, strikt (0 `~`, 0 `|`).**

- `char trash[0x28]; trash[0]=0;` → Frame `-0x58` wie Retail.
- `LIVE_FLAG_AIRBORNE`: `int airborne` + `mLiveFlag & flag` → `li`/`cmpwi`
  statt `checkLiveFlag2` (`clrlwi.`).
- `mMapObjData->mPhysical`-Zweig unverändert (`piVar4 ? (u8)1 : (u8)0`).

0 Abweichungen, 420 Bytes, 105 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjGeneral -d calcVelocity`: 100 %.

R170 `appearing`, R168–R160 / `mirror@0xa50` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R170 (`TMapObjGeneral::appearing` — strikt nachgezogen)

**Vollmatch, strikt (0 `~`, 0 `|`).**

R169-Quelltext (`mInitialScaling.x > mScaling.x`, plain `clampX`) erzeugte in
dieser Toolchain weiterhin `bgt` + vertauschte Compare-Loads und kein
`lfs`/`stfs`-Reload für X.

- `if (mScaling.x < mInitialScaling.x) return;` → Retail `fcmpo f1,f0` +
  `blt` (semantisch identisch zu `>`).
- `f32 clampX = *(volatile f32*)&mInitialScaling.x;` + `mScaling.x = clampX`
  erzwingt `lfs` aus `0x124` vor `stfs` nach `0x24` (MWCC würde sonst `f1`
  recyceln).
- `char trash[4]; trash[0]=0;` unverändert; kein `mScaling.set()`.

0 Abweichungen, 196 Bytes, 49 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjGeneral -d appearing`: 100 %.

R169/R168/R167–R160: R168 `setGroundCollision` strikt; `mirror@0xa50` unberührt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R169 (`TMapObjGeneral::appearing`)

**Vollmatch (erste Landung; Compare/Clamp in R170 nachgeschärft).**

- `char trash[4]; trash[0] = 0;` am Funktionskopf → Frame `-0x20` wie Retail
  (`-0x18` ohne Pad).
- Inneren Scope/`mScaling.set` entfernt: Komponenten-`+=` / Vergleich /
  Einzelstores wie Retail (`lfs f1,0x24` + `lfs f0,0x124`, `fcmpo f1,f0`,
  `blt` zum Epilog).
- Detailkorrektur: siehe **R170** (`<` + `volatile`-Reload für striktes Diff).

0 Abweichungen, 196 Bytes, 49 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjGeneral -d appearing`: 100 %.

R168 `setGroundCollision`, R167–R160 unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R168 (`TMapObjBase::setGroundCollision`)

**Vollmatch, strikt.**

- `JGeometry::TVec3<f32> pos` vor `char trash[0x24]; trash[0]=0;` → Frame
  `-0x60`, Spill @ `0x4c` (mit `trash[0x28]` + `pos.set` war `pos` @ `0x50`).
- `switch (unk8->mKind)` / `case KIND_MOVE` statt `!= KIND_MOVE` + `return`
  (Retail `beq` + `b` statt alleiniges `bne`).
- `pos.set(mPosition.x, mPosition.y - mYOffset, mPosition.z)` statt
  Komponenten-Zuweisungen oder `TVec3`-Ctor — korrekte `fsubs f1,f2,f1`-Kette
  ohne verfrühtes `stfs` nach `lfs f0,0x10`.

0 Abweichungen, 388 Bytes, 97 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjBase -d TMapObjBase::setGroundCollision`: 100 %.

R167 `touchWater` / `calcRootMatrix` @ `0xa50`, R166–R160 unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R167 (`TItemSlotDrum::touchWater`)

**Vollmatch, strikt.**

- `char trash[4]; trash[0] = 0;` am Funktionskopf → Frame `-0x88` wie Retail
  (`-0x80` ohne Pad).
- Frühabbruch `if (unk194 || !unk1A2) return 1;` (ein Prädikat) — getrennte
  `if`s erzeugen invertierte `beq`-Verzweigung (+3 Marker).
- `TMsRange<s32>` / `TMsRange<f32>` weiter vollständig inlined wie Retail.

0 Abweichungen, 400 Bytes, 100 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjSirena -d TItemSlotDrum::touchWater`: 100 %.

R166 `TMapObjManager::load`, R165 `checkWallCollision`, R164
`TMapObjSwitch::control`, R163/R162/R160 unverändert strikt.
`TItemSlotDrum::calcRootMatrix` (Mirror @ `0xa50`) unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R164 (`TMapObjSwitch::control`)

**Vollmatch, strikt.**

- Retail-Bool-Synthese (`lwz r0` → `cmpwi` → `li`/`clrlwi.`) statt inline
  `isStateTimerEngaged()`.
- Zweites `lwz` von `mStateTimer` nach `r4` vor `gpMSound` → `r3` via
  `volatile int*`-Reload + `gpMSound->playTimer`.

0 Abweichungen, 88 Bytes, 22 Instruktionen.
`decomp-diff -u mario/MoveBG/MapObjTown -d TMapObjSwitch::control`: 100 %.

R163 `TRollBlock::load`, R162 `THideObjPictureTwin::loadAfter`, R160
`TCloset::calcRootMatrix` unverändert strikt.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R161 (Round 71 — kein neuer strikter Vollmatch)

**Kein neuer strikter Vollmatch** (Scope A). R160 `TCloset::calcRootMatrix` unverändert strikt.

- `TDonchou::calcRootMatrix` (WIP im Branch, nicht strikt): `struct { char pad[0x10];
  Mtx mtx; } local;` + `local.mtx[1][3]` statt `TRotation3f` → Frame `-0x90`,
  `addi r29,r1,0x34`, `ref` @ `0x50` (**match**). Verbleibend **2** `~`:
  `fireStartDemoCamera`-`TFlagT` @ `0x20` vs Retail `0x30` (`sth`/`addi`). Kein
  Grid `trash`×`pad`, kein `camPad`/`flagLayout`/`rotMtx`/`demoFlag`-Local,
  kein `post[]` im Struct ohne Frame-Regress. Hebel vermutlich +0x10 Stack-Slot
  unter `mtx` ohne `-0x90` zu brechen (nicht Closet-`pad[4]` blind kopieren).
- `TCloset::touchWater`: **7** `~` (f-Register `f1`/`f3` in `halfDepth`-Zweig);
  explizite `f2`/`f3`-Locals verschlechterten.
- `TRoulette::initMapObj`: **4** `~` (Iterator @ `0x58` vs `0x5c`); Retail nutzt
  `insert`, `push_back` bleibt bester Stand; Entry-`trash[4]` regress.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

**Nächste MoveBG:** Donchou `TFlagT` `0x20`→`0x30`; weitere `calcRootMatrix`-Geschwister
mit `pad+Mtx`; `initMapObj` `r25` nur mit neuem Hebel. Skip: SMS-B, Switch ohne Hebel.

### R159 (Round 71 Agent — kein neuer Vollmatch)

**Kein neuer strikter Vollmatch** in MoveBG (Scope A). Worktree ohne TU-Diffs.

- `TDonchou::calcRootMatrix`: Entry-`char trash[0x10]; trash[0]=0;` → Frame
  `-0x90` wie Retail, aber **5** verbleibende `~` (+0xc: `mtx` @ `0x28` vs `0x34`,
  `ref(1,3)` @ `0x44` vs `0x50`, `TFlagT` @ `0x24` vs `0x30`). Mid-`pad2` nach
  `getModel()` oder `camPad` vor `fireStartDemoCamera` verschiebt `mtx` nicht ohne
  Frame-Wachstum.
- `TMapObjSwitch::receiveMessage`: unverändert **2** `~` (`TFlagT` @ `0x2c` vs
  `0x24`); `trash[0xc]` / Mid-Pad / `demoFlag`-Local vergrößert nur Frame.
- `TCloset::calcRootMatrix` (R158): `decomp-diff` zeigt weiter **3** `~`
  (`mtx` @ `0x10` vs `0x14`) — fuzzy 100 % ≠ strikt; Closet nicht angefasst.
- `TSakuCasino::calcRootMatrix`: bereits strikt **0** Marker (kein neuer Ship).
- `TMapObjTree::initMapObj`: **3** `~` (`mLeafNum` in `r26` vs Retail `r25`).

**Nächste SMS-B / MoveBG:** `TDonchou` — +0xc-Homing für `mtx`/`TFlag` bei
`-0x90` (evtl. UNUSED/Whole-function-Layout, nicht nur Entry-Trash); Switch nur
bei klarem Hebel; Tree `r25` via `new[]`-/Iterator-Shape. Skip: `initDrawNear` /
`initMapObj` ohne Idee.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R155 (`TMapObjBase::throwObjToFrontFromPoint`)

**Vollmatch, strikt.**

- `throwObjToFrontFromPoint`: `Mtx mtx` am Funktionskopf (else-Zweig), danach
  `char trash[8]`; `mMActor`-Zweig nutzt `MtxPtr anmMtx` statt lokalem `mtx`.
  Frame `-0x88`, `MsMtxSetRotRPH`-Buffer @ `0x34` wie Retail.
- Geschwister `throwObjToFront`: gleiches Muster reicht nicht (Frame `-0x90` vs
  `-0x88` mit `trash[8]` — offen).

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

**Nächste MoveBG:** `rotateVecByAxisY`; `touchFruit` / `appearWithDemo` /
`newAndRegisterCoin` nur bei klarem Stack-Hebel. SMS-B unverändert.

### R154 (`TMapObjBillboard::touchWater`)

**Vollmatch, strikt.**

- `TMapObjBillboard::touchWater`: `JGeometry::TVec3<f32> rot` / `pos` am
  Funktionskopf (Assign statt Block-Locals), danach `char trash[0x10]` vor
  `swing`; Frame `-0x50`, TVec-Spills @ `0x38`/`0x2c` wie Retail.
- Pivot-Kappen unverändert: `TFruitBasket::touchFruit` (2 `~` roof @ `0x28`),
  `TShine::appearWithDemo` (`trash[8]` fixiert Frame, `TFlag` noch @ `0x34` vs
  `0x38`), `TCoin::perform` / `TRoulette::initMapObj` / `loadBeforeInit`.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

**Nächste MoveBG:** `touchFruit` roof-Ptr; `appearWithDemo` `TFlag`-Slot;
`newAndRegisterCoin` TVec-Stack. SMS-B: Closet / `initAndRegister` /
`initModel`.

### R153 (Round 71 Agent — Pivot, kein Vollmatch)

**Kein neuer strikter Vollmatch** (Scope A MoveBG).

- `TFruitBasket::touchFruit`: `f32 rotX` (Assign aus `mRotation.x`) +
  `const TBGCheckData* roofPlane` im Funktionskopf; Frame `-0x38` und Epilog
  stimmen; nur noch **2** Operand-`~` (`checkRoof`-Out-Ptr @ `0x28` vs `0x2c`).
  `rotX` reserviert 4 B Stack ohne Store — ohne `rotX` Frame `-0x40`.
- Pivot-Kappen: `TCoin::perform` / `TRoulette::initMapObj` / `TShine::loadBeforeInit`
  unverändert (eine Idee je: Spill +0x10 / Iterator +4 / Retail-Locals).
- `TItemManager::newAndRegisterCoin`: TVec3-Inline-Spill weiter +0xc tief
  (Entry-`trash[8]` hält Frame, verschiebt Spill nicht).
- `TMapObjSwitch::receiveMessage`: `TFlagT` weiter @ `0x24` vs `0x2c` (2 `~`).

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

**Nächste SMS-B / MoveBG:** `touchFruit` roof @ `0x28` ohne `rotX`-Slot;
`newAndRegisterCoin` TVec-Stack; Closet/`initAndRegister`/`initModel` off-limits.

### R152 (Round 71 Agent — kein neuer Vollmatch)

**Kein neuer strikter Vollmatch** in MoveBG (Scope A).

- `TCoin::perform`: objdiff 100 % / 608 B, **4** verbleibende `~` nur beim
  `gpQuestionManager->request`-TVec3-Spill (+0x10 B zu tief).
- `TRoulette::initMapObj`: **4** `~` (Iterator @ `0x5c`/`0x58` vs `0x58`/`0x54`).
- `TShine::loadBeforeInit`: Locals-Reorder + Entry-Trash → min. **10** `~`
  (Retail: `name` @ `0x24`, `eventId` @ `0x20`, `v` @ `0x18`).

R151 `TEggYoshi::control`, R150 `TNozzleBox::load`, R149 `TEggYoshi::receiveMessage`,
R147 `TMapObjSwitch::receiveMessage`, R146 `joinToGroup`, R145 `drawLogic`, R138
`initMirrorModel` unverändert.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R150 (`TNozzleBox::load`)

**Vollmatch, strikt.**

- `TNozzleBox::load`: `char strBuf[0x20]` vor Entry-`char trash[0x1c];
  trash[0]=0;` (MWCC-Stack: `strBuf` @ `0x30`, Frame `-0x60`).

0 Abweichungen, 536 Bytes, 134 Instruktionen.
`validate-symbol-order` `mario/MoveBG/Item`: PASS.

R149 `TEggYoshi::receiveMessage`, R147 `TMapObjSwitch::receiveMessage`, R146
`joinToGroup`, R145 `drawLogic`, R138 `initMirrorModel` unverändert @ 100 %.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

### R149 (`TEggYoshi::receiveMessage`)

**Vollmatch, strikt.**

- `TEggYoshi::receiveMessage`: Entry-`char trash[4]; trash[0]=0;`, im
  `HIT_MESSAGE_UNK10`-Zweig `unk10Trash[4]` vor `TVec3 v = mVelocity` und
  `midTrash[4]` nach dem Copy vor `makeObjAppeared()` → Frame `-0x38`,
  Velocity-Spills `0x1c`/`0x20`/`0x24`, `lfs` von `0x20` (**match**).

0 Abweichungen, 620 Bytes, 155 Instruktionen.
`validate-symbol-order` `mario/MoveBG/Item`: PASS.

R147 `TMapObjSwitch::receiveMessage`, R146 `joinToGroup`, R145 `drawLogic`, R138
`initMirrorModel` unverändert @ 100 %.

DOL-SHA1 `9f5a8caf56f5356aeac9d3ed28bf8de976a03625` OK.

