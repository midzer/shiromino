# Emscripten

## Build

```
mkdir build
cd build
emcmake cmake -DCMAKE_BUILD_TYPE=Release ..
emmake make
```

## Link

```
emcc -flto 
-O3 *.o asset/*.o gui/*.o input/*.o menu/*.o video/*.o libsqlite3.a libtinyxml2.a -o index.html -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS='["png"]' -sUSE_SDL_MIXER=2 -sSDL2_MIXER_FORMATS='["ogg"]' -sASYNCIFY -sASYNCIFY_IGNORE_INDIRECT -sASYNCIFY_ONLY=@../../../../funcs.txt -sINITIAL_MEMORY=128mb -sENVIRONMENT=web --preload-file ../../../../data/@data/ --closure 1 -sEXPORTED_RUNTIME_METHODS=['allocate'] -Wl,-u,fileno
```
