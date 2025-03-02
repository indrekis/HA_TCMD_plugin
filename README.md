# HA_TCMD_plugin

Ha archive plugin for Total Commander 32 and 64. I took sources from the authors’ repo (see [SourceForge](https://sourceforge.net/projects/wcx/files/HA%20WCX/) and [plugin site](https://wcx.sourceforge.net/)), updated the project to the MSVC 2022, fixed several compilation bugs emerging, and fixed several bugs (memory leaks, out-of-bound accesses, and crashes on some bad archives – when the header is partly intact but the filename is already damaged). What bothers me is that the size has grown more than six times after the recompile (with several small fixes to compile by the modern compiler). 


Previous [readme.txt is here](readme_old.txt) -- it is rather outdated for the current Total Commander.

The plugin support viewing, unpacking, packing and deleting files within archives in HA format. Uses original HA sources by Harry Hirvola[^HASRC]. 

Calling the settings dialog from command line:

```rundll32 ha.wcx,Settings```

```rundll32 ha.wcx64,Settings```


[^HASRC]: Currently, HA archiver sources are [on the github](https://github.com/l-4-l/ha) too.

----------------

(c) 1999-2002 by Siarzhuk Zharski

(c) 2025 by Oleg Farenyuk
