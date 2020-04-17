# Applikationsbeschreibung Logik

Die Applikation Logik erlaubt eine Parametrisierung von Logikkanälen mit der ETS.

Sie ist in die Bereiche

* Allgemeine Parameter
* Logikdokumentation
* Logikkanäle

gegliedert, wobei die Logikkanäle wiederum in bis zu 80 Kanäle untergierdert sind.

## Änderungshistorie

Im folgenden werden Änderungen an dem Dokument erfasst, damit man nicht immer das Gesamtdokument lesen muss, um Neuerungen zu erfahren.

17.04.2020: Firmware 1.1.0, Applikation 1.4 - 1.7

* neue Optionen bei 'Logik sendet ihren Wert weiter'
* Ergänzung bei 'Wert vom Bus lesen'
* vergessene Beschreibung 'Eingang wird alle n Sekunden gelesen'
* neue Einstellung 'Nur so lange zyklisch lesen, bis erstes Telegramm eingeht'
* Ergänzung bei 'Ja - Tonwiedergabe (Buzzer)'
* **inkompatible Änderung**: Beim Update der Applikation werden für die KO der Logikkanäle die bereits zugeordneten GA nicht übernommen. Alle Parameter bleiben erhalten.

## Allgemeine Parameter

![Allgemeine Parameter](AllgemeineParameter.png)
Hier werden Einstellungen getroffen, die die generelle Arbeitsweise des Logikmoduls bestimmen.

### Anzahl verfügbarer Logikkanäle

Dieses Feld gibt an, für wie viele Logikkanäle dieses Applikationsprogramm erstellt wurde.

Es stehen ETS-Applikationen mit 10, 20, 40 und 80 Logikkanälen zur Verfügung. Die Anzahl der Logikkanäle wesentlich die Programmierzeit mit der ETS. Ein Logikmodul mit 10 Logikkanälen braucht ca. 30 Sekunden für die Programmierung, mit 80 Logikkanälen weit über 3 Minuten. Die Programmierzeit hängt immer von der Anzahl der verfügbaren Logikkanäle ab, nicht von der Anzahl der genutzen.

### Zeit bis das Gerät nach einem Neustart aktiv wird

Hier kann man festlegen, wie viele Sekunden vergehen sollen, bis nach einem Neustart des Geräts, sei es durch Busspannungsausfall, Reset über den Bus oder auch durch ein Drücken der Reset-Taste, das Gerät seine Funktion aufnimmt.

Da das Gerät prinzipiell (sofern parametriert) auch Lesetelegramme auf den Bus senden kann, kann mit dieser Einstellung verhindert werden, dass bei einem Busneustart von vielen Geräten viele Lesetelegramme auf einmal gesendet werden und so der Bus überlastet wird.

### In Betrieb senden alle...

Das Gerät kann einen Status "Ich bin noch in Betrieb" über das KO 1 senden. Hier wird das Sendeintervall in Sekunden eingestellt.

Sollte hier eine 0 angegeben werden, wird kein "In Betrieb"-Signal gesendet und das KO 1 steht nicht zur Verfügung.

### Uhrzeit und Datum nach einem Neustart vom Bus lesen

Dieses Gerät kann Uhrzeit und Datum vom Bus empfangen. Nach einem Neustart können Uhrzeit und Datum auch aktiv über Lesetelegramme abgefragt werden. Mit diesem Parameter wird bestimmt, ob Uhrzeit und Datum nach einem Neustart aktiv gelesen werden.

Derzeit werden die Informationen über Uhrzeit und Datum noch nicht verarbeitet. Sie sind für zukünftige Erweiterungen vorgesehen, vor allem für eine Zeitschaltuhrfunktion.

### Vorhandene Hardware

Die Firmware im Logikmodul unterstützt eine Vielzahl an Hardwarevarianten. Um nicht für jede Hardwarekombination ein eigenes Applikationsprogramm zu benötigen, kann über die folgenden Felder die Hardwareausstattung des Logikmoduls bestimmt werden.

Die Angaben in diesem Teil müssen der vorhandenen Hardware entsprechen, da sie das Verhalten der Applikation und auch der Firmware bestimmen. Das Applikationsprogramm hat keine Möglichkeit, die Korrektheit der Angaben zu überprüfen.

Falsche Angaben können zu falschern Konfigurationen der Applikation und somit zum Fehlverhalten des Logikmoduls führen.

#### Akustischer Signalgeber vorhanden (Buzzer)?

Das Logikmodul unterstützt auch die Ausgabe von Pieptönen mittels eines Buzzers. Mit einem Haken in diesem Feld wird angegeben, ob ein Buzzer installiert ist.

#### Optischer Signalgeber vorhanden (RGB-LED)?

Das Logikmodul unterstützt auch die Ausgabe eines Lichtsignals mittels einer RGB-LED. Mit einem Haken in diesem Feld wird angegeben, ob eine RGB-LED installiert ist.

#### Nichtflüchtiger Speicher vorhanden (EEPROM)

Ein EEPROM ist ein Speicher, der seine Informationen auch nach einem Stromausfall nicht verliert. Ein solches EEPROM wird von der Firmware genutzt, um Werte von bestimmten Kommunikationsobjekten zu speichern.

Ist kein EEPROM auf dem Board vorhanden, können diese Informationen nicht gespeichert werden. Die Applikation wird dann alle Einstellungen, die ein Speichern erlauben, nicht anbieten. In einem solchen Fall erscheint die folgende Information:
![Info EEPROM](InfoEeprom.png)

#### Zusatzhardware abschaltbar (z.B. mit dem NCN5130)?

Damit bei einem Stromausfall Daten in einem EEPROM gespeichert werden können, muss nicht nur ein EEPROM vorhanden sein, sondern auch genügend lange Strom zum Speichern vorhanden sein. Angeschlossene Hardware (RGB-LED, Buzzer) verbrauchen aber viel Strom und verhindern somit die Speicherung bei Stromausfall.

Die Firmware unterstützt aber eine Abschaltung der Zusatzhardware, falls der Strom ausfällt. Derzeit wird die Abschaltung nur über den NCN5130 (KNX-Bus-Interface) unterstützt, kann aber bei Bedarf entsprechend um weitere Abschaltmöglichkeiten erweitert werden.

Ist keine Möglichkeit zur Abschaltung vorhanden, wird die Speicherung ins EEPROM unterbunden. Die Applikation wird dann alle Einstellungen, die ein Speichern erlauben, nicht anbieten. In einem solchen Fall erscheint die folgende Information:
![Info Stromabschaltung](InfoPower.png)

## Logikdokumentation

Eine stichwortartige Abhandlung dieser Dokumentation ist auch in der Applikation enthalten und auf 3 Unterseiten aufgeteilt.

### Allgemein

Hier ist die generelle Funktionsweise des Logikmoduls beschrieben.

### Eingänge

Hier werden die Funktionsmodule für die Eingänge beschrieben.

### Ausgänge

Hier werden die Funktionsmodule für die Ausgänge beschrieben.

## Logikkanäle

Im Folgenden werden die generellen Konzepte und die grobe Funktion eines Logikkanals beschrieben. Die Parameter eines jeden Kanals werden später im Detail beschrieben.

Jeder Logikkanal, von denen bis zu 80 zur Verfügung stehen, ist identisch aufgebaut. Es stehen immer 2 externe Eingänge, 2 interne Eingänge und ein Ausgang zur Verfügung.

Zwischen die Eingänge und den Ausgang können verschiedene Funktionsblöcke geschaltet werden, die die Eingangssignale beeinflussen und Verknüpfen können und so ein Ausgangssignal erzeugen.

Alle Funktionsblöcke kann man sich wie an einer Perlenschnur aufgereiht hintereinander vorstellen, das Ergebnis eines Funktionsblocks wird für den darauffolgenden Funktionsblock als Eingabe verwendet.

![Übersicht](Uebersicht.png)

Jeder Funktionsblock arbeitet rein binär, also nur mit den Werten 0 oder 1 (DPT 1). Damit auch andere DPT möglich sind, besitzen externe Eingänge Konverter-Funktionsblöcke, die von einem beliebigen DPT nach DPT 1 konvertieren. Derzeit sind Schwellwertschalter und Vergleicher als Konverterfunktionen implementert. Interne Eingänge benötigen keinen Konverter, da sie rein binär funktionieren.

Die binäre Signalverarbeitung beginnt mit einer logischen Verknüpfung, die alle Eingänge zusammenbringt, gefolgt von

* Treppenlicht (mit einer Blinkfunktion)
* Ein- und Ausschaltverzögerung (getrennt einstellbar)
* Wiederholungsfilter
* Zyklisch senden
* Sendefilter

Wird ein Funktionsblock nicht genutzt (nicht parametrisiert), gibt er seine Eingabe unverändert als Ergebnis an den nächsten Funktionsblock weiter.

Das nach dem Sendefilter ermittelte Signal steht für die internen Eingänge der anderen Kanäle zur Verfügung. Ferner steht es auch einem Ausgangskonverter zur Verfügung, der als Wertwandler ausgelegt ist und den ermittelten Wert als einen anderen DPT ausgeben kann. Dabei können die Ausgbewerte festgelegt werden (Konstanten) oder ein am Eingang 1 oder Eingang 2 vorliegender Wert in den Ausgangs-DPT konvertiert werden.

### Startverhalten

Dem Startverhalten eines Logikkanals kommt eine besondere Bedeutung zu.

Initial sind alle Eingänge und der Ausgang unbestimmt. Es wäre möglicherweise fatal, wenn beim Start jeder Logikkanal erstmal für seinen Ausgang ein AUS auf den Bus senden würde. Ebenso sollte eine UND-Verknüpfung mit 2 Eingängen, die auf Eingang 1 noch kein Signal empfangen hat und auf dem Eingang 2 eine 1 empfängt, nicht automatisch annehmen, dass Eingang 1 auf AUS steht und dadurch bedingt eine 0 auf den Ausgang senden.

Es muss einstellbar sein, wie ein Logikkanal mit "undefinierten" Zuständen umgeht. Im folgenden werden die Möglichkeiten für jeden Logikkanal erläutert.

Jeder Eingang ist beim Start undefiniert und der Ausgang sendet ersteinmal nichts. Man kann für einen Eingang festlegen, wie er seinen Anfangswert bekommen soll.

Die einfachste Variante ist eine Konstantenbelegung: Ein Eingang kann den Wert AUS oder EIN annehmen und das kann man über Parameter festlegen. Anmerkung: Obwohl Eingänge durchaus verschiedene DPT unterstützen, ist die Vorbelegung nur mit den Werten AUS oder EIN möglich, also quasi als Ergebnis des Eingangskonverters.

Eine weitere Möglichkeit ist, dass der Eingang seinen Anfangswert vom Bus liest. Damit würde der Eingang bei einem Neustart ein Lesetelegramm schicken und auf eine Antwort warten. Bis die Antwort eintrifft, ist der Eingang weiterhin undefiniert. Da bei einem Systemstart andere Geräte, die das Lesetelegramm beantworten könnten, eventuell selbst noch nicht in der Lage sind zu senden, kann man zusätzlich zum Parameter "Allgemeine Parameter -> Zeit bis das Gerät nach einem Neustart aktiv wird" auch noch pro Kanal eine Startverzögerung festlegen. Das Lesetelegramm für diesen Kanal wird erst nach der Summe der beiden Zeiten gesendet.

Die letze Möglichkeit, einen Eingang vorzubelegen, ist mit dem letzten Wert, den er hatte. Dazu kann man einstellen, dass der Wert für diesen Eingang über einen Stromausfall bzw. Reset des Gerätes hinweg in einem nichtflüchtigen Speicher gespeichert wird. Bei einem Neustart des Gerätes wird der Wert aus dem nichtflüchtigen Speicher wieder gelesen und als Startwert angenommen.

Das bisher beschriebene führt zu der Situation, dass ein Logikkanal nach einem Neustart, der Zeit bis das Gerät aktiv wird und der Zeit, bis der Kanal aktiv wird in einem Zustand sein kann, bei dem immer noch einer oder beide Eingägne undefiniert sind.

Eine Logikverknüpfung, die aber an einigen Eingängen einen definierten und and anderen Eingängen einen undefinierten Wert hat, muss wissen, wie sie sich in so einem Fall verhalten soll, sprich, ob und wie die Verknüpfung durchgeführt werden soll.

Hier sind 2 Möglichkeiten implementiert (und somit parametrisierbar):

1. Die Verknüpfung soll erst durchgeführt werden, wenn alle Eingänge definierte Werte haben. Bevor dies nicht eintritt, passiert am Ausgang einfach nichts.
2. Die Verknüpfung soll bereits beim Eintreffen des ersten Signals reagieren. Ist dann der andere Eingang noch undefiniert, kann man für diesen vernünftigerweise weder ein EIN noch ein AUS annehmen. Der undefinierte Eingang wird dann als nicht existent behandelt und die Verknüpfung nur für die definierten Engänge durchgeführt. Beispiel: Ein UND mit 3 Eingängen, von denen 2 auf EIN und einer auf undefiniert stehen, würde wie ein UND mit 2 Eingängen behandelt werden und ein EIN liefern.

Durch die dezidierten Einstellungsmölgichkeiten des Startverhaltens pro Kanal kann man sein KNX-System sehr detailiert bezüglich des Systemstart steuern. Da genau dieses Startverhalten von vielen KNX-Geräten eher stiefmütterlich behandelt wird, hat man mit diesem Logikmodul viele Möglichkeiten, hier einzugreifen und Unzulänglichkeiten auszugleichen.

### Zusammenfassung

Die hier für jeden Kanal zur Verfügung stehenden Möglichkeiten der Beeinflussung des Signalverlaufs ermöglichen die Realsierung von vielen Steuerungsaufgaben, die sonst über viele Einzelgeräte oder gar Logikmaschinen verteilt werden müssen. Durch unterschiedliche Parametrierung der Funktionsblöcke kann man folgende klassische und im KNX übliche Funktionen erreichen:

* NOT (logische negierung eines Signals)
* Logische Verknüpfung AND, OR, EXOR (durch die Nutzung von internen Eingängen auch mit sehr vielen Eingängen)
* TOR/Sperre (lasse nur ein Signal durch/nicht durch, solange ein anderes anliegt)
* Treppenlicht
* Ein- und Ausschaltverzögerung
* Zyklisch senden
* Schwellwertschalter
* Vergleichen von 2 Werten
* Intervallvergleich (Ein Wert liegt in einem bestimmten Wertebereich)
* Hysteresevergleich (Schalte EIN oberhalb eines Wertes, AUS erst unterhalb eines anderen)
* Wiederhole n mal ein Signal
* Vervielfache ein Signal auf verschiedene GA
* Konvertiere ein DPT in einen anderen
* Verzögere ein Signal
* tbc

## Logik n: unbenannt

Da alle Kanäle identisch sind, wird hier nur ein Kanal repräsentativ beschrieben. Das gesagte kann für alle Kanäle eingestellt werden.

Ein Logikkanal wird durch einen Tab mit dem Namen "Logik n: \<Name der Logik>" repräsentiert, wobei n die Nummer des Kanals ist und der \<Name der Logik> anfänglich "unbenannt" lautet.

![Baumansicht der Kanäle](Kanalbaum.png)

Folgende Parameter kann man für einen Logikkanal angeben:

![Logikseite](Logikseite.png)

### Beschreibung des Kanals

Der hier vergebene Name hat keinen funktionalen Einfluß, erlaubt es aber, dem Kanal einen eigenen Namen zu geben, und ihn so leichter wiederzufinden. Der Name wird im Kanalbaum dargestellt und statt dem Text "unbenannt" genommen.

### Zeit bis der Kanal nach einem Neustart aktiv wird

Neben dem "Allgemeine Parameter -> Zeit bis das Gerät nach einem Neustart aktiv wird" kann auch noch pro Kanal eine Startverzögerung sinnvoll sein. Der Grund ist in "Logikkanäle -> Startverhalten" beschrieben.

Die Verzögerungszeit wird in Sekunden angegeben.

### Logische Operation

Mittels der Auswahlliste kann eine logische Operation und damit die Art der Verknüpfung der Eingänge dieses Logikkanals ausgewählt werden. Es stehen folgende Operationen zur Verfügung:

#### aus / inaktiv

Dieser Logikkanal ist außer Funktion. Er kann vollständig definiert sein und keine Einstellung geht verloren, aber der Ausgang wird kein Telegramm senden. Dies bietet die Möglichkeit, zu Testzwecken einen bereits parametrierten Logikkanal inaktiv zu setzen, um zu schauen, ob er die Ursache für eventuelles Fehlverhalten im Haus ist. Kann zur Fehlersuche hilfreich sein.

#### UND

Alle Eingänge werden über ein logisches UND verknüpft. Das Ergebnis der Verknüpfung ist EIN, wenn alle Eingänge des Funktionsblock EIN sind. Das Ergebnis ist AUS, wenn auch nur ein Eingang AUS ist.

#### ODER

Alle Eingänge werden über ein logisches ODER verknüpft. Das Ergebnis der Verknüpfung ist EIN, wenn nur ein Eingang des Funktionsblock EIN ist. Das Ergebnis ist AUS, wenn alle Eingänge AUS sind.

#### EXCLUSIV-ODER

Alle Eingänge werden über ein logisches Exklusiv-ODER verknüpft. Das Ergebnis der Verknüpfung ist EIN, wenn eine ungerade Anzahl von Eingängen des Funktionsblock EIN sind. Das Ergebnis ist AUS, wenn eine gerade Anzahl von Eingängen EIN sind.

#### TOR

Ein Tor hat normalerweise einen Dateneingang, Datenausgang und einen Toreingang. Wird das Tor über ein Signal am Toreingang geöffnet, können Daten vom Dateneingang zum Datenausgang fließen. Wird das Tor geschlossen, dann fließen keine Daten zwischen Dateneingang und Datenausgang.

Wir das Signal am Toreingang invertiert (negiert), dann sprechen wir von einer Sperre.

Da ein Logikkanal 4 Eingänge hat, ist bei einem Tor

    Dateneingang = Eingang 1 ODER Kanalausgang X
    Toreingang = Eingang 2 ODER Kanalausgang Y

(in Worten: Jeweils ein externer und ein interner Eingang werden über ein ODER verknüpft und bilden den entsprechenden Eingang der TOR-Verknüpfung).

### Eingang 1, Eingang 2

Jeder Eingang kann durch die Auswahlfelder deaktiviert bzw. normal oder invertiert (negiert) aktiviert werden.

#### inaktiv

Steht ein Eingang auf inaktiv, kann er nicht genutzt werden und es steht kein KO zur Verfügung, um ein Telgramm an diesen Eingang zu schicken.

#### normal aktiv

Für diesen Eingang erscheint ein Kommunikationsobjekt. Detailangaben zu diesem Eingang erfolgen auf einer eigenen Seite. Der aus den Eingstellungen für den Eingang ermittelte binäre Wert wird direkt der oben ausgewählten logischen Operation zur Verfügung gestellt.

#### invertiert aktiv

Für diesen Eingang erscheint ein Kommunikationsobjekt. Detailangaben zu diesem Eingang erfolgen auf einer eigenen Seite. Der aus den Eingstellungen für den Eingang ermittelte binäre Wert wird invertiert (negiert), bevor er der oben ausgewählten logischen Operation zur Verfügung gestellt wird. Invertieren (negieren) heißt, dass ein EIN-Signal zu einem AUS-Signal wird und umgekehrt.

### Kanalausgang X, Kanalausgang Y

Auch wenn der Name es anders vermuten läßt, handelt es sich um interne Eingänge, die mit einem Ausgang eines anderen Kanals verbunden sind. Jeder interne Eingang kann durch die Auswahlfelder deaktiviert bzw. normal oder invertiert (negiert) aktiviert werden.

#### inaktiv

Steht ein interner Eingang auf inaktiv, kann er nicht genutzt werden und er hat keinen Einfluß auf die logische Verknüpfung.

#### normal aktiv

Es erscheint eine eigene Seite für die Verknüpfung dieses Eingangs mit einem anderen Kanalausgang. Der Wert des Kanalausgangs wird direkt der oben ausgewählten logischen Operation zur Verfügung gestellt.

#### invertiert aktiv

Es erscheint eine eigene Seite für die Verknüpfung dieses Eingangs mit einem anderen Kanalausgang. Der Wert des Kanalausgangs wird invertiert (negiert), bevor er der oben ausgewählten logischen Operation zur Verfügung gestellt wird. Invertieren (negieren) heißt, dass ein EIN-Signal zu einem AUS-Signal wird und umgekehrt.

### Logik auswerten

Wie bereits in "Logikkanäle -> Startverhalten" beschrieben, ist es notwendig, einer Logikverknüpfung zu sagen, wie sie mit undefinierten Eingängen umgehen soll.

#### auch wenn noch nicht alle Werte gültig sind

Die logische Verknüpfung betrachtet alle undefinierten Eingänge als ob sie mit "inaktiv" parametriert wären. Ein UND mit 3 Eingängen, das von den einer undefiniert ist, wird bereits ein EIN senden, wenn die 2 restlichen Eingänge EIN sind.

Ein TOR mit einem undefinierten Dateneingang oder einem undefinierten Toreingang kann nicht sinnvoll funktionieren und sendet dann gar nichts.

#### erst wenn alle Werte gültig sind

Die logische Verknüpfung wird erst dann einen Wert ermitteln, wenn an allen Eingängen gültige Werte vorliegen.

### Beim schließen vom Tor wird

Das Auswahlfeld erscheint nur, wenn als logische Operation TOR gewählt wurde.

Mit dem Auswahlfeld kann man einstellen, ob das Tor zusätzliche Telegramme verschicken soll, wenn es gerade geschlossen wird (Toreingang geht auf AUS).

#### nichts gesendet

Beim schließen vom Tor wird nichts gesendet

#### AUS gesendet

Beim schließen vom Tor wird immer ein AUS-Signal gesendet.

#### EIN gesendet

Beim schließen vom Tor wird immer ein EIN-Signal gesendet.

#### Eingangswert gesendet

Beim schließen vom Tor wird der Eingangswert gesendet. Da dieser Wert ja faktisch schon mal gesendet worden ist (als das Tor noch offen war), ist das effektiv eine einmalige Wiederholung des letzten Wertes.

### Beim öffnen vom Tor wird

Das Auswahlfeld erscheint nur, wenn als logische Operation TOR gewählt wurde.

Mit dem Auswahlfeld kann man einstellen, ob das Tor zusätzliche Telegramme verschicken soll, wenn es gerade geöffnet wird (Toreingang geht auf EIN).

#### nichts gesendet

Beim öffnen vom Tor wird nichts gesendet, erst das nächste Telegramm am Dateneingang wird gesendet.

#### AUS gesendet

Beim öffnen vom Tor wird immer ein AUS-Signal gesendet.

#### EIN gesendet

Beim öffnen vom Tor wird immer ein EIN-Signal gesendet.

#### Eingangswert gesendet

Beim öffnen vom Tor wird der Eingangswert gesendet. Damit kann man erreichen, dass das letzte Signal, das vom Tor blockiert worden ist, nach dem öffnen doch noch durchkommt.

### Logik sendet ihren Wert weiter

Bisher wurde detailiert beschrieben, wie das Logik-Funktionsmodul die Eingänge auswertet, es ist aber ebenso wichtig zu bestimmen, wann der ermittelte Ausgangswert der Logik an die folgenden Funktionsmodule weitergeschickt wird.

Diese Auswahlbox erlaubt eine Detaillierte Einstellung des Verhaltens.

#### Nur bei geändertem Ergebnis

Das Ergebnis der Logikauswertung wird nur dann weitergeschickt, wenn sich das Ergebnis geändert hat. Dazu wird das zuvor ermittelte Ergebnis der Logik (wichtig: nicht das Ergebnis am Ausgang des Logikkanals) herangezogen und mit dem aktuellen Ergebnis verglichen. Weicht es ab, wird das gerade ermittelte Ergebnis weitergeleitet.

#### Nur bei geändertem Ergebnis, aber erstes Telegramm immer senden

Diese Einstellung hat ein spezifisches Verhalten beim Neustart der Logik. Bei einem Neustart ist nicht klar, was "geändertes Ergebnis" heißt. Mit dieser Einstellung sagt man klar, dass das erste Ergebnis der Logik immer als "geändert" behandelt wird und so weitergeschikt wird. Gleichzeitig stellt das Ergebnis den Vergleichswert für die nächste Logikoperation dar, anhand dessen ein "geändertes Ergebnis" festgestellt werden kann.

#### Nur bei geändertem Ergebnis, aber erstes Telegramm nicht senden

Diese Einstellung hat ein spezifisches Verhalten beim Neustart der Logik. Bei einem Neustart ist nicht klar, was "geändertes Ergebnis" heißt. Mit dieser Einstellung sagt man klar, dass das erste Ergebnis der Logik immer als "nicht geändert" behandelt wird und somit nicht weitergeschikt wird. Gleichzeitig stellt das Ergebnis den Vergleichswert für die nächste Logikoperation dar, anhand dessen ein "geändertes Ergebnis" festgestellt werden kann.

#### bei allen Eingangstelegrammen

Sobald ein neues Eingangstelegramm eintrifft, wird das Ergebnis der logischen Verknüpfung ermittelt und an den nächsten Funktionsblock weitergeleitet.

#### bei folgenden Eingangstelegrammen

![Logik sendet](LogikSendet.png)

Es erscheint eine Liste mit allen aktiven Eingängen. Man kann die Eingänge ankreuzen, auf die die Logikauswertung reagieren soll. Nur wenn ein Telgramm von einem dieser Eingänge kommt, wird die Logikauswertung angestoßen und das Ergebnis ermittelt und an den nächsten Funktionsblock weitergeleitet.

## Eingang 1: Wert / Eingang 2: Wert

Sobald für einen Logikkanal ein externer Eingang aktiviert wurde, erscheint für jeden Eingang eine Seite.

![Eingang](Eingangseite.png)

Jeder Eingang kann mit Hilfe der folgenden Einstellungen konfiguriert werden. Im Folgenden wird von Eingang n gesprochen, da die Beschreibung sowohl für Eingang 1 wie auch für Eingang 2 gilt.

> **Wichtig:** Wird ein Eingang als "invertiert aktiv" eingeschaltet, so passiert die Invertierung erst direkt bei der Wertübergabe an die logische Operation des Logik-Funktionsmoduls, also erst nach der Vorbelegung und nach der Konvertierung.

### Beschreibung Eingang n

Dieses Feld hat keine funktionale Auswirkung. Es erlaubt den Eingang zu benennen und diesen so leichter wiederzufinden, erhöht somit die Übersichtlichkeit.

Der hier angegebene Text erscheint in der Seitenbeschreibung "Eingang n: Wert" statt dem Wort "Wert" und als Name des Kommunikationsobjektes, das zu diesem Eingang gehört.

### DPT für Eingang n

Dieses Auswahlfeld legt den DPT für den Eingang fest. Unterstützt werden:

* DPT 1: binärer Wert
* DPT 2: Zwangsführung
* DPT 5: vorzeichenlose Zahl (0 bis 255)
* DPT 5.001: Prozentzahl (0 bis 100)
* DPT 6: vorzeichenbehaftete Zahl (-128 bis 127)
* DPT 7: vorzeichenlose Zahl (0 bis 65535)
* DPT 8: vorzeichenbehaftete Zahl (-32768 bis 32767)
* DPT 9: Gleitkommawert (-670760,96 bis 670760,96)
* DPT 17: Szenen Nummer (1-64)
* DPT 232: RGB-Wert (0-16777216)

Ist der DPT anders als DPT 1, erscheint je nach DPT ein Konverter, mit dem man den gewünschten Eingangs-DPT nach DPT 1 wandeln kann. Die gesamte weitere Verarbeitung des Eingangssignals erfolgt binär, also auf Basis von DPT 1. Alle Parameter der jeweiligen Konverter werden weiter unten im Kapitel "Eingangskonverter" beschrieben.

### Eingangswert speichern und beim nächsten Neustart als Vorbelegung nutzen

Diese Einstellung erlaubt ein dezidiertes Verhalten beim Neustart des Gerätes, wie im Kapitel "Logikkanäle -> Startverhalten" beschrieben.

Diese Einstellung erscheint nur, wenn ein nichtflüchtiger Speicher vorhanden ist und Zusatzhardware (RGB-LED, Buzzer) von der Firmware abgeschaltet werden können.

Mit "Ja" legt man fest, dass der zuletzt an diesem Eingang empfangene Wert im nichtflüchtigen Speicher abgelegt wird und nach einem Neustart wieder gelesen wird. Der dann gelesene Wert wird als Vorbelegung für den Eingang genommen, falls nötig über den Eingangskonverter in einen DPT 1 konvertiert und dann die logische Operation getriggert.

Da nichtflüchtige Speicher nur eine relativ geringe Anzahl an Schreibzyklen zulassen, wird der Eingangswert nicht direkt nach dem Empfang im Speicher geschrieben, sondern erst beim Stromausfall, bei einem "Gerät zurücksetzen" über die ETS oder bei einer neuprogrammierung über die ETS. Wird die RESET-Taste direkt am Gerät gedrückt, wird der nichtflüchtige Speicher nicht mit dem Eingangswert beschrieben.

### Falls Vorbelegung aus dem Speicher nicht möglich oder nicht gewünscht, dann vorbelegen mit

Dieses Auswahlfeld erlaubt eine Vorbelegung mit einem festgelegten Wert. Die Einstellung kommt aber nur zur Auswirkung, falls die vorhergehende Einstellung "Eingangswert speichern und beim nächsten Neustart als Vorbelegung nutzen" auf "Nein" steht, nicht vorhanden ist oder der gespeicherte Wert nicht genutzt werden kann.

Es gibt einige wenige Gründe, warum ein gespeicherter Wert nicht genutzt werden kann:

* Der gespeicherte Wert hat einen anderen DPT. Das passiert, wenn man das Gerät mit der Einstellung "Speichern" in Benutzung hat, dann in der ETS den DPT für den Eingang ändert und das Gerät neu programmiert. Nach dem Neustart passen dann der gespeicherte DPT und der DPT vom Eingang nicht zusammen. Der gespeicherte Wert wird dann verworfen und die Einstellung dieses Feldes als Vorbelegung genommen.
* Es ist gar kein Wert gespeichert, dann kann er natürlich auch nicht genutzt werden und stattdessen wird die Einstellung dieses Feldes als Vorbelegung genommen.
* Durch einen Speicherfehler konnte vor einem Neustart der Wert vom Eingang nicht gespeichert werden. Auch dann wird die Einstellung dieses Feldes als Vorbelegung genutzt. Dieser Fall ist rein Theoretisch und noch nie in der Praxis aufgetreten.

Durch ein Einspielen einer neuen Applikation über die ETS werden die gespeicherten Werte im nichtflüchtigen Speicher nicht gelöscht. Falls aber eine neue Firmware über USB in das Gerät neu eingespielt wird, kann die neue Firmware möglicherweise die gespeicherten Werte der alten Firmware nicht mehr lesen. In diesem Fall würden die gespeicherten Werte aller Eingänge gelöscht und die Vorbelegung würde durch die Einstellung dieses Feldes erfolgen.

#### nichts (undefiniert)

Der Eingang wird nicht vorbelegt und bleibt undefiniert, bis ein erstes Telegramm vom KNX-Bus empfangen wird.

#### Wert vom Bus lesen

Nach der eingestellten Starterzögerung für das gesamte Gerät zuzüglich der Startverzögerung für den Logikkanal wird ein Lesetelegramm auf den KNX-Bus geschickt. Bis die Antwort empfangen wurde ist der Eingang undefiniert.

Sollte in der Zeit, bis der Logikkanal startet, bereits ein Telegramm empfangen werden, dass das Lesetelegramm beantwortet hätte, wird das Lesetelegramm nicht gesendet. Damit wird verhindert, dass mehrere Eingänge, die mit der gleichen GA verbunden sind, viele Lesetelegramme auf die gleiche GA schicken.

#### AUS (0)

Der Eingang wird konstant mit einer 0 vorbelegt und hat somit sofort einen defnierten Zustand.

#### EIN (1)

Der Eingang wird konstant mit einer 1 vorbelegt und hat somit sofort einen defnierten Zustand.

### Eingang wird alle n Sekunden gelesen (0=nicht zyklisch lesen)

Manche Geräte können nicht von sich aus zyklisch senden. Hier kann man einstellen, dass ein Eingang aktiv den Wert zyklisch liest. In den Feld kann man angeben, wie viele Sekunden zwischen 2 Leseintervallen vergehen sollen.

### Nur so lange zyklisch lesen, bis erstes Telegramm eingeht

Erscheint nur, wenn bei "Eingang wird alle n Sekunden gelesen" ein Wert größer 0 eingegeben wurde.

Standardmäßig wird zyklisches lesen ununterbrochen durchgeführt. Mit einem 'Ja' kann man hier festlegen, dass nur so lange zyklisch gelesen wird, bis ein erstes Telegramm eingeht, dass den Wert bestimmt. Das kann sowohl ein Antworttelegramm (GroupValueResponse) wie auch ein Schreibtelegramm (GroupValueWrite) sein.

Diese Funktion vor allem nach einem Neustart der Logik von Nutzen sein, da Lesetelegramme womöglich nicht sofort beantwortet werden können, falls das antwortende Gerät sich selbst noch in der Startphase befindet. Hier kann man diese Lesetelegramme so lange wiederholen lassen, bis sie beantwortet werden können, anschließend kann ohne aktives Nachfragen auf normale Schreibtelegramme reagiert werden.

Dies erlaubt es, eine KNX-Anlage nach einem Neustart relativ schnell in einen Zustand zu versetzen, bei dem alle Initialisierungen erfolgt sind und alle Funktionen erwartungskonform ausgeführt werden.

## Eingangskonverter

Sobald für einen Eingang im Feld "DPT für Eingang n" etwas anderes als DPT 1 ausgewählt wird, erscheint ein DPT-Spezifischer Konverter, der eine Konvertierung in DPT 1 erlaubt. Dies ist zwingend notwendig, das die gesamte Logikverarbeitung nur mit binären Werten (DPT 1) erfolgt.

### DPT 1.xxx (Schalten)

Für DPT 1 ist kein Konverter notwendig.

### DPT 2.xxx (Zwangsführung)

![Zwangsführung](Zwangsführung.png)

Eine Zwangsführung kann aus genau 4 Werten bestehen. Im Konverter kann eingestellt werden, welche dieser Werte, falls er empfangen wird, zu einem EIN-Signal konvertiert wird. Logischerweise führen alle hier nichtaufgeführten Werte zu einem AUS-Signal.

In dem Bildschirmausschnitt ist der Konverter so konfiguriert, dass "normal EIN" oder "priorität EIN" zu einem EIN-Signal führen, "normal AUS" oder "priorität AUS" zu einem AUS-Signal.

In den Auswahlfeldern können folgende Werte ausgewählt werden:

#### nicht genutzt

Dieses Eingabefeld wird nicht beachtet.

#### normal AUS (00)

Wird der Zwangsführungs-Wert "normal AUS" empfangen (im KNX durch eine 00 repräsentiert), dann wird dieser zu einem EIN-Signal konvertiert.

#### normal EIN (01)

Wird der Zwangsführungs-Wert "normal EIN" empfangen (im KNX durch eine 01 repräsentiert), dann wird dieser zu einem EIN-Signal konvertiert.

##### priorität AUS (10)

Wird der Zwangsführungs-Wert "priorität AUS" empfangen (im KNX durch eine 10 repräsentiert), dann wird dieser zu einem EIN-Signal konvertiert.

#### priorität EIN (11)

Wird der Zwangsführungs-Wert "priorität EIN" empfangen (im KNX durch eine 11 repräsentiert), dann wird dieser zu einem EIN-Signal konvertiert.

### DPT 17.001 (Szene)

![Szenenkonverter](Szene.png)

Der Szenenkonverter kann bis zu 8 Szenennummern in eine EIN-Signal konvertieren. Alle anderen Szenennummern führen zu einem AUS-Signal. Sollten mehr Szenennummern benötigt werden, kann man mit einem weiteren Eingang oder einem weiteren Logikkanal jeweils 8 weitere Szenen einbeziehen.

In den Eingabefeldern werden die jeweiligen Szenennummern eingegeben, die zu einem EIN-Signal führen sollen. Der Wert "nicht genutzt" wird dann eingegeben, wenn das Eingabefeld nicht ausgewertet werden soll.

### Zahlenbasierte DPT

Alle DPT, die Zahlen repräsentieren (das sind DPT 5.xxx, 5.001, 6.xxx, 7.xxx, 8.xxx, 9.xxx und 232.xxx), können mittels 4 verschiedenen Zahlenkonvertern Konvertern in ein binäres Signal umgewandelt werden. Die Zahlenkonverter sind gleich in ihren Einstellungen, die einzugebenden Zahlen müssen nur innerhalb der Wertebereiche des jeweiligen DPT liegen.

#### Wert für Eingang n bestimmen durch

Mit dem Auswahlfeld wird der passende Zahlenkonverter augewählt.

##### Wertintervall

![Wertintervall](Wertintervall.png)

Bei diesem Konverter legt man einen Von- und einen Bis-Wert fest. Wenn der Wert vom Eingang n innerhalb der Intervallgrenzen liegt (einschließlich der Grenzen selbst), wird er in ein EIN-Signal konvertiert, sonst in ein AUS-Signal.

Formal würde man schreiben:

    WENN Von-Wert <= Eingang n <= Bis-Wert DANN EIN SONST AUS

Falls man ein EIN-Signal möchte, wenn der Wert außerhalb des Intervalls liegt, muss man den Eingang invertiert einschalten.

In dem angezeigten Bildschirmausschnitt wird der Wert 0 und 1 ein ein AUS-Signal konvertiert, die Werte 2 bis 255 in ein EIN-Signal.

##### Differenzintervall

![Differenzintervall](Differenzintervall.png)

Bei diesem Konverter legt man - wie beim Wertintervall - einen Von- und einen Bis-Wert fest. Der Wert, der mit diesem Intervall verglichen wird, ist allerdings die Differenz von dem Eingang n und dem anderen Eingang des Logikkanals. Bei Eingang 1 wird also (Eingang 1 - Eingang 2) gerechnet, bei Eingang 2 aber (Eingang 2 - Eingang 1). Wenn der errechnete Wert innerhalb der Intervallgrenzen liegt (einschließlich der Grenzen selbst), wird das Eingangssingal in eine EIN-Signal konvertiert, sonst in ein AUS-Signal.

Formal würde man (für Eingang 1) schreiben:

    WENN Von-Wert <= (Eingang 1 - Eingang 2) <= Bis-Wert DANN EIN SONST AUS

Falls man ein EIN-Signal möchte, wenn der Wert außerhalb des Intervalls liegt, muss man den Eingang invertiert einschalten.

In dem angezeigten Bildschirmausschnitt würde man bei Eingang 1 = 40 und Eingang 2 = 45 ein EIN-Signal bekommen, da 40 - 45 = -5 ist und dieser Wert innerhalb des Intervalls [-10;10] liegt.

Bei einem Differenzintervall muss der andere Eingang nicht vom gleichen DPT sein, er wird generisch in eine Zahl konvertiert. Allerdings kann das zu unerwarteten Ergebnissen führen. Empfohlen wird ein Differenzintervall für gleiche DPT.

Ein Differenzintervall kann auch als Vergleicher genutzt werden, in dem Von- und Bis-Wert auf 0 gesetzt werden. Dann wird nur bei einer Differenz = 0 (was nichts anderes als die Gleichheit der beiden Eingänge bedeutet) ein EIN-Signal erzeugt.

##### Hysterese

![Hysterese](Hysterese.png)

Bei diesem Konverter legt man einen Einschalt- und einen Ausschalt-Wert fest. Der Einschaltwert sollte immer größer als der Ausschaltwert sein. Sobald der Wert vom Eingang n den Einschaltwert erreicht oder überschreitet, wird ein EIN-Singal erzeugt. Erst wenn der Wert wieder den Ausschaltwert erreicht oder diesen unterschreitet, wird ein AUS-Signal erzeugt.

Liegt der Wert zwischen dem Ein- und Ausschaltwert, so wird das zuletzt gültige Signal (EIN oder AUS) erzeugt. Falls vorher noch kein gültiges Signal erzeugt worden ist (z.B. beim Gerätestart), wird auch weiterhin kein Signal erzeugt. Somit beginnt ein Hysterese-Konverter nach einem Systemstart erst zu arbeiten, wenn einer der beiden Schwellwerte erreicht wurde.

Formal würde man schreiben:

    WENN Eingang n <= Ausschalt-Wert DANN AUS
    WENN Eingang n >= Einschalt-Wert DANN EIN

In dem angezeigten Bildschirmausschnitt könnte das ein Hystereseschalter für Helligkeit sein. Bei einer Helligkeit von mehr als 40000 Lux würde ein EIN-Signal erzeugt werden, das erst zu einem AUS-Signal führt, wenn die Helligkeit unter 20000 Lux sinkt.

##### Differenzhysterese

![Differenzhysterese](Differenzhysterese.png)

Bei diesem Konverter legt man - wie bei der Hysterese - einen Einschalt- und einen Ausschalt-Wert fest. Der Wert, der mit den Grenzen verglichen wird, ist allerdings die Differenz von dem Eingang n und dem anderen Eingang des Logikkanals. Bei Eingang 1 wird also (Eingang 1 - Eingang 2) gerechnet, bei Eingang 2 aber (Eingang 2 - Eingang 1). Wenn der errechnete Wert den Einschaltwert erreicht oder überschreitet, wird ein EIN-Singal erzeugt. Erst wenn der errechnete Wert wieder den Ausschaltwert erreicht oder diesen unterschreitet, wird ein AUS-Signal erzeugt.

Liegt der Wert zwischen dem Ein- und Ausschaltwert, so wird das zuletzt gültige Signal (EIN oder AUS) erzeugt. Falls vorher noch kein gültiges Signal erzeugt worden ist (z.B. beim Gerätestart), wird auch weiterhin kein Signal erzeugt. Somit beginnt ein Hysterese-Konverter nach einem Systemstart erst zu arbeiten, wenn einer der beiden Schwellwerte erreicht wurde.

Formal würde man (z.B. für Eingang 1) schreiben:

    WENN (Eingang 1 - Eingang 2) <= Ausschalt-Wert DANN AUS
    WENN (Eingang 1 - Eingang 2) >= Einschalt-Wert DANN EIN

In dem angezeigten Bildschirmausschnitt könnte das ein Hystereseschalter für eine Zusatzheizung sein, bei der der Sollwert am Eingang 1 anliegt. Der Istwert am Eingang 2. Ist der Solwert nun 2 Grad größer als der Istwert, wird ein EIN-Signal erzeugt. Erst wenn der Sollwert um 2 Grad kleiner als der Istwert ist, wird ein AUS-Signal erzeugt.

Die Differenzhysterese erlaubt eine Hysterese zu definieren, bei der man den Arbeitspunkt über den KNX-Bus einstellen kann.

### Der "andere" Eingang bei Differenzkonvertern

Zahlenbasierte Konverter könenn auch als Differenzkonverter genutzt werden. Dabei wird dann automatisch der "andere" Eingang aktiviert und für die Differenzberechnung genutzt.

Mit "anderer" Eingang ist foglendes gemeint:

* Für Eingang 1 ist der Differenzeingang der Eingang 2, es wird Eingang 1 - Eingang 2 gerechnet
* Für Eingang 2 ist der Differenzeingang der Eingang 1, es wird Eingang 2 - Eingang 1 gerechnet

Der Differenzeingang muss nicht vom gleichen DPT sein, er wird generisch in eine Zahl konvertiert. Allerdings kann das zu unerwarteten Ergebnissen führen. Empfohlen wird bei Differenzkonvertern der gleiche DPT.

Wird ein Differenzeingang genutzt, sollte dieser nicht auch noch als "normal aktiv" oder "invertiert aktiv" bei einer logischen Operation genutzt werden, obwohl das grundsätzlich möglich ist. Dann würde der Differenzeingang auch einen Konverter anbieten und entsprechend ein EIN- oder AUS-Signal für die logische Operation erzeugen. Dies ist ausdrücklich nicht empfohlen und auch bisher nicht getestet. Ob man die Komplexität eines solchen Aufbaus noch durchblicken kann, ist auch zu bezweifeln.

## Kanalausgänge verbinden

![Interne Eingänge](InternerEingang.png)

Wird für eine logische Operation "Kanalausgang X" oder "Kanalausgang Y" als "normal aktiv" oder "invertiert aktiv" freigeschaltet, erscheint diese Seite.

Solange der Ausgang eines anderen Kanals nicht zugeordnet worden ist, wird der interne Eingang von der logischen Operation als undefiniert betrachtet.

Ausgänge von anderen Kanälen können dazu genutzt werden, große Logikblöcke zu bauen, ohne für jede Teillogik (jenden Logikkanal) eine eigene GA zur Verbindung von Eingang und Ausgang zu benötigen.

### Beschreibung interner Eingang 1

Erscheint nur, wenn bei der logischen Operation "Kanalausgang X" als "normal aktiv" oder "invertiert aktiv" ausgewählt wurde.

Diese Feld erlaubt eine kurze Beschreibung, wozu dieser Eingang verwendet wird. Es hat keinen Einfluß auf die Funktion des Eingangs und dient rein zu Dokumentationszwecken.

### Kanalausgang X als Eingang, X =

Erscheint nur, wenn bei der logischen Operation "Kanalausgang X" als "normal aktiv" oder "invertiert aktiv" ausgewählt wurde.

Als Eingabe wird hier die Nummer der Logik erwartet, deren Ausgang als interner Eingang genutzt werden soll. Solange der Eingang nicht verbunden ist (Wert im Eingabefeld ist 0) erscheint eine Warnmeldung, dass der Eingang inaktiv (undefiniert) ist.

Es kann auch der Ausgang des aktuellen Kanals als interner Eingang verwendet werden. Da dies aber schwer abzusehende Seiteneffekte haben kann, die im Falle einer Schleife auch den Bus mit vielen Telegrammen fluten können, erscheint in einem solchen Fall eine Warnung:
![Warnung Rueckkopplung](Rueckkopplung.png)

### Beschreibung interner Eingang 2

Erscheint nur, wenn bei der logischen Operation "Kanalausgang Y" als "normal aktiv" oder "invertiert aktiv" ausgewählt wurde.

Diese Feld erlaubt eine kurze Beschreibung, wozu dieser Eingang verwendet wird. Es hat keinen Einfluß auf die Funktion des Eingangs und dient rein zu Dokumentationszwecken.

### Kanalausgang Y als Eingang, Y =

Erscheint nur, wenn bei der logischen Operation "Kanalausgang Y" als "normal aktiv" oder "invertiert aktiv" ausgewählt wurde.

Als Eingabe wird hier die Nummer der Logik erwartet, deren Ausgang als interner Eingang genutzt werden soll. Solange der Eingang nicht verbunden ist (Wert im Eingabefeld ist 0) erscheint eine Warnmeldung, dass der Eingang inaktiv (undefiniert) ist.

Es kann auch der Ausgang des aktuellen Kanals als interner Eingang verwendet werden. Da dies aber schwer abzusehende Seiteneffekte haben kann, die im Falle einer Schleife auch den Bus mit vielen Telegrammen fluten können, erscheint in einem solchen Fall eine Warnung:
![Warnung Rueckkopplung](Rueckkopplung.png)

## Ausgang

Zwischen dem Ausgang der logischen Operation und dem physikalischen Ausgang des Logikkanals (als Kommunikationsobjekt, um KNX-Telegramme zu verschicken) können Funktionsblöcke aktiviert werden (dargestellt im Kapitel Logikblöcke), die das Aussgangssignal beeinflussen.

![Ausgang](Ausgang.png)
In der Grundeinstellung sind alle Funktionsblöcke deaktiviert und die Signale der logischen Operation gelangen direkt zum physikalischen Ausgang.

Im folgenden Werden alle Logikblöcke, deren Einstellungen und deren Beeinflussungsmöglichkeiten beschrieben.

### Beschreibung Ausgang

Diese Einstellung hat keine funktionale Auswirkung, erlaubt es aber, dem Ausgang einen Text zu geben, um ihn einfacher zu finden.

Der eingegebene Text erscheint auf dem Ausgang-Tag des Logikkanals und als Name des Kommunikationsobjekts, das diesem Ausgang zugeordnet ist.

### Ausgang hat eine Treppenlichtfunktion

Wird hier ein "Ja" ausgewählt, erscheinen folgende Felder:

![Treppenlicht](Treppenlicht.png)

Mit den Einstellungen kann ein Treppenlicht mit Blinkfunktion konfigureiert werden. Ein Treppenlicht erzeugt, sobald es durch ein EIN-Signal getriggert wird, ein EIN-Signal, dass nach einer gewissen Zeit zu einem AUS-Signal wird. Man kann bestimmen, ob ein weiterer Trigger mit einem EIN-Signal dazu führt, dass die Treppenlichtzeit erneut anfängt und somit der Trigger das Treppenlicht verlängert. Ferner kann man festlegen, ob ein weiterer Trigger mit einem AUS-Singnal das Treppenlicht ausschaltet oder nicht.

Solange das Treppenlicht aktiv ist, kann ein Blinkmodul den Ausgang des Funktionsmoduls in einem felsgelegten Intervall EIN- und AUSschalten.

#### Zeitbasis für Treppenlicht

Die Dauer, die ein Treppenlicht eingeschaltet bleiben soll, kann von 1/10 Sekunden bis zu vielen Stunden gehen. Um die Zeiteingabe einfacher zu gestalten, gibt man erst eine Zeitbasis an, gefolgt von einer Zeit passend zu dieser Zeitbasis. Die Zeitbasis kann

* 1/10 Sekunden
* 1 Sekunde
* 1 Minute
* 1 Stunde

sein.

#### Zeit für Treppenlicht

Hier gibt man die Zeit an, die das Treppenlicht eingeschaltet bleiben soll. Die Zeit wird passend zur Zeitbasis ausgewertet. Eine 10 mit der Zeitbasis "1 Minute" ergibt eine Zeit von 10 Minuten.

#### Treppenlicht kann verlängert werden

Wählt man hier "Ja", führt ein erneutes EIN-Signal am Eingang des Funktionsblocks zum erneuten Anlaufen der Zeitzählung bei 0. Somit wird die Treppenlichtzeit zurückgesetzt und beginnt von neuem, die Treppenlichtzeit wurde somit verlängert.

#### Treppenlicht kann ausgeschaltet werden

Wählt man hier ein "Ja", führt ein AUS-Signal am Eingang des Funktionsmoduls zum sofortigen Beenden des Treppenlichts und einem AUS-Signal am Ausgang des Funktionsmoduls.

Ist ein "Nein" ausgewählt, wird ein AUS-Singal am Eingang des Funktionsmoduls ignoriert und hat keine Auswirkungen, solange das Treppenlicht eingeschaltet ist.

#### Treppenlicht blinkt im n/10 Sekunden Rythmus

Bei einer Eingabe einer Zahl größer 0 wird, solange das Treppenlicht eingeschaltet ist, in dem angegebenen Takt der Ausgang des Funktionsmoduls abwechselnd ein EIN- und ein AUS-Signal erzeugt. Ist der Takt z.B. 20, wird nach 20/10=2 Sekunden von EIN auf AUS gewechselt, nach wieder 2 Sekunden dann von AUS auf EIN und so fort. Sobald das Treppenlicht beendet ist, sei es durch den Ablauf der Treppenlichtzeit oder durch ein AUS-Signal von Außen, wird das blinken beendet und ein finales AUS-Signal gesendet.

Bei der Eingabe einer 0 wird die Blinkfunktion deaktiviert.

Der Bildschirmausschnitt oben zeigt ein Beispiel für ein 3 maliges Blinken in einer Sekunde, wobei die einzelnen Blinkzyklen nur 0,2 Sekunden dauern. Wenn man den Ausgang mit einem Buzzer verbindet, erhhält man eine akustische Rückmeldung (3 mal piep) für z.B. einen Fehlerfall.

### Ausgang schaltet zeitverzögert

Wird hier ein "Ja" ausgewählt, erscheinen folgende Felder:

![Verzögerung](Verzögerung.png)

Jedes EIN- oder AUS-Signal, dass bei diesem Funktionsblock ankommt, kann verzögert werden, mit unterschiedlichen Zeiten für die EINschalt- und AUSschaltverzögerung. Die Zeitbasis sind 1/10 Sekunden, um auch kurze Verzögerungen erreichen zu können.

Ist eine Verzögerung aktiv, kann man auch angeben, was mit folgenden EIN- bzw. AUS-Singalen passieren soll.

Im folgenden werden die Parameter im Detail erklärt.

#### EINschalten wird um n/10 Sekunden verzögert (0 = nicht verzögern)

Wird hier eine Zahl größer 0 eingegeben, wird das EIN-Singal um so viele zehntel Sekunden verzögert am Ausgang des Funktionsmoduls ausgegeben.

Wird eine 0 eingegeben, findet keine Verzögerung statt.

#### Erneutes EIN führt zu

Dieses Auswahlfeld erscheint nur, wenn eine EINschaltverzögerung stattfinden soll.

Während das Funktionsmodul ein EIN-Signal verzögert, muss definiert werden, wie ein weiteres EIN-Signal während der Verzögerung behandelt werden soll.

##### Verzögerung bleibt bestehen

Während eine Verzögerung von einem EIN-Singal aktiv ist, werden daruaffolgende EIN-Signale ignoriert. Nur das erste EIN-Signal wird verzögert und nach der festgelegten Zeit weitergesendet.

##### Verzögerung wird verlängert

Während eine Verzögerung von einem EIN-Signal aktiv ist, führt ein darauffolgendes EIN-Signal zum Neustart der Verzögerungszeit. Somit läuft die Verzögerung erneut an. Dies hat zur Folge, dass das letze EIN-Singal verzögert weitergeleitet wird und faktisch alle vorhergehenden ignoriert werden.

##### Sofort schalten ohne Verzögerung

Kommt während eine Verzögerung eines EIN-Signals aktiv ist ein weiteres EIN-Signal, wird die Verzögerung sofort beendet und das zweite EIN-Signal sofort weitergeleitet. So kann man eine Aktion, die automatisiert verzögert laufen soll, durch ein manuelles Signal sofort beginnen lassen.

#### Darauffolgendes AUS führt zu

Dieses Auswahlfeld erscheint nur, wenn eine EINschaltverzögerung stattfinden soll.

Während das Funktionsmodul ein EIN-Signal verzögert, muss definiert werden, wie ein weiteres AUS-Signal während der Verzögerung behandelt werden soll.

##### Verzögerung bleibt bestehen

Während eine Verzögerung von einem EIN-Signal aktiv ist, führt ein darauffolgendes AUS-Signal zu keiner Reaktion, es wird ignoriert. Das EIN-Signal wird nach der eingestellten Verzögerungszeit gesendet.

##### Verzögerung beenden ohne zu schalten

Kommt während eine Verzögerung eines EIN-Signals aktiv ist ein AUS-Signal, wird die Verzögerung beendet und das EIN-Signal ignoriert. Auch das AUS-Singal wird nicht gesendet, da der Ausgang des Funktionsmoduls ja bereits aus war.

#### AUSschalten wird um n/10 Sekunden verzögert (0 = nicht verzögern)

Wird hier eine Zahl größer 0 eingegeben, wird das AUS-Singal um so viele zehntel Sekunden verzögert am Ausgang des Funktionsmoduls ausgegeben.

Wird eine 0 eingegeben, findet keine Verzögerung statt.

#### Erneutes AUS führt zu

Dieses Auswahlfeld erscheint nur, wenn eine AUSschaltverzögerung stattfinden soll.

Während das Funktionsmodul ein AUS-Signal verzögert, muss definiert werden, wie ein weiteres AUS-Signal während der Verzögerung behandelt werden soll.

##### Verzögerung bleibt bestehen

Während eine Verzögerung von einem AUS-Singal aktiv ist, werden daruaffolgende AUS-Signale ignoriert. Nur das erste AUS-Signal wird verzögert und nach der festgelegten Zeit weitergesendet.

##### Verzögerung wird verlängert

Während eine Verzögerung von einem AUS-Signal aktiv ist, führt ein darauffolgendes AUS-Signal zum Neustart der Verzögerungszeit. Somit läuft die Verzögerung erneut an. Dies hat zur Folge, dass das letze AUS-Singal verzögert weitergeleitet wird und faktisch alle vorhergehenden ignoriert werden.

##### Sofort schalten ohne Verzögerung

Kommt während eine Verzögerung eines AUS-Signals aktiv ist ein weiteres AUS-Signal, wird die Verzögerung sofort beendet und das zweite AUS-Signal sofort weitergeleitet. So kann man eine Aktion, die automatisiert verzögert ausgeschaltet werden soll, durch ein manuelles Signal sofort beenden.

#### Darauffolgendes EIN führt zu

Dieses Auswahlfeld erscheint nur, wenn eine AUSschaltverzögerung stattfinden soll.

Während das Funktionsmodul ein AUS-Signal verzögert, muss definiert werden, wie ein weiteres EIN-Signal während der Verzögerung behandelt werden soll.

##### Verzögerung bleibt bestehen

Während eine Verzögerung von einem AUS-Signal aktiv ist, führt ein darauffolgendes EIN-Signal zu keiner Reaktion, es wird ignoriert. Das AUS-Signal wird nach der eingestellten Verzögerungszeit gesendet.

##### Verzögerung beenden ohne zu schalten

Kommt während eine Verzögerung eines AUS-Signals aktiv ist ein EIN-Signal, wird die Verzögerung beendet und das AUS-Signal ignoriert. Auch das EIN-Singal wird nicht gesendet, da der Ausgang des Funktionsmoduls ja bereits an war.

### Wiederholungsfilter

![Wiederholungsfilter](Wiederholungsfilter.png)

Durch verschiedene Kombinationen von logischer Operation, Treppenlicht, Blinken, EIN- und AUSschaltverzögerung kann es passieren, dass mehrere EIN- oder AUS-Signale hintereinander gesendet werden und zu ungewollten Effekten auf dem KNX-Bus oder bei weiteren Logikkanälen führen.

Das Auswahlfeld "Wiederholungsfilter" erlaubt das Ausfiltern von unerwünschten Wiederholungen.

#### Alle Wiederholungen durchlassen

Es wird nichts gefiltert, sowohl mehrfach aufeinanderfolgende EIN-Signale wie auch mehrfach aufeinanderfolgende AUS-Signale werden durchgelassen und stehen auf Ausgang des Funktionsmoduls zur Verfügung.

#### Nur EIN-Wiederholugen durchlassen

Mehrfach aufeinanderfolgende EIN-Signale werden durchgelassen. Wenn mehrere AUS-Signale aufeinanderfolgen, wird nur das erste AUS-Signal zum Ausgang des Funktionsmoduls durchgelassen.

#### Nur AUS-Wiederholugen durchlassen

Mehrfach aufeinanderfolgende AUS-Signale werden durchgelassen. Wenn mehrere EIN-Signale aufeinanderfolgen, wird nur das erste EIN-Signal zum Ausgang des Funktionsmoduls durchgelassen.

#### Keine Wiederholungen durchlassen

Alle Wiederholungen von EIN- oder AUS-Singalen werden ignoriert, es wird immer nur das erste EIN- oder AUS-Signal durchgelassen. Somit stehen am Ausgang des Funktionsmoduls effektiv nur Signalwechsel zur Verfügung.

### Ausgang wiederholt zyklisch

Wird hier ein "Ja" ausgewählt, erscheinen folgende Felder:

![Zyklisch](ZyklischSenden.png)

Nachdem durch den Wiederholungsfilter unbeabsichtigte Wiederholugen ausgefiltert wurden, werden in diesem Funktionsblock beabsichtigte Wiederholungen definiert.

Man kann sowohl das EIN- wie auch das AUS-Signal in unterschiedlichen Zeitintervallen wiederholen lassen. Auch hier beträgt die Zeitbasis 1/10 Sekunden! Man sollte sehr umsichtig mit Wiederholungsintervallen unter 1 Minute umgehen, da man damit sehr leicht den KNX-Bus lahmlegen kann.

Man kann natürlich auch nur das EIN- oder das AUS-Signal wiederholen lassen.

In dem oben dargestellten Bildschirmausschnitt würde das EIN-Signal alle 5 Minuten wiederholt werden, das AUS-Signal jede Stunde.

#### EIN-Telegramm wird alle n/10 Sekunden wiederholt (0 = nicht wiederholen)

Das Feld erscheint nur, wenn bei "Ausgang wiederholt zyklisch" ein "Ja" ausgewählt wurde.

Die hier eingegebene Zahl in 1/10 Sekudnen bestimmt das Zeitintervall, in dem dem das EIN-Signal wiederholt wird.

Die Eingabe einer 0 deaktiviert eine Wiederholung.

#### AUS-Telegramm wird alle n/10 Sekunden wiederholt (0 = nicht wiederholen)

Das Feld erscheint nur, wenn bei "Ausgang wiederholt zyklisch" ein "Ja" ausgewählt wurde.

Die hier eingegebene Zahl in 1/10 Sekudnen bestimmt das Zeitintervall, in dem dem das AUS-Signal wiederholt wird.

Die Eingabe einer 0 deaktiviert eine Wiederholung.

### Ausgang für interne Eingänge

An dieser Stelle endet die binäre Verarbeitung. Alle hier ankommenden EIN- oder AUS-Signale werden, sofern verbunden, an die entsprechenden internen Eingänge weitergeleitet und triggern dort die entsprechenden logischen Operationen.

## Ausgangs-Konverter

Das letzte Funktionsmodul auf dem Ausgangsbild ist ein Konverter, der das bis hierher ermittelte EIN- oder AUS-Signal in einen bestimmten DPT konvertiert und den resultierenden Wert in ein KO schreibt, damit es auf den KNX-Bus gesendet werden kann.

![Ausgangskonverter](Ausgangskonverter.png)

### DPT für Ausgang

Dieses Auswahlfeld legt den DPT für den Ausgang fest. Unterstützt werden:

* DPT 1: binärer Wert
* DPT 2: Zwangsführung
* DPT 5: vorzeichenlose Zahl (0 bis 255)
* DPT 5.001: Prozentzahl (0 bis 100)
* DPT 6: vorzeichenbehaftete Zahl (-128 bis 127)
* DPT 7: vorzeichenlose Zahl (0 bis 65535)
* DPT 8: vorzeichenbehaftete Zahl (-32768 bis 32767)
* DPT 9: Gleitkommawert (-670760,96 bis 670760,96)
* DPT 16: Text (bis 14 Byte)
* DPT 17: Szenen Nummer (1-64)
* DPT 232: RGB-Wert (3*8 Bit Rot-, Grün-, Blauwert)

Je nach gewähltem DPT unterscheiden sich die folgenden Felder leicht. Es werden erstmal die parameter für alle DPT beschrieben und anschließend die DPT-spezifischen.

### Wert für EIN senden?

![WertEinDropdown](WertEinSenden.png)

In dieser Auswahlbox wird festgelegt, ob und was für ein Wert bei einem EIN-Signal gesendet werden soll.

#### Nein

Für ein EIN-Signal wird kein Wert auf den Bus gesendet. Das entspricht einem Augangsfilter, der alle EIN-Signale unterdrückt.

#### Ja - Wert vorgeben

Hier kann der Wert, der für ein EIN-Signal gessendet wird, konstant vorgegeben werden. In einem weiteren Feld kann der konstante Wert DPT gerecht eingegeben werden.

#### Ja - Wert von Eingang 1

Bei einem EIN-Signal wird der Wert gesendet, der am Eingang 1 anliegt. Sollte der Wert nicht den passenden DPT haben, wird er generisch gewandelt.

#### Ja - Wert von Eingang 2

Bei einem EIN-Signal wird der Wert gesendet, der am Eingang 2 anliegt. Sollte der Wert nicht den passenden DPT haben, wird er generisch gewandelt.

#### Ja - ReadRequest senden

Bei einem EIN-Signal wird kein Wert auf die GA am Ausgang gesendet sondern ein Lesetelegramm. Damit kann man für Geräte, die kein zyklisches Senden unterstützen, bei bedarf eine Abfrage eines Ausgangs erreichen.

#### Ja - 'Gerät zurücksetzen' senden

Bei einem EIN-Signal wird kein Wert gesendet, sondern die ETS-Funktion "Gerät zurücksetzen" an eine bestimmte PA geschickt. So kann man bestimmte Geräte überwachen und bei Bedarf zurücksetzen, ohne die ETS starten zu müssen.

#### Ja - Tonwiedergabe (Buzzer)

Wird nur angeboten, wenn ein Buzzer vorhanden ist.

Bei einem EIN-Signal wird kein Wert gesendet, sondern der interne Buzzer zur Tonwiedergabe angesprochen. In einem weiteren Feld wird angegeben, in welcher Lautstärke die Tonwiedergabe gestartet oder ob sie gestoppt wird.

![Tonwiedergabe](Tonwiedergabe.png)

#### Ja - RGB-LED schalten

Wird nur angeboten, wenn eine RGB-LED vorhanden ist.

Bei einem EIN-Signal wird kein Wert gesendet, sondern die interne RBG-LED angesprochen. So kann man eine optische Rückmeldung erreichen.

In einem weiteren Feld wird die Farbe eingestellt. Ist die Farbe Schwarz eingestellt, wir die LED ausgeschaltet.

### Wert für EIN senden als

![Wert senden in Prozent](WertSendenProzent.png)

Das Feld erscheint nur, wenn für "Wert für EIN senden" ein "Ja - Wert vorgeben" ausgewählt wurde.

Hier wird ein konstanter Wert erwartet, der zu dem Ausgewählten DPT für den Ausgang passt. Dieser eingegebene Wert wird auf den KNX-Bus bei einem EIN-Signal gesendet.

Man kann dies z.B. auch zur Invertierung nutzen, indem bei einem DPT 1 für ein EIN-Signal der Wert AUS gesendet wird und umgekehrt.

### Physilalische Adresse

![Gerät zurücksetzen](ResetDevice.png)

Das Feld erscheint nur, wenn für "Wert für EIN senden" ein "Ja - 'Gerät zurücksetzen' senden" ausgewählt wurde.

Hier wird eine physikalische Adresse in der üblichen Punkt-Notation erwartet. Das KNX-Gerät mit dieser physikalischen Adresse wird zurückgesetzt.

Dies entspricht genau der Funktion "Gerät zurücksetzen" in der ETS.

### LED-Farbe festlegen (Schwarz=aus)

![Led farbe festlegen](LedColor.png)

Das Feld erscheint nur, wenn für "Wert für EIN senden" ein "Ja - RGB-LED schalten" ausgewählt wurde.

Hier wird die Farbe der LED bestimmt, in der sie leuchten soll. Wird die Farbe Schwarz gewählt (#000000), geht die LED aus. Für die Auswahl der Farbe kann auch ein Farbauswahldialog verwendet werden.

Diese Option kann nur funktionieren, wenn das Gerät, auf dem die Applikation Logik läuft, auch eine RGB-LED verbaut hat.

### Wert für AUS senden?

In dieser Auswahlbox wird festgelegt, ob und was für ein Wert bei einem AUS-Signal gesendet werden soll.

#### Nein

Für ein AUS-Signal wird kein Wert auf den Bus gesendet. Das entspricht einem Augangsfilter, der alle AUS-Signale unterdrückt.

#### Ja - Wert vorgeben

Hier kann der Wert, der für ein AUS-Signal gessendet wird, konstant vorgegeben werden. In einem weiteren Feld kann der konstante Wert DPT gerecht eingegeben werden.

#### Ja - Wert von Eingang 1

Bei einem AUS-Signal wird der Wert gesendet, der am Eingang 1 anliegt. Sollte der Wert nicht den passenden DPT haben, wird er generisch gewandelt.

#### Ja - Wert von Eingang 2

Bei einem AUS-Signal wird der Wert gesendet, der am Eingang 2 anliegt. Sollte der Wert nicht den passenden DPT haben, wird er generisch gewandelt.

#### Ja - ReadRequest senden

Bei einem AUS-Signal wird kein Wert auf die GA am Ausgang gesendet sondern ein Lesetelegramm. Damit kann man für Geräte, die kein zyklisches Senden unterstützen, bei bedarf eine Abfrage eines Ausgangs erreichen.

#### Ja - 'Gerät zurücksetzen' senden

Bei einem AUS-Signal wird kein Wert gesendet, sondern die ETS-Funktion "Gerät zurücksetzen" an eine bestimmte PA geschickt. So kann man bestimmte Geräte überwachen und bei Bedarf zurücksetzen, ohne die ETS starten zu müssen.

#### Ja - Tonwiedergabe (Buzzer)

Wird nur angeboten, wenn ein Buzzer vorhanden ist.

Bei einem AUS-Signal wird kein Wert gesendet, sondern der interne Buzzer zur Tonwiedergabe angesprochen. In einem weiteren Feld wird angegeben, ob die Tonwiedergabe gestartet oder gestoppt wird.

#### Ja - RGB-LED schalten

Wird nur angeboten, wenn eine RGB-LED vorhanden ist.

Bei einem AUS-Signal wird kein Wert gesendet, sondern die interne RBG-LED angesprochen. So kann man eine optische Rückmeldung erreichen.

In einem weiteren Feld wird die Farbe eingestellt. Ist die Farbe Schwarz eingestellt, wir die LED ausgeschaltet.

### Wert für AUS senden als

Das Feld erscheint nur, wenn für "Wert für AUS senden" ein "Ja - Wert vorgeben" ausgewählt wurde.

Hier wird ein konstanter Wert erwartet, der zu dem Ausgewählten DPT für den Ausgang passt. Dieser eingegebene Wert wird auf den KNX-Bus bei einem AUS-Signal gesendet.

Man kann dies z.B. auch zur Invertierung nutzen, indem bei einem DPT 1 für ein AUS-Signal der Wert EIN gesendet wird und umgekehrt.

### Physilalische Adresse

Das Feld erscheint nur, wenn für "Wert für AUS senden" ein "Ja - 'Gerät zurücksetzen' senden" ausgewählt wurde.

Hier wird eine physikalische Adresse in der üblichen Punkt-Notation erwartet. Das KNX-Gerät mit dieser physikalischen Adresse wird zurückgesetzt.

Dies entspricht genau der Funktion "Gerät zurücksetzen" in der ETS.

### LED-Farbe festlegen (Schwarz=aus)

Das Feld erscheint nur, wenn für "Wert für AUS senden" ein "Ja - RGB-LED schalten" ausgewählt wurde.

Hier wird die Farbe der LED bestimmt, in der sie leuchten soll. Wird die Farbe Schwarz gewählt (#000000), geht die LED aus. Für die Auswahl der Farbe kann auch ein Farbauswahldialog verwendet werden.

Diese Option kann nur funktionieren, wenn das Gerät, auf dem die Applikation Logik läuft, auch eine RGB-LED verbaut hat.

## DPT Konverter

Das Gerät hat ein Funktionsmodul DPT-Konverter eingebaut, dass parameterlos funktioniert (deswegen wird es auch generischer Konverter bezeichnet).

Der DPT konverter konvertiert einen Von-DPT in einen Nach-DPT und wird implizt an Stellen aufgerufen, an den das notwendig ist, wenn also für Berechnungen oder Zuweisungen unterschiedliche DPT vorliegen.

Derzeit passiert das

* bei einem Differenzkonverter, wenn die beiden Eingänge unterschiedliche DPT haben. Hier ist der Nach-DPT immer der DPT, den der Eingang besitzt, der den Differenzkonverter nutzt. Der Von-DPT ist der DPT des "anderen" Eingangs.
* Bei einem Ausgang, wenn dieser den Wert eines Eingangs senden soll und die unterschiedliche DPT haben. Hier ist der Von-DPT immer der Eingangs-DPT und der Nach-DPT der Ausgangs-DPT.

Da die Konvertierung nicht parametrierbar ist, erfolgt sie nach einfachen (generischen) Regeln. Auch wenn prinzipiell von jedem Von-DPT zu jedem Nach-DPT konvertiert werden kann, müssen für einige Konvertierungen die Rahmenbedingungen bekannt sein, vor allem wie in Grenzfällen verfahren wird. Es ist z.B. offensichtlich, dass ein 2-Byte-Wert 365 nicht verlustfrei in einen 1-Byte-Wert (Wertebereich 0-255) konvertiert werden kann.

Konvertierungen erfolgen nach folgender Tabelle, wobei der Von-DPT in den Zeilen, der Nach-DPT in den Spalten steht:

DPT | 1 | 2 | 5 | 5.001 | 6 | 7 | 8 | 9 | 16 | 17 | 232
:---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:
1 | I | Z<sub>B</sub> | G<sub>B</sub> | G<sub>B</sub> | G<sub>B</sub> | G<sub>B</sub> | G<sub>B</sub> | G<sub>B</sub> | T<sub>B<sub> | S<sub>B</sub> | G<sub>B</sub>
2 | B | I | G<sub>Z</sub> | G<sub>Z</sub> | G<sub>Z</sub> | G<sub>Z</sub> | G<sub>Z</sub> | G<sub>Z</sub> | T<sub>Z</sub> | S<sub>Z</sub> | G<sub>Z</sub>
5 | B | Z | I | G<sub>W</sub> | G | G | G | G | T | S  | G
5.001 | B | Z | G | I | G | G | G | G | T | S | G
6 | B | Z | G<sub>V</sub> | G<sub>VW</sub> | I | G<sub>V</sub> | G | G | T | S  | G
7 | B | Z | G<sub>W</sub> | G<sub>W</sub> | G<sub>W</sub> | I | G | G | T | S  | G
8 | B | Z | G<sub>VW</sub> | G<sub>VW</sub> | G<sub>W</sub> | G<sub>V</sub> | I | G | T | S  | G
9 | B | Z | G<sub>VW</sub> | G<sub>VW</sub> | G<sub>W</sub> | G<sub>VW</sub> | G<sub>W</sub> | I | T | S  | G
17 | B | Z | G | G | G | G | G | G | T | I | G
232 | B | Z | G<sub>VW</sub> | G<sub>VW</sub> | G<sub>W</sub> | G<sub>VW</sub> | G<sub>W</sub> | G<sub>W</sub> | T | S  | I

Die Einträge an den Schnittpunkten haben folgende Bedeutung:

Eintrag | Konverter | Bedeutung
:---:|:---|:---
I | Identität | keine Konvertierung notwendig, DPT sind gleich
B | Binär | Wert wird in eine Ganzzahl gewandelt. Eine 0 wird in ein AUS-Signal konvertiert, alle anderen Werte in ein EIN-Signal.
G | Generisch | Wert wird in eine Ganzzahl gewandelt und anschließend zugewiesen.
G<sub>B</sub> | Generisch (von Binär) | AUS wird in eine 0 konvertiert, EIN in eine 1.
G<sub>V</sub> | Generisch (mit geändertem Vorzeichen) | Wie G, nur hat der Von-DPT möglicherweise ein Vorzeichen. Falls der Wert negativ ist, wird vor der Zuweisung das Vorzeichen entfernt (mit -1 multipliziert).
G<sub>W</sub> | Generisch (mit Werteinschränkung) | Wie G, nur hat der Von-DPT einen größeren Wertebereich als der Nach-DPT. Vor der Zuweisung wird noch modulo Wertebereich des Nach-DPT gerechnet.
G<sub>VW</sub> | Generisch (mit geändertem Vorzeichen und Werteinschränkung) | Wie G<sub>V</sub> gefolgt von G<sub>W</sub>.
G<sub>Z</sub> | Generisch (von Zwang) | "normal aus" (00) wird in eine 0 konvertiert, "normal ein" (01) in eine 1, "priorität aus" (10) in eine 2 und "priorität ein" (11) in eine 3.
S | Szene | Wert wird in eine Ganzzahl gewandelt. Falls negativ, wird das Vorzeichen entfernt (mit -1 multipliziert). Anschließend werden die untersten 6 Bit (Bit0 bis Bit5) genommen. Resultat ist Szene 1-64.
S<sub>B</sub> | Szene (von Binär) | AUS wird in die Szene 1 konvertiert, EIN in eine Szene 2.
S<sub>Z</sub> | Szene (von Zwang) | Wie G<sub>Z</sub>, nur ist die resultierende Szene der konvertierte Wert + 1. Es kommen somit Szenen 1-4 raus.
T | Text | Wert wird in eine Zahl gewandelt und anschließend als Text ausgegeben.
T<sub>B</sub> | Text (von Binär) | AUS wird in einden Text "0" konvertiert, EIN in den Text "1".
T<sub>Z</sub> | Text (von Zwang) | Wie G<sub>Z</sub>, nur werden die Zahlen als Text ausgegeben.
Z |Zwang | Wert wird in eine Ganzzahl gewandelt. Falls negativ, wird das Vorzeichen entfernt (mit -1 multipliziert). Anschließend werden die letzten beiden Bit (Bit0 und Bit1) genommen. Resultat sind die Werte 0 bis 3.
Z<sub>B</sub> |Zwang (von Binär) | Ein AUS wird nach "normal aus" (00) konvertiert, EIN nach "normal ein" (01). Die Werte "priorität aus" (10) und "priorität ein" sind nicht möglich.


----

## Beispiele

Die Beispiele müssen noch ausgearbeitet werden. Die gegebenen Überschriften zeigen aber bereits jetzt eine Liste der möglichen Funktionen.

### 3 Lichtszenen sollen auch den PM sperren (der das nativ nicht unterstützt)

### Ist ein Fenster zu lange offen, soll der Text "Auskühlalarm" als Meldung auf dem Glastaster erscheinen

### Ist die Markise durch einen Alarm (Windalarm/Regenalarm) gesperrt, soll ein langes Piepen und ein rotes blinken bei manueller Bedienung auf den Alarm hinweisen

### Wird die Haustür geöffnet, soll ein dreifaches Piepen darauf hinweisen, dass noch irgendeine eine Terrassentür nicht verschlossen ist

### Fernsehen oder Musikhören soll Präsenz in dem Raum simulieren

### Wenn ein Temperatursensor eine halbe Stunde lang kein Signal gesendet hat, diesen zurücksezten

----

## Hardware

Dieses Kapital beschreibt die von dieser Firmware unterstützte Hardware
(noch nicht ausgearbeitet)

Buzzer

RGB-LED

----

## Übersicht der vorhandenen Kommunikationsobjekte

KO | Name | DPT | Bedeutung
:---:|:---|---:|:--
1 | in Betrieb | 1.002 | Meldet zyklisch auf den Bus, dass das Gerät noch funktioniert. Das KO steht nicht zur Verfügung, wenn kein Sendezyklus eingestellt wurde.
2 | Uhrzeit | 10.001 | Eingnang zum empfangen der Uhrzeit
3 | Datum | 11.001 | Eingang zum empfangen des Datums
n | Eingang 1 | *) | Eingang 1 für einen Logikkanal
n+1 | Eingang 2 | *) | Eingang 2 für einen Logikkanal
n+2 | Ausgang | **) | Ausgang eines Logikkanals

*) Eingangs-DPT ist 1, 2, 5, 5.001, 6, 7, 8, 9, 17, 232

**) Ausgangs-DPT ist Eingangs-DPT ergänzt um DPT 16.

Jeder Logikkanal hat genau 3 aufeinanderfolgende Kommunikationsobjekte. Wenn n der Eingang 1 für Kanal x ist, so ist n+3 der Eingang 1 für Kanal x+1. Bei 80 Kanälen ist das letzte KO der Ausgang für Kanal 80 und hat die Nummer n+239.

n für Kanal 1 ist von dem Gerät abhängig, auf dem die Applikation Logik läuft:

* Für das Logikmodul ist n=10, somit ist das letzte belegte KO 249.
* Für das Sensormodul ist n=50, somit ist das letzte belegte KO 289.
