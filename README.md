# Misc Audio Formats Converter

This tool converts some obscure or rare audio formats used in various video games, new or old. Whenever I couldn't find an existing converter online, I implemented it myself.

## Handled conversions
- [INDYWV](#indywv-and-lab) to WAV (mono, stereo ADPCM, or stereo WVSM)
- WAV to [INDYWV](#indywv-and-lab) (mono ADPCM only)
- [LAB](#indywv-and-lab) (with embedded INDYWVs) to WAVs
- [Cryo APC](#apc) to WAV


## Handled formats

### IndyWV and LAB

IndyWV is a sound file format developed by LucasArts. It was used in multiple games in the late 90s.
LAB was another format used as a collection of files. The tool can convert individual INDYWV files but it can also extract a LAB file that contains multiple INDYWV files and convert them in a batch.

Tested games:
- Star Wars Episode 1: The Phantom Menace (1999, PC) : the file VOICE.LAB on the game disc contains all the dialog files (mono) compressed with the ADPCM algorithm.
- Indiana Jones and the Infernal Machine (1999, PC) : the .NDY files extracted from the two CDs contain all SFX, dialog and music (mono and stereo files), compressed with the WVSM algorithm.

Please let me know if you are aware of other games that also use the INDYWV format! Especially the ADPCM variant.

### APC

This format was used in several point'n click games developed by Cryo in the 90s and early 2000s:
- Egypt 1156 B.C. – Tomb of the Pharaoh (1997) and its sequel, Egypt 2 - The Heliopolis Prophecy (2000)
- China: The Forbidden City (1998)
- Atlantis: The Lost Tales (1997), Atlantis 2 (1999)

## Usage
```
-in <FileOrFolderPath> : full path of input file or folder (file types will be auto-deduced)
-out <FileOrFolderPath> : path of output file or output folder
[-unit_test] : performs unit test - checks algorithm integrity (optional)
```

Examples:
- to convert a single WV file and write the output WAV in the same folder:  `convert -in ABM3627.wv -out .`
- to parse a LAB archive file and extract all of its INDYWV files: `convert -in voice.lab -out ".\converted_files" `
- to parse a folder containing hundreds of APC files and convert them to a subfolder: ` convert -in "C:\apc_files" -out "C:\converted_files" `
- to perform the unit test (for developers: to check the algorithm's integrity when you make modifications). The program assumes that the test files are located in a "..\..\UnitTest" subfolder: ` convert -unit_test `


## Credits
- INDYWV ADPCM decompression algorithm reverse-engineered and implemented by myself.
- INDYWV WVSM decompression algorithm implemented by Crt Vavros in the [Urgon Mod Tools repository](https://github.com/smlu/Urgon)
- LABN archive format extraction was done by Guilherme Lampert. See [Reverse Engineering LucasArts Outlaws](https://github.com/glampert/reverse-engineering-outlaws)
- APC specifications and decompression algorithm: [CRYO APC - MultimediaWiki](https://wiki.multimedia.cx/index.php/CRYO_APC)
