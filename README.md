# Qualitativer Vergleich von Beaterkennungsalgorithmen
## Kurzfassung

Beaterkennungsalgorithmen sind Algorithmen,
	die den regelmäßigen Puls von Musikstücken erkennen,
	zu dem wir Menschen oft beim Hören von Musik unterbewusst mit dem Fuß oder einem Finger mittippen.
Viele dieser Algorithmen geben auch Schätzungen des Tempos des Musikstücks aus.
In dieser Arbeit wurden die drei Algorithmen
	[2001_BeatThis], [2009_DaPlSt] und [2011_PlRoSt]
	im Echtzeitbetrieb getestet und verglichen.
Dabei wurden sie auf
	Genauigkeit der Tempovorhersagen, Genauigkeit der Beatzeitpunkte, längste korrekte Beatfolge und benötigte Rechenzeit
	untersucht.
Es wurden Tests implementiert,
	die die Algorithmen auf mit Beatzeitpunkten annotieren Liedern eines Datensatzes laufen lassen
	und dabei deren Ausgaben aufzeichnen.
Die Auswertung dieser Aufzeichnungen ergab,
	dass [2009_DaPlSt] in allen vier Tests am besten abgeschnitten hat.

## Visualisierung
Des Weiteren wurde für jeden Algorithmus eine Visualisierung erstellt.

#### 2001 Cheng, Nazer, Uppuluri, Verret - Beat This
*kein Bild vefügbar*

#### 2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio
![demo_2009_DaPlSt](https://github.com/user-attachments/assets/64104dc9-f8c7-48d0-b2dd-741f82958ed5)

#### 2011 Plumbley, Robertson, Stark - Real-Time Visual Beat Tracking Using a Comb Filter Matrix
![demo_2011_PlRoSt](https://github.com/user-attachments/assets/7d7fb44d-a292-4b2c-9f32-20f516838429)

## How To Use
### Build
```shell script
$ cd Implementierung/
$ make
```

### Run
```shell script
$ cd Implementierung/
$ ./record.sh | ./2009_DaPlSt
```
### Dependencies
* ALSA
* Gamma (included as submodule)
* SDL2
* SDL2-image
* SDL2-mixer
* SDL2-ttf
* simple2d (included as submodule)
* PortAudio
* PulseAudio

#### Ubuntu
```shell script
$ sudo apt-get install \
    libasound-dev \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libsdl2-ttf-dev \
    portaudio19-dev \
    pulseaudio
```

#### NixOS
TODO

## Literatur
* [2001_BeatThis] - CHENG, Kileen ; NAZER, Bobak ; UPPULURI, Jyoti ; VERRET, Ryan: Beat This — A Beat Synchronization Project. 2001. – URL: https://www.clear.rice.edu/elec301/Projects01/beat_sync/beatalgo.html. – Zugriﬀsdatum: 08.04.2020
* [2009_DaPlSt] - STARK, Adam M. ; DAVIES, Matthew E. P. ; PLUMBLEY, Mark D.: Real-Time Beat-Synchronous Analysis of Musical Audio. (2009)
* [2011_PlRoSt] - ROBERTSON, Andrew ; STARK, Adam M. ; PLUMBLEY, Mark D.: Real-Time Visual Beat Tracking Using a Comb Filter Matrix. (2011)
