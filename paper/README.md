# Vorlage für eine Bachelorarbeit an der WWU Münster
Template for a bachelor thesis in mathematics at WWU Münster

## Zur Schriftart
In der Datei `preamble.tex` befinden sich zwei verschiedene Blöcke, die sich mit Schriftarten beschäftigen. Der obere anfangs nicht auskommentierte ist für `pdflatex` gedacht und der zweite für `xelatex`/`lualatex`. Einer von beiden muss auskommentiert bleiben und in `bachelor.tex` muss in der ersten Zeile der entsprechende Treiber stehen!!! Mit TeXlive funktionieren *beide* Konfigurationen out of the box. Da MikTeX anscheinend nicht in der Lage ist, Schriftarten, die eigentlich zu LaTeX gehören, so wie Pakete automatisch zu installieren, funktioniert die `xelatex`-Konfiguration erst nach einer manuellen Installation der Schriftarten.

Allgemein sollte man die inaktive Konfiguration auch nur verwenden, wenn man auf die OpenType-Features der Schriftarten angewiesen ist, was zum Beispiel der Fall ist, wenn man einen Nachnamen mit einem "ß" auf der Titelseite benutzen will, denn nur die OpenType-Version enthält ein großes "ß"(ẞ) (Ja, so etwas gibt es). Möchte man die Schriftarten ändern, dann möchte man unter Umständen natürlich auch diese Konfiguration benutzen und abändern.

## Troubleshooting 
Wenn die Datei nicht oder nur mit Fehlermeldungen kompiliert, dann ist in 99,9% der Fälle die LaTeX-Installation veraltet! 
Im Zweifelsfall sorgt eine Neuinstallation der aktuellsten Variante für Abhilfe. Im Folgenden erläutere ich verschiedene Probleme, die mit den beiden meist verbreiteten Distributionen von LaTeX auftreten können und wie man die Distributionen aktuell hält

*	__TeXlive__ Von TeXlive werden jährlich neue Versionen herausgebracht, aktuell ist TeXlive 2015. Hier landen bei der Installation eigentlich alle Pakete, die man jemals nutzen wird auf der Festplatte. Pakte lassen sich über einen Manager aktualisieren. Hierbei müssen wir kurz unterscheiden
	*	_MacTeX (OS X)_ ist im Kern eine normale TeXlive Installation mit ein paar extra Programmen und einem einfach zu bedienenden Installer  speziell für den Mac. Die einzelnen Pakete werden über die "Tex Live Utility" (einfach in die Suche eintippen) aktualisiert, welche ziemlich selbst erklärend ist.
	*	_TeXlive (Linux)_ Hier bemüht man den TeXlive-Manager `tlmgr` zum Updaten, zulässige Befehle findet man über die Hilfe (`tlmgr --help`) oder den `man`-Eintrag (`man tlmgr`).
	
	Unter Linux ist die Installation von TeXlive noch sehr entscheidend: Installiert man nämlich TeXlive wie unter Linux üblich über die Paketverwaltung (`apt-get` etc.) erhält man fast immer eine gnadenlos veraltetet Variante (zB 2012, mit der meines Wissens nach die Dateien nicht komplilieren). Deswegen sollte man die Installation unbedingt wie [HIER](http://tug.org/texlive/quickinstall.html "Installation Unix") erläutert vornehmen. Dabei ist insbesondere der Schritt mit dem Setzen der `PATH`-Variable _absolut notwendig_!!! Man möchte diese aber permanent ändern, was unter anderem wie folgt erreicht werden kann:
	*	Man fügt den Befehl zur Konfiguration der Kommandozeile (hier für `bash`) hinzu:
	
			echo 'export PATH=/usr/local/texlive/2015/bin/x86_64-linux:$PATH'  >> /etc/bash.bashrc
		
	*	Oder man bearbeitet die Datei `/etc/environments` entsprechend, denn dort wird die Variable `PATH` initialisiert.
* 	__MikTeX (Windows)__  hat die besondere Eigenschaft, Pakete erst dann zu installieren, wenn sie das erste Mal benötigt werden. Eine frische Installation besorgt sich also automatisch stets die aktuellsten Versionen. Im Startmenü findet man ein Update Programm für Miktex, mit dem man bereits installierte Pakete updaten kann. Eventuell sind dafür mehrere Durchläufe nötig, frag mich nicht warum…
	* 	Die Vorlage nutzt zum Erstellen der Bibliographie ein Programm namens `biber`, welches aus irgendwelchen Gründen in der 64-bit-Version von MikTeX nicht enthalten ist! Von daher sollte man besser die 32-bit-Version benutzen.
	*	Trotz all der Schwierigkeiten mit MikTeX (siehe auch _Schriftart_), ist es doch die beliebteste Distribution auf Windows (wohl auch, weil sie deutlich schneller und in der Installation deutlich kleiner ist, als die Windows-Version von TeXlive).

