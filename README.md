# timbrExplorer

To use this project, download the repo as a zip file and drag it into the BELA IDE. To run the project you will need two trill sensors and a MIDI controller connected to the BELA.

By default the script will look for two square trills with addresses square+0 (40) and square+1 (41). This can be changed in render.cpp's setup() function.

The MIDI controller will be looking for a hardware ID of "hw:1,0,0". This can be changed in note.h.
