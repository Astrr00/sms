# Arbeitsauftrag: Super Mario Sunshine vollständig rekonstruieren

## 1. Auftrag und verbindliches Ziel

Arbeite im Projekt `Astr00/sms` an der Dekompilation meines lokal vorliegenden Super-Mario-Sunshine-Builds.
Nutze den bestehenden Projektstand als Grundlage und führe ihn fort, statt ein Ersatzprojekt anzulegen.

Das Hauptziel ist lesbarer, wartbarer C/C++-Quellcode, der mit der zum Zielbuild passenden Toolchain dessen ausführbaren Code und zugehörige Programmdaten reproduziert.
Strebe eine vollständig verifizierte Matching-Dekompilation an.
Ein erfolgreicher Build, eine startende DOL oder ein passender Hash allein erfüllen den Auftrag nicht.
Prüfe zusätzlich, welche Bestandteile tatsächlich aus rekonstruiertem Quellcode entstehen und welche weiterhin Originalobjekte, Assembly-Fallbacks oder Platzhalter verwenden.

Arbeite praktisch: Untersuche Dateien, implementiere Funktionen, kompiliere und validiere.
Liefere nicht lediglich einen Plan, Pseudocode, leere Funktionen oder ein neues Build-Gerüst.
Das Gesamtziel bleibt die vollständige Rekonstruktion; jeder Arbeitsabschnitt soll nachprüfbaren Fortschritt erzeugen.

Behaupte nicht, exakt die historischen Quelldateien oder ursprünglichen Variablennamen wiederherzustellen, wenn nur eine durch Binärbefunde gestützte Rekonstruktion vorliegt.

## 2. Projektstand zuerst ermitteln

Prüfe vor Änderungen den tatsächlichen Workspace:

- Repository-Pfad, Remotes, Branch, HEAD-Commit, Submodule und Arbeitsbaumstatus.
- Vorhandene Benutzeränderungen sowie bisherige Implementierungen und Experimente.
- README, geltende AGENTS.md-Dateien, einschlägige Projektdokumentation und vorhandene Übergaben.
- Build-Konfiguration, Toolversionen, unterstützte Zielversionen, Quellstruktur, Tests und CI.
- Bisherige Matching-Berichte, Originalobjekt-Fallbacks und bekannte Blocker.

Geeignete erste Git-Abfragen sind `git status --short`, `git remote -v`, `git branch --show-current` und `git log -8 --oneline`.
Verifiziere den Projektpfad, bevor du Befehle ausführst.

Behandle `doldecomp/sms` nur dann als Upstream dieses Projekts, wenn Remotes oder Historie das bestätigen.
Übertrage dessen aktuellen Stand nicht ungefragt auf meinen Fork.
Ist GitHub nicht erreichbar, verwende einen vorhandenen lokalen Checkout und kennzeichne den nicht geprüften Remote-Stand.
Ist kein Checkout vorhanden, versuche den Zugriff auf `Astr00/sms` mit der verfügbaren autorisierten Verbindung.
Ersetze ein unzugängliches Repository nicht stillschweigend durch ein anderes.

Vorhandene Anleitungen sollen nicht durch generische neue AGENTS.md-Dateien überschrieben werden.
Bei widersprüchlichen Vorgaben dokumentiere den Konflikt und bearbeite zunächst den eindeutig freigegebenen Teil.

## 3. Exakten Referenzbuild identifizieren

Nutze meinen eigenen lokalen Dump beziehungsweise bereits extrahierte Originaldateien.
Verlange keinen Upload der vollständigen ROM oder Disc-Abbildung in den Chat.

Ermittle Zielregion, Disc-ID, Revision und die Identität der relevanten ausführbaren Dateien.
Die Zielversion darf nicht allein aus einem Ordnernamen oder einer Standardoption abgeleitet werden.
Dokumentiere Hashes der tatsächlich verwendeten Referenzdateien, vorzugsweise SHA-256 und zusätzlich die vom Projekt erwarteten Hashes.
Unterscheide Hashes des Disc-Containers, der extrahierten DOL und gegebenenfalls weiterer ausführbarer Module.

Prüfe vorhandene versionierte Konfigurationen und deren Referenzprüfungen.
Verwende keine Hashwerte, Adressen, Symboltabellen oder Strukturannahmen einer anderen Revision ungeprüft.
Ändere niemals einen erwarteten Referenzhash, damit ein fehlerhafter Build als korrekt erscheint.

Ist mein Build bisher nicht unterstützt, beschreibe die Abweichung und ergänze einen getrennten Zielversionspfad, soweit die vorhandenen Daten das erlauben.
Beschädige dabei keine unterstützte Version.
Ein unbekannter oder modifizierter Build muss als solcher gekennzeichnet bleiben, solange seine Herkunft nicht verifiziert wurde.

Untersuche die DOL und nur tatsächlich vorhandene zusätzliche ausführbare Module.
Erfinde keine REL-Dateien oder anderen Module.
Unterscheide ausführbaren Code, eingebettete Programmdaten und extern geladene Spielassets.

## 4. Umgebung und unveränderten Ausgangsbuild prüfen

Meine Arbeitsumgebung ist Windows.
Nutze dort passende PowerShell- beziehungsweise native Befehle und korrekt zitierte Pfade.
Ein tatsächlich anders konfigurierter Remote-Workspace soll mit seinem realen Betriebssystem behandelt werden, nicht mit angenommenen Windows-Pfaden.

Bevorzuge die bereits vorgesehene Toolchain und das vorhandene Buildsystem.
Führe keine CMake-Migration, keinen Compilerwechsel und keine generelle Modernisierung ein, nur weil sie dir vertrauter sind.
Prüfe pro Übersetzungseinheit Compiler, Sprachdialekt, Optionen, Includes, Linker und Bibliotheken.
Halte den Matching-Code mit der ursprünglichen Zieltoolchain kompatibel.

Finde vorhandene Analysewerkzeuge und prüfe deren tatsächliche Versionen und Hilfeausgaben.
Verwende projektlokale, nachvollziehbare Toolinstallation und die dokumentierten Bezugsquellen.
Installiere nicht mehrere alternative Toolchains ohne konkreten Bedarf.

Wenn der Checkout den entsprechenden Python/Ninja-Workflow verwendet, prüfe zunächst `python configure.py --help`.
Konfiguriere anschließend ausdrücklich die ermittelte Zielversion und führe den dokumentierten Build aus.
Nutze nur tatsächlich vorhandene Targets und Skripte.

Halte vor der ersten Codeänderung fest:

- Ausgeführte Befehle, Toolversionen, Exitcodes und relevante Logpfade.
- Ergebnis des unveränderten Builds und der Referenzprüfung.
- Quellcodeabdeckung, Matching-Messwerte und Umfang verbleibender Fallbacks.
- Bereits vorher bestehende Fehler oder Regressionen.

Nutze `ninja baseline` und später `ninja changes_all`, sofern diese Targets im Checkout vorhanden und vorgesehen sind.
Sichere die Anfangsbaseline separat; überschreibe sie nicht während der Experimente.
Bei fehlenden Targets verwende die vorhandenen gleichwertigen Reporting-Funktionen.

Ein kaputter Ausgangsbuild ist zunächst ein Buildproblem und kein Anlass, wahllos Spielcode zu ändern.
Behebe die kleinste nachgewiesene Ursache und prüfe erneut.

## 5. Arbeitsumfang und ehrliche Fortschrittsmessung

Lege einen aus den tatsächlichen Builddaten abgeleiteten Arbeitsvorrat an.
Erfasse Übersetzungseinheiten, Funktionen und relevante Datenobjekte mit ihren Abhängigkeiten und ihrem Status.
Trenne Spielcode, Middleware, SDK und Compiler-Runtime.

Beachte Einschränkungen für autonome Arbeiten in den geltenden Repository-Regeln.
Falls beispielsweise Arbeiten an JSystem, Dolphin-SDK, MSL, MetroTRK oder THPPlayer untersagt beziehungsweise reviewpflichtig sind, umgehe diese Vorgaben nicht.
Bearbeite freigegebenen Spielcode und dokumentiere eingeschränkte Abhängigkeiten als gesonderten Restumfang.
Nenne das Ergebnis dann nicht uneingeschränkt „vollständig“, solange diese Bestandteile fehlen.

Führe mindestens diese getrennten Angaben:

1. Implementierungsabdeckung aus C/C++: rekonstruierte Funktionen und Codebytes.
2. Matching-Status: verifizierte Funktionen, Daten und Übersetzungseinheiten.
3. Herkunft der Link-Eingaben: rekonstruierter Quellcode, Originalobjekt, Assembly oder andere Abhängigkeit.
4. Laufzeitvalidierung: getestete Szenarien und weiterhin ungetestete Bereiche.

Verwende für Vorher-Nachher-Vergleiche dieselbe Revision, denselben Umfang und dieselbe Messmethode.
Nenne Zähler und Nenner beziehungsweise deren Definition.
Ein bytegewichteter Code-Messwert darf nicht als Anteil getesteter Spielfunktionen ausgegeben werden.

Unterscheide Projektstatus wie `Matching`, `NonMatching` und `Equivalent` anhand ihrer tatsächlichen Definition im Checkout.
Deklariere einen Statuswechsel erst nach der dazugehörigen Prüfung.

## 6. Vorgehen für jede Funktion oder zusammenhängende Einheit

Wähle eine klar abgegrenzte Aufgabe, untersuche ihre Abhängigkeiten und arbeite in kurzen überprüfbaren Zyklen.
Bevorzuge anfangs Aufgaben, die zugleich vorhandenen Code vervollständigen und wichtige Abhängigkeiten auflösen.
Verliere zentrale Subsysteme nicht aus dem Blick, nur weil isolierte Kleinstfunktionen leichter zu matchen sind.

### A. Befunde sammeln

Identifiziere Zieladresse, Symbol, Größe, Abschnitt und zuständige Übersetzungseinheit.
Untersuche Aufrufer, aufgerufene Funktionen, globale Zugriffe und benachbarte Routinen.
Nutze vorhandene Maps, Debuginformationen und Symboldaten, soweit sie zur ermittelten Revision gehören.
Durchsuche große Maps gezielt statt sie vollständig in den Kontext zu laden.

Berücksichtige verfügbare ungenutzte beziehungsweise herausoptimierte Symbole als Strukturhinweise.
Gib jedoch nicht vor, deren vollständigen Funktionskörper aus nicht vorhandenen Maschinenbefehlen beweisen zu können.

Prüfe vorhandene Header und Klassenhierarchien, bevor du neue Deklarationen anlegst.
Notiere für unbekannte Felder Offset, Zugriffsbreite und beobachtete Verwendung.
Unterscheide ausdrücklich zwischen beobachtet, abgeleitet und noch unbestätigt.

### B. Lokale Analysewerkzeuge einsetzen

Nutze vorhandene decomp-toolkit-, objdiff- und gegebenenfalls Ghidra- oder m2c-Workflows, wenn sie zur Aufgabe passen.
Bevorzuge das projektspezifische Diff-Skript gegenüber einer neu geschriebenen Parallelimplementierung.
Prüfe Optionen mit `--help`, statt Toolaufrufe zu erfinden.

Für m2c kann nach Prüfung der installierten Version folgendes Muster dienen:

    python "<m2c-Pfad>/m2c.py" -t ppc -f "<mangled_symbol>" --globals=used "<lokale_asm_datei>"

Ersetze alle Platzhalter durch ermittelte Werte.
Ein Decompiler-Entwurf ist ein Analysehilfsmittel, kein Korrektheitsnachweis.
Gleiche Kontrollfluss, Typen, globale Zugriffe und Nebenwirkungen mit dem Referenzcode ab.
Vermeide ungeprüfte Übernahme automatisch erfundener Typen und Felder.

### C. Rekonstruktion implementieren

Implementiere im bestehenden Quell- und Headerlayout eine nachvollziehbare Lösung.
Erhalte das beobachtete Verhalten einschließlich Fehlerpfaden, Grenzfällen und ungewöhnlicher Details.
Verbessere das Spielverhalten nicht nebenbei.

Prüfe besonders:

- Parameter, Rückgaben, Aufrufkonvention, Signedness und Integerbreiten.
- Klassenlayout, Basisklassen, Alignment, virtuelle Methoden und erforderliche this-Anpassungen.
- Initialisierung, Destruktion, globale Zustände und Lebensdauer.
- Float-/Double-Operationen, Auswertungsreihenfolge und compilerabhängige Rundung.
- Endianness, Bitfelder, Tabellenindizes, Pointerarithmetik und Datenzugriffe.
- Symbolbindung, Inlining, Abschnittszuordnung und Reihenfolge relevanter Definitionen.

Verwende compilerverträgliche Layoutprüfungen, soweit sie sinnvoll und belegbar sind.
Benenne Unbekanntes vorläufig neutral, beispielsweise anhand seines Offsets, statt eine unbelegte Bedeutung als Tatsache festzuschreiben.

### D. Kompilieren und vergleichen

Baue zunächst die betroffene Einheit, danach den erforderlichen Gesamtbuild.
Vergleiche nicht nur den angezeigten Prozentwert, sondern die relevanten Instruktionen, Daten, Referenzen und Relokationen.
Prüfe, dass die verglichenen Objekte frisch aus dem aktuellen Quellstand erzeugt wurden.

Untersuche Abweichungen systematisch: zuerst Signatur und Layout, danach Kontrollfluss, Konstanten, Typen, Inlining, Compileroptionen und schließlich feinere Codegenerierungsdetails.
Ändere bei schwierigen Abweichungen möglichst nur eine Hypothese pro Versuch.
Halte aussagekräftige gescheiterte Ansätze fest, damit sie nicht ständig wiederholt werden.

Existiert `tools/decomp-diff.py`, ermittle gültige Unit-Namen aus der tatsächlichen Projektkonfiguration.
Mögliche Aufrufmuster nach Prüfung der Hilfe:

    python tools/decomp-diff.py -u "<unit>"
    python tools/decomp-diff.py -u "<unit>" -d "<symbol>"

Existiert eine Symbolreihenfolge-Prüfung, führe sie ebenfalls aus, beispielsweise:

    python tools/validate-symbol-order.py -u "<unit>"

Keine beispielhaften Unit-Namen als tatsächlich untersuchte Einheiten ausgeben.

### E. Regressionen und Integration

Führe nach der Änderung den Baseline-Vergleich und die vorgeschriebenen Prüfungen aus.
Bei Headeränderungen müssen auch die betroffenen abhängigen Einheiten berücksichtigt werden.
Prüfe den Gesamtbuild und dessen Referenzvergleich.

Kann eine Funktion noch nicht matchen, behalte eine nützliche Rekonstruktion nur im dafür vorgesehenen nichtmatchenden Status.
Dokumentiere die konkrete Restabweichung.
Verschlechtere nicht unbemerkt bereits verifizierte Teile.

## 7. Keine Scheinlösungen

Erzeuge keine leeren Erfolgspfade, pauschalen `return 0`-Stubs oder Dummyobjekte, um Compilerfehler zu verstecken.
Vorübergehende Platzhalter müssen sichtbar als unvollständig gelten und dürfen nicht als fertig gezählt werden.

Übertrage nicht den ausführbaren Referenzcode in Bytearrays, Inline-Assembly oder eingebettete Binärblöcke, um vollständige Quellrekonstruktion vorzutäuschen.
Unterscheide legitime Datentabellen von verstecktem Maschinencode.
Nachvollziehbar rekonstruierte, architekturspezifisch notwendige Assembly ist gesondert auszuweisen und darf nicht als C/C++-Dekompilation gezählt werden.

Manipuliere weder Referenzdateien noch Diff-Konfigurationen oder Ausschlusslisten, um schlechtere Ergebnisse zu verbergen.
Führe keine sachfremden Kontrollflüsse, toten Zweige oder undefinierten Verhaltensweisen allein zur Verbesserung eines Scores ein.
Behandle Spezialfälle mit konkreter Evidenz und nachvollziehbarer Dokumentation.

Ein gewöhnlicher Matching-Build darf während der Arbeit vorhandene Originalobjekte weiterverwenden.
Diese bleiben aber offen ausgewiesene Fallbacks und verhindern die Behauptung vollständiger Quellcodeabdeckung.

## 8. Laufzeitprüfung

Wenn lokale Laufzeitwerkzeuge und erforderliche Spieldaten vorhanden sind, ergänze statische Prüfungen durch reproduzierbare Tests.
Nutze einen passenden GameCube-Emulator oder vorhandene Hardware als Referenz- und Testumgebung.
Die Testumgebung ersetzt weder Quellrekonstruktion noch Matching-Prüfung.

Stelle bei nichtmatchenden Implementierungen sicher, dass der Testbuild wirklich den geänderten Quellcode verwendet und nicht weiterhin das Originalobjekt ausführt.
Halte Matching- und experimentelle Testkonfigurationen getrennt.

Wähle Szenarien passend zum geänderten Code, beispielsweise Menüwechsel, Spielerbewegung, Kamera, Wasseraktionen, Kollisionen, Gegnerzustände oder Szenenwechsel.
Erfasse Spielrevision, Build-Commit, relevante Einstellungen, Ausgangszustand, Eingaben und Beobachtungen.
Bevorzuge reproduzierbare Starts und dokumentierte Eingaben gegenüber zufälligem manuellem Spielen.

Verwende vorhandene Eingabeautomation nur nach Prüfung ihrer tatsächlichen Schnittstelle.
Drücke keine erfundenen Tastenfolgen ins falsche Fenster.
Schütze bestehende Spielstände und benutze getrennte Testkopien.
Lade nicht ungeprüft Savestates eines anderen Builds und interpretiere das Ergebnis als gültigen Test.

Kennzeichne nicht ausgeführte Tests ausdrücklich als nicht ausgeführt.
Ein Screenshot allein ist kein Nachweis korrekter Spiellogik.

## 9. Git, Originaldaten und Herkunft

Erhalte meine nicht zum Auftrag gehörenden Änderungen.
Kein `git reset --hard`, pauschales `git clean`, automatisches Stashen oder Zurücksetzen fremder Änderungen.
Nutze für Vergleiche bei Bedarf einen getrennten Arbeitsbaum, ohne bestehende Arbeit zu überschreiben.

Prüfe Änderungen vor dem Staging gezielt.
Erstelle bei zulässigem lokalem Git-Workflow kleine, thematisch zusammengehörige Commits ausschließlich mit deinen Änderungen.
Keine Force-Pushes, automatischen Upstream-Merges, Veröffentlichungen oder Releases.
Pushe erst bei ausdrücklich vorliegender Freigabe.

Originalabbilder, extrahierte Binärdateien, Assets, Zugangsdaten und umfangreiche lokale Analysedumps dürfen nicht versehentlich committed oder hochgeladen werden.
Prüfe dafür Ignore-Regeln und den tatsächlichen staged Diff; `.gitignore` allein reicht nicht als Kontrolle.
Nutze lokale funktionsbezogene Analysen statt vollständiger Binärdateien im Chatkontext.
Lade Material nicht automatisch zu Online-Decompilern oder anderen externen Diensten hoch.

Übernimm keine geleakten Quellen oder ungeprüften proprietären SDK-Implementierungen.
Beachte die Herkunfts- und Lizenzregeln des Projekts.
Bei zulässigen externen Quellen dokumentiere Projekt, Commit und Lizenz und prüfe die technische Passung.
Kennzeichne binärgestützte Rekonstruktion nicht pauschal als streng getrenntes Clean-Room-Verfahren.

## 10. Sitzungsübergreifende Weiterarbeit

Nutze vorhandene Status- und Übergabedokumente, bevor du neue anlegst.
Fehlen solche Dateien, führe eine kompakte Projektübersicht, eine Aufgabenliste und eine Übergabe im bestehenden Dokumentationsbereich.
Erzeuge keine umfangreiche Dokumentationsstruktur ohne praktischen Nutzen.

Die Übergabe enthält:

- Repository, Branch, Commit und Zielbuild mit Referenzhashes.
- Funktionierende Build-, Diff- und Testbefehle sowie Toolpfade.
- Letzte verifizierte Baseline und aktuelle Messwerte.
- Bearbeitete Dateien, offene Änderungen und verbleibende Abweichungen.
- Relevante Befunde, verworfene Hypothesen und konkrete Blocker.
- Die nächsten drei ausführbaren Aufgaben mit ihren Prüfschritten.

Trenne kurze versionierbare Zusammenfassungen von großen lokalen Logs und Dumps.
Aktualisiere die Übergabe nach einem sinnvollen Arbeitsblock und vor dem Ende einer Sitzung.
Beim Fortsetzen verifiziere den dokumentierten Zustand gegen die tatsächlichen Dateien.

Nutze gezielte Suchen, Funktionsausschnitte und kurze Diff-Ausgaben.
Lade nicht immer wieder das gesamte Repository oder vollständige Maps in den Kontext.
Delegiere nur klar getrennte Aufgaben, wenn geeignete Agenten tatsächlich vorhanden und erlaubt sind.
Mehrere Agenten dürfen nicht unkoordiniert dieselben Header oder Builddateien ändern.

## 11. Umgang mit Schwierigkeiten

Stoppe nicht allein wegen der Größe des Gesamtprojekts oder einer schwierigen Funktion.
Zerlege Blocker in überprüfbare Teilfragen und untersuche angrenzenden Code auf neue Evidenz.
Nach mehreren erfolglosen Versuchen ohne neue Erkenntnisse dokumentiere den Stand und bearbeite eine andere sinnvolle Aufgabe.
Kein endloses zufälliges Umstellen von Quellcode.

Unterscheide fehlende Werkzeuge, fehlende Referenzdaten, unklare Semantik, reine Codegenerierungsabweichungen und notwendige menschliche Reviews.
Fordere nur Eingaben an, die sich weder aus Dateien noch aus vorhandenen Werkzeugen ermitteln lassen.
Bei fehlenden Daten nenne exakt Datei, Revision oder lokalen Pfad, der benötigt wird.
Bearbeite währenddessen nur Aufgaben, die ohne diese Angaben verlässlich möglich sind.

Versprich keine Hintergrundarbeit nach dem Ende der aktiven Sitzung.
Hinterlasse stattdessen einen überprüften Arbeitsstand und eine ausführbare Fortsetzung.

## 12. Nachgelagertes Ziel: nativer Windows-Port

Mein langfristiges Ziel bleibt ein nativer Windows-10/11-x86-64-Port ohne Emulator-Runtime.
Er soll später unlocked Rendering-FPS, 16:9, 21:9, Ultrawide, 4K, frei wählbare Auflösungen und geeignete Fenstermodi unterstützen.
Die spätere Validierung soll unter anderem 30/60/90/120/144/165/240 Hz und unbegrenztes Rendering berücksichtigen.

Das ist eine getrennte Arbeitsphase und kein Ersatz für die aktuelle Dekompilation.
Schreibe jetzt keinen neuen Renderer und baue den Matching-Code nicht vorzeitig auf moderne Host-APIs um.
Erhalte bereits vorhandene Portierungsarbeit, trenne sie aber von der unveränderten Referenzkonfiguration.

Dokumentiere Erkenntnisse zu Plattformabhängigkeiten, Zeitsteuerung, Eingaben, Rendering und Assetformaten, die dem späteren Port helfen.
Die spätere Architektur soll Renderfrequenz und Simulation trennen und die ursprüngliche Spiellogik reproduzieren, statt lediglich alle Timer mit einem pauschalen FPS-Faktor zu multiplizieren.
Die tatsächlichen Zeitannahmen müssen aus dem Spiel ermittelt und getestet werden.

## 13. Abschlusskriterien

Eine Einheit ist erst fertig, wenn ihr Quellcode integriert, mit der richtigen Toolchain gebaut und durch die passenden Vergleiche geprüft wurde.
Ihr Status muss den Befund wiedergeben, und erforderliche Regressionstests dürfen nicht fehlen.

Eine vollständige Matching-Dekompilation des festgelegten Umfangs darf erst behauptet werden, wenn:

1. Der Zielbuild eindeutig dokumentiert ist.
2. Alle ausführbaren Einheiten des erklärten Umfangs aus geprüftem Quellcode entstehen.
3. Relevante Programmdaten, Symbole, Layouts und Referenzen berücksichtigt sind.
4. Keine Originalcode-Fallbacks, versteckten Binärblöcke oder unfertigen Stubs verbleiben.
5. Ein frischer Build im getrennten Ausgabeordner die Referenzprüfung besteht.
6. Link-Eingaben und Buildprotokolle die Quellherkunft bestätigen.
7. Vorgeschriebene Prüfungen und passende Laufzeittests dokumentiert sind.
8. SDK-/Middleware-Ausnahmen und andere Grenzen ausdrücklich ausgewiesen werden.

Ein vollständiges Disc-Image ist nicht automatisch Bestandteil dieses Auftrags.
Spieldaten können weiterhin aus meinem lokalen Dump stammen; das darf nicht mit fehlendem rekonstruiertem Programmcode vermischt werden.
Ein natives PC-Programm gilt erst nach gesonderter Implementierung und Prüfung als fertig.

## 14. Beginne jetzt

Prüfe Repository und lokalen Zielbuild, ermittle die geltenden Regeln und führe den unveränderten Ausgangsbuild aus.
Sichere die Baseline und wähle anhand echter Befunde die erste freigegebene Rekonstruktionsaufgabe.
Bearbeite sie unmittelbar einschließlich Build und Vergleich.
Beende die Arbeit nicht allein mit einer Roadmap, sofern Implementierung und Validierung möglich sind.

Berichte knapp und auf Deutsch über wesentliche Befunde und erreichte Zwischenergebnisse.
Gib am Ende an: geändert, ausgeführte Prüfungen, gemessene Verbesserung, verbleibende Blocker und nächster konkreter Schritt.
Trenne beobachtete Ergebnisse von Vermutungen.
Erfinde keine ausgeführten Tests, Commit-IDs, Messwerte oder Erfolgsmeldungen.