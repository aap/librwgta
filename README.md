librwgta
========

This repository contains GTA specific code.
The library librwgta proper has Rockstar specific plugins for librw.

Under tools/convdff and tools/convtxd there is code for some
dff and txd conversion utilities.

tools/storiesconv converts a number of file formats of GTA Liberty and Vice city
stories to standard RW files.

Under tools/IIItest you can find an ongoing project to reverse engineer GTA III.
Currently it can render the map:
![Liberty City (GTA3) rendered by IIItest](http://aap.papnet.eu/gta/screens/mapviewer1.png)

[Click here for a video](https://www.youtube.com/watch?v=6H8YP2q_-ls)

tools/storiesview is a map viewer for PS2 LCS and VCS.

tools/euryopa is a map viewer for GTA III-SA that will become an editor eventually hopefully.
Since it depends on LZO, which is GPL, consider my code in this repo dual-licenced as GPL.

# Building

To build, you'll need to set the following environment variables, depending on the tool:

| Variable      | Description                  | Used by                  |
|---------------|------------------------------|--------------------------|
| `LIBRW`       | Path to librw directory      | All                      |
| `LUADIR`      | Path to lua directory        | Selanna                  |
| `ZLIBDIR`     | Path to zlib directory       | lcsview/vcsview          |

Next, just use premake5 as you normally would. These variables should be automatically replaced with the values you configured on your build device.

NOTE: The build script is tailored to an older version of premake, so for convenience it is included in the repository.

## Additional variables
It is possible to set a special environment variable for the debug directory. To do this, simply take the tool name and append `_DEBUGDIR` to it.
So to set one for Euryopa, simply use `EURYOPA_DEBUGDIR`.